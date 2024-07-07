//#include "platform.h"
//#include "xil_io.h"
//#include "xil_printf.h"
//#include "xparameters.h"
//
//#define ADDR_CANCION XPAR_CANCION_0_S00_AXI_BASEADDR
//#define ADDR_BUZZER XPAR_BUZZER_0_S00_AXI_BASEADDR
//
//u32 notas[12] = {
//			  261,
//			  277,
//			  294,
//			  311,
//			  330,
//			  349,
//			  370,
//			  392,
//			  415,
//			  440,
//			  466,
//			  494
//};
//
//u32 partitura[3] = {
//		0x44445577,
//		0x77554422,
//		0x00002244
//};
//
//int main() {
//	init_platform();
//
//	sleep(10);
//
//	xil_printf("Cargando cancion...\n\r");
//	for (int i = 0; i < 12; i++) {
//		Xil_Out32(ADDR_CANCION + i*4, (u32)100000000/notas[i]);
//	}
//
//	xil_printf("Cargando partitura...\n\r");
//	for (int i = 0; i < 3; i++) {
//		Xil_Out32(ADDR_CANCION + 12*4 + i*4, partitura[i]);
//	}
//
//	Xil_Out32(ADDR_BUZZER, 1); // Buzzer activo
//	Xil_Out32(ADDR_BUZZER + 4, 1); // Ciclo de trabajo 50%
//
//	return 0;
//}
