@echo off
setlocal

cd /d "%~dp0"

powershell -NoProfile -ExecutionPolicy Bypass -Command ^
  "$ErrorActionPreference = 'Stop';" ^
  "$dist = Join-Path (Get-Location) 'dist';" ^
  "$siteRoot = 't:\www\timthompson.com\html';" ^
  "$target = Join-Path $siteRoot 'keykitwasm';" ^
  "$zip = Get-ChildItem -LiteralPath $dist -Filter '*.zip' | Sort-Object LastWriteTime -Descending | Select-Object -First 1;" ^
  "if (-not $zip) { throw 'No distribution zip files found in dist.' }" ^
  "if (-not (Test-Path -LiteralPath $siteRoot)) { throw \"Website root not found: $siteRoot\" }" ^
  "Write-Host \"Using distribution: $($zip.FullName)\";" ^
  "if (Test-Path -LiteralPath $target) { Write-Host \"Removing existing: $target\"; Remove-Item -LiteralPath $target -Recurse -Force }" ^
  "Write-Host \"Expanding to: $siteRoot\";" ^
  "Expand-Archive -LiteralPath $zip.FullName -DestinationPath $siteRoot -Force;" ^
  "if (-not (Test-Path -LiteralPath (Join-Path $target 'index.html'))) { throw \"Deployment did not produce $target\index.html\" }" ^
  "Write-Host \"Updated website: $target\""

exit /b %errorlevel%
