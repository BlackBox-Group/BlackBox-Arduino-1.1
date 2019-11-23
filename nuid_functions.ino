// Функция сравнения NUID в двух байтовых массивах
bool matchesNUID(byte nuid1[], byte nuid2[]) {
  for (byte i = 0; i < 4; i++)
    if (nuid1[i] != nuid2[i]) return false;

  return true;
}

// Копирование NUID из одного массива в другой
void copyNUID(byte origin[], byte dest[]) {
  for (int i = 0; i < 4; i++)
    dest[i] = origin[i];
}

// Строковое отображение NUID
String nuidToStr(byte nuid[]) {
  String buff;
  for (byte i = 0; i < 4; i++) buff += String(nuid[i], HEX) + ':';

  return buff;
}
