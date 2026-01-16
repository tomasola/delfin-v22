Add-Type -AssemblyName System.Drawing

function Resize-Image {
    param (
        [string]$Path,
        [int]$Width,
        [int]$Height
    )

    $fullPath = Resolve-Path $Path
    Write-Host "Resizing $fullPath to ${Width}x${Height}..."

    try {
        $img = [System.Drawing.Image]::FromFile($fullPath)
        $bitmap = New-Object System.Drawing.Bitmap($Width, $Height)
        $graph = [System.Drawing.Graphics]::FromImage($bitmap)
        
        $graph.InterpolationMode = [System.Drawing.Drawing2D.InterpolationMode]::HighQualityBicubic
        $graph.PixelOffsetMode = [System.Drawing.Drawing2D.PixelOffsetMode]::HighQuality
        $graph.CompositingQuality = [System.Drawing.Drawing2D.CompositingQuality]::HighQuality
        
        $graph.DrawImage($img, 0, 0, $Width, $Height)
        
        $img.Dispose()
        $graph.Dispose()
        
        $bitmap.Save($fullPath, [System.Drawing.Imaging.ImageFormat]::Png)
        $bitmap.Dispose()
        
        Write-Host "Success!"
    }
    catch {
        Write-Error "Failed to resize $Path : $_"
    }
}

Resize-Image -Path "public/icon-192.png" -Width 192 -Height 192
Resize-Image -Path "public/icon-512.png" -Width 512 -Height 512
Resize-Image -Path "public/screenshot-narrow.png" -Width 540 -Height 720
Resize-Image -Path "public/screenshot-wide.png" -Width 720 -Height 540
