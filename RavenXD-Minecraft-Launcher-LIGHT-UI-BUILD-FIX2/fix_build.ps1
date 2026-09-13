$ErrorActionPreference = "Stop"
Write-Host "RavenXD build compatibility fixer" -ForegroundColor Cyan

# The uploaded build log shows ImGui receiving a raw D3D11 SRV pointer in SplashScreen.cpp.
# ImGui expects ImTextureID, so explicitly cast the SRV pointer at the call site.
$splash = Join-Path $PSScriptRoot "src\UI\Screens\SplashScreen.cpp"
if (Test-Path $splash) {
    $s = Get-Content $splash -Raw
    $s = [regex]::Replace($s, 'AddImage\(([^,\r\n]+),', 'AddImage(reinterpret_cast<ImTextureID>($1),')
    Set-Content $splash $s -Encoding UTF8
    Write-Host "Fixed SplashScreen.cpp ImTextureID conversion." -ForegroundColor Green
} else {
    Write-Host "SplashScreen.cpp is not present in this package; no splash patch applied." -ForegroundColor Yellow
}

Write-Host "Modrinth regex and string/path fixes are already included in this package." -ForegroundColor Green
