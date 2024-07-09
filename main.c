#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "platform.h"
//#include "xil_printf.h"
#include "xparameters.h"
#include "xgpio.h"
#include "xstatus.h"
#include "Delay.h"
//#include "LCD_SPI.h"
#include "LCD_GUI.h"
#include "ADC.h"
#include "I2C.h"
#include "xtmrctr.h"
#include "game_of_life.h"
#include "xscugic.h"
#include "xil_exception.h"

// Macros
#define INTC_DEVICE_ID XPAR_PS7_SCUGIC_0_DEVICE_ID
#define TMR_DEVICE_ID0 XPAR_TMRCTR_0_DEVICE_ID
#define INTC_TMR_INTERRUPT_ID_0 XPAR_FABRIC_AXI_TIMER_0_INTERRUPT_INTR
#define TMR_DEVICE_ID1 XPAR_TMRCTR_1_DEVICE_ID
#define INTC_TMR_INTERRUPT_ID_1 XPAR_FABRIC_AXI_TIMER_1_INTERRUPT_INTR

#define LIGHT_INT 61U

#define ADDR_CANCION XPAR_NEW_CANCION_0_S00_AXI_BASEADDR
#define ADDR_BUZZER XPAR_NEW_BUZZER_0_S00_AXI_BASEADDR

const u32 notas[12] = {
		261,
		294,
		294,
		440,
		330,
		349,
		523,
		392,
		587,
		659,
		698,
		494
};

// Variables
int ITERATIONS = 0;

XScuGic INTCInst;

XTmrCtr TMRInst0;
uint32_t TMR_LOAD0 = 0xFF67697F; // ~100ms

XTmrCtr TMRInst1;
uint32_t TMR_LOAD1 = 0xFFE17B7F; // ~20ms

extern XGpio gpio0;
XGpio gpio1;
extern XSpi SpiInstance;
extern XSpi SpiInstance1;

uint16_t pot_x;
uint16_t pot_y;
uint16_t prev_x = 0;
uint16_t prev_y = 0;

int acy;
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
void printMenuGui();
void printEditGui();
void printSimGui();
void printGOGui();
void printPauseGui();

void cargar_cancion(u32 *secuencia);
void buzzer_start();
void buzzer_stop();

static int InterruptSystemSetup(XScuGic *XScuGicInstancePtr);
static int IntcInitFunction(u16 DeviceId, XTmrCtr *TmrInstancePtr, XTmrCtr *TmrInstancePtr1);

void TMR_Intr_Handler_0(void *data);
void TMR_Intr_Handler_1(void *data);
void OPT_Intr_Handler(void *data);

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

	// Initialize Timer 1
	Status = XTmrCtr_Initialize(&TMRInst1, TMR_DEVICE_ID1);
	if (Status != XST_SUCCESS) {
		xil_printf("Timer 1 Initialization failed\r\n");
		return XST_FAILURE;
	}
	XTmrCtr_SetHandler(&TMRInst1, (XTmrCtr_Handler) TMR_Intr_Handler_1, &TMRInst1);
	XTmrCtr_SetResetValue(&TMRInst1, 0, TMR_LOAD1);
	XTmrCtr_SetOptions(&TMRInst1, 0, XTC_INT_MODE_OPTION | XTC_AUTO_RELOAD_OPTION);

	// Setup IIC
	Status = init_IIC();
	if (Status != XST_SUCCESS) {
		xil_printf("IIC initialization failed\r\n");
		return XST_FAILURE;
	}
	setup_opt();

	// Initialize Interrupt Controller
	Status = IntcInitFunction(INTC_DEVICE_ID, &TMRInst0, &TMRInst1);
	if (Status != XST_SUCCESS) {
		xil_printf("Interrupt controller initialization failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("**********Init LCD**********\r\n");
	LCD_SCAN_DIR LCD_ScanDir = SCAN_DIR_DFT;//SCAN_DIR_DFT = D2U_L2R
	LCD_Init(LCD_ScanDir );

	xil_printf("LCD Show \r\n");
	LCD_Clear(GUI_BACKGROUND);

	GUI_DisString_EN(20, 30, "Ingrese", &Font16, GUI_BACKGROUND, WHITE);
	GUI_DisString_EN(20, 45, "Cancion", &Font16, GUI_BACKGROUND, WHITE);

	char buf[64];
	uint32_t orden_notas[3];
	xil_printf("Ingrese la secuencia de notas: ");
	scanf("%s", buf);

	char hex[30];
	strncpy (hex, buf, 8);
	hex[8] = '\0';   /* null character manually added */
	orden_notas[0] = (uint32_t) strtoull(hex, NULL, 16);
	xil_printf("\r\n%lu\r\n", orden_notas[0]);

	strncpy (hex, buf + 8, 8);
	hex[8] = '\0';   /* null character manually added */
	orden_notas[1] = (uint32_t) strtoull(hex, NULL, 16);
	xil_printf("%lu\r\n", orden_notas[1]);

	strncpy (hex, buf + 16, 8);
	hex[8] = '\0';   /* null character manually added */
	orden_notas[2] = (uint32_t) strtoull(hex, NULL, 16);
	xil_printf("%lu\r\n", orden_notas[2]);

	cargar_cancion(orden_notas);

	LCD_Clear(GUI_BACKGROUND);

	XTmrCtr_Start(&TMRInst0, 0);
	XTmrCtr_Start(&TMRInst1, 0);

	while (1) {
		switch (GameState) {
		case Menu:
			if (mic > 1000) {
				LCD_Clear(GUI_BACKGROUND);
				GameState = Edit;
			} else {
				printMenuGui();
			}
			break;

		case Edit:
			printEditGui();
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
			printSimGui();
			if (btn1 == 0) {
				update_game_state();
				ITERATIONS++;

				if (check_empty_grid()) {
					LCD_Clear(GUI_BACKGROUND);

					GameState = GameOver;
				}
			}
			millisleep(100);
			break;

		case GameOver:
			printGOGui();
			buzzer_start();
			millisleep(5000);
			buzzer_stop();
			LCD_Clear(GUI_BACKGROUND);
			GameState = Menu;
			break;

		case Pause:
			printPauseGui();
			if (btn2 == 0) {
				print_game_state();
				GameState = Simulation;
			}
			break;
		}
	}
}

void printMenuGui() {
	GUI_DisString_EN(40, 30, "Game", &Font16, GUI_BACKGROUND, WHITE);
	GUI_DisString_EN(50, 45, "of", &Font16, GUI_BACKGROUND, WHITE);
	GUI_DisString_EN(40, 60, "Life", &Font16, GUI_BACKGROUND, WHITE);
}

void printEditGui() {
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
};

void printSimGui() {
	print_game_state_2();
}

void printGOGui() {
	GUI_DisString_EN(40, 50, "GAME", &Font16, GUI_BACKGROUND, WHITE);
	GUI_DisString_EN(40, 65, "OVER", &Font16, GUI_BACKGROUND, WHITE);
}

void printPauseGui() {
	GUI_DisString_EN(35, 55, "PAUSA", &Font16, GUI_BACKGROUND, WHITE);
}

void TMR_Intr_Handler_0(void *data) {
	if (XTmrCtr_IsExpired(&TMRInst0, 0)) {
		XTmrCtr_Stop(&TMRInst0, 0);

		btn1 = XGpio_DiscreteRead(&gpio1, 1) & 0x1;
		btn2 = XGpio_DiscreteRead(&gpio1, 1) & 0x2;

		XTmrCtr_Reset(&TMRInst0, 0);
		XTmrCtr_Start(&TMRInst0, 0);
	}
}

void TMR_Intr_Handler_1(void *data) {
	if (XTmrCtr_IsExpired(&TMRInst1, 0)) {
		XTmrCtr_Stop(&TMRInst1, 0);

		pot_x = read_POT1();
		pot_y = read_POT2();
		acy = read_acy();
		mic = read_MIC();

		XTmrCtr_Reset(&TMRInst1, 0);
		XTmrCtr_Start(&TMRInst1, 0);
	}
}

void OPT_Intr_Handler(void *data) {
	GameState = Pause;
	reset_opt();
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
static int IntcInitFunction(u16 DeviceId, XTmrCtr *TmrInstancePtr, XTmrCtr *TmrInstancePtr1) {
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
	XScuGic_SetPriorityTriggerType(&INTCInst, INTC_TMR_INTERRUPT_ID_1, 0x20, 0x1);
	XScuGic_SetPriorityTriggerType(&INTCInst, LIGHT_INT, 0x22, 0x1);

	// Connect timer 0 interrupt to handler
	status = XScuGic_Connect(&INTCInst,
			INTC_TMR_INTERRUPT_ID_0,
			(Xil_ExceptionHandler) TMR_Intr_Handler_0,
			(void *) TmrInstancePtr);
	if (status != XST_SUCCESS) {
		xil_printf("timer 0 setup failed\r\n");
		return XST_FAILURE;
	}

	status = XScuGic_Connect(&INTCInst,
				INTC_TMR_INTERRUPT_ID_1,
				(Xil_ExceptionHandler) TMR_Intr_Handler_1,
				(void *) TmrInstancePtr1);
	if (status != XST_SUCCESS) {
		xil_printf("timer 1 setup failed\r\n");
		return XST_FAILURE;
	}

	status = XScuGic_Connect(&INTCInst,
				LIGHT_INT,
				(Xil_ExceptionHandler) OPT_Intr_Handler,
				(void *) 0);
	if (status != XST_SUCCESS) {
		xil_printf("light sensor setup failed\r\n");
		return XST_FAILURE;
	}

	// Enable timer interrupts in the controller
	XScuGic_Enable(&INTCInst, INTC_TMR_INTERRUPT_ID_0);
	XScuGic_Enable(&INTCInst, INTC_TMR_INTERRUPT_ID_1);
	XScuGic_Enable(&INTCInst, LIGHT_INT);

	return XST_SUCCESS;
}

// Buzzer
void cargar_cancion(u32 *secuencia) {
	for (int i = 0; i < 12; i++) {
		Xil_Out32(ADDR_CANCION + i*4, (u32)100000000/notas[i]);
	}

	for (int i = 0; i < 3; i++) {
		Xil_Out32(ADDR_CANCION + 12*4 + i*4, secuencia[i]);
	}

	Xil_Out32(ADDR_BUZZER + 4, 1); // Ciclo de trabajo 50%
}

void buzzer_start() {
	Xil_Out32(ADDR_CANCION + 15*4, 0xFFFFFFFF);
}

void buzzer_stop() {
	Xil_Out32(ADDR_CANCION + 15*4, 0);
}
