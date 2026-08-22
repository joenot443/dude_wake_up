<#
.SYNOPSIS
End-to-end Windows release build for Nottawa.

.DESCRIPTION
Reproducible pipeline used by CI and manual releases:
  1. Restore ffmpeg LGPL binaries (if missing).
  2. Patch the .vcxproj so Nottawa's flags survive projectGenerator regen.
  3. MSBuild Release|x64.
  4. Stage bin/ + data/ into a signed layout.
  5. Sign the .exe with signtool (EV cert if -Certificate provided,
     self-signed fallback for preview builds).
  6. Compile the Inno Setup installer.
  7. Sign the installer.

Output: dist\Nottawa-Setup-{Version}-{Channel}.exe

.PARAMETER Version
Version string embedded in the installer filename and metadata.

.PARAMETER Channel
'release' or 'preview'. Preview builds accept self-signed cert; release
builds require an EV cert and fail if signing fails.

.PARAMETER Certificate
Thumbprint (SHA1) or path to .pfx of the signing cert. Optional for preview.

.PARAMETER PfxPassword
Password if -Certificate is a .pfx path. Ignored otherwise.

.PARAMETER TimestampUrl
RFC 3161 timestamp server. Default is DigiCert's RFC 3161 endpoint.

.PARAMETER InnoSetup
Path to ISCC.exe. Autodetected from PATH and default install location.

.PARAMETER SkipBuild
Skip MSBuild step (useful for iterating on installer packaging).

.EXAMPLE
pwsh packaging\windows\build_release.ps1 -Version 0.0.0 -Channel preview
pwsh packaging\windows\build_release.ps1 -Version 1.0.0 -Channel release -Certificate <thumbprint>
#>

[CmdletBinding()]
param(
    [Parameter(Mandatory=$true)][string]$Version,
    [ValidateSet('release','preview')][string]$Channel = 'preview',
    [string]$Certificate,
    [string]$PfxPassword,
    [string]$TimestampUrl = 'http://timestamp.digicert.com',
    [string]$InnoSetup,
    [switch]$SkipBuild
)

$ErrorActionPreference = 'Stop'
$RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
$Proj     = Join-Path $RepoRoot 'apps\myApps\Nottawa\Nottawa.vcxproj'
$Sln      = Join-Path $RepoRoot 'apps\myApps\Nottawa\Nottawa.sln'
$BinDir   = Join-Path $RepoRoot 'apps\myApps\Nottawa\bin'
$DataDir  = Join-Path $RepoRoot 'data'
$DistDir  = Join-Path $RepoRoot 'dist'
$StageDir = Join-Path $RepoRoot "dist\stage-$Channel"

Write-Host "Nottawa release build: v$Version ($Channel)" -ForegroundColor Cyan

# ----- 1. ffmpeg present ------------------------------------------------

$fetchScript = Join-Path $RepoRoot 'scripts\fetch_ffmpeg_lgpl.ps1'
$anyFfmpeg = Get-ChildItem -Path $BinDir -Filter 'avcodec-*.dll' -ErrorAction SilentlyContinue
if (-not $anyFfmpeg) {
    Write-Host "ffmpeg DLLs missing from $BinDir; invoking fetch script." -ForegroundColor Yellow
    & pwsh -File $fetchScript
    if ($LASTEXITCODE -ne 0) {
        throw "fetch_ffmpeg_lgpl.ps1 exited $LASTEXITCODE"
    }
}

# ----- 2. vcxproj patch -------------------------------------------------

$patchScript = Join-Path $RepoRoot 'scripts\patch-vcxproj.ps1'
if (Test-Path $patchScript) {
    & pwsh -File $patchScript -Project $Proj -RepoRoot $RepoRoot
} else {
    Write-Warning "patch-vcxproj.ps1 not found; assuming .vcxproj is hand-maintained."
}

# ----- 3. MSBuild -------------------------------------------------------

if (-not $SkipBuild) {
    $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    if (-not (Test-Path $vswhere)) {
        throw "vswhere.exe not found. Install Visual Studio 2022 with C++ workload."
    }
    $vsPath = & $vswhere -latest -products '*' -requires Microsoft.Component.MSBuild -property installationPath
    if (-not $vsPath) { throw "No VS install with MSBuild found." }
    $msbuild = Join-Path $vsPath 'MSBuild\Current\Bin\MSBuild.exe'
    if (-not (Test-Path $msbuild)) { throw "MSBuild not found at $msbuild" }

    Write-Host "Building $Sln" -ForegroundColor Cyan
    & $msbuild $Sln `
        /p:Configuration=Release `
        /p:Platform=x64 `
        /p:PlatformToolset=v143 `
        /verbosity:minimal `
        /nologo
    if ($LASTEXITCODE -ne 0) { throw "MSBuild failed ($LASTEXITCODE)" }
}

# ----- 4. Stage ---------------------------------------------------------

if (Test-Path $StageDir) { Remove-Item $StageDir -Recurse -Force }
New-Item -ItemType Directory -Path $StageDir | Out-Null
New-Item -ItemType Directory -Path $DistDir -Force | Out-Null

$exe = Join-Path $BinDir 'Nottawa.exe'
if (-not (Test-Path $exe)) {
    # OF generator sometimes lowercases; fall back.
    $exe = Get-ChildItem -Path $BinDir -Filter '*.exe' | Select-Object -First 1 -ExpandProperty FullName
    if (-not $exe) { throw "No .exe found in $BinDir. Did MSBuild actually produce output?" }
}

Copy-Item -LiteralPath $exe -Destination $StageDir
Get-ChildItem -Path $BinDir -Filter '*.dll' | Copy-Item -Destination $StageDir

$stagedData = Join-Path $StageDir 'data'
if (Test-Path $DataDir) {
    robocopy $DataDir $stagedData /MIR /NFL /NDL /NJH /NJS /NC /NS /NP | Out-Null
    if ($LASTEXITCODE -ge 8) { throw "robocopy data failed ($LASTEXITCODE)" }
}

# ----- 5. Sign the exe --------------------------------------------------

function Invoke-Signtool {
    param([string]$Target)
    if (-not $Certificate) {
        if ($Channel -eq 'release') {
            throw "release build requires -Certificate (EV cert thumbprint or .pfx)"
        }
        Write-Warning "No -Certificate; leaving $Target UNSIGNED (preview channel only)."
        return
    }

    $signtool = (Get-Command signtool.exe -ErrorAction SilentlyContinue).Source
    if (-not $signtool) {
        # Fallback: check latest Windows SDK.
        $sdk = Get-ChildItem "${env:ProgramFiles(x86)}\Windows Kits\10\bin" -Directory `
             | Where-Object { $_.Name -match '^10\.' } `
             | Sort-Object Name -Descending | Select-Object -First 1
        if ($sdk) {
            $signtool = Join-Path $sdk.FullName 'x64\signtool.exe'
        }
    }
    if (-not $signtool -or -not (Test-Path $signtool)) {
        throw "signtool.exe not found. Install Windows 10/11 SDK."
    }

    $args = @('sign', '/fd', 'SHA256', '/td', 'SHA256', '/tr', $TimestampUrl)
    if ($Certificate -like '*.pfx') {
        $args += @('/f', $Certificate)
        if ($PfxPassword) { $args += @('/p', $PfxPassword) }
    } else {
        # Treat as SHA1 thumbprint from CurrentUser\My.
        $args += @('/sha1', $Certificate)
    }
    $args += $Target

    & $signtool @args
    if ($LASTEXITCODE -ne 0) { throw "signtool sign failed on $Target ($LASTEXITCODE)" }
}

$stagedExe = Join-Path $StageDir (Split-Path -Leaf $exe)
Invoke-Signtool -Target $stagedExe

# ----- 6. Inno Setup ---------------------------------------------------

if (-not $InnoSetup) {
    $InnoSetup = (Get-Command ISCC.exe -ErrorAction SilentlyContinue).Source
    if (-not $InnoSetup) {
        $cand = "${env:ProgramFiles(x86)}\Inno Setup 6\ISCC.exe"
        if (Test-Path $cand) { $InnoSetup = $cand }
    }
}
if (-not $InnoSetup) {
    Write-Warning "Inno Setup (ISCC.exe) not found; skipping installer build."
    Write-Host "Staged tree at $StageDir" -ForegroundColor Green
    exit 0
}

$iss = Join-Path $PSScriptRoot 'Nottawa.iss'
if (-not (Test-Path $iss)) {
    Write-Warning "$iss not found — installer .iss must exist to build installer. Staged tree only."
    exit 0
}

$installerName = "Nottawa-Setup-$Version-$Channel.exe"
& $InnoSetup `
    "/DAppVersion=$Version" `
    "/DAppChannel=$Channel" `
    "/DStageDir=$StageDir" `
    "/DOutputDir=$DistDir" `
    "/DOutputBaseFilename=$($installerName -replace '\.exe$','')" `
    $iss
if ($LASTEXITCODE -ne 0) { throw "ISCC failed ($LASTEXITCODE)" }

# ----- 7. Sign the installer --------------------------------------------

$installerPath = Join-Path $DistDir $installerName
if (Test-Path $installerPath) {
    Invoke-Signtool -Target $installerPath
    Write-Host "Installer: $installerPath" -ForegroundColor Green
} else {
    Write-Warning "Installer not found at $installerPath — check ISCC output naming."
}
