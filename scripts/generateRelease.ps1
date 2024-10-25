# Получаем текущий Git-тег
$tag = git describe --tags --abbrev=0

$filename = "RF_HACK"

# Проверяем, установлен ли Git-тег
if ($tag -ne "") {
  # Создаем имя архива с использованием тега
  $archiveName = "$filename$tag.zip"
  

  # Путь к файлу, который нужно архивировать
  $filePathHex = "build/$filename.hex"
  $filePathBin = "build/$filename.bin"

  # Архивируем файл в ZIP-архив
  Compress-Archive -Update -Path $filePathHex -DestinationPath $archiveName
  Compress-Archive -Update -Path $filePathBin -DestinationPath $archiveName

  # Вывод сообщения об успешном архивировании
  Write-Host "The file was successfully archived $archiveName"
} else {
  # Вывод сообщения об отсутствии тега
  Write-Host "Error: Git tag is not set. Unable to create archive!"
}