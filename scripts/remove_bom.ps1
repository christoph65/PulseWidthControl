$dir = 'C:\git\Boatsteering\projects\GyroMotorControler'
Get-ChildItem -Path $dir -Filter *.cpp | ForEach-Object {
  $path = $_.FullName
  $bytes = [System.IO.File]::ReadAllBytes($path)
  if ($bytes.Length -ge 3 -and $bytes[0] -eq 0xEF -and $bytes[1] -eq 0xBB -and $bytes[2] -eq 0xBF) {
    [System.IO.File]::WriteAllBytes($path, $bytes[3..($bytes.Length - 1)])
    Write-Host "Removed BOM from $path"
  }
}