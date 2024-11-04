#define TRANSCEIVER_RX_PIN      19    // Set the pin that receives data from your 433.42 Mhz Receiver
#define TRANSCEIVER_MODE_PIN    15    // Aurel transceivers have a pin that let the hardware switch to RX and TX mode
#define TRANSCEIVER_ENABLE_PIN  22    // Aurel transceivers have a pin that must be set to HIGH to enable the transmitter
#define TRANSCEIVER_TX HIGH
#define TRANSCEIVER_RX  LOW

volatile long duration = 0;
volatile long prev_time = 0;
volatile boolean data_dispo = false;
byte input = TRANSCEIVER_RX_PIN;
String trame[3];
byte message = 0;
byte start = true;
byte intro = 0;
String prefixe = "1100";
 
void setup() {
  Serial.begin(57600);
  
  // set the Aurel transceiver to RX mode
  pinMode(TRANSCEIVER_MODE_PIN, OUTPUT);
  digitalWrite(TRANSCEIVER_MODE_PIN, TRANSCEIVER_RX);

  // enable Aurel transmitter
  pinMode(TRANSCEIVER_ENABLE_PIN, OUTPUT);
  digitalWrite(TRANSCEIVER_ENABLE_PIN, HIGH);

  delay(500);

  // assign an interrupt on pin x on state change
  pinMode(TRANSCEIVER_RX_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(input), declenche, CHANGE);
  
  pinMode (LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
}
 
void loop() {
  if (data_dispo == true) {
    int bit_state = digitalRead(input);
    if ((duration > 700) && (duration < 950)) { // pulse off court
      trame[message] = trame[message] + !bit_state;
    } else if ((duration > 1400) && (duration < 1950)) { // pulse off long
      // verifie qu'on commence avec un pulse long à 1
      if ((start == true) && (!bit_state == HIGH)) {
        trame[message] = trame[message] + !bit_state + !bit_state;
        start = false;
      } else if (start == false) {
        trame[message] = trame[message] + !bit_state + !bit_state;
      }
    } else {
      start = true; // debut d'une nouvelle trame
      if (trame[message].length() > 200) {
        message++;
        if (message > 2) {
          for (message = 0; message < 3; message++) {
            for (int x = 0; x <= 50; x=x+4) {
              if (trame[message].substring(x,x+4).compareTo(prefixe) == 0) {
                intro++;
              } else {
                break;
              }
            }
            if (intro > 6 ) {
              digitalWrite(LED_BUILTIN, HIGH);
              // décodage des bits de la trame: 00=0, 11=0, 10=1, 01=1
              String sortie = "";
              for (int x = (intro*4); x <= trame[message].length(); x=x+2) {
                if ((trame[message].substring(x,x+2) == "00") or (trame[message].substring(x,x+2) == "11")) {
                  sortie = sortie + "0";
                } else {
                  sortie = sortie + "1";
                }
              }
              String sortie_without_bitstuff = remove_bitstuff(sortie);
              sortie = sortie_without_bitstuff;
              String sortie_hex = convert_binary_string_to_hex_string(sortie);
              Serial.print("Trame reçue :");
              Serial.println(sortie_hex);

              // repartition en bytes
              byte id1 = convert(sortie.substring(0,8)); // byte 3
              byte id2 = convert(sortie.substring(8,16)); // byte 4
              byte id3 = convert(sortie.substring(16,24)); // byte 5
              byte num = convert(sortie.substring(48,56)); // byte 9 (repeat)
              byte prechauff = convert(sortie.substring(56,64)); // byte 10 (pre-heated) 0=Reduit, 3=Confort, 4=Hors gel
              byte chauffage = convert(sortie.substring(64,72)); // Heating
              byte battery = convert(sortie.substring(72,80)); // Battery
              Serial.print("Message #");
              Serial.println(num);
              Serial.print("ID: ");
              Serial.print(id1, HEX);
              Serial.print("-");
              Serial.print(id2, HEX);
              Serial.print("-");
              Serial.println(id3, HEX);
              Serial.print("Mode : ");
              prechauff &= 0x0F;
              switch (prechauff) {
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
                  Serial.print(prechauff);
                  Serial.println(")");
              } 
              Serial.print("Température eau : ");
              Serial.println(chauffage);
              Serial.print("Batterie : ");
              if (battery > 0)
                Serial.println("LOW");
              else
                Serial.println("OK");
              if (num > 1)
                Serial.println("##########");
              else
                Serial.println("----------");
              intro = 0;
              digitalWrite(LED_BUILTIN, LOW);
            }
          }  
          trame[0] = "";
          trame[1] = "";
          trame[2] = "";
          message = 0;
        }
      } else {
        // Re-initialise la trame s'il n'y avait pas de données juste avant      
        if ((message == 0) || (trame[message-1].length() < 200)) { 
          trame[0] = "";
          trame[1] = "";
          trame[2] = "";
          message = 0;
        }
      }
    }
    data_dispo = false;
  }
}

void declenche() {
  duration = micros()-prev_time;
  prev_time = micros();
  data_dispo = true;
}

byte convert(String entree) {
  byte result = 0;
  for (int f = 0; f <= 7 ; f++) {
    if (entree.substring(f,f+1) == "1") bitSet(result, f);
  }
return result;
}

String convert_binary_string_to_hex_string(String data) {
  String sortie = "";
  for (int x = 0; x < (data.length()/8);x++) {
    byte hex = 0;
    for (int f = 0; f <= 7 ; f++) {
      if (data.substring((x*8)+f,(x*8)+f+1) == "1")
        bitSet(hex, f);
    }
    if (hex <= 0x0F)
      sortie += "0";
    sortie += String(hex, HEX);
  }
  sortie.toUpperCase();
  return sortie;
}

String remove_bitstuff(String data) {
  // The protocol uses bit-stuffing => remove 0 bit after five consecutive 1 bits
  // Also, each byte is represented with least significant bit first -> swap them!
  String sortie = "";
  int ones = 0;
  for (uint16_t k = 0; k < data.length(); k++) {
    if (data[k] == '1') {
      sortie += "1";
      ones++;
    } else {
      if (ones != 5) // Ignore a 0 bit after five consecutive 1 bits:
        sortie += "0";
      ones = 0;
    }
  }
  return sortie;
}
