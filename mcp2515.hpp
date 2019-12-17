#ifndef MCP2515_H
#define MCP2515_H

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
    /*
     Reset internal registers to the
     default state and set CONFIGURATION mode.
  */
    RESET = 0xC0,

    /*
     Read data from the register beginning
     at selected address.
  */
    READ = 0x03,

    /*
     Read RX buffer register beggining at
  */
    READ_RX_BUFFER_RXB0SIDH = 0x90, // TXB0SIDH
    READ_RX_BUFFER_RXB0D0 = 0x92,   // TXB0D0
    READ_RX_BUFFER_RXB1SIDH = 0x94, // TXB1SIDH
    READ_RX_BUFFER_RXB1D0 = 0x96,   // TXB1D0

    /*
     Write data to the register beginning
     at the selected address.
  */
    WRITE = 0x02,

    /*
     Load TX buffer beginning at
  */
    LOAD_TX_BUFFER_TXB0SIDH = 0x40, // TXB0SIDH
    LOAD_TX_BUFFER_TXB0D0 = 0x41,   // TXB0D0
    LOAD_TX_BUFFER_TXB1SIDH = 0x42, // TXB1SIDH
    LOAD_TX_BUFFER_TXB1D0 = 0x43,   // TXB1D0
    LOAD_TX_BUFFER_TXB2SIDH = 0x44, // TXB2SIDH
    LOAD_TX_BUFFER_TXB2D0 = 0x45,   // TXB2D0

    /*
     Instruct MCP2515 to begin message transmission
     sequence for any transmit buffers. For multipul
     buffers, perform bitwise OR operation for each
     instructions and pass it to
     MCP2515Handler::instruct(unsigned char instruction)
     method directly.
     
     e.g.) RTS for TXB0 and TXB2
             unsigned char instruction =
               static_cast<unsigned char>(Instruction::RTS_TXB0)
               | static_cast<unsigned char>(Instruction::RTS_TXB2);
             handler->instruct(instruction);
  */
    RTS_TXB0 = 0x81,
    RTS_TXB1 = 0x82,
    RTS_TXB2 = 0x84,

    /*
     Quick polling command that reads several status
     bits for transmit and receive functions.
  */
    READ_STATUS = 0xA0,

    /*
     Quick polling command that indicates filter match
     and message type (standard, extended and/or remote)
     of received message.
  */
    RX_STATUS = 0xB0,

    /*
     Set or clear individual bits in a paticular register.
  */
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
    LISTENON = 3,
    CONFIG = 4
};

/*
 * Result
 */
enum struct Result
{
    OK = 0,
    FAIL = 1,
    INVALID = 2
};

class MCP2515Handler
{
private:
    int csPin;
    int resetPin;
    void select();
    void unselect();

public:
    MCP2515Handler(int csPin, int resetPin);
    ~MCP2515Handler();
    Result init();
    Result reset();
    Result switchMode(const Mode mode);
    Result instruct(const Instruction instruction);
    Result setRegister(const Register reg, const unsigned char data);
    Result modRegister(const Register reg, const unsigned char mask, const unsigned char data);
    Result readRegister(const Register reg, unsigned char *data);
    Result setTXB0ID(const bool isExtended, const long id);
    Result setTXB1ID(const bool isExtended, const long id);
    Result setTXB2ID(const bool isExtended, const long id);
    Result readRXB0ID(long int *id);
    Result readRXB1ID(long int *id);
    Result loadTXB0Data(unsigned char *data, unsigned char len);
    Result loadTXB1Data(unsigned char *data, unsigned char len);
    Result loadTXB2Data(unsigned char *data, unsigned char len);
    Result readRXB0Data(unsigned char *data);
    Result readRXB1Data(unsigned char *data);
    Result readRXB0(long int *id, unsigned char *data);
    Result readRXB1(long int *id, unsigned char *data);
    Result transmitTXB0();
    Result transmitTXB1();
    Result transmitTXB2();
};

#endif
