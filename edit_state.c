//#include <stdint.h>
//#include "ADC.h"
//#include "game_of_life.c"
//
//
//int pot_x = 0;
//int pot_y = 0;
//
//uint16_t prev_x = 0;
//uint16_t prev_y = 0;
//
//void update_edit_state() {
//	pot_x = read_POT1();
//	pot_y = read_POT2();
//}
//
//int printEditGui() {
//	uint16_t pos_x = 64 - (uint16_t)((double)pot_x / 15.5);
//	uint16_t pos_y = 64 - (uint16_t)((double)pot_y / 15.5);
//
////	xil_printf("x: %d\t\t", pos_x);
////	xil_printf("y: %d\r\n", pos_y);
//
//
////
////	if ((uint16_t) pos_x != prev_x || (uint16_t) pos_y != prev_y) {
//	if (prev_x != pos_x || prev_y != pos_y){
//		LCD_SetPointlColor((uint16_t) 2*prev_x, (uint16_t) 2*prev_y, 0x0000 );
//		LCD_SetPointlColor((uint16_t) 2*prev_x + 1, (uint16_t) 2*prev_y, 0x0000 );
//		LCD_SetPointlColor((uint16_t) 2*prev_x, (uint16_t) 2*prev_y + 1, 0x0000 );
//		LCD_SetPointlColor((uint16_t) 2*prev_x + 1, (uint16_t) 2*prev_y + 1, 0x0000 );
//	}
//
//	if ((XGpio_DiscreteRead(&gpio1, 1) & 0x2) {
//		toggle_cell_state(pos_x, pos_y);
//
//		print_game_state();
//	}
//
//	LCD_SetPointlColor((uint16_t) 2*pos_x, (uint16_t) 2*pos_y, 0xF0F0 );
//	LCD_SetPointlColor((uint16_t) 2*pos_x + 1, (uint16_t) 2*pos_y, 0xF0F0 );
//	LCD_SetPointlColor((uint16_t) 2*pos_x, (uint16_t) 2*pos_y + 1, 0xF0F0 );
//	LCD_SetPointlColor((uint16_t) 2*pos_x + 1, (uint16_t) 2*pos_y + 1, 0xF0F0 );
//
//
////	}
//
//	prev_x = pos_x;
//	prev_y = pos_y;
////
////	if (CHANGE STATE BUTTON) {
////		return 1;
////	}
//
//	return 0;
//};
