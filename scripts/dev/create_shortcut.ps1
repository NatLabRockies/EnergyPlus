param (
    [string]$TargetPath,
    [string]$ShortcutPath,
    [string]$Arguments
)

$WScriptShell = New-Object -ComObject WScript.Shell
$Shortcut = $WScriptShell.CreateShortcut($ShortcutPath)
$Shortcut.TargetPath = $TargetPath
$Shortcut.Arguments = $Arguments
$Shortcut.Save()
