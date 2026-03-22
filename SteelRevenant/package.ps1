param(
    [string]$Configuration = "Debug",
    [string]$Platform = "x64",
    [string]$OutDir = "dist"
)

$solution = Join-Path -Path (Get-Location) -ChildPath 'SteelRevenant.sln'
if (-not (Test-Path $solution)) { Write-Error "Solution not found"; exit 1 }

# build first
msbuild $solution /p:Configuration=$Configuration /p:Platform=$Platform /m
if ($LASTEXITCODE -ne 0) { Write-Error "Build failed"; exit $LASTEXITCODE }

# find exe
$bin = Get-ChildItem -Recurse -Filter "SteelRevenant.exe" | Where-Object { $_.FullName -match "\\$Configuration\\$Platform\\" } | Select-Object -First 1
if (-not $bin) { Write-Error "Built exe not found"; exit 1 }

# prepare dist
if (Test-Path $OutDir) { Remove-Item -Recurse -Force $OutDir }
New-Item -ItemType Directory -Path $OutDir | Out-Null
Copy-Item $bin.FullName -Destination $OutDir

# copy runtime assets (spritefonts, shaders) - heuristic
Get-ChildItem -Recurse -Include *.spritefont,*.png,*.wav,*.json,*.xml | ForEach-Object { Copy-Item $_.FullName -Destination $OutDir -Force }

Compress-Archive -Path $OutDir\* -DestinationPath "SteelRevenant_${Configuration}_${Platform}.zip" -Force
Write-Host "Packaged to SteelRevenant_${Configuration}_${Platform}.zip"