#include "mcp2515.hpp"

#define RESET_PIN 8

// #define MODE_TRANSMISSION
#define MODE_RECEPTION

MCP2515Handler *can;
bool flag = false;

void setup()
{
  // put your setup code here, to run once:

  Serial.begin(115200);

  pinMode(CS_PIN, OUTPUT);

  digitalWrite(CS_PIN, HIGH);

  Serial.println("Initializing...");
  can = new MCP2515Handler(CS_PIN);
  can->init();
  Serial.println("Initialization complete.");

#ifdef MODE_TRANSMISSION
  Serial.println("Preparing for transmission...");
  CANFrame frame;
  frame.id = 0x0000075F;
  frame.dlc = 1;
  frame.data[0] = 0x42;
  frame.isRemoteFrame = false;
  can->loadTXB0(frame);
  can->switchMode(Mode::NORMAL);
  Serial.println("Complete.");

  Serial.println("Transmitting...");
  can->transmitTXB0();
#else
  Serial.println("Preparing for reception...");
  can->switchMode(Mode::LISTENON);
  Serial.println("Complete.");
#endif

  Serial.println(flag);
}

void loop()
{
#ifdef MODE_TRANSMISSION
  can->transmitTXB0();
#else
  if (can->checkRXB0Flag())
  {
    CANFrame frame;
    can->readRXB0(&frame);
    Serial.println("==== RECEPTION COMPLETE ====");
    Serial.print("ID: 0x");
    Serial.println(frame.id, HEX);
    Serial.print("DLC: ");
    Serial.println(frame.dlc);
    for (int i = 0; i < frame.dlc; i++)
    {
      Serial.print("Data ");
      Serial.print(i);
      Serial.print(": ");
      Serial.println(frame.data[i]);
    }
    Serial.println();
  }
#endif
}
