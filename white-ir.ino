#include <IRremote.h>
#include <IRremoteInt.h>
#include <Wire.h>

#define SLAVE_ADDRESS     0x04
#define MAX_MESSAGE_SIZE  512

unsigned char buffer[(MAX_MESSAGE_SIZE)];
unsigned int ircodeLength = 0;
unsigned int  pos = 0;
unsigned int  len = 0;
unsigned int  cnt = 0;
int  retries = -1;
int send = 0;
byte state = 0;
unsigned long lastMessageTimestamp;


IRsend irsend;

void setup() {
  // initialize i2c as slave
  Wire.begin(SLAVE_ADDRESS);
  Wire.onReceive(receiveEvent); // register event
  Wire.onRequest(sendResponse); // register event
}

void loop() {
  if (send) {
      for (int i = 0; i <= retries; i++) {
        irsend.sendRaw((unsigned int*) buffer, ircodeLength, 38);
        delay(50);
      }
      retries = -1;
      send = 0;
      state = 0;
      len = 0;
      pos = 0;
      cnt = 0;
  }

  unsigned long now = millis();

  if (cnt > 0 && (now - lastMessageTimestamp) > 1000) {
      len = 0;
      pos = 0;
      cnt = 0;
  }
  
  delay(100);
}

void receiveEvent(int byteCount) {
  if (send) {
    state = 127 ;//254; // I'm busy
    return;
  }
  if (len == 0) { // First package
    state = 125;//252;
  }
  else {
    state = 126;//253;
  }

  while (Wire.available()) { // loop through all but the last
    unsigned char c = Wire.read(); // receive byte as a character
    state = 124;
    if (cnt == 0) {
      len = c;
      pos = 0;
      lastMessageTimestamp = millis();
    }
    else if (cnt == 1) {
      retries = c;
    }
    else if (pos < MAX_MESSAGE_SIZE) {
      buffer[pos] = c;
      pos++;
    }
    else { // overflow
      len = 0;
      pos = 0;
      cnt = 0;
      send = 0;
      state = 122;
      break;
    }
    cnt++;
    if (pos + 2 == len) {
      ircodeLength = len / 2;
      pos = 0;
      len = 0;
      cnt = 0;
      send = 1;
      state = 123;
    }
  }
}

void sendResponse() {
  Wire.write(state);
}


