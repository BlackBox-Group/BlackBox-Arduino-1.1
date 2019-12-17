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

// Запросить мастер-пароль
void askForUsername() {
  Serial.println("username?");
  state.usernameRequired = true;
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

  else if (command.startsWith("username") && state.usernameRequired) {
    state.username = command.substring(command.indexOf(' ') + 1);

    // TODO: Проверка
    state.gotUsername = true;
    state.usernameRequired = false;
  }

  else if (command == "addService") {
    Serial.println("# Okay1");
    initPutRFID();
    state.serviceAddition = true;
    /*AuthState as = authorizeUser();
    switch (as) {
      case IN_PROGRESS:
        break;
      case FAILED:
        Serial.println("servicefail");
        break;
      case SUCCESS:
        Serial.println("service?");
        state.serviceAddition = true;
    }*/
  }

  else if (command.startsWith("service") && state.serviceAuthed) {
    state.serviceName = command.substring(command.indexOf(' ') + 1, command.lastIndexOf(' '));
    state.servicePass = command.substring(command.lastIndexOf(' ') + 1);

    Serial.println("# Writing to " + state.userFileName);

    File f = openFile("usr/" + state.userFileName, FILE_WRITE);
    f.print(state.serviceName + "\n");
    f.print(state.servicePass + "\n");
    f.close();
    
    state.serviceAuthed = false;
  }

  else if (command == "userlogin") {
    initPutRFID();
    state.userLogin = true;
  }

  else if (command.startsWith("password")) {
    initPutRFID();
    state.passwordReveal = true;
    state.revealPassName = command.substring(command.indexOf(' ') + 1);
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
        askForUsername();
//        askForMaster();
      }
    }
    else if (state.gotUsername) {
      Serial.println("# " + state.username);
      askForMaster();
    }
    else if (state.gotMaster) {
      createUser(rfid.serNum, &state.master);

      state.userCreation = false;
    }
  }

  // Общая авторизация, используя NUID + Master
  if (state.authState == IN_PROGRESS) {
//    if (millis() % 1000 == 0) Serial.println("# Okay3");
    if (state.rfidTimeout) {
      Serial.println("# Okay4");
      state.authState = FAILED;
    }
    else if (state.rfidSuccess) {
      Serial.println("# Okay5");
      // RFID получен, нужно проверить, если такая карта есть и запросить master-пароль

      File f = openFile("cards.txt", FILE_READ);
      String currNuid = nuidToStr(rfid.serNum), savedNuid;
      
      do {
        savedNuid = fileReadUntil(&f, '\n');
      }
      while (savedNuid != "" && currNuid != savedNuid);
      f.close();

      if (currNuid == savedNuid) {
        if (state.timeFromLastLogin != -1 && state.lastNuidLogged == currNuid) {
          Serial.println("# User has logged in before");
          state.timeFromLastLogin = 0;
          state.authState = SUCCESS;
        }
        
        else askForMaster();
      }
      else {
        Serial.println("nosuchcard");
        state.authState = FAILED;
      }
    }
    else if (state.gotMaster) {
      // Данные готовы для обработки
      state.authState = DATA_COLLECTED;
    }
  }

  if (state.serviceAddition) {
    AuthState as = authorizeUser();
    switch (as) {
      case IN_PROGRESS:
//        Serial.println("# Okay2");
        break;
      case FAILED:
        Serial.println("servicefail");
        state.serviceAddition = false;
        state.authState = STOPPED;
        break;
      case SUCCESS:
        Serial.println("service?");
        state.serviceAuthed = true;
        state.authState = STOPPED;
        state.serviceAddition = false;
    }
  }

  if (state.userLogin) {
    AuthState as = authorizeUser();
    switch (as) {
      case IN_PROGRESS:
        break;
      case FAILED:
        /*Serial.println("masterincorrect");
        state.authState = STOPPED;
        askForMaster();*/
        Serial.println("loginfail");
        state.authState = STOPPED;
        state.userLogin = false;
        break;
      case SUCCESS:
        File f = openFile("usr/" + state.userFileName, FILE_READ);
        String line;
        // "Перемотка" на нужный участок
        fileReadUntil(&f, '\n');
        fileReadUntil(&f, '\n');
        Serial.println("username " + fileReadUntil(&f, '\n'));
        
        do {
          line = fileReadUntil(&f, '\n');
        }
        while (line != "hservice");

        while (f.available()) {
          String sname = fileReadUntil(&f, '\n');
          Serial.println("service " + sname);
          // Пропускаем пароли, они пока не нужны
          fileReadUntil(&f, '\n');
        }
        f.close();
        state.authState = STOPPED;
        state.userLogin = false;
    }
  }

  if (state.passwordReveal) {
    AuthState as = authorizeUser();
    switch (as) {
      case IN_PROGRESS:
        break;
      case FAILED:
        Serial.println("passwordfail");
        state.passwordReveal = false;
        state.authState = STOPPED;
        break;
      case SUCCESS:
        File f = openFile("usr/" + state.userFileName, FILE_READ);
        String line;
        // "Перемотка" на нужный участок
        do {
          line = fileReadUntil(&f, '\n');
        }
        while (line != "hservice");

        while (f.available()) {
          String sname = fileReadUntil(&f, ':');
          String surl = fileReadUntil(&f, '\n');
          String password = fileReadUntil(&f, '\n');

          Serial.println("# " + sname + " " + state.revealPassName);
          if (sname == state.revealPassName) {
            f.close();

            Serial.println("password " + password);
                  
            state.authState = STOPPED;
            state.passwordReveal = false;
            break;
          }
        }
        if (state.authState != STOPPED) {
          f.close();
          state.authState = STOPPED;
          state.passwordReveal = false;
  
          Serial.println("passwordfail");
          break;
        }
    }
  }

  // Обработка одно-цикленных переменных (эти флаги должны быть истинными не более одного цикла)
  state.rfidTimeout = false;
  state.rfidSuccess = false;
  state.gotMaster = false;
  state.gotUsername = false;
}

// Общая авторизация пользователя через NUID и мастер пароль. Может возвращать IN_PROGRESS, SUCCESS и FAILED.
int8_t authorizeUser() {
  // Запускаем процесс, если он был остановлен
  if (state.authState == STOPPED) state.authState = IN_PROGRESS;
  else if (state.authState == DATA_COLLECTED) {
    // Нужно обработать пришедшие данные
    Serial.println("# " + nuidToStr(rfid.serNum));
    Serial.println("# " + state.master);

    String usrFile = findUser(nuidToStr(rfid.serNum), state.master);
    if (usrFile == "") {
      state.authState = FAILED;
    } else {
      state.authState = SUCCESS;
      state.userFileName = usrFile;
      state.lastNuidLogged = nuidToStr(rfid.serNum);
      state.timeFromLastLogin = 0;
    }
  }

  return state.authState;
}

// Создание пустого пользователя по мастеру и nuid
void createUser(uint8_t* nuid, String* m) {
  //state.key = generateKey(nuid, m);
  /*Serial.print("# "); dumpBufferHex(state.key, 32); Serial.println();*/

  File f = openFile("cards.txt", FILE_WRITE);
  f.print(nuidToStr(nuid) + "\n");
  f.close();

  Serial.println("# " + state.master);
  Serial.println("# " + state.username);
  Serial.println("# " + nuidToStr(nuid));

  String fileName = getAvailableUser();
  Serial.println("# writing to " + fileName);
  f = openFile("usr/" + fileName, FILE_WRITE);
  f.print(/*"hfile\n"*/nuidToStr(nuid) + "\n" + state.master + "\n" + state.username + "\nhservice\n");
  f.close();
  
  Serial.println("usercreated");

  //delete state.key;
}
