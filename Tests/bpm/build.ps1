# Builds bpm_eval (Release) into Tests\bpm\build, or with -Trace (prints the
# tracker's tempo candidates every update) into Tests\bpm\build-trace.
param([switch]$Trace)
$ErrorActionPreference = 'Stop'
$vs = & "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products '*' -property installationPath
$cmake = Join-Path $vs 'Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe'
$dir = Join-Path $PSScriptRoot $(if ($Trace) { 'build-trace' } else { 'build' })
& $cmake -S $PSScriptRoot -B $dir "-DTRACE=$(if ($Trace) { 'ON' } else { 'OFF' })" | Out-Null
& $cmake --build $dir --config Release -- /v:minimal /nologo 2>&1 |
    Select-String -Pattern ' error |error C|error LNK' | Select-Object -First 30 | ForEach-Object { $_.Line }
"build exit=$LASTEXITCODE"
