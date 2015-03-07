/*
 * delay.c
 *
 *  Created on: Oct 20, 2013
 *      Author: Tim
 */

//#include "integer.h"
//#include "inc/hw_sysctl.h"
//#include "driverlib/sysctl.h"

//delay
void delay(long x10ms) { // this delay uses the SysCtlDelay supplied in Stellarisware

	SysCtlDelay(x10ms);
}


