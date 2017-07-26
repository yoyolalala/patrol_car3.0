#ifndef __LED_H
#define __LED_H
#include "sys.h"
#include "delay.h"

#define LED PAout(6)
#define raspberryLED PDout(4)

void LED_Init(void);	
void rasLED_Init(void);
void ledToggle();
void rasLedToggle();
#endif
