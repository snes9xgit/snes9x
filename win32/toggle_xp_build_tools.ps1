# Toggle between VS2022 (v143) and Windows XP (v141_xp) build toolsets.
#
# Uses Directory.Build.targets which is imported AFTER vcxproj files,
# so it can override the default v143 toolset.
#
# When Directory.Build.targets exists -> XP mode (v141_xp)
# When it doesn't exist -> VS2022 mode (v143, from vcxproj defaults)

$repoRoot = Split-Path $PSScriptRoot -Parent
$dirs = @($PSScriptRoot, (Join-Path $repoRoot "libretro"))

$targetsFile = Join-Path $PSScriptRoot "Directory.Build.targets"

if (Test-Path $targetsFile) {
    foreach ($dir in $dirs) {
        $tf = Join-Path $dir "Directory.Build.targets"
        if (Test-Path $tf) { Remove-Item $tf }
    }
    Write-Host ""
    Write-Host "  Switched to VS2022 (v143) build tools" -ForegroundColor Green
    Write-Host "  Windows 10+ target | Requires VS2022" -ForegroundColor DarkGray
    Write-Host ""
} else {
    foreach ($dir in $dirs) {
        $tf = Join-Path $dir "Directory.Build.targets"
        $xpTemplate = Join-Path $dir "Directory.Build.targets.xp"
        if (Test-Path $xpTemplate) {
            Copy-Item $xpTemplate $tf
        } else {
            @"
<Project>
  <PropertyGroup>
    <PlatformToolset>v141_xp</PlatformToolset>
    <WindowsTargetPlatformVersion>8.1</WindowsTargetPlatformVersion>
  </PropertyGroup>
</Project>
"@ | Set-Content $tf -Encoding UTF8
        }
    }
    Write-Host ""
    Write-Host "  Switched to Windows XP (v141_xp) build tools" -ForegroundColor Yellow
    Write-Host "  Windows XP+ target | Requires VS2017 with v141_xp" -ForegroundColor DarkGray
    Write-Host ""
}

Write-Host "  Restart Visual Studio for changes to take effect." -ForegroundColor Cyan
Write-Host ""
