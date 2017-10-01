/*
 * Copyright (c) 2010 by Cristian Maglie <c.maglie@bug.st>
 * SPI Master library for arduino.
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of either the GNU General Public License version 2
 * or the GNU Lesser General Public License version 2.1, both as
 * published by the Free Software Foundation.
 */

#ifndef _SPI_H_INCLUDED
#define _SPI_H_INCLUDED

#include "variant.h"
#include <stdio.h>

// SPI_HAS_TRANSACTION means SPI has
//   - beginTransaction()
//   - endTransaction()
//   - usingInterrupt() -  NOT IMPLEMENTED
//   - SPISetting(clock, bitOrder, dataMode)
#define SPI_HAS_TRANSACTION 1

// SPI_HAS_EXTENDED_CS_PIN_HANDLING means SPI has automatic 
// CS pin handling and provides the following methods:
//   - begin(pin)
//   - end(pin)
//   - setBitOrder(pin, bitorder)
//   - setDataMode(pin, datamode)
//   - setClockDivider(pin, clockdiv)
//   - transfer(pin, data, SPI_LAST/SPI_CONTINUE)
//   - beginTransaction(pin, SPISettings settings) (if transactions are available)
#define SPI_HAS_EXTENDED_CS_PIN_HANDLING 1

#define SPI_MODE0 0x02
#define SPI_MODE1 0x00
#define SPI_MODE2 0x03
#define SPI_MODE3 0x01

enum SPITransferMode {
	SPI_CONTINUE,
	SPI_LAST
};

class SPISettings {
public:
	SPISettings(uint32_t clock, BitOrder bitOrder, uint8_t dataMode) {
		if (__builtin_constant_p(clock)) {
			init_AlwaysInline(clock, bitOrder, dataMode);
		} else {
			init_MightInline(clock, bitOrder, dataMode);
		}
	}
	SPISettings() { init_AlwaysInline(4000000, MSBFIRST, SPI_MODE0); }
private:
	void init_MightInline(uint32_t clock, BitOrder bitOrder, uint8_t dataMode) {
		init_AlwaysInline(clock, bitOrder, dataMode);
	}
	void init_AlwaysInline(uint32_t clock, BitOrder bitOrder, uint8_t dataMode) __attribute__((__always_inline__)) {
		border = bitOrder;
		uint8_t div;
		if (clock < (F_CPU / 255)) {
			div = 255;
		} else if (clock >= (F_CPU / 2)) {
			div = 2;
		} else {
			div = (F_CPU / (clock + 1)) + 1;
		}
		config = (dataMode & 3) | SPI_CSR_CSAAT | SPI_CSR_SCBR(div) | SPI_CSR_DLYBCT(1);
	}
	uint32_t config;
	BitOrder border;
	friend class SPIClass;
};

class SPIClass {
  public:
	SPIClass(Spi *_spi, uint32_t _id, void(*_initCb)(void));

	byte transfer(uint8_t _data, SPITransferMode _mode = SPI_LAST) { return transfer(BOARD_SPI_DEFAULT_SS, _data, _mode); }	
	byte transfer(byte _channel, uint8_t _data, SPITransferMode _mode = SPI_LAST);
	
	uint16_t transfer16(byte _pin, uint16_t _data, SPITransferMode _mode = SPI_LAST);
	uint16_t transfer16(uint16_t _data, SPITransferMode _mode = SPI_LAST) { return transfer16(BOARD_SPI_DEFAULT_SS, _data, _mode); }

	void transfer(byte _pin, void *_buf, size_t _count, SPITransferMode _mode = SPI_LAST);
	void transfer(void *_buf, size_t _count, SPITransferMode _mode = SPI_LAST) { transfer(BOARD_SPI_DEFAULT_SS, _buf, _count, _mode); }
	
	// SPI Configuration methods

	void attachInterrupt(void);
	void detachInterrupt(void);

	void begin(void);
	void end(void);

	// Attach/Detach pin to/from SPI controller
	void begin(uint8_t _pin);
	void end(uint8_t _pin);
	
	//Trying to make a pseudo begin/endtransaction functions
	void beginTransaction(SPISettings settings) { beginTransaction(BOARD_SPI_DEFAULT_SS, settings); }
	void beginTransaction(uint8_t pin, SPISettings settings);
	void endTransaction(uint8_t _pin);
	void endTransaction(void);

	// These methods sets a parameter on a single pin
	void setBitOrder(uint8_t _pin, BitOrder);
	void setDataMode(uint8_t _pin, uint8_t);
	void setClockDivider(uint8_t _pin, uint8_t);

	// These methods sets the same parameters but on default pin BOARD_SPI_DEFAULT_SS
	void setBitOrder(BitOrder _order) { setBitOrder(BOARD_SPI_DEFAULT_SS, _order); };
	void setDataMode(uint8_t _mode) { setDataMode(BOARD_SPI_DEFAULT_SS, _mode); };
	void setClockDivider(uint8_t _div) { setClockDivider(BOARD_SPI_DEFAULT_SS, _div); };

  private:
	void init();

	Spi *spi;
	uint32_t id;
	BitOrder bitOrder[SPI_CHANNELS_NUM];
	uint32_t divider[SPI_CHANNELS_NUM];
	uint32_t mode[SPI_CHANNELS_NUM];
	void (*initCb)(void);
	bool initialized;
};

#if SPI_INTERFACES_COUNT > 0
extern SPIClass SPI0;
#endif

#endif
