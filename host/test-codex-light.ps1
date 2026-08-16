$ErrorActionPreference = 'Stop'
$scriptPath = Join-Path $PSScriptRoot 'codex-light.ps1'

$cases = @(
    @{ Event = 'SessionStart'; Expected = 'I'; Json = '{}' },
    @{ Event = 'UserPromptSubmit'; Expected = 'W'; Json = '{}' },
    @{ Event = 'PreToolUse'; Expected = 'W'; Json = '{}' },
    @{ Event = 'PermissionRequest'; Expected = 'A'; Json = '{}' },
    @{ Event = 'PostToolUse'; Expected = 'E'; Json = '{"success":false}' },
    @{ Event = 'PostToolUse'; Expected = 'E'; Json = '{"tool_response":{"exit_code":1}}' },
    @{ Event = 'PostToolUse'; Expected = ''; Json = '{"success":true}' },
    @{ Event = 'Stop'; Expected = "S`n{}"; Json = '{}' },
    @{ Event = 'SessionEnd'; Expected = 'O'; Json = '{}' }
)

foreach ($case in $cases) {
    $actual = $case.Json | pwsh -NoProfile -File $scriptPath -Event $case.Event -DryRun
    $actualText = (($actual | Out-String).Trim() -replace "`r`n", "`n")
    $expectedText = ($case.Expected -replace "`r`n", "`n")
    if ($actualText -ne $expectedText) {
        throw "Event $($case.Event): expected '$($case.Expected)', got '$actualText'."
    }
}

Write-Output "Host adapter tests passed: $($cases.Count)"
