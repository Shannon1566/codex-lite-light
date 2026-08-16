[CmdletBinding()]
param(
    [string]$Port = 'COM5',
    [string]$TitleContains = ''
)

$ErrorActionPreference = 'Stop'
$watcher = Join-Path $PSScriptRoot 'codex_title_watcher.py'
$python = (Get-Command pythonw.exe -ErrorAction Stop).Source
$arguments = @($watcher, '--port', $Port)
if (-not [string]::IsNullOrWhiteSpace($TitleContains)) {
    $arguments += @('--title-contains', $TitleContains)
}

Start-Process -FilePath $python -ArgumentList $arguments -WindowStyle Hidden
Write-Output "Codex Lite Light watcher started for $Port."
