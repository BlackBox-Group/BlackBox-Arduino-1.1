// Читать строку из файла до стоп-символа
String fileReadUntil(File* f, char terminator) {
  String s = "";
  while (f->available()) {
    char c = f->read();

    if (c != terminator) s += c;
    else break;
  }

  return s;
}

// Работа с зашифрованными файлами
// Размер зашифрованного блока: 16 символов
// Указатель нужно освободить после работы
uint8_t* fileReadBlock(File* f) {
  uint8_t* blockBuffer = new uint8_t[16];
  for (uint8_t i = 0; i < 16; i++)
    blockBuffer[i] = f->read();

  return blockBuffer;
}

// Аналогично функции выше, но запись идёт во внешний указатель
// Должен быть размера 16 или больше (startPos указывает куда необходимо начать запись)
void fileReadBlock(File* f, uint8_t* blockBuffer, uint32_t startPos = 0) {
  for (int i = startPos; i < startPos + 16; i++)
    blockBuffer[i] = f->read();
}

// В теории эта функция должна рекурсивно удалять директорию, но я почему-то этого не наблюдаю :(
void rm(File dir, String tempPath) {
  while(true) {
    File entry = dir.openNextFile();
    String localPath;

    if (entry) {
      if ( entry.isDirectory() )
      {
        localPath = tempPath + entry.name() + '/' + '\0';
        char folderBuf[localPath.length()];
        localPath.toCharArray(folderBuf, localPath.length() );
        rm(entry, folderBuf);

        SD.rmdir( folderBuf );
      }
      else
      {
        localPath = tempPath + entry.name() + '\0';
        char charBuf[localPath.length()];
        localPath.toCharArray(charBuf, localPath.length() );

        SD.remove( charBuf );
      }
    } 
    else {
      // break out of recursion
      return;
    }
  }
}
