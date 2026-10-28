# Installs the pre-push hook (runs Tests\run_tests.ps1 before every push) into
# this clone's .git\hooks. Run once per clone.
$Repo = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
$Target = Join-Path $Repo '.git\hooks\pre-push'
Copy-Item (Join-Path $PSScriptRoot 'pre-push') $Target -Force
Write-Host "Installed $Target"
