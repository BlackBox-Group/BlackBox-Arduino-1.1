/*
    Arduino-часть проекта BlackBox
    Схема:
      MOSI  -  ICSP-4
      MISO  -  ICSP-1   -> MISO от SD карты подключён через резистор в ~360 Ом
      SCK   -  ICSP-3

      SD-Card Reader:
        CS - 7

      RFID:
        RST -  13
        SS  -  10
        SDA -  9


    Справка по файлам:
      Мои врапперы для упращения работы с AES, NUID и менеджментом файлов:
        aes_functions    
        nuid_functions
        sdfile_functions
      Работа с RFID ридером:
        rfid_functions
      Работа с доп. дебажными выводами:
        debug_functions
      Мелкие вспомогательные функции:
        helper_functions

     
    Создано BlackBox Group
                                    23.11.19
*/

// Требуемые библиотеки
#include <SD.h>
#include <RFID.h>
#include <aes.hpp>

// Выбранные цифровые выводы
#define SS_RFID   10
#define RST_RFID  9
#define CS_SD     7

// RFID ридер
const RFID rfid(SS_RFID, RST_RFID);

void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:

}
