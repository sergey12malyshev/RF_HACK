# Getting the current Git tag
$tag = git describe --tags --abbrev=0

$git_hash = git log -1 --format="%h"

$filename = "RF_HACK"

# Checking if the Git tag is installed
if ($tag -ne "") {
  
  $timestamp = Get-Date -Format "dd.MM.yyyy"
  # Creating an archive name using a tag
  $archiveName = "$filename" + "-v" + "$tag" + "-d" + "$timestamp.zip"
  

  # The path to the file to archive
  $filePathHex = "build/$filename.hex"
  $filePathBin = "build/$filename.bin"
  $fileReadme =  "build/Readme.txt"

  Set-Content -Path $fileReadme -Value "Firmware $filename version: $tag data build: $timestamp hash commit: $git_hash."
  
  # Archiving the file to a ZIP archive
  Compress-Archive -Update -Path $filePathHex -DestinationPath $archiveName
  Compress-Archive -Update -Path $filePathBin -DestinationPath $archiveName
  Compress-Archive -Update -Path $fileReadme -DestinationPath $archiveName

  # The output of the successful archiving message
  Write-Host "The file was successfully archived $archiveName"
} else {
  # Displaying a message about the absence of a tag
  Write-Host "Error: Git tag is not set. Unable to create archive!"
}