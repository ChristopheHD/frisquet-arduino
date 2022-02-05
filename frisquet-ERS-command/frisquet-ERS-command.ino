#include <SerialTerminal.hpp>

#define TRANSCEIVER_TX_PIN      14
#define TRANSCEIVER_MODE_PIN    15    // Aurel transceivers have a pin that let the hardware switch to RX and TX mode
#define TRANSCEIVER_ENABLE_PIN  22    // Aurel transceivers have a pin that must be set to HIGH to enable the transmitter

// DO NOT CHANGE THESE SETTINGS BELOW
#define TRANSCEIVER_TX HIGH
#define TRANSCEIVER_RX  LOW

// Variables
byte ERS_pin = TRANSCEIVER_TX_PIN;
int long_pulse = 825;
byte message[17] = {0x00, 0x00, 0x00, 0x7E, 0x6A, 0xF4, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFD, 0x00, 0xFF, 0x80};
byte old_state, num_byte;
byte bitstuff = 0;
maschinendeck::SerialTerminal* term;

void writeBit(bool Bit) {
  old_state = !old_state;
  digitalWrite(ERS_pin, old_state);
  delayMicroseconds(long_pulse);

  if (Bit) {
    old_state = !old_state;
    digitalWrite(ERS_pin, old_state);
  }
  delayMicroseconds(long_pulse);
}

void conversion(byte input) {
  for (byte n = 0; n < 8; n++) { // boucle pour chaque bit
    writeBit(bitRead(input, n));
    if (num_byte >= 4 && num_byte <= 14) {
      if (bitRead(input, n) == 1)
        bitstuff++;  // incrémente le compteur bitstuffing
    }
    if (bitRead(input, n) == 0)
      bitstuff = 0;
    if (bitstuff >= 5) {
      writeBit(0);
      bitstuff = 0;
    }
  }
}

void commande(byte prechauffage, byte chauffage) {
  if ((chauffage <= 100) and ((prechauffage==0) or (prechauffage==3) or (prechauffage==4))) {
    for (byte x = 0; x < 3; x++) { // boucle de 3 messages
      old_state = 0;
      message[9] = x;  // numero message : 0 à 2
      if (x == 2) {
        message[10] = prechauffage;
      } else {
        message[10] = prechauffage + 0x80;
      }
      message[11] = chauffage;
      
      int checksum = 0;
      for (int i = 4; i <= 12; i++) {
        checksum -= message[i];
      }
      message[13] = highByte(checksum);
      message[14] = lowByte(checksum);

      Serial.println("Trame envoyée : ");
      for (byte i = 1; i <= 16; i++) {
        if (message[i] <= 0x0F)
          Serial.print("0");
        Serial.print(message[i], HEX);
      }
      Serial.println("");
      for (num_byte = 1; num_byte < 17; num_byte++) { // boucle de 16 bytes
        conversion(message[num_byte]);
      }
      digitalWrite(ERS_pin, LOW);
      delay(33);
    }
    digitalWrite(ERS_pin, LOW);
  } else {
    return;
  }
}

void ERS_command(String opts) {
  digitalWrite(LED_BUILTIN, HIGH);
  maschinendeck::Pair<String, String> operands = maschinendeck::SerialTerminal::ParseCommand(opts);
  int mode = operands.first().toInt();
  Serial.print("Mode : ");
  switch (mode) {
    case 0:
      Serial.println("Réduit");
      break;
    case 3:
      Serial.println("Confort");
      break;
    case 4:
      Serial.println("Hors gel");
      break;
    default:
      Serial.print("Inconnu (");
      Serial.print(mode);
      Serial.println(")");
  }
  
  Serial.print("Température d'eau ");
  Serial.print(operands.second());
  Serial.println(" degré(s)");
  
  commande(mode,operands.second().toInt());   // reduit 0, confort 3, hors gel 4, chauffage 0 à 100
  Serial.print("Commande envoyée");
  digitalWrite(LED_BUILTIN, LOW);
}

void setup() {
  // set the Aurel transceiver to RX mode
  pinMode(TRANSCEIVER_MODE_PIN, OUTPUT);
  digitalWrite(TRANSCEIVER_MODE_PIN, TRANSCEIVER_TX);

  // enable Aurel transmitter
  pinMode(TRANSCEIVER_ENABLE_PIN, OUTPUT);
  digitalWrite(TRANSCEIVER_ENABLE_PIN, HIGH);

  pinMode(TRANSCEIVER_TX_PIN, OUTPUT);
  digitalWrite(TRANSCEIVER_TX_PIN, LOW);

  pinMode (LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
 
  term = new maschinendeck::SerialTerminal(57600);
  term->add("ERS", &ERS_command, "ERS mode heat\nmode : 0=Reduit, 3=Confort, 4=Hors gel\nheat : Température de chauffage entre 0 et 100");
}

void loop() {
 term->loop();
}
