#include <SoftwareSerial.h>

#define BlueTooth mySerial
SoftwareSerial mySerial(A5, A4); // RX, TX


boolean bleDataMode() {
  char buf[32];
  int c = 0;
  bool connected = false;
  long timeout = millis() + 20000;

  Serial.println("Init BLE data mode...");
  mySerial.print("AT+INQ\r\n");
  while (!connected) {
    if (millis() >= timeout) {
      Serial.println("Init BLE timeout!");
      break;
    }
    if (!mySerial.available()) continue;
    int ch = mySerial.read();
    if (ch == 0x0d) continue;
    if (ch == 0x0a) { // NL char, recevied end of string
      buf[c] = 0; c = 0;
      Serial.println(buf);
      if (strcmp(buf, "+INQE") == 0) {
        mySerial.print("AT+CONN1\r\n");
      } else if (strcmp(buf, "+Connected") == 0) {
        connected = true;
      }
    } else
      buf[c++] = ch;
  }
  return connected;
}
unsigned int BeepFreq = 100;   // frequency of next beep command, 0 means no beep, should be range 50 to 2000 otherwise
unsigned int BeepDur = 100;

int sendbeep(int noheader) {

  //  if (BeepFreq != 0) {
  //    Serial.print("#BTBEEP="); Serial.print("B+"); Serial.print(BeepFreq); Serial.print("+"); Serial.println(BeepDur);
  //  }

  unsigned int beepfreqhigh = highByte(BeepFreq);
  unsigned int beepfreqlow = lowByte(BeepFreq);
  if (!noheader) {
    BlueTooth.print("B");
  }
  BlueTooth.write(beepfreqhigh);
  BlueTooth.write(beepfreqlow);

  unsigned int beepdurhigh = highByte(BeepDur);
  unsigned int beepdurlow = lowByte(BeepDur);
  BlueTooth.write(beepdurhigh);
  BlueTooth.write(beepdurlow);

  // return checksum info
  if (noheader) {
    return beepfreqhigh + beepfreqlow + beepdurhigh + beepdurlow;
  } else {
    return 'B' + beepfreqhigh + beepfreqlow + beepdurhigh + beepdurlow;
  }

}

long period;
const int SW_pin = 2; // digital pin connected to switch output
const int X_pin = A0; // analog pin connected to X output
const int Y_pin = A1; // analog pin connected to Y output

void setup() {
  Serial.begin(38400);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.println("started!");
  //
  pinMode(SW_pin, INPUT);
  digitalWrite(SW_pin, HIGH);
  //
  mySerial.begin(38400);
  bleDataMode();
  period = millis() + 500;
}

void loop() {
  char CurCmd = 'W';
  char CurSubCmd, CurSubCmds[] = {'1', '2', '3', '4'};
  char CurDpad;
  int button = 0, xx, yy;
  static int c = 3;

  button = digitalRead(SW_pin);
  if (!button) {
    c = (c + 1) % 4;
  }
  CurSubCmd = CurSubCmds[c];
  CurDpad = 's';
  xx = analogRead(X_pin);
  if (xx == 0) {
    CurDpad = 'l';
  } else if (xx == 1023) {
    CurDpad = 'r';
  }
  yy = analogRead(Y_pin);
  if (yy == 0) {
    CurDpad = 'f';
  } else if (yy == 1023) {
    CurDpad = 'b';
  }

  if (millis() >= period) {
    BlueTooth.print("V1"); // Vorpal hexapod radio protocol header version 1
    int three = 3;
    BlueTooth.write(three);
    BlueTooth.write(CurCmd);
    BlueTooth.write(CurSubCmd);
    BlueTooth.write(CurDpad);
    //unsigned int checksum = sendbeep(0);
     unsigned int checksum=0;
    checksum += three + CurCmd + CurSubCmd + CurDpad;
    checksum = (checksum % 256);
    BlueTooth.write(checksum);
    period = millis() + 500;
    Serial.print("#NOPL:"); Serial.print(CurCmd); Serial.print(CurSubCmd); Serial.println(CurDpad);
  }
}


void loop1() {
  char CurCmd = 'W';
  char CurSubCmd = '1';
  char CurDpad = 'f';

  if (millis() >= period) {
    BlueTooth.print("V1"); // Vorpal hexapod radio protocol header version 1
    int eight = 8;
    BlueTooth.write(eight);
    BlueTooth.write(CurCmd);
    BlueTooth.write(CurSubCmd);
    BlueTooth.write(CurDpad);
    unsigned int checksum = sendbeep(0);
    checksum += eight + CurCmd + CurSubCmd + CurDpad;
    checksum = (checksum % 256);
    BlueTooth.write(checksum);
    period = millis() + 500;
    Serial.print("#NOPL:"); Serial.print(CurCmd); Serial.print(CurSubCmd); Serial.println(CurDpad);
  }
}


