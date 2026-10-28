<#
.SYNOPSIS
Runs the Windows App Certification Kit (the Store's own checks) on the MSIX.

.DESCRIPTION
The kit installs the package to test it, and Windows only installs signed
packages; the Store signs the real one. So this signs a copy
(dist\wack\Nottawa-<version>-test.msix) with a throwaway certificate whose
subject is Nottawa's publisher, trusts that certificate for the run, tests,
then removes the certificate and the test install. The package in dist\ is
left unsigned for the Store.

Needs an elevated PowerShell (the kit and the machine certificate store do).
Report: dist\wack\report.xml; log: dist\wack\run.log.

.EXAMPLE
powershell -File packaging\windows\run_wack.ps1    (from an elevated prompt)
#>

$ErrorActionPreference = 'Stop'
$RepoRoot  = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
$Version   = (Get-Content (Join-Path $RepoRoot 'VERSION') -TotalCount 1).Trim()
$Msix      = Join-Path $RepoRoot "dist\Nottawa-$Version.msix"
$Work      = Join-Path $RepoRoot 'dist\wack'
$Publisher = 'CN=30C50E17-1DED-455C-93DC-3B02982966B0'
$AppCert   = "${env:ProgramFiles(x86)}\Windows Kits\10\App Certification Kit\appcert.exe"

New-Item -ItemType Directory -Force $Work | Out-Null
Start-Transcript -Path (Join-Path $Work 'run.log') -Force | Out-Null
$cert = $null
$trusted = $null
try {
    if (-not ([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)) {
        throw 'Run this from an elevated PowerShell.'
    }
    if (-not (Test-Path $Msix)) { throw "$Msix not built: run build_release.ps1 and build_msix.ps1." }
    $signtool = Get-ChildItem "${env:ProgramFiles(x86)}\Windows Kits\10\bin" -Recurse -Filter signtool.exe |
        Where-Object { $_.FullName -match '\\x64\\' } | Sort-Object FullName -Descending | Select-Object -First 1
    if (-not $signtool) { throw 'signtool.exe not found (Windows SDK).' }

    Write-Host "==> Throwaway certificate for $Publisher"
    $cert = New-SelfSignedCertificate -Type Custom -Subject $Publisher -KeyUsage DigitalSignature `
        -FriendlyName 'Nottawa WACK test (temporary)' -CertStoreLocation 'Cert:\CurrentUser\My' `
        -TextExtension @('2.5.29.37={text}1.3.6.1.5.5.7.3.3', '2.5.29.19={text}') -NotAfter (Get-Date).AddDays(1)
    $pfx = Join-Path $Work 'test.pfx'
    $password = ConvertTo-SecureString -String ([Guid]::NewGuid().ToString()) -Force -AsPlainText
    Export-PfxCertificate -Cert $cert -FilePath $pfx -Password $password | Out-Null
    $cer = Join-Path $Work 'test.cer'
    Export-Certificate -Cert $cert -FilePath $cer | Out-Null
    $trusted = Import-Certificate -FilePath $cer -CertStoreLocation 'Cert:\LocalMachine\TrustedPeople'

    Write-Host '==> Signing a copy of the package'
    $testMsix = Join-Path $Work "Nottawa-$Version-test.msix"
    Copy-Item $Msix $testMsix -Force
    $plain = [Runtime.InteropServices.Marshal]::PtrToStringAuto([Runtime.InteropServices.Marshal]::SecureStringToBSTR($password))
    & $signtool.FullName sign /fd SHA256 /f $pfx /p $plain $testMsix | Out-Host
    if ($LASTEXITCODE -ne 0) { throw 'signtool failed.' }

    Write-Host '==> Running the App Certification Kit (this takes a few minutes)'
    $report = Join-Path $Work 'report.xml'
    Remove-Item $report -ErrorAction SilentlyContinue
    & $AppCert reset | Out-Null
    & $AppCert test -appxpackagepath $testMsix -reportoutputpath $report | Out-Host
    Write-Host "==> appcert exit $LASTEXITCODE; report $report"
} finally {
    Write-Host '==> Cleaning up'
    Get-AppxPackage -Name 'JoeCrozier.Nottawa' -ErrorAction SilentlyContinue | Remove-AppxPackage -ErrorAction SilentlyContinue
    if ($trusted) { Remove-Item "Cert:\LocalMachine\TrustedPeople\$($trusted.Thumbprint)" -ErrorAction SilentlyContinue }
    if ($cert) { Remove-Item "Cert:\CurrentUser\My\$($cert.Thumbprint)" -ErrorAction SilentlyContinue }
    Remove-Item (Join-Path $Work 'test.pfx'), (Join-Path $Work 'test.cer') -ErrorAction SilentlyContinue
    Stop-Transcript | Out-Null
}
