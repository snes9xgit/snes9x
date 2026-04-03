# Toggle between VS2022 (v143) and Windows XP (v141_xp) build toolsets.
# Run this script from the win32/ directory.
#
# When Directory.Build.props exists -> XP mode (v141_xp)
# When it doesn't exist -> VS2022 mode (v143, from vcxproj defaults)

$propsFile = Join-Path $PSScriptRoot "Directory.Build.props"
$xpTemplate = Join-Path $PSScriptRoot "Directory.Build.props.xp"

if (Test-Path $propsFile) {
    Remove-Item $propsFile
    Write-Host ""
    Write-Host "  Switched to VS2022 (v143) build tools" -ForegroundColor Green
    Write-Host "  Windows 10+ target | Requires VS2022" -ForegroundColor DarkGray
    Write-Host ""
} else {
    if (Test-Path $xpTemplate) {
        Copy-Item $xpTemplate $propsFile
    } else {
        @"
<Project>
  <PropertyGroup>
    <PlatformToolset>v141_xp</PlatformToolset>
    <WindowsTargetPlatformVersion>8.1</WindowsTargetPlatformVersion>
  </PropertyGroup>
</Project>
"@ | Set-Content $propsFile -Encoding UTF8
    }
    Write-Host ""
    Write-Host "  Switched to Windows XP (v141_xp) build tools" -ForegroundColor Yellow
    Write-Host "  Windows XP+ target | Requires VS2017 with v141_xp" -ForegroundColor DarkGray
    Write-Host ""
}

Write-Host "  Restart Visual Studio for changes to take effect." -ForegroundColor Cyan
Write-Host ""
