
#define TRANSCEIVER_TX_PIN      14    // Set the pin that emits data towards the 433.42 Mhz Transmitter
#define TRANSCEIVER_MODE_PIN    15    // Aurel transceivers have a pin that let the hardware switch to RX and TX mode
#define TRANSCEIVER_ENABLE_PIN  22    // Aurel transceivers have a pin that must be set to HIGH to enable the transmitter

// Variables
int long_pulse = 825;
byte message[17] = {0x00, 0x00, 0x00, 0x7E, 0x??, 0x??, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFD, 0x00, 0xFF, 0x80};
byte old_state, num_byte;
byte bitstuff = 0;

String chaineCaractere = "";
String mode = "";
String temperature = "";
String modeTexte = "" ;
String tempTexte = "" ;
String invalide = "INVALIDE";

// A l'init
// Setup du chip Aurel
void setup() {
  // set the Aurel transceiver to RX mode
  pinMode(TRANSCEIVER_MODE_PIN, OUTPUT);
  digitalWrite(TRANSCEIVER_MODE_PIN, HIGH);

  // enable Aurel transmitter
  pinMode(TRANSCEIVER_ENABLE_PIN, OUTPUT);
  digitalWrite(TRANSCEIVER_ENABLE_PIN, HIGH);

  delay(500);

  // assign an interrupt on pin x on state change
  pinMode(TRANSCEIVER_TX_PIN, OUTPUT);
  digitalWrite(TRANSCEIVER_TX_PIN, LOW);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  Serial.begin(57600);
}

// En continu
// Surveillance et émission de trame si commande texte
void loop() {

  if (Serial.available() > 0) {

    Serial.println("##### Réception d'une commande #####");

    // Récup de l'input (ligne de commande)
    chaineCaractere = Serial.readStringUntil('\n');;
    delay(80);

    // Récup des deux infos pertinentes de la commande texte
    mode = getValue(chaineCaractere,' ',1);
    temperature = getValue(chaineCaractere,' ',2);

    // Interprétation & validité
    modeTexte=invalide;
    tempTexte=invalide;
    switch (mode.toInt()) {
      case 0:
        modeTexte="Réduit";break;
      case 3:
        modeTexte="Confort";break;
      case 4:
        modeTexte="Hors Gel";break;
    }
    if (temperature.toInt()>=10 and temperature.toInt()<=100) tempTexte="ok";
    Serial.println("    Mode   --> " + mode + " - " + modeTexte );
    Serial.println("    T° eau --> " + temperature + "°C - " + tempTexte);

    // Action en conséquence
    if (tempTexte==invalide or modeTexte==invalide) {
      // Commande invalide => on signale mais on ne fait rien
      Serial.println("##### Commande invalide ignorée ! #####");
    } else {
      // Commande valide => génération et émission de la trame
      Serial.println("Génération des trames...");
      commande(mode.toInt(), temperature.toInt());
      Serial.println("##### Commande traitée #####");
    }
    Serial.println("");
  }
}

// Génération et émission d'une trame
void commande(byte mode, byte temperature) {

  for (byte x = 0; x < 3; x++) { // boucle de 3 messages

    // Génération de la trame
    // **********************

    old_state = 0;

    // Message 9 --> numéro de message 0 à 2
    message[9] = x;

    // Message : 10 -> mode
    if (x == 2) message[10] = mode;
    else message[10] = mode + 0x80;

    // Message : 11 -> temperature
    message[11] = temperature;

    // Messages 13 et 14 -> checksum
    int checksum = 0;
    for (int i = 4; i <= 12; i++) {
      checksum -= message[i];
    }
    message[13] = highByte(checksum);
    message[14] = lowByte(checksum);

    Serial.print("Trame ");
    Serial.print(x);
    Serial.print(" : ");

    for (int i = 1; i <= 16; i++) {
      if (message[i] <= 0x0F) Serial.print("0");
      Serial.print(message[i], HEX);
    }
    Serial.println("");

    // Emission de la trame
    // ********************
    for (int i = 1; i <= 16; i++) { // boucle de 16 bytes
      conversion(message[i]);
    }
    digitalWrite(TRANSCEIVER_TX_PIN, LOW);
    delay(33);
  }
  digitalWrite(TRANSCEIVER_TX_PIN, LOW);
}

// "Emission"
// Pas complètement boîte aux lettres car il y a un bitstuffing à faire
void conversion(byte input) {
  for (byte n = 0; n < 8; n++) { // boucle pour chaque bit
    // Emission proprement dite du bit
    writeBit(bitRead(input, n));
    // Bitstuffing
    if (num_byte >= 4 && num_byte <= 14) {
      if (bitRead(input, n) == 1) bitstuff++; // incrémente le compteur bitstuffing
    }
    if (bitRead(input, n) == 0) bitstuff = 0;
    if (bitstuff >= 5) {
      writeBit(0);
      bitstuff = 0;
    }
  }
}

// Emission bit à bit bas niveau
void writeBit(bool Bit) {
  old_state = !old_state;
  digitalWrite(TRANSCEIVER_TX_PIN, old_state);
  delayMicroseconds(long_pulse);
  if (Bit) {
    old_state = !old_state;
    digitalWrite(TRANSCEIVER_TX_PIN, old_state);}

  else {
    digitalWrite(TRANSCEIVER_TX_PIN, old_state);
  }
  delayMicroseconds(long_pulse);
}

// Récup du n-ème mot dans un string (n=index)
String getValue(String data, char separator, int index) {
  int found = 0;
  int strIndex[] = {
    0,
    -1
  };
  int maxIndex = data.length() - 1;
  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}
