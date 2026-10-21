# Run this script from an Emscripten-enabled terminal.
$ErrorActionPreference = "Stop"
Set-Location $PSScriptRoot

em++ smartpark.cpp -o smartpark.js `
  -std=c++17 `
  --bind `
  -O2 `
  -s MODULARIZE=1 `
  -s EXPORT_NAME=createSmartParkModule `
  -s ENVIRONMENT=web `
  -s ALLOW_MEMORY_GROWTH=1

if ($LASTEXITCODE -ne 0) {
    throw "Emscripten build failed. Read the compiler errors above."
}

Write-Host "Build successful. smartpark.js and smartpark.wasm were generated." -ForegroundColor Green
