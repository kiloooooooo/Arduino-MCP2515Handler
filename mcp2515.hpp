#ifndef MCP2515_H
#define MCP2515_H

#include <SPI.h>

namespace MCP2515
{
/*
   Register addresses
*/
enum struct Register
{
  RXF0SIDH = 0x00,
  RXF0SIDL = 0x01,
  RXF0EID8 = 0x02,
  RXF0EID0 = 0x03,
  RXF1SIDH = 0x04,
  RXF1SIDL = 0x05,
  RXF1EID8 = 0x06,
  RXF1EID0 = 0x07,
  RXF2SIDH = 0x08,
  RXF2SIDL = 0x09,
  RXF2EID8 = 0x0A,
  RXF2EID0 = 0x0B,
  BFPCTRL = 0x0C,
  TXRTSCTRL = 0x0D,
  CANSTAT = 0x0E,
  CANCTRL = 0x0F,
  RXF3SIDH = 0x10,
  RXF3SIDL = 0x11,
  RXF3EID8 = 0x12,
  RXF3EID0 = 0x13,
  RXF4SIDH = 0x14,
  RXF4SIDL = 0x15,
  RXF4EID8 = 0x16,
  RXF4EID0 = 0x17,
  RXF5SIDH = 0x18,
  RXF5SIDL = 0x19,
  RXF5EID8 = 0x1A,
  RXF5EID0 = 0x1B,
  TEC = 0x1C,
  REC = 0x1D,
  RXM0SHDH = 0x20,
  RXM0SIDL = 0x21,
  RXM0EID8 = 0x22,
  RXM0EID0 = 0x23,
  RXM1SIDH = 0x24,
  RXM1SIDL = 0x25,
  RXM1EID8 = 0x26,
  RXM1EID0 = 0x27,
  CNF3 = 0x28,
  CNF2 = 0x29,
  CNF1 = 0x2A,
  CANINTE = 0x2B,
  CANINTF = 0x2C,
  EFLG = 0x2D,
  TXB0CTRL = 0x30,
  TXB0SIDH = 0x31,
  TXB0SIDL = 0x32,
  TXB0EID8 = 0x33,
  TXB0EID0 = 0x34,
  TXB0DLC = 0x35,
  TXB0D0 = 0x36,
  TXB0D1 = 0x37,
  TXB0D2 = 0x38,
  TXB0D3 = 0x39,
  TXB0D4 = 0x3A,
  TXB0D5 = 0x3B,
  TXB0D6 = 0x3C,
  TXB0D7 = 0x3D,
  TXB1CTRL = 0x40,
  TXB1SIDH = 0x41,
  TXB1SIDL = 0x42,
  TXB1EID8 = 0x43,
  TXB1EID0 = 0x44,
  TXB1DLC = 0x45,
  TXB1D0 = 0x46,
  TXB1D1 = 0x47,
  TXB1D2 = 0x48,
  TXB1D3 = 0x49,
  TXB1D4 = 0x4A,
  TXB1D5 = 0x4B,
  TXB1D6 = 0x4C,
  TXB1D7 = 0x4D,
  TXB2CTRL = 0x50,
  TXB2SIDH = 0x51,
  TXB2SIDL = 0x52,
  TXB2EID8 = 0x53,
  TXB2EID0 = 0x54,
  TXB2DLC = 0x55,
  TXB2D0 = 0x56,
  TXB2D1 = 0x57,
  TXB2D2 = 0x58,
  TXB2D3 = 0x59,
  TXB2D4 = 0x5A,
  TXB2D5 = 0x5B,
  TXB2D6 = 0x5C,
  TXB2D7 = 0x5D,
  RXB0CTRL = 0x60,
  RXB0SIDH = 0x61,
  RXB0SIDL = 0x62,
  RXB0EID8 = 0x63,
  RXB0EID0 = 0x64,
  RXB0DLC = 0x65,
  RXB0D0 = 0x66,
  RXB0D1 = 0x67,
  RXB0D2 = 0x68,
  RXB0D3 = 0x69,
  RXB0D4 = 0x6A,
  RXB0D5 = 0x6B,
  RXB0D6 = 0x6C,
  RXB0D7 = 0x6D,
  RXB1CTRL = 0x70,
  RXB1SIDH = 0x71,
  RXB1SIDL = 0x72,
  RXB1EID8 = 0x73,
  RXB1EID0 = 0x74,
  RXB1DLC = 0x75,
  RXB1D0 = 0x76,
  RXB1D1 = 0x77,
  RXB1D2 = 0x78,
  RXB1D3 = 0x79,
  RXB1D4 = 0x7A,
  RXB1D5 = 0x7B,
  RXB1D6 = 0x7C,
  RXB1D7 = 0x7D
};

/*
   SPI Instruction set
*/
enum struct Instruction
{
  RESET = 0xC0,
  READ_REGISTER = 0x03,
  WRITE_REGISTER = 0x02,
  READ_STATUS = 0xA0,
  READ_RX_STATUS = 0xB0,
  BIT_MODIFY = 0x05
};

/*
 * Operation mode
 */
enum struct Mode
{
  NORMAL = 0,
  SLEEP = 1,
  LOOPBACK = 2,
  LISTENONLY = 3,
  CONFIG = 4
};

/*
 * CAN frame
 */
typedef struct
{
  long id;
  bool isRemoteFrame;
  byte dlc;
  byte data[8];
} CANFrame;

class MCP2515Handler
{
private:
  SPISettings spiSettings;
  int csPin;
  void select();
  void unselect();
  void instruct(const Instruction inst);
  void instruct(const byte inst);

public:
  MCP2515Handler(const int csPin);
  ~MCP2515Handler();
  void setup(void);
  void reset(void);
  void switchMode(const Mode mode);
  void writeReg(const Register reg, const byte value);
  void modReg(const Register reg, const byte mask, const byte value);
  byte readReg(const Register reg);
  void loadTXB0(const CANFrame frame);
  void loadTXB1(const CANFrame frame);
  void loadTXB2(const CANFrame frame);
  bool checkRXB0Flag(void);
  bool checkRXB1Flag(void);
  void readRXB0(CANFrame *frame);
  void readRXB1(CANFrame *frame);
  void transmitTXB0(void);
  void transmitTXB1(void);
  void transmitTXB2(void);
  void transmit(const bool txb0, const bool txb1, const bool txb2);
};
} // namespace MCP2515

#endif
