#include "mcp2515.hpp"

#define RESET_PIN 8
#define CS_PIN    10

#define MODE_TRANSMISSION 0
#define MODE_RECEPTION 1

MCP2515Handler* can;
bool flag = false;

void setup() {
  // put your setup code here, to run once:

  Serial.begin(115200);

  pinMode(CS_PIN, OUTPUT);
  pinMode(RESET_PIN, OUTPUT);

  digitalWrite(CS_PIN, HIGH);
  digitalWrite(RESET_PIN, HIGH);
  
  Serial.println("Initializing...");
  can = new MCP2515Handler(CS_PIN, RESET_PIN);
  can->init();
  Serial.println("Initialization complete.");

  if (MODE_TRANSMISSION)
  {
    Serial.println("Preparing for transmission...");
    can->setId(0, false, 0x0000075F);
    can->setRegister(Register::TXB0DLC, 0x01);
    can->setRegister(Register::TXB0D0, 0x42);
    can->switchMode(Mode::NORMAL);
    Serial.println("Complete.");

    Serial.println("Transmitting...");
    can->transmit(true, false, false);
  }
  else if (MODE_RECEPTION)
  {
    Serial.println("Preparing for reception...");
    can->switchMode(Mode::LISTENON);
    Serial.println("Complete.");
  }

  Serial.println(flag);
}

void loop() {
  // put your main code here, to run repeatedly:

  if (MODE_TRANSMISSION && (!flag))
  {
    flag = true;
    
    unsigned char canintf = can->readRegister(Register::CANINTF);
    unsigned char txb0ctrl = can->readRegister(Register::TXB0CTRL);
    unsigned char abtf = (txb0ctrl & 0x40) >> 6;
    unsigned char txb0if = (canintf & 0x04) >> 2;

    if (!abtf && txb0if)
    {
      Serial.println("==== TRANSMISSION COMPLETE ====");
    }
  }

  if (MODE_RECEPTION && (!flag))
  {
    flag = true;

    unsigned char canintf = can->readRegister(Register::CANINTF);
    unsigned char rxb0if = canintf & 0x01;

    if (rxb0if)
    {
      Serial.println("==== RECEPTION COMPLETE ====");
      unsigned char rxb0dlc = can->readRegister(Register::RXB0DLC);
      unsigned char rxb0sidh = can->readRegister(Register::RXB0SIDH);
      unsigned char rxb0sidl = can->readRegister(Register::RXB0SIDL);
      unsigned char len = rxb0dlc & 0x0F;
      unsigned char data[len];

      Register registers[8] = {
        Register::RXB0D0,
        Register::RXB0D1,
        Register::RXB0D2,
        Register::RXB0D3,
        Register::RXB0D4,
        Register::RXB0D5,
        Register::RXB0D6,
        Register::RXB0D7
      };
      for (int i = 0; i < len; i++)
      {
        data[i] = can->readRegister(registers[i]);
      }

      Serial.print("RXB0SIDH = ");
      Serial.println(rxb0sidh, BIN);
      Serial.print("RXB0SIDL = ");
      Serial.println(rxb0sidl, BIN);

      for (int i = 0; i < len; i++)
      {
        Serial.print("RXB0D");
        Serial.print(i);
        Serial.print(" = ");
        Serial.println(data[i], BIN);
      }
      Serial.println("============================");
    }
  }
}
