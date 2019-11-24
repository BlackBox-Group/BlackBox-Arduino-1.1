// Фукция, переводящее состояние Arduino в "ожидающий RFID"
void initPutRFID() {
  Serial.println("putRFID");
  state.rfidRequired = true;
  state.timeStamp_rfid = millis();
}

// Запросить мастер-пароль
void askForMaster() {
  Serial.println("masterpass?");
  state.masterRequired = true;
}

void analyzeCommand(String command) {
  
  // Пинг
  if (command == "ping_blackbox") {
    Serial.println("pong_blackbox");
  }
  
  // Команда по сбросу текущих сохранённых данных
  else if (command == "resetcards") {
    SD.remove("cards.txt");
    File dir = SD.open("usr");
    rm(dir, "usr/");
    dir.close();
    SD.rmdir("usr");

    // Повторное создание данных
    SD.open("cards.txt", FILE_WRITE).close();
    SD.mkdir("usr");
  }

  // Создание пользователя
  else if (command == "usercreate") {
    initPutRFID();
    state.userCreation = true;
  }

  else if (command.startsWith("masterpass") && state.masterRequired) {
    state.master = command.substring(command.indexOf(' ') + 1);

    if (state.master.length() < 10 || state.master.length() > 28) {
      Serial.println("masterincorrect");
    }
    else {
      state.gotMaster = true;
      state.masterRequired = false;
    }
  }

  // Команда не опознана / не принимается системой в текущем состоянии
  else {
    Serial.println("# Command unknown / can't be accepted now");
    return;
  }

  Serial.println("# Command OK.");
}

// Обрабатывание текущего состояние и переход к следующему
void processState() {
  if (state.rfidRequired) {
    digitalWrite(LED_BUILTIN, HIGH);

    if (readRFID()) {
      state.rfidRequired = false;
      state.rfidSuccess = true; 
    }
    else if (millis() - state.timeStamp_rfid > RFID_TIMEOUT_SECONDS * 1000) {
      state.rfidRequired = false;
      state.rfidTimeout = true;

      Serial.println("timeout");
    }
  }
  else {
    digitalWrite(LED_BUILTIN, LOW);
  }

  // Создание пользователя
  if (state.userCreation) {
    if (state.rfidTimeout) {
      state.userCreation = false;
    }
    else if (state.rfidSuccess) {
      // RFID получен, нужно проверить, если такая карта есть и запросить master-пароль
      File f = openFile("cards.txt", FILE_READ);
      String currNuid = nuidToStr(rfid.serNum), savedNuid;
      
      do {
        savedNuid = fileReadUntil(&f, '\n');
      }
      while (savedNuid != "" && currNuid != savedNuid);

      f.close();
      
      if (currNuid == savedNuid) {
        Serial.println("cardexists");
        state.userCreation = false;
      }
      else {
        askForMaster();
      }
    }
    else if (state.gotMaster) {
      createUser(rfid.serNum, &state.master);

      state.userCreation = false;
    }
  }

  // Обработка одно-цикленных переменных (эти флаги должны быть истинными не более одного цикла)
  state.rfidTimeout = false;
  state.rfidSuccess = false;
  state.gotMaster = false;
}

void createUser(uint8_t* nuid, String* m) {
  state.key = generateKey(nuid, m);
  Serial.print("# "); dumpBufferHex(state.key, 32); Serial.println();

  File f = openFile("cards.txt", FILE_WRITE);
  f.print(nuidToStr(nuid) + "\n");
  f.close();
  Serial.println("usercreated");

  delete state.key;
}
