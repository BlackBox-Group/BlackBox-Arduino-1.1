// Основная структура, хранящая в себе параметры для CBC AES
struct AES_ctx ctx;

// Соль, ранее используемое для паддинга "рандомными" символами в конце ключа
// Сейчас сам массив является iv (начальным вектором для шифрования)
const char salt[16] = "985e166be2e9d300";

// Генерация приватного ключа по RFID ID и мастер-паролю
const uint8_t* generateKey(byte* nuid, String* master) {
  // Базовый размер - 4 байта от nuid и 6-28 байт мастер-пароль
  // Остальное заполняется через PKCS#7 (см. aes.h и aes.c) 
  byte mLen = master->length();
  uint8_t finalSize = 4 + mLen;
  uint8_t* key = new uint8_t[finalSize];
  
  for (int i = 0; i < 4; i++)
    key[i] = nuid[i];

  for (int i = 0; i < mLen; i++)
    key[4 + i] = master->operator[](i);

  if (finalSize == 32) {
    return key;
  }
  else {
    // Заполняем пустое место в конце и удаляем старый указатель
    uint8_t* paddedKey = PKCS7_pad(32, key, finalSize, NULL); 
    delete[] key;
  
    return paddedKey;
  }
}

// Реинициализация AES по ключу
void initAES(uint8_t key[], bool keepKey = true) {
   // Используем соль как IV
   AES_init_ctx_iv(&ctx, key, salt);
   
   if (!keepKey) delete[] key;
}

// То же самое, только с автоматическим освобождением указателя ключа
void initAES_deleteKey(uint8_t key[]) {
  initAES(key, false);
}

// Структура для хранения расшифрованных данных
struct AES_Buffer {
  uint8_t* data;
  uint32_t length;
};

void fileReadBlock(File* f, uint8_t* blockBuffer, uint32_t startPos = 0);

// Расшифровывать файл до того, пока не будет прочитан определённый байт
// Нужно убедиться, что ctx инициализирован корректно, или (скорее всего) будет прочитан весь файл
struct AES_Buffer decryptUntil(File* f, uint8_t terminator) {
  uint8_t* blockBuffer = new uint8_t[16];
  struct AES_Buffer buffer = {NULL, 0};
    
  while (f->available()) {
    fileReadBlock(f, blockBuffer);
    AES_CBC_decrypt_buffer(&ctx, blockBuffer, 16);
    Serial.print("# "); dumpBuffer(blockBuffer, 16); Serial.println();
    
    concatBytes_delete(buffer.data, buffer.length, blockBuffer, 16);
    
    if (indexOfByte(blockBuffer, 16, terminator) != -1) {
      break;
    }
  }

  delete[] blockBuffer;
  
  if (buffer.data != NULL) {
    uint8_t* old = buffer.data;
    buffer.data = PKCS7_unpad(buffer.data, buffer.length, &buffer.length);

    delete[] old;
  }

  return buffer;
}

// Расшифровать данные произвольной длины в формате AES_Buffer
// Нужно убедиться, что ctx инициализирован корректно
void decryptBuffer(AES_Buffer* buffer) {
  uint8_t* old = buffer->data;

  buffer->data = PKCS7_unpad(buffer->data, buffer->length, &buffer->length);
  for (uint32_t i = 0; i < buffer->length; i += 16)
    AES_CBC_decrypt_buffer(&ctx, buffer->data + i, 16);

  delete[] old;
}

// Зашифровать данные произвольной длины в формате AES_Buffer
// Нужно убедиться, что ctx инициализирован корректно
void encryptBuffer(AES_Buffer* buffer) {
  uint8_t* old = buffer->data;
  
  buffer->data = PKCS7_pad(16, buffer->data, buffer->length, &buffer->length);
  for (uint32_t i = 0; i < buffer->length; i += 16)
    AES_CBC_encrypt_buffer(&ctx, buffer->data + i, 16);
  
  delete[] old;
}
