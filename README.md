# Arduino-часть проекта BlackBox

## Схема:
    MOSI  -  ICSP-4
    MISO  -  ICSP-1   ->   MISO от SD карты подключён через резистор в ~360 Ом
    SCK   -  ICSP-3

    SD-Card Reader:
      CS - 7

    RFID:
      RST -  13
      SS  -  10
      SDA -  9


## Справка по файлам:
    Основной файл:
      BlackBox1.1.ino      

    Мои врапперы для упращения работы с AES, NUID и менеджментом файлов:
      aes_functions.ino    
      nuid_functions.ino
      sdfile_functions.ino

    Работа с RFID ридером:
      rfid_functions.ino

    Работа с доп. дебажными выводами:
      debug_functions.ino

    Мелкие вспомогательные функции:
      helper_functions.ino


Создано BlackBox Group
                                23.11.19
