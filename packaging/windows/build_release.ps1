<#
.SYNOPSIS
Builds the Windows release of Nottawa (the Qt app, app/nottawa_app).

.DESCRIPTION
  1. Builds the engine and the app (Release) into app\build\release.
  2. Stages dist\Nottawa-<version>\: Nottawa.exe, Qt (windeployqt), the
     ffmpeg LGPL DLLs, the Visual C++ runtime (app-local), data\ and
     licenses\.
  3. Smoke-tests the staged copy: launches it with a throwaway app data
     folder and runs a short stress scenario, which must exit cleanly.
  4. Signs the exe (when -Certificate is given).
  5. Builds the installer with Inno Setup (when installed) and signs it.
  6. Zips a portable copy and the symbols (PDB) for crash reports.

The version comes from the VERSION file at the repo root.

Output (dist\):
  Nottawa-Setup-<version>.exe     the installer
  Nottawa-<version>-portable.zip  the staged folder, zipped
  Nottawa-<version>-symbols.zip   Nottawa.pdb, to symbolise crash logs

.PARAMETER Certificate
SHA1 thumbprint (CurrentUser\My) or path to a .pfx. Without it nothing is
signed, and Windows SmartScreen will warn whoever runs the installer.

.EXAMPLE
powershell -File packaging\windows\build_release.ps1
powershell -File packaging\windows\build_release.ps1 -Certificate <thumbprint>
#>

[CmdletBinding()]
param(
    [string]$Certificate,
    [string]$PfxPassword,
    [string]$TimestampUrl = 'http://timestamp.digicert.com',
    [string]$InnoSetup,
    [string]$QtDir = 'C:\Qt\6.8.0\msvc2022_64',
    [switch]$SkipBuild,
    [switch]$SkipSmokeTest
)

$ErrorActionPreference = 'Stop'
$RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
$Version  = (Get-Content (Join-Path $RepoRoot 'VERSION') -TotalCount 1).Trim()
if ($Version -notmatch '^\d+\.\d+\.\d+$') { throw "VERSION must be MAJOR.MINOR.PATCH, got '$Version'" }
$BuildDir = Join-Path $RepoRoot 'app\build\release'
$DistDir  = Join-Path $RepoRoot 'dist'
$Stage    = Join-Path $DistDir "Nottawa-$Version"

function Step($text) { Write-Host "==> $text" -ForegroundColor Cyan }

# ----- Tools ---------------------------------------------------------------

$vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
if (-not (Test-Path $vswhere)) { throw 'vswhere.exe not found: install Visual Studio 2022 with the C++ workload.' }
$vs = & $vswhere -latest -products '*' -requires Microsoft.Component.MSBuild -property installationPath
if (-not $vs) { throw 'No Visual Studio installation with MSBuild found.' }
$cmake = Join-Path $vs 'Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe'
$windeployqt = Join-Path $QtDir 'bin\windeployqt.exe'
if (-not (Test-Path $windeployqt)) { throw "windeployqt not found under $QtDir (pass -QtDir)." }
# The newest Visual C++ runtime matching the compiler (app-local deployment).
$crt = Get-ChildItem (Join-Path $vs 'VC\Redist\MSVC') -Directory |
       Where-Object { $_.Name -match '^\d+\.' } | Sort-Object { [version]$_.Name } -Descending |
       ForEach-Object { Join-Path $_.FullName 'x64\Microsoft.VC143.CRT' } |
       Where-Object { Test-Path $_ } | Select-Object -First 1
if (-not $crt) { throw 'Visual C++ runtime (Microsoft.VC143.CRT) not found in the Visual Studio install.' }

Write-Host "Nottawa $Version" -ForegroundColor Green

# ----- 1. Build ------------------------------------------------------------

if (-not $SkipBuild) {
    Step 'Building engine and app (Release)'
    & $cmake -S (Join-Path $RepoRoot 'app\nottawa_app') -B $BuildDir | Out-Null
    if ($LASTEXITCODE -ne 0) { throw "CMake configure failed ($LASTEXITCODE)" }
    & $cmake --build $BuildDir --config Release
    if ($LASTEXITCODE -ne 0) { throw "Build failed ($LASTEXITCODE)" }
}
$built = Join-Path $BuildDir 'Release'
$exe = Join-Path $built 'Nottawa.exe'
if (-not (Test-Path $exe)) { throw "$exe not found; build first." }

# ----- 2. Stage ------------------------------------------------------------

Step "Staging $Stage"
if (Test-Path $Stage) { Remove-Item $Stage -Recurse -Force }
New-Item -ItemType Directory -Path $Stage | Out-Null
Copy-Item $exe $Stage

# Qt: only what the app uses. No translations, no software OpenGL, no D3D/DXC
# shader compilers (the app renders with OpenGL).
& $windeployqt --release --no-translations --no-opengl-sw --no-system-d3d-compiler --no-system-dxc-compiler `
    --no-compiler-runtime (Join-Path $Stage 'Nottawa.exe') | Out-Null
if ($LASTEXITCODE -ne 0) { throw "windeployqt failed ($LASTEXITCODE)" }

# ffmpeg (LGPL build, dynamically linked).
$ffmpeg = Get-ChildItem (Join-Path $RepoRoot 'bin') -File | Where-Object { $_.Name -match '^(avcodec|avformat|avutil|swscale|swresample)-\d+\.dll$' }
if ($ffmpeg.Count -ne 5) { throw "Expected 5 ffmpeg DLLs in bin\, found $($ffmpeg.Count) (run scripts\fetch_ffmpeg_lgpl.ps1)." }
$ffmpeg | Copy-Item -Destination $Stage

# Visual C++ runtime, app-local, so no separate redistributable is needed.
Get-ChildItem $crt -Filter '*.dll' | Copy-Item -Destination $Stage

# Data: the same folders the Mac app bundles, without development scripts.
$data = Join-Path $Stage 'data'
foreach ($folder in 'images', 'shaders', 'markdown', 'fonts', 'misc', 'audio') {
    robocopy (Join-Path $RepoRoot $folder) (Join-Path $data $folder) /E /XF *.sh *.js .DS_Store Thumbs.db /NFL /NDL /NJH /NJS /NP | Out-Null
    if ($LASTEXITCODE -ge 8) { throw "Copying $folder failed (robocopy $LASTEXITCODE)" }
}

# Licences: Nottawa's own, and every third-party component's.
$licenses = Join-Path $Stage 'licenses'
New-Item -ItemType Directory -Path $licenses | Out-Null
# Nottawa is GPL-3.0 (licenses\GPL-3.0.txt, see THIRD_PARTY_NOTICES.txt).
Copy-Item (Join-Path $PSScriptRoot 'licenses\*') $licenses -Recurse

# ----- 3. Smoke test -------------------------------------------------------

if (-not $SkipSmokeTest) {
    Step 'Smoke-testing the staged copy'
    $appData = Join-Path $env:TEMP "nottawa-smoke-$PID"
    if (Test-Path $appData) { Remove-Item $appData -Recurse -Force }
    $env:NOTTAWA_APPDATA = $appData
    $env:NOTTAWA_NO_PUBLISH = '1'
    $env:NOTTAWA_STRESS = 'churn,previews,workspace'
    $env:NOTTAWA_STRESS_ROUNDS = '1'
    $env:NOTTAWA_STRESS_CYCLES = '3'
    try {
        $p = Start-Process (Join-Path $Stage 'Nottawa.exe') -WorkingDirectory $Stage -PassThru
        if (-not $p.WaitForExit(300000)) { Stop-Process -Id $p.Id -Force; throw 'Smoke test timed out' }
        $log = Join-Path $appData 'logs\engine.log'
        $crashed = (Test-Path $log) -and (Select-String -Path $log -Pattern '=== CRASH' -Quiet)
        if ($p.ExitCode -ne 0 -or $crashed) { throw "Smoke test failed (exit $($p.ExitCode); see $log)" }
        Remove-Item $appData -Recurse -Force
    } finally {
        foreach ($v in 'NOTTAWA_APPDATA', 'NOTTAWA_NO_PUBLISH', 'NOTTAWA_STRESS', 'NOTTAWA_STRESS_ROUNDS', 'NOTTAWA_STRESS_CYCLES') {
            Remove-Item "env:$v" -ErrorAction SilentlyContinue
        }
    }
}

# ----- 4. Sign -------------------------------------------------------------

function Invoke-Sign([string]$Target) {
    if (-not $Certificate) { Write-Warning "Not signed (no -Certificate): $Target"; return }
    $signtool = Get-ChildItem "${env:ProgramFiles(x86)}\Windows Kits\10\bin" -Recurse -Filter signtool.exe -ErrorAction SilentlyContinue |
                Where-Object { $_.FullName -match '\\x64\\' } | Sort-Object FullName -Descending | Select-Object -First 1 -ExpandProperty FullName
    if (-not $signtool) { throw 'signtool.exe not found: install the Windows 10/11 SDK.' }
    $a = @('sign', '/fd', 'SHA256', '/td', 'SHA256', '/tr', $TimestampUrl)
    if ($Certificate -like '*.pfx') { $a += @('/f', $Certificate); if ($PfxPassword) { $a += @('/p', $PfxPassword) } }
    else { $a += @('/sha1', $Certificate) }
    & $signtool @a $Target
    if ($LASTEXITCODE -ne 0) { throw "Signing $Target failed ($LASTEXITCODE)" }
}
Step 'Signing'
Invoke-Sign (Join-Path $Stage 'Nottawa.exe')

# ----- 5. Installer --------------------------------------------------------

if (-not $InnoSetup) {
    $InnoSetup = @((Get-Command ISCC.exe -ErrorAction SilentlyContinue).Source,
                   "${env:ProgramFiles(x86)}\Inno Setup 6\ISCC.exe",
                   "$env:LOCALAPPDATA\Programs\Inno Setup 6\ISCC.exe") | Where-Object { $_ -and (Test-Path $_) } | Select-Object -First 1
}
if ($InnoSetup) {
    Step 'Building the installer'
    & $InnoSetup /Q "/DAppVersion=$Version" "/DStageDir=$Stage" "/DOutputDir=$DistDir" (Join-Path $PSScriptRoot 'Nottawa.iss')
    if ($LASTEXITCODE -ne 0) { throw "Inno Setup failed ($LASTEXITCODE)" }
    Invoke-Sign (Join-Path $DistDir "Nottawa-Setup-$Version.exe")
} else {
    Write-Warning 'Inno Setup (ISCC.exe) not found: no installer built.'
}

# ----- 6. Portable zip and symbols ----------------------------------------

Step 'Zipping portable copy and symbols'
$portable = Join-Path $DistDir "Nottawa-$Version-portable.zip"
$symbols  = Join-Path $DistDir "Nottawa-$Version-symbols.zip"
Remove-Item $portable, $symbols -ErrorAction SilentlyContinue
Compress-Archive -Path $Stage -DestinationPath $portable -CompressionLevel Optimal
Compress-Archive -Path (Join-Path $built 'Nottawa.pdb') -DestinationPath $symbols -CompressionLevel Optimal

Write-Host ''
Get-ChildItem $DistDir -File -Filter "Nottawa*$Version*" | ForEach-Object { '{0,-40} {1,8:N1} MB' -f $_.Name, ($_.Length / 1MB) }
