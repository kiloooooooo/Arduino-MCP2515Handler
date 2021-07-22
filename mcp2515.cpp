#include <SPI.h>
#include "mcp2515.hpp"

using namespace MCP2515;

MCP2515Handler::MCP2515Handler(const int csPin)
{
    this->spiSettings = SPISettings(10000000, MSBFIRST, SPI_MODE0);
    this->csPin = csPin;
    SPI.begin();
}

MCP2515Handler::~MCP2515Handler()
{
    SPI.end();
}

void MCP2515Handler::select()
{
    digitalWrite(this->csPin, LOW);
}

void MCP2515Handler::unselect()
{
    digitalWrite(this->csPin, HIGH);
}

void MCP2515Handler::instruct(const Instruction inst)
{
    this->instruct(static_cast<byte>(inst));
}

void MCP2515Handler::instruct(const byte inst)
{
    SPI.beginTransaction(this->spiSettings);
    this->select();
    SPI.transfer(inst);
    this->unselect();
    SPI.endTransaction();
}

void MCP2515Handler::setup()
{
    pinMode(this->csPin, OUTPUT);
    digitalWrite(this->csPin, HIGH);
    this->reset();
}

void MCP2515Handler::reset()
{
    this->instruct(Instruction::RESET);
}

void MCP2515Handler::switchMode(const Mode mode)
{
    this->modReg(Register::CANCTRL, 0xE0, (static_cast<byte>(mode)) << 5);
}

void MCP2515Handler::writeReg(const Register reg, const byte value)
{
    SPI.beginTransaction(this->spiSettings);
    this->select();
    SPI.transfer(static_cast<byte>(Instruction::WRITE_REGISTER));
    SPI.transfer(static_cast<byte>(reg));
    SPI.transfer(value);
    this->unselect();
    SPI.endTransaction();
}

void MCP2515Handler::modReg(const Register reg, const byte mask, const byte value)
{
    SPI.beginTransaction(this->spiSettings);
    this->select();
    SPI.transfer(static_cast<byte>(Instruction::BIT_MODIFY));
    SPI.transfer(static_cast<byte>(reg));
    SPI.transfer(mask);
    SPI.transfer(value);
    this->unselect();
    SPI.endTransaction();
}

byte MCP2515Handler::readReg(const Register reg)
{
    byte value;

    SPI.beginTransaction(this->spiSettings);
    this->select();
    SPI.transfer(static_cast<byte>(Instruction::READ_REGISTER));
    SPI.transfer(static_cast<byte>(reg));
    value = SPI.transfer(0x00);
    this->unselect();
    SPI.endTransaction();

    return value;
}

void MCP2515Handler::loadTXB0(const CANFrame frame)
{
    // ID
    if (frame.isEID)
    {
        // Extended ID
        byte sidh = (frame.id & 0x1FE00000) >> 21;
        byte sidl = ((frame.id & 0x001C0000) >> 13) + 0x08 + ((frame.id & 0x00030000) >> 16);
        byte eid8 = (frame.id & 0x0000FF00) >> 8;
        byte eid0 = frame.id & 0x000000FF;
        this->writeReg(Register::TXB0SIDH, sidh);
        this->writeReg(Register::TXB0SIDL, sidl);
        this->writeReg(Register::TXB0EID8, eid8);
        this->writeReg(Register::TXB0EID0, eid0);
    }
    else
    {
        // Standard ID
        byte sidh = (frame.id & 0x000007F8) >> 3;
        byte sidl = (frame.id & 0x00000007) << 5;
        this->writeReg(Register::TXB0SIDH, sidh);
        this->writeReg(Register::TXB0SIDL, sidl);
    }

    // DLC
    byte dlc = (frame.dlc & 0x0F) + (frame.isRemoteFrame ? 0x40 : 0x00);
    this->writeReg(Register::TXB0DLC, dlc);

    // Data
    const Register txb0dn[8] = {
        Register::TXB0D0,
        Register::TXB0D1,
        Register::TXB0D2,
        Register::TXB0D3,
        Register::TXB0D4,
        Register::TXB0D5,
        Register::TXB0D6,
        Register::TXB0D7};
    for (byte i = 0; i < 8; i++)
    {
        if (i < frame.dlc)
            this->writeReg(txb0dn[i], frame.data[i]);
        else
            this->writeReg(txb0dn[i], 0x00);
    }
}

void MCP2515Handler::loadTXB1(const CANFrame frame)
{
    // ID
    if (frame.isEID)
    {
        // Extended ID
        byte sidh = (frame.id & 0x1FE00000) >> 21;
        byte sidl = ((frame.id & 0x001C0000) >> 13) + 0x08 + ((frame.id & 0x00030000) >> 16);
        byte eid8 = (frame.id & 0x0000FF00) >> 8;
        byte eid0 = frame.id & 0x000000FF;
        this->writeReg(Register::TXB1SIDH, sidh);
        this->writeReg(Register::TXB1SIDL, sidl);
        this->writeReg(Register::TXB1EID8, eid8);
        this->writeReg(Register::TXB1EID0, eid0);
    }
    else
    {
        // Standard ID
        byte sidh = (frame.id & 0x000007F8) >> 3;
        byte sidl = (frame.id & 0x00000007) << 5;
        this->writeReg(Register::TXB1SIDH, sidh);
        this->writeReg(Register::TXB1SIDL, sidl);
    }

    // DLC
    byte dlc = (frame.dlc & 0x0F) + (frame.isRemoteFrame ? 0x40 : 0x00);
    this->writeReg(Register::TXB1DLC, dlc);

    // Data
    const Register txb0dn[8] = {
        Register::TXB1D0,
        Register::TXB1D1,
        Register::TXB1D2,
        Register::TXB1D3,
        Register::TXB1D4,
        Register::TXB1D5,
        Register::TXB1D6,
        Register::TXB1D7};
    for (byte i = 0; i < 8; i++)
    {
        if (i < frame.dlc)
            this->writeReg(txb0dn[i], frame.data[i]);
        else
            this->writeReg(txb0dn[i], 0x00);
    }
}

void MCP2515Handler::loadTXB2(const CANFrame frame)
{
    // ID
    if (frame.isEID)
    {
        // Extended ID
        byte sidh = (frame.id & 0x1FE00000) >> 21;
        byte sidl = ((frame.id & 0x001C0000) >> 13) + 0x08 + ((frame.id & 0x00030000) >> 16);
        byte eid8 = (frame.id & 0x0000FF00) >> 8;
        byte eid0 = frame.id & 0x000000FF;
        this->writeReg(Register::TXB2SIDH, sidh);
        this->writeReg(Register::TXB2SIDL, sidl);
        this->writeReg(Register::TXB2EID8, eid8);
        this->writeReg(Register::TXB2EID0, eid0);
    }
    else
    {
        // Standard ID
        byte sidh = (frame.id & 0x000007F8) >> 3;
        byte sidl = (frame.id & 0x00000007) << 5;
        this->writeReg(Register::TXB2SIDH, sidh);
        this->writeReg(Register::TXB2SIDL, sidl);
    }

    // DLC
    byte dlc = (dlc & 0x0F) + (frame.isRemoteFrame ? 0x40 : 0x00);
    this->writeReg(Register::TXB2DLC, dlc);

    // Data
    const Register txb0dn[8] = {
        Register::TXB2D0,
        Register::TXB2D1,
        Register::TXB2D2,
        Register::TXB2D3,
        Register::TXB2D4,
        Register::TXB2D5,
        Register::TXB2D6,
        Register::TXB2D7};
    for (byte i = 0; i < 8; i++)
    {
        if (i < frame.dlc)
            this->writeReg(txb0dn[i], frame.data[i]);
        else
            this->writeReg(txb0dn[i], 0x00);
    }
}

bool MCP2515Handler::checkRXB0Flag(void)
{
    byte canintf = this->readReg(Register::CANINTF);
    return canintf & 0x01;
}

bool MCP2515Handler::checkRXB1Flag(void)
{
    byte canintf = this->readReg(Register::CANINTF);
    return canintf & 0x02;
}

void MCP2515Handler::readRXB0(CANFrame *frame)
{
    // RTR?
    byte rxb0ctrl = this->readReg(Register::RXB0CTRL);
    frame->isRemoteFrame = rxb0ctrl & 0x08;

    // ID
    long sidh = this->readReg(Register::RXB0SIDH);
    long sidl = this->readReg(Register::RXB0SIDL);
    long eid8 = this->readReg(Register::RXB0EID8);
    long eid0 = this->readReg(Register::RXB0EID0);
    long id = 0;
    if ((sidl & 0x08) == 0x08)
    {
        // Extended ID
        id |= sidh << 21;          // SID[10:3]
        id |= (sidl & 0xE0) << 13; // SID[2:0]
        id |= (sidl & 0x03) << 16; // EID[17:16]
        id |= eid8 << 8;
        id |= eid0;
        frame->isEID = true;
    }
    else
    {
        // Standard ID
        id |= sidh << 3;
        id |= (sidl & 0xE0) >> 5;
        frame->isEID = false;
    }
    frame->id = id;

    // DLC
    byte dlc = this->readReg(Register::RXB0DLC);
    frame->dlc = dlc & 0x0F;

    // Data
    Register registers[8] = {
        Register::RXB0D0,
        Register::RXB0D1,
        Register::RXB0D2,
        Register::RXB0D3,
        Register::RXB0D4,
        Register::RXB0D5,
        Register::RXB0D6,
        Register::RXB0D7,
    };
    for (byte i = 0; i < 8; i++)
    {
        if (i < frame->dlc)
            frame->data[i] = this->readReg(registers[i]);
        else
            frame->data[i] = 0x00;
    }

    this->modReg(Register::CANINTF, 0x01, 0x00);
}

void MCP2515Handler::readRXB1(CANFrame *frame)
{
    // RTR?
    byte rxb1ctrl = this->readReg(Register::RXB1CTRL);
    frame->isRemoteFrame = rxb1ctrl & 0x08;

    // ID
    long sidh = this->readReg(Register::RXB1SIDH);
    long sidl = this->readReg(Register::RXB1SIDL);
    long eid8 = this->readReg(Register::RXB1EID8);
    long eid0 = this->readReg(Register::RXB1EID0);
    long id = 0;
    if ((sidl & 0x08) == 0x08)
    {
        // Extended ID
        id |= sidh << 21;          // SID[10:3]
        id |= (sidl & 0xE0) << 13; // SID[2:0]
        id |= (sidl & 0x03) << 16; // EID[17:16]
        id |= eid8 << 8;
        id |= eid0;
        frame->isEID = true;
    }
    else
    {
        // Standard ID
        id |= sidh << 3;
        id |= (sidl & 0xE0) >> 5;
        frame->isEID = false;
    }
    frame->id = id;

    // DLC
    byte dlc = this->readReg(Register::RXB1DLC);
    frame->dlc = dlc & 0x0F;

    // Data
    Register registers[8] = {
        Register::RXB1D0,
        Register::RXB1D1,
        Register::RXB1D2,
        Register::RXB1D3,
        Register::RXB1D4,
        Register::RXB1D5,
        Register::RXB1D6,
        Register::RXB1D7,
    };
    for (byte i = 0; i < 8; i++)
    {
        if (i < frame->dlc)
            frame->data[i] = this->readReg(registers[i]);
        else
            frame->data[i] = 0x00;
    }

    this->modReg(Register::CANINTF, 0x02, 0x00);
}

void MCP2515Handler::transmitTXB0(void)
{
    this->instruct(0x81);
}

void MCP2515Handler::transmitTXB1(void)
{
    this->instruct(0x82);
}

void MCP2515Handler::transmitTXB2(void)
{
    this->instruct(0x84);
}

void MCP2515Handler::transmit(const bool txb0, const bool txb1, const bool txb2)
{
    byte inst = 0x80;

    if (txb0)
        inst |= 0x01;
    if (txb1)
        inst |= 0x02;
    if (txb2)
        inst |= 0x04;

    this->instruct(inst);
}
