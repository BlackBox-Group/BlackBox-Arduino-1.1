// Вывод "сырого" буфера в консоль
void dumpBuffer(uint8_t buf[], uint32_t len) { 
  Serial.print("'");
  Serial.write(buf, len);
  Serial.print("'");
}

// Вывод буфера с конвертацией в 16-ричную систему
void dumpBufferHex(uint8_t buf[], uint32_t len) {
  Serial.print("'");
  for (uint32_t i = 0; i < len; i++) {
    Serial.print(String(buf[i], HEX) + " ");
  }
  Serial.print("'");
}
