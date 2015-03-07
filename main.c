/*
 * read from and write to sd card
 * using the 43oh sd card booster pack
 * and RobG's 2.2" 176x220 lcd
 *
 */

#include "string.h"
#include "color.h"
#include "colorLCD.h"
#include "inc/hw_memmap.h"
#include "inc/hw_ssi.h"
#include "inc/hw_ints.h"
#include "inc/hw_types.h"
#include "driverlib/ssi.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "driverlib/uart.h"
#include "driverlib/systick.h"
#include "diskio.h"
#include "tff.h"
#include "xprintf.h"
#include "delay.h"
//#include "HaD2.h"


// interrupts
void spiRxIsr(void);
//void PortAIntIsr(void);
//void uartIsr(void);

//void watchDogIsr(void);
void sysTickIsr(void);


// function prototypes
//void uartInit(void);
void gpioInit(void);
//void ssiInit8(void);
void ssiInit16(void);
//void clearBuffer(void);
//void put_bigBuffer();
//void uartPutC(unsigned char c);   // function to write to UART0
unsigned char puke(BYTE);
void sysTickInit(void);
//void watchdogInit(void);


UCHAR RX_Counter = 0;      // counter for loading SPI rx data
BYTE uartString[10] = {0};
BYTE uartChar = 0;
INT x = 0;
INT y = 40;
BYTE status = 0;
LONG tt = 0;
ULONG sysTickValue = 0;
//UINT bigBuffer[4840] = {0};   // for full screen image/8
BYTE readBuffer[128] = {0};    //for text
BYTE testBuffer[] = {"1st write test to newFile"};
BYTE testBuffer2[] = {"2nd write test to newFile"};

int main(void) {

	//xdev_out(uartPutC);        //this sets the output stream for ChaN's xprintf to uart0
	xdev_out(LCD_output_func);  //this sets the output stream for ChaN's xprintf to RobG's 2.2" lcd
	FATFS fs;                  // Work area (file system object) for logical drive
	//DRESULT dres;            // FatFs function common result code
	FRESULT res;               // FatFs function common result code
	FIL file1,file2,file3;     // file objects
	WORD br, bw;               // File R/W count
	DWORD clust;               // # of clusters
	FATFS *pfs;                // pointer to a pointer to file system object
	DIR dir;				   // Directory object
	FILINFO fno;			   // File information object

	// Setup the system clock to run at 80Mhz from PLL with crystal reference
	SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ | SYSCTL_OSC_MAIN);


    //uartInit();           // initialize uart

	gpioInit();           // initialize GPIO's
	ssiInit16();          // initialize hardware to 16bit SPI
	initILI9225B();       // 2.2" LCD with touch
	sysTickInit();

	//drawImage(0,0,174,218,HaD2);
	//delay(1000000000);

    clearScreen(1);
	setColor16(COLOR_16_GREEN);
	setArea(0, 0, 0 + 4, 0 + 7);

	/*
    //xputs("initializing UART\n");
	xputs("initializing GPIO'S\n");
	xputs("initializing SSI\n");
	xputs("initializing LCD\n");
	xputs("initializing sysTick\n");
    */

	// initialize sd card
	xputs("initializing SD card\n");
	status = disk_initialize(0);
	xprintf("initialization status:%d\n", status);

	// mount the drive
	xputs("mount SD card\n");
	res = f_mount(0,&fs);
	xprintf("mount status:%d\n", res);
	if(res) puke(res);

	// open directory
	res = f_opendir(&dir, "root");
	xprintf("Open root directory:%d\n",res);
	if(res) puke(res);

	/*
	//delete files and directory
	res = f_unlink("/root/newFile.txt");
	if(res) puke(res);
	res = f_unlink("/root/root2");
	if(res) puke(res);
	xputs("Shits Gone!\n");

	// create directory
    res = f_mkdir("root/root2");
    xprintf("root2 created:%d\n",res);
    if(res) puke(res);

    */

	xprintf("Directory listing...\n");
		for (;;) {
			res = f_readdir(&dir, &fno);		/* Read a directory item */
			if (res || !fno.fname[0]) break;	/* Error or end of dir */
			if (fno.fattrib & AM_DIR)
				xprintf("   <dir>  %s\n", fno.fname);
			else
				xprintf("%6d  %s\n", fno.fsize, fno.fname);
     }

	// create a new file
    res = f_open(&file1, "/root/newFile.txt", FA_CREATE_ALWAYS | FA_WRITE | FA_READ);
	xprintf("file created:%X\n", res);
	if(res) puke(res);

	// write to the new file
	res = f_write(&file1, testBuffer, sizeof(testBuffer), &bw);
	xprintf("new file write status:%d\n", res);
    xprintf("writing file:\n%s", testBuffer);
    if(res) puke(res);

    // close the newly written file
    f_close(&file1);

    // open an existing file
    res = f_open(&file2, "/root/test.TXT", FA_READ);
    xprintf("\nfile opened status:%d\n", res);
    if(res) puke(res);

	// read the existing file
	res = f_read(&file2, readBuffer, sizeof(readBuffer), &br);
	// print the file contents to the uart or lcd
	xprintf("read back existing file:\n%s", readBuffer);
	if(res) puke(res);

	// clear the readBuffer
	//clearReadBuffer();

	// close the existing file
	f_close(&file2);

	// open the newFile for reading
	res = f_open(&file1, "/root/newFile.TXT", FA_OPEN_EXISTING | FA_READ | FA_WRITE);
	xprintf("\nnewFile opened status:%d\n", res);
	if(res) puke(res);

	// read the newFile
	res = f_read(&file1, readBuffer, sizeof(readBuffer), &br);
    // print the file contents to the uart or lcd
	xprintf("read back newFile:\n%s", readBuffer);
	if(res) puke(res);

	// close newFile
	f_close(&file1); // close the newly written file

	// clear the readBuffer
	//clearReadBuffer();

	// open the newFile for writing
	res = f_open(&file1, "/root/newFile.TXT", FA_OPEN_EXISTING | FA_WRITE);
	xprintf("\nnewFile opened status:%d\n", res);
	if(res) puke(res);

	// write to the newFile for the 2nd time
	res = f_write(&file1, testBuffer2, sizeof(testBuffer2), &bw);
	//xprintf("2nd write to newFile status:%d\n", res);
	xprintf("writing file:\n%s", testBuffer2);
	if(res) puke(res);

	// close newFile
	f_close(&file1);

	// open the newFile for reading for 2nd time
	res = f_open(&file1, "/root/newFile.TXT", FA_OPEN_EXISTING | FA_READ);
	xprintf("\nnewFile opened status:%d\n", res);
	if(res) puke(res);

	// read the newFile for the 2nd time
	res = f_read(&file1, readBuffer, sizeof(readBuffer), &br);
	// print the newFile contents to the uart or lcd
	xprintf("2nd read of newFile:\n%s", readBuffer);
	if(res) puke(res);

	//clearReadBuffer();

	// Close all files
	//f_close(&file2);
	f_close(&file1);
    xputs("\nfiles closed\n");

	xputs("computing disc space..........\n");
	// Get free clusters
	res = f_getfree("", &clust, &pfs);
	xprintf("%u KB total disk space.\n"
	           "%u KB available on the disk.\n",
	           (DWORD)(pfs->max_clust - 2) * pfs->sects_clust / 2,
	           clust * pfs->sects_clust / 2);

/*
	setColor16(COLOR_16_WHITE);
    //clearScreen(1);

    // open directory
    res = f_opendir(&dir, "root/root2");
    xprintf("Open root2 directory:%d\n",res);
    if(res) puke(res);

    // open the newFile for writing
    res = f_open(&file3, "/root/root2/HaD2.h", FA_OPEN_EXISTING | FA_READ);
    xprintf("HaD2.h opened status:%d\n", res);
    if(res) puke(res);


    //while(counter <= 4840)
    // read the newFile for the 2nd time
    res = f_read(&file3, bigBuffer, 4840, &br);
    // print the newFile contents to the uart or lcd
    xprintf("read HaD file:%s\n");
    if(res) puke(res);

    delay(100000000);


    clearScreen(1);
    setArea(0, 0, 176,220);
    put_bigBuffer(bigBuffer);

    //clearBuffer();



	xprintf("%s", "string\n");
	xputs("test\n");
	xputs("frameBuffer?\n");
*/

   // now do nothing
	while(1){

		//toggleRed;
		//toggleBlue;

	}
}

///////////////////functions////////////////////////////

/*
void uartInit(void){

	// enable UART0
	SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

	// enable port A
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

	// Set GPIO A0 and A1 as UART pins.
	GPIOPinConfigure(GPIO_PA0_U0RX);
	GPIOPinConfigure(GPIO_PA1_U0TX);
	GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    // Configure the UART for 9600, 8-N-1 operation.
	//UARTStdioInit(0);
	UARTConfigSetExpClk(UART0_BASE, 80000000, 9600,
	                            (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
	                            UART_CONFIG_PAR_NONE));

	UARTEnable(UART0_BASE);
	// enable the UART interrupt
	//UARTIntEnable(UART0_BASE, UART_INT_RX);
	//IntEnable(INT_UART0);

}
*/

void gpioInit(void){

	// Setup Gpio's these are defined in colorLCD.h
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF | SYSCTL_PERIPH_GPIOE | SYSCTL_PERIPH_GPIOA);

	// set the onboard leds as output
	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, redLed | greenLed | blueLed); //rgb led on launchpad

	// set LCD_DC_PIN as output
	GPIOPinTypeGPIOOutput(GPIO_PORTE_BASE, LCD_DC_PIN);

	// set TSC_CS_PIN as output
	//GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE, TSC_CS_PIN);

	// set PENIRQ as input
	//GPIOPinTypeGPIOInput(GPIO_PORTA_BASE, TSC_PENIRQ_PIN);

	// weak pull up on PENIRQ
	//GPIOPadConfigSet(GPIO_PORTA_BASE, TSC_PENIRQ_PIN, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

	// PENIRQ interrupt on falling edge
	//GPIOIntTypeSet(GPIO_PORTA_BASE, TSC_PENIRQ_PIN, GPIO_FALLING_EDGE);

	// clear the PENIRQ interrupt
	//GPIOPinIntClear(GPIO_PORTA_BASE, TSC_PENIRQ_PIN);

	// enable the interrupt
	//GPIOPinIntEnable(GPIO_PORTA_BASE, TSC_PENIRQ_PIN);

	redOff;
	blueOff;
	greenOff;
}

void ssiInit16(void) {

	// Enable the SSI2 peripheral.
	SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI2);

	// Enable GPIOB for SSI usage
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);

	// Configure the pin alternate function for SSI2 on pins PB4, PB5, PB6, and PB7.
	GPIOPinConfigure(GPIO_PB4_SSI2CLK);
	GPIOPinConfigure(GPIO_PB5_SSI2FSS);
	GPIOPinConfigure(GPIO_PB6_SSI2RX);
	GPIOPinConfigure(GPIO_PB7_SSI2TX);

	// Configure the GPIO settings for the SSI pins.
	GPIOPinTypeSSI(GPIO_PORTB_BASE,GPIO_PIN_5 | GPIO_PIN_4 | GPIO_PIN_7 | GPIO_PIN_6);

	// Configure and enable the SSI port for SPI master mode.  Use SSI2,
	// system clock supply, idle clock level low and active low clock in
	// freescale SPI mode, master mode, 20MHz SSI frequency, and 16-bit data.
	//this has the clock hard coded at 80Mhz and the bit rate at 20MHz
	//you can also use SysCtlClockGet() in place of the hard coded clock speed
	SSIConfigSetExpClk(SSI2_BASE, 80000000, SSI_FRF_MOTO_MODE_0, SSI_MODE_MASTER, 20000000, 16);

	// Enable the SSI2 module.
	SSIEnable(SSI2_BASE);
}
/*
void ssiInit8(void) {

	// Disable SSI2
    SSIDisable(SSI2_BASE);

	// Enable the SSI2 peripheral.
	SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI2);

	// Enable GPIOB for SSI usage
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);

	// Configure the pin alternate function for SSI2 on pins PB4, PB5, PB6, and PB7.
	GPIOPinConfigure(GPIO_PB4_SSI2CLK);
	GPIOPinConfigure(GPIO_PB5_SSI2FSS);
	GPIOPinConfigure(GPIO_PB6_SSI2RX);
	GPIOPinConfigure(GPIO_PB7_SSI2TX);

	// Configure the GPIO settings for the SSI pins.
	GPIOPinTypeSSI(GPIO_PORTB_BASE,GPIO_PIN_5 | GPIO_PIN_4 | GPIO_PIN_7 | GPIO_PIN_6);

	// Configure and enable the SSI port for SPI master mode.  Use SSI2,
	// system clock supply, idle clock level low and active low clock
	// freescale SPI mode, master mode, 2MHz SSI frequency, and 8-bit data.
	// this has the clock hard coded at 80Mhz and the bit rate at 2MHz
	// you can also use SysCtlClockGet() in place of the hard coded clock speed
	SSIConfigSetExpClk(SSI2_BASE, 80000000, SSI_FRF_MOTO_MODE_0, SSI_MODE_MASTER, 2000000, 8);

	// Enable the SSI interrupt
	SSIIntEnable(SSI2_BASE,SSI_RXFF);

	// Enable the SSI2 module.
	SSIEnable(SSI2_BASE);
}

// clear buffer for reading sd card
void clearBuffer(void){

	UINT i = 0;
	for(i = 0; i <= 4840; i++)
		bigBuffer[i] = 0;

	xputs("\nreadBuffer cleared\n");
}

// put the contents of the bigBuffer on the lcd
void put_bigBuffer(const UINT * bigBuffer){

	UINT i;
	for(i = 0; i <= LCD_WIDTH * LCD_HEIGHT; i++){
		writeData(bigBuffer[i]);
		}
}
*/
unsigned char puke(FRESULT res){

	xprintf("\nFailed with res = %u.\n", res);
	xputs("!!!!puke!!!!\n");
	while(1);
}

void sysTickInit(void){

    // set up sysTick period
	SysTickPeriodSet(SysCtlClockGet()/1000); // 1 ms interrupt
	//SysTickPeriodSet(14304*6);

	// Enable the SysTick Interrupt.
	SysTickIntEnable();

    // Enable SysTick.
	SysTickEnable();

}

void sysTickIsr(void){

	++tt;    //increment the timer

	if(tt == 1000){ // 1000 ms is 1 second
	//toggleGreen;  // toggle the green led so we know the interrupt is happening
	sysTickValue = SysTickPeriodGet();
	tt = 0;
	}

}

// interrupt handler for spi
void spiRxIsr(void) {

	unsigned long data;
	SSIIntClear(SSI2_BASE,SSI_RXFF); // clear the rx interrupt

	SSIDataGet(SSI2_BASE, &data);    // read the spi for position data

	//tempArray[RX_Counter] = data;

	if(RX_Counter <= 2)
		RX_Counter++;
	else
		RX_Counter = 0;

	//toggleBlue;

}
/*
// The UART interrupt handler.
void uartIsr(void)
{
	unsigned long ulStatus;

	// Get the interrupt status.
	ulStatus = UARTIntStatus(UART0_BASE, true);

	// Clear the asserted interrupts.
	UARTIntClear(UART0_BASE, ulStatus);

	toggleGreen;  // toggle the green led so we know the interrupt is happening

	//uartChar = UARTCharGet(UART0_BASE);
	uartChar = UARTgetc();

}

// interrupt handler for PEN IRQ
void PortAIntIsr(void){

	GPIOPinIntClear(GPIO_PORTA_BASE, TSC_PENIRQ_PIN);       // clear the PENIRQ interrupt
	//touch = 1;
	toggleRed;

}

// function to write to UATR0
void uartPutC(unsigned char c){

	UARTCharPut(UART0_BASE, c);
}

void scrollLCD(unsigned char x, unsigned char y){  // this gives the illusion of a scrolling terminal

}


*/



