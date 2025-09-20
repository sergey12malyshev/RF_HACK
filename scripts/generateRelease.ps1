# Getting the current Git tag
$tag = git describe --tags --abbrev=0

$filename = "RF_HACK"

# Checking if the Git tag is installed
if ($tag -ne "") {
  # Creating an archive name using a tag
  $archiveName = "$filename$tag.zip"
  

  # The path to the file to archive
  $filePathHex = "build/$filename.hex"
  $filePathBin = "build/$filename.bin"

  # Archiving the file to a ZIP archive
  Compress-Archive -Update -Path $filePathHex -DestinationPath $archiveName
  Compress-Archive -Update -Path $filePathBin -DestinationPath $archiveName

  # The output of the successful archiving message
  Write-Host "The file was successfully archived $archiveName"
} else {
  # Displaying a message about the absence of a tag
  Write-Host "Error: Git tag is not set. Unable to create archive!"
}