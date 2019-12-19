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
MCP2515Handler::MCP2515Handler(const int csPin, const int resetPin)
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
 * MCP2515 を初期化します．
 */
Result MCP2515Handler::init()
{
    this->reset();
    return this->switchMode(Mode::CONFIG);
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
Result MCP2515Handler::switchMode(const Mode mode)
{
    unsigned char canctrl = static_cast<unsigned char>(mode) << 5;
    this->modRegister(Register::CANCTRL, 0xE0, canctrl);

    unsigned char canstat;
    this->readRegister(Register::CANSTAT, &canstat);

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
Result MCP2515Handler::instruct(const Instruction instruction)
{
    SPI.beginTransaction(mcpSpiSettings);
    this->select();
    SPI.transfer(static_cast<unsigned char>(instruction));
    this->unselect();
    SPI.endTransaction();

    return Result::OK;
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
Result MCP2515Handler::setRegister(const Register reg, const unsigned char data)
{
    SPI.beginTransaction(mcpSpiSettings);
    this->select();
    SPI.transfer(static_cast<unsigned char>(Instruction::WRITE));
    SPI.transfer(static_cast<unsigned char>(reg));
    SPI.transfer(data);
    this->unselect();
    SPI.endTransaction();

    unsigned char regData;
    this->readRegister(reg, &regData);
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
Result MCP2515Handler::modRegister(const Register reg, const unsigned char mask, const unsigned char data)
{
    SPI.beginTransaction(mcpSpiSettings);
    this->select();
    SPI.transfer(static_cast<unsigned char>(Instruction::BIT_MODIFY));
    SPI.transfer(static_cast<unsigned char>(reg));
    SPI.transfer(mask);
    SPI.transfer(data);
    this->unselect();
    SPI.endTransaction();

    unsigned char regData;
    this->readRegister(reg, &regData);
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
 * 
 * Returns:
 *   Result
 */
Result MCP2515Handler::readRegister(const Register reg, unsigned char *data)
{
    SPI.beginTransaction(mcpSpiSettings);
    this->select();
    SPI.transfer(static_cast<unsigned char>(Instruction::READ));
    SPI.transfer(static_cast<unsigned char>(reg));
    unsigned char output = SPI.transfer(0);
    this->unselect();
    SPI.endTransaction();

    *data = output;

    return Result::OK;
}

/*
 * TXB0 のアービトレーションIDを設定します．
 * 
 * Arguments:
 *   bool isExtended:
 *     拡張データフレームである場合は true を渡します．
 *   long id:
 *     アービトレーションID．詳細は下を参照．
 * 
 * Returns:
 *   Result
 */
Result MCP2515Handler::setTXB0ID(const bool isExtended, const long id)
{
    bool error = false;
    if (isExtended)
    {
        // long id:
        // 31 30 29 28   27 26 25 24   23 22 21 20   19 18 17 16
        //  0  0  0  x    x  x  x  x    x  x  x  x    x  x  x  x
        // ^^^^^^^^ ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ =====
        //  ignore                    SID                   EID

        // 15 14 13 12   11 10  9  8    7  6  5  4    3  2  1  0
        //  x  x  x  x    x  x  x  x    x  x  x  x    x  x  x  x
        // =====================================================
        //                          EID

        unsigned char sidh = (id & 0x1FE00000) >> 21;
        unsigned char sidl = ((id & 0x001C0000) >> 13) + 0x08 + ((id & 0x00030000) >> 16);
        unsigned char eid8 = (id & 0x0000FF00) >> 8;
        unsigned char eid0 = id & 0x000000FF;

        unsigned char sidh_ = 0x00;
        unsigned char sidl_ = 0x00;
        unsigned char eid8_ = 0x00;
        unsigned char eid0_ = 0x00;

        this->setRegister(Register::TXB0SIDH, sidh);
        this->setRegister(Register::TXB0SIDL, sidl);
        this->setRegister(Register::TXB0EID8, eid8);
        this->setRegister(Register::TXB0EID0, eid0);

        this->readRegister(Register::TXB0SIDH, &sidh_);
        this->readRegister(Register::TXB0SIDL, &sidl_);
        this->readRegister(Register::TXB0EID8, &eid8_);
        this->readRegister(Register::TXB0EID0, &eid0_);

        error = (sidh != sidh_) || (sidl != sidl_) || (eid8 != eid8_) || (eid0 != eid0_);
    }
    else
    {
        // long id:
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

        this->setRegister(Register::TXB0SIDH, sidh);
        this->setRegister(Register::TXB0SIDL, sidl);

        this->readRegister(Register::TXB0SIDH, &sidh_);
        this->readRegister(Register::TXB0SIDL, &sidl_);

        error = (sidh != sidh_) || (sidl != sidl_);
    }

    return error ? Result::FAIL : Result::OK;
}

/*
 * TXB1 のアービトレーションIDを設定します．
 * 
 * Arguments:
 *   bool isExtended:
 *     拡張データフレームである場合は true を渡します．
 *   long id:
 *     アービトレーションID．詳細は下を参照．
 * 
 * Returns:
 *   Result
 */
Result MCP2515Handler::setTXB1ID(const bool isExtended, const long id)
{
    bool error = false;
    if (isExtended)
    {
        // long id:
        // 31 30 29 28   27 26 25 24   23 22 21 20   19 18 17 16
        //  0  0  0  x    x  x  x  x    x  x  x  x    x  x  x  x
        // ^^^^^^^^ ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ =====
        //  ignore                    SID                   EID

        // 15 14 13 12   11 10  9  8    7  6  5  4    3  2  1  0
        //  x  x  x  x    x  x  x  x    x  x  x  x    x  x  x  x
        // =====================================================
        //                          EID

        unsigned char sidh = (id & 0x1FE00000) >> 21;
        unsigned char sidl = ((id & 0x001C0000) >> 13) + 0x08 + ((id & 0x00030000) >> 16);
        unsigned char eid8 = (id & 0x0000FF00) >> 8;
        unsigned char eid0 = id & 0x000000FF;

        unsigned char sidh_ = 0x00;
        unsigned char sidl_ = 0x00;
        unsigned char eid8_ = 0x00;
        unsigned char eid0_ = 0x00;

        this->setRegister(Register::TXB1SIDH, sidh);
        this->setRegister(Register::TXB1SIDL, sidl);
        this->setRegister(Register::TXB1EID8, eid8);
        this->setRegister(Register::TXB1EID0, eid0);

        this->readRegister(Register::TXB1SIDH, &sidh_);
        this->readRegister(Register::TXB1SIDL, &sidl_);
        this->readRegister(Register::TXB1EID8, &eid8_);
        this->readRegister(Register::TXB1EID0, &eid0_);

        error = (sidh != sidh_) || (sidl != sidl_) || (eid8 != eid8_) || (eid0 != eid0_);
    }
    else
    {
        // long id:
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

        this->setRegister(Register::TXB1SIDH, sidh);
        this->setRegister(Register::TXB1SIDL, sidl);

        this->readRegister(Register::TXB1SIDH, &sidh_);
        this->readRegister(Register::TXB1SIDL, &sidl_);

        error = (sidh != sidh_) || (sidl != sidl_);
    }

    return error ? Result::FAIL : Result::OK;
}

/*
 * TXB2 のアービトレーションIDを設定します．
 * 
 * Arguments:
 *   bool isExtended:
 *     拡張データフレームである場合は true を渡します．
 *   long id:
 *     アービトレーションID．詳細は下を参照．
 * 
 * Returns:
 *   Result
 */
Result MCP2515Handler::setTXB2ID(const bool isExtended, const long id)
{
    bool error = false;
    if (isExtended)
    {
        // long id:
        // 31 30 29 28   27 26 25 24   23 22 21 20   19 18 17 16
        //  0  0  0  x    x  x  x  x    x  x  x  x    x  x  x  x
        // ^^^^^^^^ ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ =====
        //  ignore                    SID                   EID

        // 15 14 13 12   11 10  9  8    7  6  5  4    3  2  1  0
        //  x  x  x  x    x  x  x  x    x  x  x  x    x  x  x  x
        // =====================================================
        //                          EID

        unsigned char sidh = (id & 0x1FE00000) >> 21;
        unsigned char sidl = ((id & 0x001C0000) >> 13) + 0x08 + ((id & 0x00030000) >> 16);
        unsigned char eid8 = (id & 0x0000FF00) >> 8;
        unsigned char eid0 = id & 0x000000FF;

        unsigned char sidh_ = 0x00;
        unsigned char sidl_ = 0x00;
        unsigned char eid8_ = 0x00;
        unsigned char eid0_ = 0x00;

        this->setRegister(Register::TXB2SIDH, sidh);
        this->setRegister(Register::TXB2SIDL, sidl);
        this->setRegister(Register::TXB2EID8, eid8);
        this->setRegister(Register::TXB2EID0, eid0);

        this->readRegister(Register::TXB2SIDH, &sidh_);
        this->readRegister(Register::TXB2SIDL, &sidl_);
        this->readRegister(Register::TXB2EID8, &eid8_);
        this->readRegister(Register::TXB2EID0, &eid0_);

        error = (sidh != sidh_) || (sidl != sidl_) || (eid8 != eid8_) || (eid0 != eid0_);
    }
    else
    {
        // long id:
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

        this->setRegister(Register::TXB2SIDH, sidh);
        this->setRegister(Register::TXB2SIDL, sidl);

        this->readRegister(Register::TXB2SIDH, &sidh_);
        this->readRegister(Register::TXB2SIDL, &sidl_);

        error = (sidh != sidh_) || (sidl != sidl_);
    }

    return error ? Result::FAIL : Result::OK;
}

/*
 * RXB0 のIDを読み込みます．
 * 
 * Returns:
 *   Result
 */
Result MCP2515Handler::readRXB0ID(long int *id)
{
    unsigned char sidh_;
    unsigned char sidl_;
    unsigned char eid8_;
    unsigned char eid0_;

    this->readRegister(Register::RXB0SIDH, &sidh_);
    this->readRegister(Register::RXB0SIDL, &sidl_);
    this->readRegister(Register::RXB0EID8, &eid8_);
    this->readRegister(Register::RXB0EID0, &eid0_);

    long sidh = sidh_;
    long sidl = sidl_;
    long eid8 = eid8_;
    long eid0 = eid0_;
    long rxId = 0x00000000;

    if ((sidl & 0x08) >> 3)
    {
        // Extended ID
        rxId |= sidh << 21;
        rxId |= ((sidl & 0xE0) << 13);
        rxId |= ((sidl & 0x03) << 16) + (eid8 << 8) + eid0;
    }
    else
    {
        // Standard ID
        rxId |= sidh << 3;
        rxId |= (sidl & 0xE0) >> 5;
    }

    *id = rxId;

    return Result::OK;
}

/*
 * RXB1 のIDを読み込みます．
 * 
 * Returns:
 *   Result
 */
Result MCP2515Handler::readRXB1ID(long int *id)
{
    unsigned char sidh_;
    unsigned char sidl_;
    unsigned char eid8_;
    unsigned char eid0_;

    this->readRegister(Register::RXB1SIDH, &sidh_);
    this->readRegister(Register::RXB1SIDL, &sidl_);
    this->readRegister(Register::RXB1EID8, &eid8_);
    this->readRegister(Register::RXB1EID0, &eid0_);

    long sidh = sidh_;
    long sidl = sidl_;
    long eid8 = eid8_;
    long eid0 = eid0_;
    long rxId = 0x00000000;

    if ((sidl & 0x08) >> 3)
    {
        // Extended ID
        rxId |= sidh << 21;
        rxId |= ((sidl & 0xE0) << 13);
        rxId |= ((sidl & 0x03) << 16) + (eid8 << 8) + eid0;
    }
    else
    {
        // Standard ID
        rxId |= sidh << 3;
        rxId |= (sidl & 0xE0) >> 5;
    }

    *id = rxId;

    return Result::OK;
}

/*
 * TXB0Dm にデータを書き込みます．
 * 
 * Arguments:
 *   unsigned char data[]:
 *     書き込むデータ配列．
 *   unsigned char len:
 *     data の長さ [Byte]
 * 
 * Returns:
 *   Result
 */
Result MCP2515Handler::loadTXB0Data(const unsigned char data[], const unsigned char len)
{
    bool error = false;

    Register registers[8] = {
        Register::TXB0D0,
        Register::TXB0D1,
        Register::TXB0D2,
        Register::TXB0D3,
        Register::TXB0D4,
        Register::TXB0D5,
        Register::TXB0D6,
        Register::TXB0D7};

    this->modRegister(Register::TXB0DLC, 0x0F, len & 0x0F);

    for (int i = 0; i < len; i++)
    {
        this->setRegister(registers[i], data[i]);
    }

    for (int i = 0; i < len; i++)
    {
        unsigned char regData;
        this->readRegister(registers[i], &regData);
        if (regData != data[i])
        {
            error = true;
            break;
        }
    }

    return error ? Result::FAIL : Result::OK;
}

/*
 * TXB1Dm にデータを書き込みます．
 * 
 * Arguments:
 *   unsigned char data[]:
 *     書き込むデータ配列．
 *   unsigned char len:
 *     data の長さ [Byte]
 * 
 * Returns:
 *   Result
 */
Result MCP2515Handler::loadTXB1Data(const unsigned char data[], const unsigned char len)
{
    bool error = false;

    Register registers[8] = {
        Register::TXB1D0,
        Register::TXB1D1,
        Register::TXB1D2,
        Register::TXB1D3,
        Register::TXB1D4,
        Register::TXB1D5,
        Register::TXB1D6,
        Register::TXB1D7};

    this->modRegister(Register::TXB1DLC, 0x0F, len & 0x0F);

    for (int i = 0; i < len; i++)
    {
        this->setRegister(registers[i], data[i]);
    }

    for (int i = 0; i < len; i++)
    {
        unsigned char regData;
        this->readRegister(registers[i], &regData);
        if (regData != data[i])
        {
            error = true;
            break;
        }
    }

    return error ? Result::FAIL : Result::OK;
}

/*
 * TXB2Dm にデータを書き込みます．
 * 
 * Arguments:
 *   unsigned char data[]:
 *     書き込むデータ配列．
 *   unsigned char len:
 *     data の長さ [Byte]
 * 
 * Returns:
 *   Result
 */
Result MCP2515Handler::loadTXB2Data(const unsigned char data[], const unsigned char len)
{
    bool error = false;

    Register registers[8] = {
        Register::TXB2D0,
        Register::TXB2D1,
        Register::TXB2D2,
        Register::TXB2D3,
        Register::TXB2D4,
        Register::TXB2D5,
        Register::TXB2D6,
        Register::TXB2D7};

    this->modRegister(Register::TXB2DLC, 0x0F, len & 0x0F);

    for (int i = 0; i < len; i++)
    {
        this->setRegister(registers[i], data[i]);
    }

    for (int i = 0; i < len; i++)
    {
        unsigned char regData;
        this->readRegister(registers[i], &regData);
        if (regData != data[i])
        {
            error = true;
            break;
        }
    }

    return error ? Result::FAIL : Result::OK;
}

/*
 * RXB0 を読み込みます．unsigned char の配列(長さ8)が返されますが，
 * その長さについては別途 RXBnDLC レジスタを参照してください．
 * また読み込み後の CANINTF.RXB0IF のクリアは不要です．
 * 
 * Arguments:
 *   unsigned char *data:
 *     読み込んだデータを格納する変数へのポインタ
 * 
 * Returns:
 *   Result
 */
Result MCP2515Handler::readRXB0Data(unsigned char data[])
{
    unsigned char instruction = static_cast<unsigned char>(Instruction::READ_RX_BUFFER_RXB0D0);

    SPI.beginTransaction(mcpSpiSettings);
    this->select();
    SPI.transfer(instruction);
    for (int byte = 0; byte < 8; byte++)
        data[byte] = SPI.transfer(0);
    this->unselect();
    SPI.endTransaction();

    return Result::OK;
}

/*
 * RXB1 を読み込みます．unsigned char の配列(長さ8)が返されますが，
 * その長さについては別途 RXBnDLC レジスタを参照してください．
 * また読み込み後の CANINTF.RXB1IF のクリアは不要です．
 * 
 * Arguments:
 *   unsigned char *data:
 *     読み込んだデータを格納する変数へのポインタ
 * 
 * Returns:
 *   Result
 */
Result MCP2515Handler::readRXB1Data(unsigned char data[])
{
    unsigned char instruction = static_cast<unsigned char>(Instruction::READ_RX_BUFFER_RXB1D0);

    SPI.beginTransaction(mcpSpiSettings);
    this->select();
    SPI.transfer(instruction);
    for (int byte = 0; byte < 8; byte++)
        data[byte] = SPI.transfer(0);
    this->unselect();
    SPI.endTransaction();

    return Result::OK;
}

/*
 * TXバッファ0に対する送信要求(RTS)命令を送信します．
 * 
 * Returns:
 *     Result
 */
Result MCP2515Handler::transmitTXB0()
{
    this->instruct(Instruction::RTS_TXB0);
    return Result::OK;
}

/*
 * TXバッファ1に対する送信要求(RTS)命令を送信します．
 * 
 * Returns:
 *     Result
 */
Result MCP2515Handler::transmitTXB1()
{
    this->instruct(Instruction::RTS_TXB1);
    return Result::OK;
}

/*
 * TXバッファ2に対する送信要求(RTS)命令を送信します．
 * 
 * Returns:
 *     Result
 */
Result MCP2515Handler::transmitTXB2()
{
    this->instruct(Instruction::RTS_TXB2);
    return Result::OK;
}
