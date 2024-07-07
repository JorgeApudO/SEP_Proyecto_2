#include <stdio.h>
#include <stdint.h>

#include "platform.h"
//#include "xil_printf.h"
#include "xparameters.h"
#include "xgpio.h"
#include "xstatus.h"
////#include "Delay.h"
//#include "LCD_SPI.h"
#include "LCD_GUI.h"
#include "ADC.h"
//#include "I2C.h"
#include "xtmrctr.h"
#include "game_of_life.h"
#include "xscugic.h"
#include "xil_exception.h"

// Macros
#define INTC_DEVICE_ID XPAR_PS7_SCUGIC_0_DEVICE_ID
#define TMR_DEVICE_ID0 XPAR_TMRCTR_0_DEVICE_ID
#define INTC_TMR_INTERRUPT_ID_0 XPAR_FABRIC_AXI_TIMER_0_INTERRUPT_INTR
//#define TMR_DEVICE_ID1 XPAR_TMRCTR_1_DEVICE_ID

// Variables
XScuGic INTCInst;

XTmrCtr TMRInst0;
uint32_t TMR_LOAD0 = 0xF8000000; // 1 seg?

XTmrCtr TMRInst1;

extern XGpio gpio0;
XGpio gpio1;
extern XSpi SpiInstance;
extern XSpi SpiInstance1;

uint16_t pot_x = 0;
uint16_t pot_y = 0;
uint16_t prev_x = 0;
uint16_t prev_y = 0;

int acy = 0;
int mic;

int btn1;
int btn2;

// Maquina de estados
enum ProgramState {
	Simulation,
	Pause,
	Edit,
	Menu,
	GameOver
};

enum ProgramState GameState = Menu;

// Functions primitives
int printMenuGui();
int printEditGui();
int printSimGui();
void printGOGui();

static int InterruptSystemSetup(XScuGic *XScuGicInstancePtr);
static int IntcInitFunction(u16 DeviceId, XTmrCtr *TmrInstancePtr);
void TMR_Intr_Handler_0_backup();

void TMR_Intr_Handler_0(void *data);

int main()
{
	int Status;

    //Initialize the UART
    init_platform();

    // Initialize GPIO 0 driver
    Status = XGpio_Initialize(&gpio0, XPAR_AXI_GPIO_0_DEVICE_ID);
    if (Status != XST_SUCCESS) {
    	xil_printf("Gpio 0 Initialization Failed\r\n");
    	return XST_FAILURE;
    }

    // Initialize GPIO 1 driver
    Status = XGpio_Initialize(&gpio1, XPAR_AXI_GPIO_1_DEVICE_ID);
    if (Status != XST_SUCCESS) {
		xil_printf("Gpio 1 Initialization Failed\r\n");
		return XST_FAILURE;
	}

    // Set up the AXI SPI Controller
    Status = XSpi_Init(&SpiInstance, SPI_DEVICE_ID);
    if (Status != XST_SUCCESS) {
    	xil_printf("SPI Mode Failed\r\n");
		return XST_FAILURE;
    }

    Status = init_adc(&SpiInstance1, SPI_DEVICE_ID_1);
	if (Status != XST_SUCCESS) {
		xil_printf("SPI-ADC Mode Failed\r\n");
		return XST_FAILURE;
	}
	xil_printf("TFT initialized \r\n");

	// Initialize Timer 0
	Status = XTmrCtr_Initialize(&TMRInst0, TMR_DEVICE_ID0);
	if (Status != XST_SUCCESS) {
		xil_printf("Timer 0 Initialization failed\r\n");
		return XST_FAILURE;
	}
	XTmrCtr_SetHandler(&TMRInst0, (XTmrCtr_Handler) TMR_Intr_Handler_0, &TMRInst0);
	XTmrCtr_SetResetValue(&TMRInst0, 0, TMR_LOAD0);
	XTmrCtr_SetOptions(&TMRInst0, 0, XTC_INT_MODE_OPTION | XTC_AUTO_RELOAD_OPTION);

	// Initialize Interrupt Controller
	Status = IntcInitFunction(INTC_DEVICE_ID, &TMRInst0);
	if (Status != XST_SUCCESS) {
		xil_printf("Interrupt controller initialization failed\r\n");
		return XST_FAILURE;
	}

	XTmrCtr_Start(&TMRInst0, 0);

	xil_printf("**********Init LCD**********\r\n");
	LCD_SCAN_DIR LCD_ScanDir = SCAN_DIR_DFT;//SCAN_DIR_DFT = D2U_L2R
	LCD_Init(LCD_ScanDir );

	xil_printf("LCD Show \r\n");
	LCD_Clear(GUI_BACKGROUND);

	char buf[64];
	xil_printf("Ingresa algun texto: ");
	scanf("%s", buf);
	if (strcmp(buf, "hola") == 0) {
		xil_printf("\r\nTexto ingresado: %s\r\n", buf);
	}

	while (1) {
		pot_x = read_POT1();
		pot_y = read_POT2();
		acy = read_acy();
		mic = read_MIC();

		btn1 = XGpio_DiscreteRead(&gpio1, 1) & 0x1;
		btn2 = XGpio_DiscreteRead(&gpio1, 1) & 0x2;

		TMR_Intr_Handler_0_backup();

		switch (GameState) {
		case Menu:
			if (mic > 1000) {
				LCD_Clear(GUI_BACKGROUND);
				GameState = Edit;
			}
			break;

		case Edit:
			if (acy < 350) {
				reset_grid();
				LCD_Clear(GUI_BACKGROUND);
			}
			if (btn2 == 0) {
				LCD_Clear(GUI_BACKGROUND);
				print_game_state();
				GameState = Simulation;
			}
			break;

		case Simulation:
			if (btn1 == 0) {
				update_game_state();

				if (check_empty_grid()) {
					LCD_Clear(GUI_BACKGROUND);
					GameState = GameOver;
				}
			}
			break;

		case GameOver:
			break;
		}
	}
}

void TMR_Intr_Handler_0_backup() {
	switch (GameState) {
		case Menu:
			if (printMenuGui() == 1) {
				GameState = Edit;
			}
			break;

		case Edit:
			if (printEditGui() == 1) {
				GameState = Simulation;
			}
			break;

		case Simulation:
			if (printSimGui() == 1) {
				GameState = GameOver;
			}
			break;

		case Pause:
//			printPauseGui();
			break;

		case GameOver:
			printGOGui();
			break;
	}
}

int printMenuGui() {
	GUI_DisString_EN(40, 30, "Game", &Font16, GUI_BACKGROUND, WHITE);
	GUI_DisString_EN(50, 45, "of", &Font16, GUI_BACKGROUND, WHITE);
	GUI_DisString_EN(40, 60, "Life", &Font16, GUI_BACKGROUND, WHITE);

	return 0;
}

int printEditGui() {
	uint16_t pos_x = 64 - (uint16_t)((double)pot_x / 15.5);
	uint16_t pos_y = 64 - (uint16_t)((double)pot_y / 15.5);

	if (prev_x != pos_x || prev_y != pos_y) {
		LCD_SetPointlColor((uint16_t) 2*prev_x, (uint16_t) 2*prev_y, 0x0000 );
		LCD_SetPointlColor((uint16_t) 2*prev_x + 1, (uint16_t) 2*prev_y, 0x0000 );
		LCD_SetPointlColor((uint16_t) 2*prev_x, (uint16_t) 2*prev_y + 1, 0x0000 );
		LCD_SetPointlColor((uint16_t) 2*prev_x + 1, (uint16_t) 2*prev_y + 1, 0x0000 );
	}

	if (btn1 == 0) {
		toggle_cell_state(pos_x, pos_y);
	}

	print_alive_cells();

	LCD_SetPointlColor((uint16_t) 2*pos_x, (uint16_t) 2*pos_y, 0xF0F0 );
	LCD_SetPointlColor((uint16_t) 2*pos_x + 1, (uint16_t) 2*pos_y, 0xF0F0 );
	LCD_SetPointlColor((uint16_t) 2*pos_x, (uint16_t) 2*pos_y + 1, 0xF0F0 );
	LCD_SetPointlColor((uint16_t) 2*pos_x + 1, (uint16_t) 2*pos_y + 1, 0xF0F0 );

	prev_x = pos_x;
	prev_y = pos_y;

	return 0;
};

int printSimGui() {
	print_game_state();

	return 0;
}

void printGOGui() {
	GUI_DisString_EN(40, 50, "GAME", &Font16, GUI_BACKGROUND, WHITE);
	GUI_DisString_EN(40, 65, "OVER", &Font16, GUI_BACKGROUND, WHITE);
}

void TMR_Intr_Handler_0(void *data) {
	if (XTmrCtr_IsExpired(&TMRInst0, 0)) {
		XTmrCtr_Stop(&TMRInst0, 0);
		xil_printf("Timer expired\r\n");
		XTmrCtr_Reset(&TMRInst0, 0);
		XTmrCtr_Start(&TMRInst0, 0);
	}
}

// Interrupt functions
static int InterruptSystemSetup(XScuGic *XScuGicInstancePtr) {
	// Enable Interrupt
	Xil_ExceptionRegisterHandler(
			XIL_EXCEPTION_ID_INT,
			(Xil_ExceptionHandler) XScuGic_InterruptHandler,
			XScuGicInstancePtr);

	Xil_ExceptionEnable();

	return XST_SUCCESS;
}
static int IntcInitFunction(u16 DeviceId, XTmrCtr *TmrInstancePtr) {
	XScuGic_Config *IntcConfig;
	int status;

	// Interrupt controller initialization
	IntcConfig = XScuGic_LookupConfig(DeviceId);
	status = XScuGic_CfgInitialize(&INTCInst, IntcConfig, IntcConfig->CpuBaseAddress);
	if (status != XST_SUCCESS) {
		xil_printf("Interrupt config failed\r\n");
		return XST_FAILURE;
	}

	// Call to interrupt controller setup
	status = InterruptSystemSetup(&INTCInst);
	if (status != XST_SUCCESS) {
		xil_printf("Interrupt system setup failed\r\n");
		return XST_FAILURE;
	}

	// Asignacion de prioridades
	XScuGic_SetPriorityTriggerType(&INTCInst, INTC_TMR_INTERRUPT_ID_0, 0x18, 0x1);

	// Connect timer 0 interrupt to handler
	status = XScuGic_Connect(&INTCInst,
			INTC_TMR_INTERRUPT_ID_0,
			(Xil_ExceptionHandler) TMR_Intr_Handler_0,
			(void *) TmrInstancePtr);
	if (status != XST_SUCCESS) {
		xil_printf("timer 0 setup failed\r\n");
		return XST_FAILURE;
	}

	// Enable timer interrupts in the controller
	XScuGic_Enable(&INTCInst, INTC_TMR_INTERRUPT_ID_0);

	return XST_SUCCESS;
}

