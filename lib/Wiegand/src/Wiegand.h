#ifndef _WIEGAND_H
#define _WIEGAND_H

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

class WIEGAND {

public:
	WIEGAND();
	void begin();
	void begin(int pinD1,int pinD0);
	bool available();
	unsigned long getCode();
	int getWiegandType();
	static void ReadD0();
	static void ReadD1();

private:

	static bool DoWiegandConversion ();
	static unsigned long GetCardId (volatile unsigned long *codehigh, volatile unsigned long *codelow, char bitlength);

	static volatile unsigned long 	_cardTempHigh;
	static volatile unsigned long 	_cardTemp;
	static volatile unsigned long 	_lastWiegand;
	static volatile int				_bitCount;
	static int				_wiegandType;
	static unsigned long	_code;
};

#endif
