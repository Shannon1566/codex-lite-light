[CmdletBinding()]
param(
    [ValidateSet('', 'SessionStart', 'UserPromptSubmit', 'PreToolUse',
        'PermissionRequest', 'PostToolUse', 'Stop', 'SessionEnd')]
    [string]$Event = '',
    [string]$Port = $env:CODEX_LIGHT_PORT,
    [switch]$DryRun
)

$ErrorActionPreference = 'Stop'

function Read-HookPayload {
    $raw = [Console]::In.ReadToEnd()
    if ([string]::IsNullOrWhiteSpace($raw)) { return $null }
    return $raw | ConvertFrom-Json -Depth 20
}

function Get-HookEventName($payload) {
    if (-not [string]::IsNullOrWhiteSpace($Event)) { return $Event }
    foreach ($name in @('hook_event_name', 'event', 'type')) {
        $property = $payload.PSObject.Properties[$name]
        if ($null -ne $property -and
            -not [string]::IsNullOrWhiteSpace([string]$property.Value)) {
            return [string]$property.Value
        }
    }
    throw 'Hook event name is missing. Pass -Event or provide hook_event_name in JSON.'
}

function Test-PostToolFailure($payload) {
    if ($null -eq $payload) { return $false }
    foreach ($name in @('success', 'ok')) {
        $property = $payload.PSObject.Properties[$name]
        if ($null -ne $property -and $property.Value -eq $false) { return $true }
    }
    foreach ($name in @('error', 'failure')) {
        $property = $payload.PSObject.Properties[$name]
        if ($null -ne $property -and $null -ne $property.Value) { return $true }
    }
    $response = $payload.PSObject.Properties['tool_response']
    if ($null -ne $response) {
        $responseJson = $response.Value | ConvertTo-Json -Depth 20 -Compress
        if ($responseJson -match '"isError":true' -or
            $responseJson -match '"exit_code":(?:-[0-9]+|[1-9][0-9]*)' -or
            $responseJson -match 'Exit code:\s*[1-9][0-9]*') {
            return $true
        }
    }
    return $false
}

function ConvertTo-LightCommand([string]$eventName, $payload) {
    switch ($eventName) {
        'SessionStart' { return 'I' }
        'UserPromptSubmit' { return 'W' }
        'PreToolUse' { return 'W' }
        'PermissionRequest' { return 'A' }
        'PostToolUse' {
            if (Test-PostToolFailure $payload) { return 'E' }
            return $null
        }
        'Stop' { return 'S' }
        'SessionEnd' { return 'O' }
        default { return $null }
    }
}

function Resolve-SerialPort([string]$configuredPort) {
    if (-not [string]::IsNullOrWhiteSpace($configuredPort)) { return $configuredPort }
    $ports = @([System.IO.Ports.SerialPort]::GetPortNames() | Sort-Object)
    if ($ports.Count -eq 1) { return $ports[0] }
    if ($ports.Count -eq 0) {
        throw 'No serial port found. Set CODEX_LIGHT_PORT (for example COM5).'
    }
    throw "Multiple serial ports found ($($ports -join ', ')). Set CODEX_LIGHT_PORT."
}

$payload = Read-HookPayload
$eventName = Get-HookEventName $payload
$command = ConvertTo-LightCommand $eventName $payload

if ($null -eq $command) {
    Write-Verbose "Ignoring hook event: $eventName"
    exit 0
}

if ($DryRun -or $env:CODEX_LIGHT_DRY_RUN -eq '1') {
    Write-Output $command
    if ($eventName -eq 'Stop') { Write-Output '{}' }
    exit 0
}

$serialPortName = Resolve-SerialPort $Port
$serial = [System.IO.Ports.SerialPort]::new($serialPortName, 115200, 'None', 8, 'One')
$serial.NewLine = "`n"
$serial.WriteTimeout = 1000
$serial.DtrEnable = $true

try {
    $serial.Open()
    Start-Sleep -Milliseconds 50
    $serial.WriteLine($command)
}
finally {
    if ($serial.IsOpen) { $serial.Close() }
    $serial.Dispose()
}

# Stop hooks must emit valid JSON on a successful exit. An empty object means
# "do not alter the Codex turn".
if ($eventName -eq 'Stop') {
    Write-Output '{}'
}
