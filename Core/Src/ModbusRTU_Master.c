/*
 * ModbusRTU_Master.c
 *
 *  Created on: Feb 17, 2022
 *      Author: Ümit Can Güveren
 */

#include <ModbusRTU_Master.h>
#include "main.h"


extern UART_HandleTypeDef huart2;
uint8_t BytetoBit[8] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};


funcState_t sendMessage(char *msg, uint16_t len)
{
	uartState_t stateTx;

	stateTx = HAL_UART_Transmit_IT(&huart2, (uint8_t *)msg, len);

	if(stateTx == uartOkState)
	{
		return funcOk;
	}
	else
	{
		return funcErr;
	}
}

funcState_t takeMessage(char *msg, uint16_t len, uint8_t SlaveID)
{
	uartState_t stateRx;
	crcState_t crcCheck;
	uint16_t CRCVal, rxCRC;

	stateRx = HAL_UART_Receive(&huart2, (uint8_t *)msg, len, 100);


	rxCRC = (msg[len -1] << 8) | (msg[len - 2]);
	CRCVal = MODBUS_CRC16(msg, len - 2);

	if(CRCVal == rxCRC)
	{
		crcCheck = crcOk;
	}
	else
	{
		crcCheck = crcErr;
	}

	if(stateRx == uartOkState && crcCheck == crcOk && msg[0] == SLAVEID)
	{
		return funcOk;
	}
	else
	{
		return funcErr;
	}

}


/*
 * @brief Modbus_ReadHoldingRegister, Reads Holding Register #0x03
 *
 * @param SlaveID = Slave address to be read
 *
 * @param StartAddress = First address of the register to be read
 *
 * @param NumberOfReg = Number of register to be read
 *
 * @funcState_t returns funcOk if the received data CRC and SlaveID is true after the sent data
 */
funcState_t Modbus_ReadHoldingRegister(uint8_t SlaveID, uint16_t StartAddress, uint16_t NumberOfReg)
{
	uint16_t CRCVal = 0;
	funcState_t funcTxState, funcRxState;
	uint8_t i, m = 0;

	char ModbusRx[BUFFERSIZE];
	char ModbusTx[8];

	memset(ModbusTx, 0, 8);

	ModbusTx[0] = SlaveID;
	ModbusTx[1] = ReadHoldingRegister;
	ModbusTx[2] = StartAddress >> 8;
	ModbusTx[3] = (uint8_t)(StartAddress & 0x00FF);
	ModbusTx[4] = NumberOfReg >> 8;
	ModbusTx[5] = (uint8_t)(NumberOfReg & 0x00FF);

	CRCVal = MODBUS_CRC16(ModbusTx, 6);

	ModbusTx[6] = (uint8_t)(CRCVal & 0x00FF);
	ModbusTx[7] = CRCVal >> 8;

	funcTxState = sendMessage(&ModbusTx[0], 8);

	if(funcTxState == funcOk)
	{
		funcRxState = takeMessage(ModbusRx, 5 + (NumberOfReg * 2), SlaveID);
	}
	if(funcRxState == funcOk)
	{
		for(i = 0; i < NumberOfReg; i++)
		{
			ModbusRegister[StartAddress + i] = (uint16_t)((ModbusRx[3 + m] << 8) | (ModbusRx[4 + m]));
			m += 2;
		}
	}

	return funcRxState;
}

/*
 * @brief Modbus_ReadHoldingRegister, Reads Holding Register #0x03
 *
 * @param SlaveID = Slave address to be read
 *
 * @param RegAddress = Address of the register to be write
 *
 * @param RegValue = Value to be write to register
 *
 * @funcState_t returns funcOk if the received data CRC and SlaveID is true after the sent data
 */
funcState_t Modbus_WriteSingleRegister(uint8_t SlaveID, uint16_t RegAddress, uint16_t RegValue)
{
	uint16_t CRCVal = 0;
	funcState_t funcTxState, funcRxState;

	char ModbusRx[BUFFERSIZE];
	char ModbusTx[8];

	memset(ModbusTx, 0, 8);

	ModbusTx[0] = SlaveID;
	ModbusTx[1] = WriteSingleRegister;
	ModbusTx[2] = (uint8_t)(RegAddress >> 8);
	ModbusTx[3] = (uint8_t)(RegAddress & 0x00FF);
	ModbusTx[4] = (uint8_t)(RegValue >> 8);
	ModbusTx[5] = (uint8_t)(RegValue & 0x00FF);

	CRCVal = MODBUS_CRC16(ModbusTx, 6);

	ModbusTx[6] = (uint8_t)(CRCVal & 0x00FF);
	ModbusTx[7] = CRCVal >> 8;

	funcTxState = sendMessage(&ModbusTx[0], 8);

	if(funcTxState == funcOk)
	{
		funcRxState = takeMessage(ModbusRx, 8, SlaveID);
	}

	return funcRxState; //CRC and ID is true.
}


/*
 * @brief Modbus_ReadCoil, Reads Coils #0x01
 *
 * @param SlaveID = Slave address to be read
 *
 * @param StartAddress = First address of the coil to be read
 *
 * @param NumberOfCoil = Number of coils to be read
 *
 * @funcState_t returns funcOk if the received data CRC and SlaveID is true after the sent data
 */
funcState_t Modbus_ReadCoil(uint8_t SlaveID, uint16_t StartAddress, int16_t NumberOfCoil)
{
	uint16_t CRCVal = 0, Modbus_i, ByteCounter = 0, BitCounter = 0;

	funcState_t funcTxState, funcRxState;
	char ModbusRx[BUFFERSIZE];
	char ModbusTx[8];

	uint8_t NumberOfByte;

	NumberOfByte = findByte(NumberOfCoil);

	ModbusTx[0] = SlaveID;
	ModbusTx[1] = ReadCoil;
	ModbusTx[2] = (uint8_t)(StartAddress >> 8);
	ModbusTx[3] = (uint8_t)(StartAddress & 0x00FF);
	ModbusTx[4] = (uint8_t)(NumberOfCoil >> 8);
	ModbusTx[5] = (uint8_t)(NumberOfCoil & 0x00FF);

	CRCVal = MODBUS_CRC16(ModbusTx, 6);

	ModbusTx[6] = (uint8_t)(CRCVal & 0x00FF);
	ModbusTx[7] = CRCVal >> 8;

	funcTxState = sendMessage(&ModbusTx[0], 8);

	if(funcTxState == funcOk)
	{
		funcRxState = takeMessage(ModbusRx, 5 + NumberOfByte, SlaveID);
	}

	if(funcRxState == funcOk)
	{
		for(Modbus_i = 0; Modbus_i < NumberOfCoil; Modbus_i++)
		{
			ModbusCoil[StartAddress + Modbus_i] = (bool)(ModbusRx[3 + ByteCounter] & BytetoBit[BitCounter]);

			BitCounter++;

			if(Modbus_i % 8 == 7)
			{
				ByteCounter++;
				BitCounter = 0;
			}
		}
	}
	return funcRxState;
}

/*
 * @brief Modbus_WriteSingleCoil, Writes Holding Register #0x05
 *
 * @param SlaveID = Slave address to be read
 *
 * @param RegAddress = First address of the coil to be write
 *
 * @param CoilState = coilOn or coilOff @def group coilState_t
 *
 * @funcState_t returns funcOk if the received data CRC and SlaveID is true after the sent data
 */
funcState_t Modbus_WriteSingleCoil(uint8_t SlaveID, uint16_t RegAddress, coilState_t CoilState)
{
	uint16_t CRCVal = 0;

	funcState_t funcTxState, funcRxState;
	char ModbusRx[BUFFERSIZE];
	char ModbusTx[8];

	ModbusTx[0] = SlaveID;
	ModbusTx[1] = WriteSingleCoil;
	ModbusTx[2] = (uint8_t)(RegAddress >> 8);
	ModbusTx[3] = (uint8_t)(RegAddress & 0x00FF);
	ModbusTx[4] = CoilState;
	ModbusTx[5] = 0x00;

	CRCVal = MODBUS_CRC16(ModbusTx, 6);

	ModbusTx[6] = (uint8_t)(CRCVal & 0x00FF);
	ModbusTx[7] = CRCVal >> 8;

	funcTxState = sendMessage(&ModbusTx[0], 8);

	if(funcTxState == funcOk)
	{
		funcRxState = takeMessage(ModbusRx, 8, SlaveID);
	}

	return funcRxState; //CRC and ID is true.
}

/*
 * @brief Modbus_WriteSingleCoil, Writes Holding Register #0x15
 *
 * @param SlaveID = Slave address to be read
 *
 * @param StartAddress = First address of the coil to be read
 *
 * @param NumberOfCoils = Number of coils to be write
 *
 * @param coilValue = Array of values to write to coils
 *
 * @funcState_t returns funcOk if the received data CRC and SlaveID is true after the sent data
 */
funcState_t Modbus_WriteMultipleCoils(uint8_t SlaveID, uint16_t StartAddress, uint16_t NumberOfCoils, char *coilValue)
{
	uint16_t CRCVal = 0;
	uint8_t i, NumberOfByte;
	funcState_t funcTxState, funcRxState;
	char ModbusRx[BUFFERSIZE];
	char ModbusTx[8];

	ModbusTx[0] = SlaveID;
	ModbusTx[1] = WriteMultipleCoils;
	ModbusTx[2] = (uint8_t)(StartAddress >> 8);
	ModbusTx[3] = (uint8_t)(StartAddress & 0x00FF);
	ModbusTx[4] = (uint8_t)(NumberOfCoils >> 8);
	ModbusTx[5] = (uint8_t)(NumberOfCoils & 0x00FF);

	NumberOfByte = findByte(NumberOfCoils - 1);

	ModbusTx[6] = NumberOfByte;

	for(i = 0; i < NumberOfByte ; i++)
	{
		ModbusTx[7 + i] = coilValue[i];
	}

	CRCVal = MODBUS_CRC16(ModbusTx, 8 + i);

	ModbusTx[8 + i] = (uint8_t)(CRCVal & 0x00FF);
	ModbusTx[9 + i] = CRCVal >> 8;

	funcTxState = sendMessage(&ModbusTx[0], 10 + i);

	if(funcTxState == funcOk)
	{
		funcRxState = takeMessage(ModbusRx, 8, SlaveID);
	}

	return funcRxState; //CRC and ID is true.
}

/*
 * @brief Modbus_WriteSingleCoil, Writes Holding Register #0x16
 *
 * @param SlaveID = Slave address to be read
 *
 * @param StartAddress = First address of the coil to be read
 *
 * @param NumberOfReg = Number of registers to be write
 *
 * @param regValue = Array of values to write to registers
 *
 * @funcState_t returns funcOk if the received data CRC and SlaveID is true after the sent data
 */
funcState_t Modbus_WriteMultipleRegisters(uint8_t SlaveID, uint16_t StartAddress, uint16_t NumberOfReg, uint16_t *regValue)
{
	uint16_t CRCVal = 0;
	uint8_t i, m = 0;
	funcState_t funcTxState, funcRxState;
	char ModbusRx[BUFFERSIZE];
	char ModbusTx[8];

	ModbusTx[0] = SlaveID;
	ModbusTx[1] = WriteMultipleRegisters;
	ModbusTx[2] = (uint8_t)(StartAddress >> 8);
	ModbusTx[3] = (uint8_t)(StartAddress & 0x00FF);
	ModbusTx[4] = (uint8_t)(NumberOfReg >> 8);
	ModbusTx[5] = (uint8_t)(NumberOfReg & 0x00FF);
	ModbusTx[6] = NumberOfReg * 2;

	for(i = 0; i < NumberOfReg * 2 ; i += 2)
	{
		ModbusTx[7 + i] = (uint8_t)(regValue[m] >> 8);
		ModbusTx[8 + i] = (uint8_t)(regValue[m] & 0x00FF);
		m++;
	}

	CRCVal = MODBUS_CRC16(ModbusTx, 8 + i);

	ModbusTx[8 + i] = (uint8_t)(CRCVal & 0x00FF);
	ModbusTx[9 + i] = CRCVal >> 8;

	funcTxState = sendMessage(&ModbusTx[0], 10 + i);

	if(funcTxState == funcOk)
	{
		funcRxState = takeMessage(ModbusRx, 8, SlaveID);
	}

	return funcRxState; //CRC and ID is true.
}


uint8_t findByte(int16_t NumberOfCoil)
{
	volatile uint8_t NumberOfByte = 0;

	while(NumberOfCoil >= 0)
	{
		NumberOfCoil -= 8;

		NumberOfByte++;
		if(NumberOfCoil < 0)
		{
			break;
		}
	}
	return NumberOfByte;
}

uint16_t MODBUS_CRC16(char *buf, uint8_t len )
{
	static const uint16_t table[2] = { 0x0000, 0xA001 };
	uint16_t crc = 0xFFFF;
	unsigned int i = 0;
	char bit = 0;
	unsigned int xor = 0;

	for( i = 0; i < len; i++ )
	{
		crc ^= buf[i];

		for( bit = 0; bit < 8; bit++ )
		{
			xor = crc & 0x01;
			crc >>= 1;
			crc ^= table[xor];
		}
	}

	return crc;
}
