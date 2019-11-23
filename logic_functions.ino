void analyzeCommand(String command) {
  if (command == "ping_blackbox") {
    Serial.println("pong_blackbox");
  }
  else {
    Serial.println("# Command unknown / can't be accepted now");
    return;
  }

  Serial.println("# Command OK.");
}
