<#
.SYNOPSIS
Download, verify, and stage a pinned LGPL-only ffmpeg shared build for Windows.

.DESCRIPTION
Nottawa is GPLv3, but its ffmpeg dependency MUST be LGPL-configured (no
--enable-gpl) so Nottawa is not compelled to distribute GPL-tainted
sources of anything ffmpeg's GPL components depend on. This script fetches
a known-good LGPL shared build from BtbN's ffmpeg-windows-build-helpers
release, verifies its SHA256, and stages the DLLs into bin/ and .lib
import libraries into a project-local libs\ffmpeg\lib folder.

Runtime belt-and-braces: Phase 4 will add a startup check that queries
avutil_configuration() and refuses to initialize if '--enable-gpl' is
present. This script is the build-time guard.

.PARAMETER Version
BtbN release tag to fetch. Default: 'latest' (rolling — the only tag with
stable-named assets; dated autobuild-* tags include the git hash in the
filename, so the URL changes daily and can't be pinned by name).

.PARAMETER Archive
Asset filename inside the release. Default: the n8.1 release-branch build,
which is the current LTS-ish stable branch. Alternatives at the 'latest'
tag: ffmpeg-master-latest-win64-lgpl-shared.zip (bleeding-edge master) or
ffmpeg-n9.0-latest-win64-lgpl-shared-9.0.zip (newest release branch).

.PARAMETER RepoRoot
Repo root. Defaults to two levels up from this script.

.PARAMETER Force
Re-download even if the archive is already cached.

.PARAMETER PinHash
If empty, first-run mode: script downloads, prints SHA256, exits. Rerun
after pasting the printed hash into $DefaultSha256 below (or via -PinHash).
If non-empty, script fails hard when the archive's hash differs.

Because the 'latest' tag's assets are rebuilt with every patch backport,
the SHA changes on every BtbN CI run. You bump the pin deliberately when
you take an upstream fix; between bumps, the pin is what proves the DLLs
you're about to link against are bit-exact what you approved.

.EXAMPLE
pwsh scripts\fetch_ffmpeg_lgpl.ps1                 # First-run: prints SHA256
pwsh scripts\fetch_ffmpeg_lgpl.ps1 -PinHash <hash> # Verified fetch + stage
#>

[CmdletBinding()]
param(
    [string]$Version = 'latest',
    [string]$Archive = 'ffmpeg-n8.1-latest-win64-lgpl-shared-8.1.zip',
    [string]$RepoRoot,
    [switch]$Force,
    [string]$PinHash
)

$ErrorActionPreference = 'Stop'

# ----- CONFIG -----------------------------------------------------------

# BtbN's ffmpeg-windows-build-helpers releases. LGPL variant only.
# https://github.com/BtbN/FFmpeg-Builds/releases
#
# Only the 'latest' rolling tag has stable-named assets. Dated releases
# (autobuild-YYYY-MM-DD-HH-MM) embed the git hash in each filename so the
# URL changes daily. Reproducibility comes from $DefaultSha256, not the tag.
$DownloadUrl = "https://github.com/BtbN/FFmpeg-Builds/releases/download/$Version/$Archive"

# Paste the SHA256 the first-run mode prints into this constant to make
# subsequent runs verify. Leaving it empty means every run is unverified;
# CI should treat empty pin as a build failure.
#
# Pinned 2026-08-25 by joe.crozier — from `ffmpeg-n8.1-latest-win64-lgpl-
# shared-8.1.zip` on BtbN's rolling 'latest' tag. Bump deliberately when
# taking a security fix or feature — every bump is a supply-chain review.
$DefaultSha256 = '6579c91e81e9091098f52653f94cd5168193632729e441fa5effcfad4bca1c9d'

# DLLs we ship. Must match the runtime `dumpbin /dependents nottawa.exe`.
$RequiredDlls = @(
    'avformat-*.dll',
    'avcodec-*.dll',
    'avutil-*.dll',
    'swscale-*.dll',
    'swresample-*.dll'
)

# Import libs consumed at link time.
$RequiredLibs = @(
    'avformat.lib',
    'avcodec.lib',
    'avutil.lib',
    'swscale.lib',
    'swresample.lib'
)

# Headers used from src/**.
$HeaderRoots = @(
    'libavformat',
    'libavcodec',
    'libavutil',
    'libswscale',
    'libswresample'
)

# ----- SETUP ------------------------------------------------------------

if (-not $RepoRoot) {
    $RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
}
if (-not $PinHash) { $PinHash = $DefaultSha256 }

$CacheDir  = Join-Path $RepoRoot '.cache\ffmpeg'
# DLLs must sit next to the built exe. MSBuild's OutDir for our .vcxproj is
# `bin\` relative to the solution, and $(SolutionDir) == repo root through
# the OF junction (<OF>\apps\myApps\Nottawa → repo). So bin\ at repo root
# is where the runtime DLLs need to live.
$StageDir  = Join-Path $RepoRoot 'bin'
$LibsDir   = Join-Path $RepoRoot 'libs\ffmpeg'
New-Item -ItemType Directory -Path $CacheDir  -Force | Out-Null
New-Item -ItemType Directory -Path $LibsDir   -Force | Out-Null
New-Item -ItemType Directory -Path (Join-Path $LibsDir 'lib')     -Force | Out-Null
New-Item -ItemType Directory -Path (Join-Path $LibsDir 'include') -Force | Out-Null

$ArchivePath = Join-Path $CacheDir $Archive

# ----- DOWNLOAD ---------------------------------------------------------

if ($Force -and (Test-Path $ArchivePath)) { Remove-Item $ArchivePath -Force }

if (-not (Test-Path $ArchivePath)) {
    Write-Host "Downloading $DownloadUrl" -ForegroundColor Cyan
    # TLS 1.2 forced for older PS/Windows combos.
    [Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12
    Invoke-WebRequest -Uri $DownloadUrl -OutFile $ArchivePath -UseBasicParsing
} else {
    Write-Host "Using cached archive $ArchivePath" -ForegroundColor DarkGray
}

# ----- VERIFY -----------------------------------------------------------

$actual = (Get-FileHash -LiteralPath $ArchivePath -Algorithm SHA256).Hash.ToLower()

if (-not $PinHash) {
    Write-Host ""
    Write-Host "FIRST-RUN MODE: no pinned SHA256." -ForegroundColor Yellow
    Write-Host "  archive: $ArchivePath"
    Write-Host "  sha256 : $actual"
    Write-Host ""
    Write-Host "Verify this hash out-of-band, then paste it into"
    Write-Host '  $DefaultSha256 = ''<hash>''' -ForegroundColor Yellow
    Write-Host "at the top of this script, commit, and re-run."
    exit 2
}

if ($actual -ne $PinHash.ToLower()) {
    Write-Host "SHA256 MISMATCH" -ForegroundColor Red
    Write-Host "  expected: $PinHash"
    Write-Host "  actual  : $actual"
    Write-Host "REFUSING to stage possibly-tampered binaries."
    exit 1
}
Write-Host "SHA256 OK ($actual)" -ForegroundColor Green

# ----- EXTRACT ----------------------------------------------------------

$ExtractDir = Join-Path $CacheDir 'extract'
if (Test-Path $ExtractDir) { Remove-Item $ExtractDir -Recurse -Force }
Expand-Archive -LiteralPath $ArchivePath -DestinationPath $ExtractDir -Force

# BtbN archives extract to ffmpeg-master-latest-win64-lgpl-shared\{bin,lib,include}
$Root = Get-ChildItem -Path $ExtractDir -Directory | Select-Object -First 1 -ExpandProperty FullName

# ----- LICENSE SANITY CHECK --------------------------------------------

# Look for GPL markers in the archive's LICENSE / README. If any surface,
# the build is NOT LGPL — bail loudly.
$licenseFiles = Get-ChildItem -Path $Root -Recurse -Include LICENSE, LICENSE.txt, README.txt -ErrorAction SilentlyContinue
$gplHit = $false
foreach ($lf in $licenseFiles) {
    $content = Get-Content -LiteralPath $lf.FullName -Raw
    if ($content -match '(?i)--enable-gpl' -or $content -match '(?i)configured with:.*--enable-gpl') {
        Write-Host "GPL configuration detected in $($lf.FullName)" -ForegroundColor Red
        $gplHit = $true
    }
}
if ($gplHit) {
    Write-Host "REFUSING to stage GPL-configured ffmpeg into a GPLv3 project" -ForegroundColor Red
    Write-Host "(GPLv3-compat requires LGPL-only ffmpeg. See docs/WINDOWS_ARCHITECTURE.md#ffmpeg-licensing.)"
    exit 1
}
Write-Host "License sanity: no --enable-gpl markers found." -ForegroundColor Green

# ----- STAGE DLLS -------------------------------------------------------

$SrcBin = Join-Path $Root 'bin'
if (-not (Test-Path $SrcBin)) { throw "Expected bin/ in extracted archive at $SrcBin" }
New-Item -ItemType Directory -Path $StageDir -Force | Out-Null

$copied = 0
foreach ($mask in $RequiredDlls) {
    $matches = Get-ChildItem -Path $SrcBin -Filter $mask -ErrorAction SilentlyContinue
    if ($matches.Count -eq 0) {
        throw "Required DLL mask $mask not found in $SrcBin"
    }
    foreach ($m in $matches) {
        Copy-Item -LiteralPath $m.FullName -Destination $StageDir -Force
        Write-Host "  bin\  $($m.Name)" -ForegroundColor DarkGray
        $copied++
    }
}

# ----- STAGE .LIB + HEADERS --------------------------------------------

$SrcLib = Join-Path $Root 'lib'
$SrcInc = Join-Path $Root 'include'
$DstLib = Join-Path $LibsDir 'lib'
$DstInc = Join-Path $LibsDir 'include'

foreach ($lib in $RequiredLibs) {
    $p = Join-Path $SrcLib $lib
    if (-not (Test-Path $p)) { throw "Required .lib $lib not found in $SrcLib" }
    Copy-Item -LiteralPath $p -Destination $DstLib -Force
}
foreach ($h in $HeaderRoots) {
    $p = Join-Path $SrcInc $h
    if (-not (Test-Path $p)) { throw "Header dir $h not found in $SrcInc" }
    Copy-Item -LiteralPath $p -Destination $DstInc -Recurse -Force
}

Write-Host ""
Write-Host "Staged $copied DLL(s) into $StageDir" -ForegroundColor Green
Write-Host "Staged .lib + headers into $LibsDir" -ForegroundColor Green
Write-Host ""
Write-Host "Add to .vcxproj Linker > Input > Additional Dependencies:" -ForegroundColor Yellow
Write-Host "  " ($RequiredLibs -join ';')
Write-Host "Add to Linker > General > Additional Library Directories:" -ForegroundColor Yellow
Write-Host "  `$(SolutionDir)..\..\..\libs\ffmpeg\lib"
Write-Host "Add to C/C++ > General > Additional Include Directories:" -ForegroundColor Yellow
Write-Host "  `$(SolutionDir)..\..\..\libs\ffmpeg\include"
