$sourceDir = "D:\DELFIN\delfin_referencias"
$targetDir = "D:\PROYECTOS_aplicaciones\DELFIN\public\images"
$jsonPath = "D:\PROYECTOS_aplicaciones\DELFIN\src\references.json"

# Create target directory
New-Item -ItemType Directory -Force -Path $targetDir | Out-Null

# Scan for images
$images = Get-ChildItem -Path $sourceDir -Recurse -Include *.jpg,*.jpeg,*.png,*.webp -File

$references = @()
$counter = 0

foreach ($img in $images) {
    $counter++
    $code = [System.IO.Path]::GetFileNameWithoutExtension($img.Name)
    $newName = "$code$($img.Extension)"
    
    # Copy image to public folder
    Copy-Item -Path $img.FullName -Destination "$targetDir\$newName" -Force
    
    # Add to index
    $references += @{
        code = $code
        image = "/images/$newName"
        category = $img.Directory.Name
    }
    
    if ($counter % 100 -eq 0) {
        Write-Host "Procesadas $counter imágenes..."
    }
}

# Generate JSON
$references | ConvertTo-Json -Depth 3 | Set-Content -Path $jsonPath -Encoding UTF8

Write-Host "✓ Completado: $counter imágenes procesadas"
Write-Host "✓ JSON generado en: $jsonPath"
