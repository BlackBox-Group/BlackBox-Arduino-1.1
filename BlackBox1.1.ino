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
      Поведение системы:
        logic_functions

     
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
  // Инициализация встроенного светодиода
  pinMode(LED_BUILTIN, OUTPUT);

  // Инициализация Serial
  Serial.begin(9600);
  while (!Serial) {
    ; // Ждём, пока подключатся по Serial порту
  }

  // Запуск SPI и инициализация RFID
  SPI.begin();
  rfid.init();

  // Инициализация SD карты
  if (!SD.begin(CS_SD)) {
    Serial.println("# SD card init failed.");
    // Останавливаем выполнение программы
    while (1);
  }

  if (!SD.exists("cards.txt")) {
    SD.open("cards.txt", FILE_WRITE).close();
  }
  if (!SD.exists("usr/")) {
    SD.mkdir("usr");
  }

  Serial.println("# OK");
}

// Переменная для хранения снапшотов времени
unsigned long time_now;

// Структура, хранящая в себе текущее состояние системы
// Нужна для сохранения состояния между циклами loop
struct State {
  bool usernameRequired = false;
  bool masterRequired   = false;
  bool userCreation     = false;
  bool loginProcess     = false;
} state;

void loop() {
  // Работа с событиями по времени
  if (millis() - time_now > 1000) {   // Прошла секунда
    time_now = millis();

    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);
  }

  // Если доступны байты к чтению по Serial
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    
    // Все сообщения, которые начинаются с '#', являются комментариями и их нужно игнорировать
    if (command.startsWith("#")) {
      return;
    }

    // В ином случае, отправить команду на обработку в логический модуль
    analyzeCommand(command);
  }
}
