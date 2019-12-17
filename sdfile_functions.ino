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
      entry.close();
      return;
    }
  }
}

// Открыть файл и автоматически предупредить, если произошла ошибка
File openFile(String name, int mode) {
  File f = SD.open(name, mode);
  if (!f) {
    Serial.println("# Failed to open " + name);
    while (1);
  }

  return f;
}

// Прошерстить папку usr/ и сказать, какой следующее имя файла пользователя можно использовать для создания
String getAvailableUser() {
  File directory = openFile("usr/", FILE_READ);
  int maxN = 0;
  while (true) {
    File f = directory.openNextFile();
    if (!f) {
      break;
    }

    if (!f.isDirectory()) {
      String n = f.name();
      if (n.endsWith(".USR")) {
        // 0 - если перевести не удалось
        int num = n.substring(0, n.indexOf('.')).toInt();
        maxN = max(num, maxN);
      }
    }
  }

  directory.close();

  return String(maxN + 1) + ".USR";
}

// Нахождение USR файла пользователя по паролю и NUID. Пустая строка, если не найден
String findUser(String nuid, String master) {
  File directory = openFile("usr/", FILE_READ);
  int maxN = 0;
  while (true) {
    File f = directory.openNextFile();
    if (!f) {
      break;
    }

    if (!f.isDirectory()) {
      String n = f.name();
      if (n.endsWith(".USR")) {
        // Пробуем читать и смотрим на совпадение
        String file_nuid = fileReadUntil(&f, '\n');
        if (nuid == file_nuid) {
          String file_master = fileReadUntil(&f, '\n');
          if (master == file_master) {
            directory.close();
            f.close();
            return n;
          }
        }
      }
    }
  }

  directory.close();
  return "";
}
