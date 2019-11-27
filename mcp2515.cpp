#include <SPI.h>
#include "mcp2515.hpp"

SPISettings mcpSpiSettings(10000000, MSBFIRST, SPI_MODE0);

/*
 * MCP2515Handler コンストラクタ．
 * 
 * Arguments:
 *   int csPin:
 *     CS / SS ピン番号．
 *   int resetPin:
 *     リセットピン番号．
 */
MCP2515Handler::MCP2515Handler(int csPin, int resetPin)
{
  this->csPin = csPin;
  this->resetPin = resetPin;
  SPI.begin();
}

/*
 * MCP2515Handler デストラクタ
 */
MCP2515Handler::~MCP2515Handler()
{
  SPI.end();
}

/*
 * *private*
 * MCP2515 をセレクトします．
 */
void MCP2515Handler::select()
{
  digitalWrite(this->csPin, LOW);
}

/*
 * *private*
 * MCP2515 をアンセレクトします．
 */
void MCP2515Handler::unselect()
{
  digitalWrite(this->csPin, HIGH);
}

/*
 * *private*
 * MCP2515 に特定の命令を送信します．引数には命令コードを直に渡します．
 * 
 * Arguments:
 *  unsigned char instruction:
 *    送信する命令．
 */
Result MCP2515Handler::instruct(unsigned char instruction)
{
  SPI.beginTransaction(mcpSpiSettings);
  this->select();
  SPI.transfer(instruction);
  this->unselect();
  SPI.endTransaction();

  return Result::OK;
}

/*
 * MCP2515 を初期化します．
 */
Result MCP2515Handler::init()
{
  this->reset();
  this->switchMode(Mode::CONFIG);

  unsigned char canstat = this->readRegister(Register::CANSTAT);
  if ((canstat >> 5) == static_cast<unsigned char>(Mode::CONFIG))
  {
    return Result::OK;
  }
  else
  {
    return Result::FAIL;
  }
}

/*
 * MCP2515 をリセットします．
 */
Result MCP2515Handler::reset()
{
  digitalWrite(this->resetPin, LOW);
  delay(1);
  digitalWrite(this->resetPin, HIGH);

  return Result::OK;
}

/*
 * MCP2515 の動作モードを変更します．
 * 
 * Arguments:
 *   Mode mode:
 *     新しい動作モード．
 */
Result MCP2515Handler::switchMode(Mode mode)
{
  unsigned char canctrl = static_cast<unsigned char>(mode) << 5;
  this->modRegister(Register::CANCTRL, 0xE0, canctrl);

  unsigned char canstat = this->readRegister(Register::CANSTAT);

  if ((canstat >> 5) == static_cast<unsigned char>(mode))
  {
    return Result::OK;
  }
  else
  {
    return Result::FAIL;
  }
}

/*
 * MCP2515 に特定の命令を送信します．
 * 
 * Arguments:
 *   Instruction instruction:
 *     送信する命令．
 */
Result MCP2515Handler::instruct(Instruction instruction)
{
  return this->instruct(static_cast<unsigned char>(instruction));
}

/*
 * 指定したレジスタに値を書き込みます．
 * 
 * Arguments:
 *   Register reg:
 *     書き込み先のレジスタ．
 *   unsigned char data:
 *     データバイト．
 */
Result MCP2515Handler::setRegister(Register reg, unsigned char data)
{
  SPI.beginTransaction(mcpSpiSettings);
  this->select();
  SPI.transfer(static_cast<unsigned char>(Instruction::WRITE));
  SPI.transfer(static_cast<unsigned char>(reg));
  SPI.transfer(data);
  this->unselect();
  SPI.endTransaction();

  unsigned char regData = this->readRegister(reg);
  if (data == regData)
  {
    return Result::OK;
  }
  else
  {
    return Result::FAIL;
  }
}

/*
 * 指定したレジスタの一部を書き換えます．
 * 
 * Arguments:
 *   Register reg:
 *     書き換えるレジスタ．
 *   unsigned char mask:
 *     マスクバイト．
 *   unsigned char data:
 *     データバイト．
 *     
 * Memo:
 *   マスクバイトの使い方
 *     before | 0 1 0 1 0 0 0 1
 *     mask   | 0 0 1 1 0 1 0 1
 *     data   | X X 1 0 X 0 X 1
 *     -------+-----------------
 *     after  | 0 1 1 0 0 0 0 1
 */
Result MCP2515Handler::modRegister(Register reg, unsigned char mask, unsigned char data)
{
  SPI.beginTransaction(mcpSpiSettings);
  this->select();
  SPI.transfer(static_cast<unsigned char>(Instruction::BIT_MODIFY));
  SPI.transfer(static_cast<unsigned char>(reg));
  SPI.transfer(mask);
  SPI.transfer(data);
  this->unselect();
  SPI.endTransaction();

  unsigned char regData = this->readRegister(reg);
  if (data == regData)
  {
    return Result::OK;
  }
  else
  {
    return Result::FAIL;
  }
}

/*
 * 特定のレジスタの値を読み込みます．
 * 
 * Arguments:
 *   Register reg:
 *     読み込むレジスタ．
 */
unsigned char MCP2515Handler::readRegister(Register reg)
{
  SPI.beginTransaction(mcpSpiSettings);
  this->select();
  SPI.transfer(static_cast<unsigned char>(Instruction::READ));
  SPI.transfer(static_cast<unsigned char>(reg));
  unsigned char output = SPI.transfer(0);
  this->unselect();
  SPI.endTransaction();

  return output;
}

/*
 * TXBn のアービトレーションIDを設定します．
 * 
 * Arguments:
 *   unsigned char n:
 *     ID は TXBnSIDH, TXBnSIDL, TXBnEID8, TXBnEID0 に書き込まれます．
 *   bool isExtended:
 *     拡張データフレームである場合は true を渡します．
 *   int id:
 *     アービトレーションID．詳細は下を参照．
 */
Result MCP2515Handler::setId(unsigned char n, bool isExtended, int id)
{
  bool error = false;
  if (isExtended)
  {
    // int id:
    // 31 30 29 28   27 26 25 24   23 22 21 20   19 18 17 16
    //  0  0  0  x    x  x  x  x    x  x  x  x    x  x  x  x
    // ^^^^^^^^ ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ =====
    //  ignore                    SID                   EID
  
    // 15 14 13 12   11 10  9  8    7  6  5  4    3  2  1  0
    //  x  x  x  x    x  x  x  x    x  x  x  x    x  x  x  x
    // =====================================================
    //                          EID

    unsigned char sidh = (id & 0x1FE00000) >> 21;
    unsigned char sidl = ((id & 0x001C0000) >> 18)
                         + 0x08
                         + ((id & 0x00030000) >> 16);
    unsigned char eid8 = (id & 0x0000FF00) >> 8;
    unsigned char eid0 = id & 0x000000FF;

    unsigned char sidh_ = 0x00;
    unsigned char sidl_ = 0x00;
    unsigned char eid8_ = 0x00;
    unsigned char eid0_ = 0x00;

    switch (n)
    {
      case 0:
        this->setRegister(Register::TXB0SIDH, sidh);
        this->setRegister(Register::TXB0SIDL, sidl);
        this->setRegister(Register::TXB0EID8, eid8);
        this->setRegister(Register::TXB0EID0, eid0);
        
        sidh_ = this->readRegister(Register::TXB0SIDH);
        sidl_ = this->readRegister(Register::TXB0SIDL);
        eid8_ = this->readRegister(Register::TXB0EID8);
        eid0_ = this->readRegister(Register::TXB0EID0);
        
        error = (sidh != sidh_)
                || (sidl != sidl_)
                || (eid8 != eid8_)
                || (eid0 != eid0_);
        break;
      case 1:
        this->setRegister(Register::TXB1SIDH, sidh);
        this->setRegister(Register::TXB1SIDL, sidl);
        this->setRegister(Register::TXB1EID8, eid8);
        this->setRegister(Register::TXB1EID0, eid0);

        sidh_ = this->readRegister(Register::TXB1SIDH);
        sidl_ = this->readRegister(Register::TXB1SIDL);
        eid8_ = this->readRegister(Register::TXB1EID8);
        eid0_ = this->readRegister(Register::TXB1EID0);

        error = (sidh != sidh_)
                || (sidl != sidl_)
                || (eid8 != eid8_)
                || (eid0 != eid0_);
        break;
      case 2:
        this->setRegister(Register::TXB2SIDH, sidh);
        this->setRegister(Register::TXB2SIDL, sidl);
        this->setRegister(Register::TXB2EID8, eid8);
        this->setRegister(Register::TXB2EID0, eid0);

        sidh_ = this->readRegister(Register::TXB2SIDH);
        sidl_ = this->readRegister(Register::TXB2SIDL);
        eid8_ = this->readRegister(Register::TXB2EID8);
        eid0_ = this->readRegister(Register::TXB2EID0);

        error = (sidh != sidh_)
                || (sidl != sidl_)
                || (eid8 != eid8_)
                || (eid0 != eid0_);
        break;
      default:
        return Result::INVALID;
    }

    return error ? Result::FAIL : Result::OK;
  }
  else
  {
    // int id:
    // 31 30 29 28   27 26 25 24   23 22 21 20   19 18 17 16
    //  0  0  0  x    x  x  x  x    x  x  x  x    x  x  x  x
    // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
    //                       ignore
  
    // 15 14 13 12   11 10  9  8    7  6  5  4    3  2  1  0
    //  x  x  x  x    x  x  x  x    x  x  x  x    x  x  x  x
    // ^^^^^^^^^^^^^^^^ ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    //     ignore                        SID

    unsigned char sidh = id >> 3;
    unsigned char sidl = (id & 0x00000007) << 5;

    unsigned char sidh_ = 0x00;
    unsigned char sidl_ = 0x00;

    switch (n)
    {
      case 0:
        this->setRegister(Register::TXB0SIDH, sidh);
        this->setRegister(Register::TXB0SIDL, sidl);

        sidh_ = this->readRegister(Register::TXB0SIDH);
        sidl_ = this->readRegister(Register::TXB0SIDL);

        error = (sidh != sidh_)
                || (sidl != sidl_);
        break;
      case 1:
        this->setRegister(Register::TXB1SIDH, sidh);
        this->setRegister(Register::TXB1SIDL, sidl);

        sidh_ = this->readRegister(Register::TXB1SIDH);
        sidl_ = this->readRegister(Register::TXB1SIDL);

        error = (sidh != sidh_)
                || (sidl != sidl_);
        break;
      case 2:
        this->setRegister(Register::TXB2SIDH, sidh);
        this->setRegister(Register::TXB2SIDL, sidl);

        sidh_ = this->readRegister(Register::TXB2SIDH);
        sidl_ = this->readRegister(Register::TXB2SIDL);

        error = (sidh != sidh_)
                || (sidl != sidl_);
        break;
      default:
        return Result::INVALID;
    }
  }

  return error ? Result::FAIL : Result::OK;
}

/*
 * RXBn の ID を読み込みます．
 * 
 * Arguments:
 *   unsigned char n:
 *     RXBn の ID が読み込まれます．
 */
int MCP2515Handler::readId(unsigned char n)
{
  int sidh = 0x00;
  int sidl = 0x00;
  int eid8 = 0x00;
  int eid0 = 0x00;
  int rxId = 0x00000000;
  
  switch (n)
  {
    case 0:
      sidh = this->readRegister(Register::RXB0SIDH);
      sidl = this->readRegister(Register::RXB0SIDL);
      eid8 = this->readRegister(Register::RXB0EID8);
      eid0 = this->readRegister(Register::RXB0EID0);
      break;
    case 1:
      sidh = this->readRegister(Register::RXB1SIDH);
      sidl = this->readRegister(Register::RXB1SIDL);
      eid8 = this->readRegister(Register::RXB1EID8);
      eid0 = this->readRegister(Register::RXB1EID0);
      break;
    default:
      return -1;
      break;
  }

  if ((sidl & 0x08) >> 3)
  {
    // Extended ID
    rxId |= sidh << 21;
    rxId |= ((sidl & 0xE0) << 13);
    rxId |= ((sidl & 0x03) << 16)
            + (eid8 << 8)
            + eid0;
  }
  else
  {
    // Standard ID
    rxId |= sidh << 3;
    rxId |= (sidl & 0xE0) >> 5;
  }

  return rxId;
}

/*
 * TXBnDm にデータを書き込みます．
 * 
 * Arguments:
 *   unsigned char n:
 *     データは TXBnD0, TXBnD1, ..., TXBnD7 に書き込まれます．
 *   unsigned char* data:
 *     書き込むデータ配列．
 *   unsigned char len:
 *     data の長さ [Byte]
 */
Result MCP2515Handler::loadTXData(unsigned char n, unsigned char* data, unsigned char len)
{
  bool error = false;
  Register* registers;

  switch (n)
  {
    case 0:
      registers = new Register[8] {
        Register::TXB0D0,
        Register::TXB0D1,
        Register::TXB0D2,
        Register::TXB0D3,
        Register::TXB0D4,
        Register::TXB0D5,
        Register::TXB0D6,
        Register::TXB0D7
      };

      this->modRegister(Register::TXB0DLC, 0x0F, len & 0x0F);

      for (int i = 0; i < len; i++)
      {
        this->setRegister(registers[i], data[i]);
      }

      for (int i = 0; i < len; i++)
      {
        if (this->readRegister(registers[i]) != data[i])
        {
          error = true;
          break;
        }
      }
      
      break;
    case 1:
      registers = new Register[8] {
        Register::TXB1D0,
        Register::TXB1D1,
        Register::TXB1D2,
        Register::TXB1D3,
        Register::TXB1D4,
        Register::TXB1D5,
        Register::TXB1D6,
        Register::TXB1D7
      };

      this->modRegister(Register::TXB1DLC, 0x0F, len & 0x0F);

      for (int i = 0; i < len; i++)
      {
        this->setRegister(registers[i], data[i]);
      }

      for (int i = 0; i < len; i++)
      {
        if (this->readRegister(registers[i]) != data[i])
        {
          error = true;
          break;
        }
      }
      
      break;
    case 2:
      registers = new Register[8] {
        Register::TXB2D0,
        Register::TXB2D1,
        Register::TXB2D2,
        Register::TXB2D3,
        Register::TXB2D4,
        Register::TXB2D5,
        Register::TXB2D6,
        Register::TXB2D7
      };

      this->modRegister(Register::TXB2DLC, 0x0F, len & 0x0F);

      for (int i = 0; i < len; i++)
      {
        this->setRegister(registers[i], data[i]);
      }

      for (int i = 0; i < len; i++)
      {
        if (this->readRegister(registers[i]) != data[i])
        {
          error = true;
          break;
        }
      }
      
      break;
    default:
      return Result::INVALID;
  }

  return error ? Result::FAIL : Result::OK;
}

/*
 * RXBnDm の値を読み込みます．unsigned char の配列が返されますが，
 * その長さについては別途 RXBnDLC レジスタを参照してください．
 * 
 * Arguments:
 *   unsigned char n:
 *     RXBnD0, RXBnD1, ..., RXBnDk の値を読み込みます．(k = DLC)
 */
unsigned char* MCP2515Handler::readRXData(unsigned char n)
{
  unsigned char dlc = 0x00;
  unsigned char* data;
  Register* registers;
  switch (n)
  {
    case 0:
      dlc = this->readRegister(Register::RXB0DLC) & 0x0F;

      registers = new Register[8] {
        Register::RXB0D0,
        Register::RXB0D1,
        Register::RXB0D2,
        Register::RXB0D3,
        Register::RXB0D4,
        Register::RXB0D5,
        Register::RXB0D6,
        Register::RXB0D7,
      };

      data = new unsigned char[8];
      for (int i = 0; i < dlc; i++)
      {
        data[i] = this->readRegister(registers[i]);
      }
      break;
    case 1:
      dlc = this->readRegister(Register::RXB0DLC) & 0x0F;

      registers = new Register[8] {
        Register::RXB1D0,
        Register::RXB1D1,
        Register::RXB1D2,
        Register::RXB1D3,
        Register::RXB1D4,
        Register::RXB1D5,
        Register::RXB1D6,
        Register::RXB1D7,
      };

      data = new unsigned char[8];
      for (int i = 0; i < dlc; i++)
      {
        data[i] = this->readRegister(registers[i]);
      }
      break;
    default:
      return NULL;
      break;
  }

  return data;
}

/*
 * 送信要求(RTS)命令を送信します．
 * 
 * Arguments:
 *   bool txb0:
 *     TXB0 の送信要求．
 *   bool txb1:
 *     TXB1 の送信要求．
 *   bool txb2:
 *     TXB2 の送信要求．
 */
Result MCP2515Handler::transmit(bool txb0, bool txb1, bool txb2)
{
  unsigned char inst = 0x00;
  if (txb0)
    inst |= static_cast<unsigned char>(Instruction::RTS_TXB0);
  if (txb1)
    inst |= static_cast<unsigned char>(Instruction::RTS_TXB1);
  if (txb2)
    inst |= static_cast<unsigned char>(Instruction::RTS_TXB2);

  this->instruct(inst);

  return Result::OK;
}
