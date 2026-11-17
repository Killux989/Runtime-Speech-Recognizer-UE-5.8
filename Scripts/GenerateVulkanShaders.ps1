# Georgy Treshchev 2024.
# One-time step that enables Vulkan GPU acceleration. It generates ggml's Vulkan shaders for the vendored whisper.cpp
# commit into Source/WhisperCpp/Private/VulkanShaders. WhisperCpp.Build.cs turns on GGML_USE_VULKAN automatically
# once those files exist. Only the person building the plugin needs this, not players.
# Requires: git, the Vulkan SDK (for glslc, sets VULKAN_SDK), and Visual Studio / Build Tools with C++ (CMake is bundled).

$ErrorActionPreference = "Stop"
$Commit = "aba02defd6bb6ff5de63721c21e3456645c153e2" # must match Source/ThirdParty (whisper.cpp 1.9.5 / ggml 0.29.0)
$PluginDir = Split-Path -Parent $PSScriptRoot
$OutDir = Join-Path $PluginDir "Source\WhisperCpp\Private\VulkanShaders"
$WorkDir = Join-Path $env:TEMP "rsr-vulkan-shaders"

if (-not $env:VULKAN_SDK) { throw "VULKAN_SDK is not set. Install the Vulkan SDK from https://vulkan.lunarg.com and reopen the terminal." }

$CMake = (Get-Command cmake -ErrorAction SilentlyContinue).Source
if (-not $CMake) {
    $VsWhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    $VsPath = & $VsWhere -latest -products * -property installationPath
    $CMake = Join-Path $VsPath "Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
}
if (-not (Test-Path $CMake)) { throw "CMake not found. Install the 'C++ CMake tools' Visual Studio component." }

if (-not (Test-Path (Join-Path $WorkDir ".git"))) {
    git init $WorkDir | Out-Null
    git -C $WorkDir remote add origin https://github.com/ggml-org/whisper.cpp.git
}
git -C $WorkDir fetch --depth 1 origin $Commit
git -C $WorkDir checkout --force FETCH_HEAD

$Build = Join-Path $WorkDir "build"
& $CMake -S $WorkDir -B $Build -DGGML_VULKAN=ON -DWHISPER_BUILD_EXAMPLES=OFF -DWHISPER_BUILD_TESTS=OFF -DWHISPER_BUILD_SERVER=OFF
if ($LASTEXITCODE -ne 0) { throw "CMake configure failed" }
& $CMake --build $Build --config Release --target ggml-vulkan --parallel
if ($LASTEXITCODE -ne 0) { throw "Shader generation build failed" }

$Gen = Join-Path $Build "ggml\src\ggml-vulkan"
if (Test-Path $OutDir) { Remove-Item -Recurse -Force $OutDir }
New-Item -ItemType Directory -Force $OutDir | Out-Null
Copy-Item (Join-Path $Gen "ggml-vulkan-shaders.hpp") $OutDir
Get-ChildItem $Gen -Filter "*.comp.cpp" | Copy-Item -Destination $OutDir

# The glslc feature probes decide which shader variants exist, so the plugin build must use the same defines
$Proj = Get-ChildItem $Gen -Filter "ggml-vulkan.vcxproj" | Select-Object -First 1
$Defines = [regex]::Matches((Get-Content $Proj.FullName -Raw), "GGML_VULKAN_[A-Z0-9_]+_GLSLC_SUPPORT") | ForEach-Object { $_.Value } | Sort-Object -Unique
Set-Content -Encoding ascii -Path (Join-Path $OutDir "Defines.txt") -Value $Defines

Write-Host "Generated $((Get-ChildItem $OutDir -Filter *.cpp).Count) shader units into $OutDir"
Write-Host "Defines: $($Defines -join ', ')"
Write-Host "Rebuild the plugin to enable Vulkan."
