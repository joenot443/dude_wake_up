<#
.SYNOPSIS
Builds Nottawa for Windows and runs its test suite: golden images, workflow
pixel checks, and the stress scenarios that need no media or sound.

.DESCRIPTION
Builds into app\build\test (a running Nottawa from app\build\nottawa_app is
left alone), then runs the test scenarios (app/nottawa_app/StressTest.cpp) in
the built app with throwaway app data. Fails (exit 1) on any test failure, a
crash or a UI hang. Known bugs listed in the scenarios are reported as KNOWN
without failing; one that starts passing is reported as KNOWN-FIXED.

Golden images live in Tests\golden\windows. After a change that's meant to
alter what effects draw, record them again with -Golden record (renders
everything twice, then three more launches to find anything that differs
between launches: those are listed in unstable.txt and only checked for
rendering something), look at the changed images, and commit them.

Runs before every push from .git/hooks/pre-push (Tests/hooks/pre-push;
install it with Tests\hooks\install.ps1). `git push --no-verify` skips it.

.EXAMPLE
powershell -File Tests\run_tests.ps1
powershell -File Tests\run_tests.ps1 -Golden record
#>

[CmdletBinding()]
param(
    [ValidateSet('check', 'record')]
    [string]$Golden = 'check',
    [switch]$SkipBuild,
    [string]$Scenarios = 'golden,workflows,blenddrag,settings,settingsui,churn,connect,undo,swap,copypaste,optional,strands,workspace,previews',
    [int]$Timeout = 1800
)

$ErrorActionPreference = 'Stop'
$Repo      = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$BuildDir  = Join-Path $Repo 'app\build\test'
$Exe       = Join-Path $BuildDir 'Release\Nottawa.exe'
$GoldenDir = Join-Path $Repo 'Tests\golden\windows'
$Work      = Join-Path $env:TEMP 'nottawa-tests'

function Step($text) { Write-Host "==> $text" -ForegroundColor Cyan }

# ----- Build ---------------------------------------------------------------

if (-not $SkipBuild) {
    $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    $vs = & $vswhere -latest -products '*' -requires Microsoft.Component.MSBuild -property installationPath
    if (-not $vs) { throw 'No Visual Studio installation with MSBuild found.' }
    $cmake = Join-Path $vs 'Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe'
    Step "Building (engine and app) into $BuildDir"
    & $cmake -S (Join-Path $Repo 'app\nottawa_app') -B $BuildDir | Out-Null
    if ($LASTEXITCODE -ne 0) { Write-Host 'CMake configure failed.' -ForegroundColor Red; exit 1 }
    $output = & $cmake --build $BuildDir --config Release 2>&1
    if ($LASTEXITCODE -ne 0) {
        $output | Select-String -Pattern ' error |error LNK' | Select-Object -First 20 | ForEach-Object { $_.Line }
        Write-Host 'Build failed.' -ForegroundColor Red
        exit 1
    }
}
if (-not (Test-Path $Exe)) { throw "$Exe not built." }

# ----- Run -----------------------------------------------------------------

# One launch of the app running `scenarios`; returns its log lines, the
# engine log and whether it finished.
function Invoke-Nottawa([string]$scenarios, [hashtable]$extraEnv = @{}) {
    $appdata = Join-Path $Work 'appdata'
    Remove-Item -Recurse -Force $appdata -ErrorAction SilentlyContinue
    New-Item -ItemType Directory -Force (Join-Path $appdata 'settings\Nottawa') | Out-Null
    Set-Content (Join-Path $appdata 'settings\Nottawa\Nottawa.ini') "[ui]`r`ntheme=Midnight"
    $log = Join-Path $Work 'test.log'
    Remove-Item $log -ErrorAction SilentlyContinue
    $env:NOTTAWA_APPDATA = $appdata; $env:NOTTAWA_LOG = $log; $env:NOTTAWA_ADD = ''
    $env:NOTTAWA_NO_PUBLISH = '1'
    $env:NOTTAWA_STRESS = $scenarios; $env:NOTTAWA_STRESS_ROUNDS = '1'; $env:NOTTAWA_STRESS_SEED = '1'
    $env:NOTTAWA_STRESS_CYCLES = '6'; $env:NOTTAWA_GOLDEN_DIR = $GoldenDir; $env:NOTTAWA_GOLDEN = 'check'
    foreach ($k in $extraEnv.Keys) { Set-Item "env:$k" $extraEnv[$k] }
    $p = Start-Process $Exe -WorkingDirectory (Split-Path $Exe) -PassThru
    $finished = $p.WaitForExit($Timeout * 1000)
    if (-not $finished) { Stop-Process -Id $p.Id -Force }
    $lines = if (Test-Path $log) { Get-Content $log -Encoding UTF8 } else { @() }
    $engineLog = Join-Path $appdata 'logs\engine.log'
    $engine = if (Test-Path $engineLog) { Get-Content $engineLog -Encoding UTF8 } else { @() }
    [pscustomobject]@{ Lines = $lines; Engine = $engine; Finished = $finished; ExitCode = $(if ($finished) { $p.ExitCode } else { $null }) }
}

New-Item -ItemType Directory -Force $Work | Out-Null

if ($Golden -eq 'record') {
    New-Item -ItemType Directory -Force $GoldenDir | Out-Null
    Get-ChildItem $GoldenDir -Filter '*.png' | Remove-Item
    Step 'Recording golden images'
    $r = Invoke-Nottawa 'golden' @{ NOTTAWA_GOLDEN = 'record' }
    $r.Lines | Select-String '\[stress\] (golden|FAIL)' | ForEach-Object { $_.Line }
    for ($i = 1; $i -le 3; $i++) {
        Step "Stabilizing ($i of 3): what differs between launches"
        $r = Invoke-Nottawa 'golden' @{ NOTTAWA_GOLDEN = 'stabilize' }
        $r.Lines | Select-String '\[stress\] golden' | ForEach-Object { $_.Line }
    }
    Step "Recorded in ${GoldenDir}: look at the changed images, then commit them."
    exit 0
}

Step "Running: $Scenarios"
$started = Get-Date
$r = Invoke-Nottawa $Scenarios
$failures = @($r.Lines | Select-String '\[stress\] FAIL' | ForEach-Object { $_.Line })
$known    = @($r.Lines | Select-String '\[stress\] KNOWN' | ForEach-Object { $_.Line })
$crashes  = @($r.Engine | Select-String '^=== CRASH' | ForEach-Object { $_.Line })
$hangs    = @($r.Engine | Select-String '^=== HANG' | ForEach-Object { $_.Line })
$ran      = @($r.Lines | Select-String '\[stress\] >>> ').Count

$r.Lines | Select-String '\[stress\] (golden|workflows):' | ForEach-Object { $_.Line -replace '^\S+ ', '' }
$problems = @()
if (-not $r.Finished) { $problems += "the app didn't finish within $Timeout s" }
elseif ($r.ExitCode -ne 0) { $problems += "the app exited with code $($r.ExitCode)" }
$problems += $crashes + $hangs + $failures
$expected = @($Scenarios.Split(',')).Count
if ($ran -lt $expected) { $problems += "only $ran of $expected scenarios ran" }

Write-Host ''
foreach ($k in $known) { Write-Host ($k -replace '^\S+ ', '') -ForegroundColor Yellow }
if ($problems.Count -gt 0) {
    foreach ($p in $problems) { Write-Host ($p -replace '^\S+ ', '') -ForegroundColor Red }
    Write-Host ("Tests FAILED ({0} problem(s), {1:N0} s). Logs: {2}" -f $problems.Count, ((Get-Date) - $started).TotalSeconds, $Work) -ForegroundColor Red
    exit 1
}
Write-Host ("Tests passed: {0} scenarios, {1} known issue(s), {2:N0} s." -f $ran, $known.Count, ((Get-Date) - $started).TotalSeconds) -ForegroundColor Green
exit 0
