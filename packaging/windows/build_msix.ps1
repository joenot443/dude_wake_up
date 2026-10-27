<#
.SYNOPSIS
Packages the staged Windows release as an MSIX for the Microsoft Store.

.DESCRIPTION
Takes dist\Nottawa-<version>\ (staged by build_release.ps1), adds an
AppxManifest.xml and the tile / taskbar images (generated from the app icon),
indexes them (resources.pri) and packs dist\Nottawa-<version>.msix.

The Store signs the package when it's submitted, so it's left unsigned
unless -Certificate is given (to install it locally from the .msix; with
Developer Mode on, -Register installs the unpacked layout instead).

The package identity comes from Partner Center (Apps > Nottawa > Product
identity): -IdentityName, -Publisher and -PublisherDisplayName. Until the
name is reserved, the defaults below are test values: fine for local
installs, rejected by the Store.

.EXAMPLE
powershell -File packaging\windows\build_msix.ps1 -Register
powershell -File packaging\windows\build_msix.ps1 -IdentityName 12345Joe.Nottawa -Publisher "CN=ABCD-1234" -PublisherDisplayName "Joe Crozier"
#>

[CmdletBinding()]
param(
    [string]$IdentityName = 'Nottawa.Test',
    [string]$Publisher = 'CN=Nottawa Test',
    [string]$PublisherDisplayName = 'Joe Crozier',
    [string]$Certificate,
    [string]$PfxPassword,
    [switch]$Register
)

$ErrorActionPreference = 'Stop'
$RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
$Version  = (Get-Content (Join-Path $RepoRoot 'VERSION') -TotalCount 1).Trim()
$Stage    = Join-Path $RepoRoot "dist\Nottawa-$Version"
$Layout   = Join-Path $RepoRoot "dist\msix-$Version"
$Msix     = Join-Path $RepoRoot "dist\Nottawa-$Version.msix"
if (-not (Test-Path (Join-Path $Stage 'Nottawa.exe'))) { throw "$Stage not staged: run build_release.ps1 first." }

$sdk = Get-ChildItem "${env:ProgramFiles(x86)}\Windows Kits\10\bin" -Directory | Where-Object { $_.Name -match '^10\.' } |
       Sort-Object { [version]$_.Name } -Descending | ForEach-Object { Join-Path $_.FullName 'x64' } |
       Where-Object { Test-Path (Join-Path $_ 'makeappx.exe') } | Select-Object -First 1
if (-not $sdk) { throw 'makeappx.exe not found: install the Windows 10/11 SDK.' }
$makeappx = Join-Path $sdk 'makeappx.exe'
$makepri  = Join-Path $sdk 'makepri.exe'

Write-Host "==> MSIX layout $Layout" -ForegroundColor Cyan
if (Test-Path $Layout) { Remove-Item $Layout -Recurse -Force }
robocopy $Stage $Layout /E /NFL /NDL /NJH /NJS /NP | Out-Null
if ($LASTEXITCODE -ge 8) { throw "Copying the staged folder failed (robocopy $LASTEXITCODE)" }

# ----- Images ----------------------------------------------------------------
# The app icon at each size Windows asks for: tiles at scale-100..400, the
# taskbar / Start icon at target sizes (unplated: drawn without a tile behind).

Add-Type -AssemblyName System.Drawing
$assets = Join-Path $Layout 'Assets'
New-Item -ItemType Directory $assets | Out-Null
$icon = [System.Drawing.Image]::FromFile((Join-Path $RepoRoot 'app\nottawa_app\resources\nottawa-1024.png'))
function Save-Image([string]$Name, [int]$Width, [int]$Height, [double]$IconFraction = 1.0) {
    $bmp = New-Object System.Drawing.Bitmap $Width, $Height, ([System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
    $g = [System.Drawing.Graphics]::FromImage($bmp)
    $g.InterpolationMode = [System.Drawing.Drawing2D.InterpolationMode]::HighQualityBicubic
    $g.PixelOffsetMode = [System.Drawing.Drawing2D.PixelOffsetMode]::HighQuality
    $g.CompositingQuality = [System.Drawing.Drawing2D.CompositingQuality]::HighQuality
    $side = [int]([Math]::Min($Width, $Height) * $IconFraction)
    $g.DrawImage($icon, [int](($Width - $side) / 2), [int](($Height - $side) / 2), $side, $side)
    $g.Dispose()
    $bmp.Save((Join-Path $assets $Name), [System.Drawing.Imaging.ImageFormat]::Png)
    $bmp.Dispose()
}
foreach ($scale in 100, 125, 150, 200, 400) {
    $f = $scale / 100.0
    Save-Image "Square44x44Logo.scale-$scale.png"   ([int](44 * $f))  ([int](44 * $f))
    Save-Image "Square150x150Logo.scale-$scale.png" ([int](150 * $f)) ([int](150 * $f)) 0.7
    Save-Image "Wide310x150Logo.scale-$scale.png"   ([int](310 * $f)) ([int](150 * $f)) 0.7
    Save-Image "StoreLogo.scale-$scale.png"         ([int](50 * $f))  ([int](50 * $f))
}
foreach ($size in 16, 20, 24, 30, 32, 36, 40, 48, 60, 64, 72, 80, 96, 256) {
    Save-Image "Square44x44Logo.targetsize-$size.png" $size $size
    Save-Image "Square44x44Logo.targetsize-${size}_altform-unplated.png" $size $size
    Save-Image "Square44x44Logo.targetsize-${size}_altform-lightunplated.png" $size $size
}
$icon.Dispose()

# ----- Manifest --------------------------------------------------------------

$esc = [System.Security.SecurityElement]
@"
<?xml version="1.0" encoding="utf-8"?>
<Package xmlns="http://schemas.microsoft.com/appx/manifest/foundation/windows10"
         xmlns:uap="http://schemas.microsoft.com/appx/manifest/uap/windows10"
         xmlns:rescap="http://schemas.microsoft.com/appx/manifest/foundation/windows10/restrictedcapabilities"
         IgnorableNamespaces="uap rescap">
  <Identity Name="$($esc::Escape($IdentityName))" Publisher="$($esc::Escape($Publisher))"
            Version="$Version.0" ProcessorArchitecture="x64" />
  <Properties>
    <DisplayName>Nottawa</DisplayName>
    <PublisherDisplayName>$($esc::Escape($PublisherDisplayName))</PublisherDisplayName>
    <Logo>Assets\StoreLogo.png</Logo>
    <Description>Audio-reactive visuals for live performance.</Description>
  </Properties>
  <Dependencies>
    <!-- Windows 10 1903: the UTF-8 code page Nottawa.exe's manifest asks for. -->
    <TargetDeviceFamily Name="Windows.Desktop" MinVersion="10.0.18362.0" MaxVersionTested="10.0.26100.0" />
  </Dependencies>
  <Resources>
    <Resource Language="en-us" />
  </Resources>
  <Applications>
    <Application Id="Nottawa" Executable="Nottawa.exe" EntryPoint="Windows.FullTrustApplication">
      <uap:VisualElements DisplayName="Nottawa" Description="Audio-reactive visuals for live performance."
                          BackgroundColor="transparent"
                          Square150x150Logo="Assets\Square150x150Logo.png" Square44x44Logo="Assets\Square44x44Logo.png">
        <uap:DefaultTile Wide310x150Logo="Assets\Wide310x150Logo.png" />
      </uap:VisualElements>
      <Extensions>
        <!-- nottawa://strands/<slug> links open shared strands (the link is
             passed to Nottawa.exe as its argument). -->
        <uap:Extension Category="windows.protocol">
          <uap:Protocol Name="nottawa">
            <uap:DisplayName>Nottawa</uap:DisplayName>
          </uap:Protocol>
        </uap:Extension>
      </Extensions>
    </Application>
  </Applications>
  <Capabilities>
    <Capability Name="internetClient" />
    <rescap:Capability Name="runFullTrust" />
    <DeviceCapability Name="microphone" />
    <DeviceCapability Name="webcam" />
  </Capabilities>
</Package>
"@ | Set-Content (Join-Path $Layout 'AppxManifest.xml') -Encoding UTF8

# ----- Resource index and package ---------------------------------------------

Write-Host '==> Indexing resources' -ForegroundColor Cyan
$priConfig = Join-Path $env:TEMP "nottawa-priconfig-$PID.xml"
& $makepri createconfig /cf $priConfig /dq en-US /pv 10.0.0 /o | Out-Null
if ($LASTEXITCODE -ne 0) { throw "makepri createconfig failed ($LASTEXITCODE)" }
& $makepri new /pr $Layout /cf $priConfig /mn (Join-Path $Layout 'AppxManifest.xml') /of (Join-Path $Layout 'resources.pri') /o | Out-Null
if ($LASTEXITCODE -ne 0) { throw "makepri new failed ($LASTEXITCODE)" }
Remove-Item $priConfig

Write-Host "==> Packing $Msix" -ForegroundColor Cyan
& $makeappx pack /d $Layout /p $Msix /o | Out-Null
if ($LASTEXITCODE -ne 0) { throw "makeappx pack failed ($LASTEXITCODE)" }

if ($Certificate) {
    $a = @('sign', '/fd', 'SHA256')
    if ($Certificate -like '*.pfx') { $a += @('/f', $Certificate); if ($PfxPassword) { $a += @('/p', $PfxPassword) } }
    else { $a += @('/sha1', $Certificate) }
    & (Join-Path $sdk 'signtool.exe') @a $Msix
    if ($LASTEXITCODE -ne 0) { throw "Signing $Msix failed ($LASTEXITCODE)" }
}

if ($Register) {
    # Developer Mode: install the unpacked layout (no signature needed).
    Write-Host '==> Registering the layout (Developer Mode)' -ForegroundColor Cyan
    Get-AppxPackage -Name $IdentityName | Remove-AppxPackage
    Add-AppxPackage -Register (Join-Path $Layout 'AppxManifest.xml')
    Get-AppxPackage -Name $IdentityName | Select-Object Name, Version, PackageFamilyName | Format-List
}

'{0} {1:N1} MB' -f (Split-Path -Leaf $Msix), ((Get-Item $Msix).Length / 1MB)
if ($IdentityName -eq 'Nottawa.Test') { Write-Warning 'Test identity: pass the Partner Center values for a Store submission.' }
