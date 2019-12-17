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
  /*while (!Serial) {
    ; // Ждём, пока подключатся по Serial порту
  }*/

  // Запуск SPI и инициализация RFID
  SPI.begin();
  rfid.init();

  // Инициализация SD карты
  if (!SD.begin(CS_SD)) {
    Serial.println("# SD card init failed.");
    digitalWrite(LED_BUILTIN, HIGH);
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

// Константы
#define RFID_TIMEOUT_SECONDS 10
#define LOGIN_EXPIRE_SECONDS 300

enum AuthState {
  STOPPED = -3,
  IN_PROGRESS = -2,
  DATA_COLLECTED = -1,
  SUCCESS = 1,
  FAILED = 2
};

// Структура, хранящая в себе текущее состояние системы
// Нужна для сохранения состояния между циклами loop
struct State {
  bool userCreation     = false;

  AuthState authState   = STOPPED;
  String userFileName   = "";
  String lastNuidLogged = "";
  
  bool rfidRequired     = false;
  bool rfidTimeout      = false;
  bool rfidSuccess      = false;

  bool masterRequired   = false;
  bool gotMaster        = false;
  String master         = "";

  bool usernameRequired = false;
  bool gotUsername      = false;
  String username       = "";

  bool serviceAddition  = false;
  bool serviceAuthed    = false;
  String serviceName    = "";
  String servicePass    = "";

  uint8_t* key          = NULL;

  unsigned long timeStamp_rfid = 0;
  int timeFromLastLogin = -1;
  
} state;

/*void blink(int amount) {
  for (int i = 0; i < amount; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(50);
    digitalWrite(LED_BUILTIN, LOW);
    delay(50);
  }

  //delay(500);
}*/

void loop() {
  // Работа с событиями по времени
  if (millis() - time_now > 1000) {   // Прошла секунда
    time_now = millis();

    if (state.timeFromLastLogin != -1) {
      state.timeFromLastLogin++;
      if (state.timeFromLastLogin >= LOGIN_EXPIRE_SECONDS) state.timeFromLastLogin = -1;
    }

    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);
  }

//  File f;

  // Если доступны байты к чтению по Serial
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    
    // Все сообщения, которые начинаются с '#', являются комментариями и их нужно игнорировать
    if (!command.startsWith("#")) {
      // В ином случае, отправить команду на обработку в логический модуль
      analyzeCommand(command);
    }
  }

  processState();
}
