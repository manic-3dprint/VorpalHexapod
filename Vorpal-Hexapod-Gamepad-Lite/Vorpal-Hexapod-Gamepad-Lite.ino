#include <SoftwareSerial.h>


#define __DEBUG__
#define __CC2540_BLE__

#define CONSOLE_BAUD 38400
#define BLUETOOTH_BAUD 38400

#define Console Serial
#ifdef __DEBUG__
SoftwareSerial BlueTooth(A5, A4);
#else
#define BlueTooth Serial
#endif

//left hand side joystick
const int SW1_pin = 5;
const int X1_pin = A2;
const int Y1_pin = A3;

// right hand side joystick
const int SW2_pin = 7;
const int X2_pin = A0;
const int Y2_pin = A1;


void setup() {
  Console.begin(CONSOLE_BAUD);
  while (!Console) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Console.println("started!");
  //
  pinMode(SW1_pin, INPUT);
  digitalWrite(SW1_pin, HIGH);
  pinMode(SW2_pin, INPUT);
  digitalWrite(SW2_pin, HIGH);
  //
#ifdef __DEBUG__
  BlueTooth.begin(BLUETOOTH_BAUD);
#endif

#ifdef __CC2540_BLE__
  bleDataMode();
#endif

}

void loop() {
  static long cmdPeriod = 0;
  static long buttonPeriod = 0;
  static char CurCmd = 'W';
  char CurSubCmd, CurSubCmds[] = {'1', '2', '3', '4'};
  char CurDpad;
  int xx1, yy1, xx2, yy2;
  static int subCmdIndex = 3, cmdIndex = 0;
  boolean sw1ButtonState = false, sw2ButtonState = false;
  static boolean sw2PrevButtonState = false;


  sw2ButtonState = digitalRead(SW2_pin);
  if (sw2ButtonState != sw2PrevButtonState) {
    if (!sw2ButtonState) {
      subCmdIndex = (subCmdIndex + 1) % 4;
    }
    sw2PrevButtonState = sw2ButtonState;
  }
  CurSubCmd = CurSubCmds[subCmdIndex];
  //
  xx2 = analogRead(X2_pin);
  if (xx2 == 0) {
    CurCmd = 'F';
  } else if (xx2 == 1023) {
    CurCmd = 'D';
  }
  yy2 = analogRead(Y2_pin);
  if (yy2 == 0 || yy2 == 1023) {
    CurCmd = 'W';
  }
  CurDpad = 's';
  //
  xx1 = analogRead(X1_pin);
  if (xx1 < 300) {
    CurDpad = 'b';
  } else if (xx1 > 900) {
    CurDpad = 'f';
  }
  yy1 = analogRead(Y1_pin);
  if (yy1 < 300) {
    CurDpad = 'l';
  } else if (yy1 > 900) {
    CurDpad = 'r';
  }
  sw1ButtonState = digitalRead(SW1_pin);
  if (!sw1ButtonState) {
    CurDpad = 'w';
  }
  if (millis() >= cmdPeriod) {
    BlueTooth.print("V1"); // Vorpal hexapod radio protocol header version 1
    int three = 3;
    BlueTooth.write(three);
    BlueTooth.write(CurCmd);
    BlueTooth.write(CurSubCmd);
    BlueTooth.write(CurDpad);

    unsigned int checksum = 0;
    checksum += three + CurCmd + CurSubCmd + CurDpad;
    checksum = (checksum % 256);
    BlueTooth.write(checksum);
    cmdPeriod = millis() + 200;
#ifdef __DEBUG__
    Console.print("#NOPL:"); Console.print(CurCmd); Console.print(CurSubCmd); Console.println(CurDpad);
#endif
  }
}

#ifdef __CC2540_BLE__
boolean bleDataMode() {
  char buf[32];
  int c = 0;
  bool connected = false;
  long timeout = millis() + 20000;

#ifdef __DEBUG__
  Console.println("Init BLE data mode...");
#endif
  BlueTooth.print("AT+INQ\r\n");
  while (!connected) {
    if (millis() >= timeout) {
#ifdef __DEBUG__      
      Console.println("Init BLE timeout!");
#endif      
      break;
    }
    if (!BlueTooth.available()) continue;
    int ch = BlueTooth.read();
    if (ch == 0x0d) continue;
    if (ch == 0x0a) { // NL char, recevied end of string
      buf[c] = 0; c = 0;
#ifdef __DEBUG__      
      Console.println(buf);
#endif      
      if (strcmp(buf, "+INQE") == 0) {
        BlueTooth.print("AT+CONN1\r\n");
      } else if (strcmp(buf, "+Connected") == 0) {
        connected = true;
      }
    } else
      buf[c++] = ch;
  }
#ifdef __DEBUG__
  if (connected)
    Console.println("Init BLE data mode,done.");
#endif
  return connected;
}
#endif
