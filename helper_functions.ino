// Позиция байта по указателю (-1 если не найдено)
int16_t indexOfByte(uint8_t* data, uint16_t len, uint8_t b) {
  int16_t pos = -1;
  for (uint16_t i = 0; i < len; i++)
    if (data[i] == b) {
      pos = i;
      break;
    }

  return pos;
}

// Объединить массивы байт по указателям
uint8_t* concatBytes(uint8_t* base, uint32_t baseLen, uint8_t* buff, uint32_t len, bool deleteOriginals = false) {
  uint8_t* conc = new uint8_t[baseLen + len];
  memcpy(conc, base, baseLen);
  memcpy(conc + baseLen, buff, len);

  if (deleteOriginals) {
    delete[] base;
    delete[] buff;
  }
  return conc;
}

// Объединить массивы байт по указателям (удаляет оригинальные)
uint8_t* concatBytes_delete(uint8_t* base, uint32_t baseLen, uint8_t* buff, uint32_t len) {
  return concatBytes(base, baseLen, buff, len, true);
}
