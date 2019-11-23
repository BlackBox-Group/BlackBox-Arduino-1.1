// Одна мгновенная попытка (итерация) чтения карты
bool readRFID() {
  digitalWrite(LED_BUILTIN, HIGH);
  
  // Ожидание, пока новая карта появится на сенсоре
  if (rfid.isCard()) {
    // Убеждаемся, что NUID было успешно прочитано
    if (rfid.readCardSerial()) {
      digitalWrite(LED_BUILTIN, LOW);
      return true;
    }
  }

  return false;
}
