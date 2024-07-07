#include <stdio.h>
#include <sleep.h>
#include <time.h>
#include <unistd.h>

#include "platform.h"
#include "xil_printf.h"
#include "xparameters.h"
#include "xgpio.h"
#include "xstatus.h"
#include "Delay.h"
#include "LCD_SPI.h"
#include "LCD_Driver.h"
#include "LCD_GUI.h"
#include "ADC.h"
#include "I2C.h"


extern XGpio gpio0;
extern XSpi  SpiInstance;	 /* The instance of the SPI device */
extern XSpi  SpiInstance1;
extern const unsigned char font[] ;

#define BACKGROUND  WHITE
#define FOREGROUND BLUE
#define DELAY 1000


int main()
{
	int Status;

    //Initialize the UART
    init_platform();
	/* Initialize the GPIO 0 driver */
	Status = XGpio_Initialize(&gpio0, XPAR_AXI_GPIO_0_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("Gpio 0 Initialization Failed\r\n");
		return XST_FAILURE;
	}

	// Set up the AXI SPI Controller 0 (Screen)
	Status = XSpi_Init(&SpiInstance,SPI_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("SPI Mode Failed\r\n");
		return XST_FAILURE;
	}
	// Set up the AXI SPI Controller 0 (Joystick(x,y), accelerometer, potentiometer, mic)
	Status = init_adc(&SpiInstance1, SPI_DEVICE_ID_1);
	if (Status != XST_SUCCESS) {
		xil_printf("SPI-ADC Mode Failed\r\n");
		return XST_FAILURE;
	}
	// Set up the AXI IIC Controller 0 (temperature sensor, light sensor)
	Status = init_IIC();
	if (Status != XST_SUCCESS) {
		xil_printf("IIC Mode Failed\r\n");
		return XST_FAILURE;
	}
	//Write through UART to PC
	xil_printf("TFT initialized \r\n");
	xil_printf("**********Init LCD**********\r\n");
	// Init screen
	LCD_SCAN_DIR LCD_ScanDir = SCAN_DIR_DFT;//SCAN_DIR_DFT = D2U_L2R
	LCD_Init(LCD_ScanDir );
	// Default intro image from screen company
	xil_printf("LCD Show \r\n");

	GUI_Show();
	delay_ms(500);
	LCD_Clear(GUI_BACKGROUND);
	// Default intro image given by SEP course.
	GUI_INTRO();
	delay_ms(500);
	LCD_Clear(GUI_BACKGROUND);
	//Define arrays for joystick, accelerometer, temperatura and ligth sensors.
	// The ADC transform the data in 10bits, for temperatura and light sensors the data lenght is 16bits. For simplicity all
	// arrays are defined as 16bits.
	char joyx[16] = {};
	char joyy[16] = {};
	char acx[16] = {};
	char acy[16] = {};
	char acz[16] = {};
	char tmp[16] = {};
	char opt[16] = {};
	char pot1[16] = {};
	char pot2[16] = {};
	char mic[16] = {};
	while(1){

	/*

	// Here we write in black the space where the data will be written. It is like "clear" the space.
	GUI_DisString_EN(5,40,joyx,&Font16,GUI_BACKGROUND,GUI_BACKGROUND);
	GUI_DisString_EN(5,100,joyy,&Font16,GUI_BACKGROUND,GUI_BACKGROUND);
	GUI_DisString_EN(70,40,tmp,&Font16,GUI_BACKGROUND,GUI_BACKGROUND);
	GUI_DisString_EN(70,100,opt,&Font16,GUI_BACKGROUND,GUI_BACKGROUND);

	// Function "sprintf" copy the value of "read_XXXX()" into the value of the array defined.
	// Here we read joystick , temperature sensor, light sensor and accelerometer.
	sprintf(joyx, "%d", read_joyx());
	sprintf(joyy, "%d", read_joyy());
	sprintf(tmp, "%d", read_tmp());
	sprintf(opt, "%i", read_opt());
	//sprintf(acx, "%d", read_acx());
	//sprintf(acy, "%d", read_acy());
	//sprintf(acz, "%d", read_acz());

	// GUI_DisString_EN() is the function to draw a string in the screen.
	// Here we write the text "Ejex", "EjeY", "Temp", "Luz"
	GUI_DisString_EN(5,10,"Ejex",&Font16,GUI_BACKGROUND,CYAN);
	GUI_DisString_EN(5,70,"EjeY",&Font16,GUI_BACKGROUND,CYAN);
	GUI_DisString_EN(70,10,"Temp",&Font16,GUI_BACKGROUND,CYAN);
	GUI_DisString_EN(70,70,"Luz",&Font16,GUI_BACKGROUND,CYAN);
	// Here we write the value which is constantly refresh. Position is the same as where we previously put in black.
	GUI_DisString_EN(5,40,joyx,&Font16,GUI_BACKGROUND,YELLOW);
	GUI_DisString_EN(5,100,joyy,&Font16,GUI_BACKGROUND,YELLOW);
	GUI_DisString_EN(70,40,tmp,&Font16,GUI_BACKGROUND,YELLOW);
	GUI_DisString_EN(70,100,opt,&Font16,GUI_BACKGROUND,YELLOW);
*/
// GUI_DisString_EN() is the function to draw a string in the screen.
// Here we write the text "Ejex", "EjeY", "Temp", "Luz",etc.
GUI_DisString_EN(5,10,"Ejex",&Font12,GUI_BACKGROUND,CYAN);
GUI_DisString_EN(5,45,"EjeY",&Font12,GUI_BACKGROUND,CYAN);
GUI_DisString_EN(50,10,"Temp",&Font12,GUI_BACKGROUND,CYAN);
GUI_DisString_EN(50,45,"Luz",&Font12,GUI_BACKGROUND,CYAN);
GUI_DisString_EN(95,10,"POT1",&Font12,GUI_BACKGROUND,CYAN);
GUI_DisString_EN(95,45,"POT2",&Font12,GUI_BACKGROUND,CYAN);
GUI_DisString_EN(5,80,"ACX",&Font12,GUI_BACKGROUND,CYAN);
GUI_DisString_EN(50,80,"ACY",&Font12,GUI_BACKGROUND,CYAN);
GUI_DisString_EN(95,80,"MIC",&Font12,GUI_BACKGROUND,CYAN);

// Here we overwrite in black the value of the data. It is like "clear" the space.(as background is black)
GUI_DisString_EN(5,30,joyx,&Font12,GUI_BACKGROUND,GUI_BACKGROUND);
GUI_DisString_EN(5,65,joyy,&Font12,GUI_BACKGROUND,GUI_BACKGROUND);
GUI_DisString_EN(50,30,tmp,&Font12,GUI_BACKGROUND,GUI_BACKGROUND);
GUI_DisString_EN(50,65,opt,&Font12,GUI_BACKGROUND,GUI_BACKGROUND);
GUI_DisString_EN(95,30,pot1,&Font12,GUI_BACKGROUND,GUI_BACKGROUND);
GUI_DisString_EN(95,65,pot2,&Font12,GUI_BACKGROUND,GUI_BACKGROUND);
GUI_DisString_EN(5,100,acx, &Font12,GUI_BACKGROUND,GUI_BACKGROUND);
GUI_DisString_EN(50,100,acy, &Font12,GUI_BACKGROUND,GUI_BACKGROUND);
GUI_DisString_EN(95,100,mic, &Font12,GUI_BACKGROUND,GUI_BACKGROUND);

// Function "sprintf" copy the value of "read_XXXX()" into the value of the array defined.
// Here we read joystick , temperature sensor, light sensor, accelerometer, etc.
sprintf(joyx, "%d", read_joyx());
sprintf(joyy, "%d", read_joyy());
sprintf(tmp, "%d", read_tmp());
sprintf(opt, "%d", read_opt());
sprintf(pot1, "%d", read_POT1());
sprintf(pot2, "%d", read_POT2());
sprintf(acx, "%d", read_acx());
sprintf(acy, "%d", read_acy());
sprintf(mic, "%d", read_MIC());

// Here we write the value which is constantly refresh.
// Position is the same as where we previously put in black.
GUI_DisString_EN(5,30,joyx,&Font12,GUI_BACKGROUND,YELLOW);
GUI_DisString_EN(5,65,joyy,&Font12,GUI_BACKGROUND,YELLOW);
GUI_DisString_EN(50,30,tmp,&Font12,GUI_BACKGROUND,YELLOW);
GUI_DisString_EN(50,65,opt,&Font12,GUI_BACKGROUND,YELLOW);
GUI_DisString_EN(95,30,pot1,&Font12,GUI_BACKGROUND,YELLOW);
GUI_DisString_EN(95,65,pot2,&Font12,GUI_BACKGROUND,YELLOW);
GUI_DisString_EN(5,100,acx, &Font12,GUI_BACKGROUND,YELLOW);
GUI_DisString_EN(50,100,acy, &Font12,GUI_BACKGROUND,YELLOW);
GUI_DisString_EN(95,100,mic, &Font12,GUI_BACKGROUND,YELLOW);
	//We send through UART the data for each reading. This allows us to see what is happening inside the uP.
	xil_printf("JX :%d\n", read_joyx());
	xil_printf("JY :%d\n", read_joyy());
	xil_printf("ACX :%d\n", read_acx());
	xil_printf("ACY :%d\n", read_acy());
	xil_printf("ACZ :%d\n", read_acz());
	xil_printf("MIC :%d\n", read_MIC());
	xil_printf("POT1 :%d\n", read_POT1());
	xil_printf("POT2 :%d\n", read_POT2());
	xil_printf("Luz :%i\n", read_opt());
	xil_printf("\n");
	delay_ms(20);
	}
    return 0;
}
