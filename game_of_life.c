#include <stdio.h>
#include <string.h>

//#include "LCD_Driver.h"

#define SIZE 64

enum Condition {
	Alive = 1,
	Dead = 0
};

enum Condition GRID[SIZE][SIZE] = {Dead};

void set_initial_conditions(enum Condition *new_state[SIZE]) {
	memcpy(GRID, new_state, SIZE*SIZE);
}

void reset_grid() {
	memset(GRID, 0, SIZE*SIZE);
}

void update_game_state() {
	xil_printf("Started cycle\r\n");
	enum Condition future[SIZE][SIZE] = {Dead};

	// Iterar por todas las celdas
	for (int i = 0; i < SIZE; i++) {
		for (int j = 0; j < SIZE; j++) {

			// Encontrar numero de vecinos vivos
			int alive_neighbours = 0;
			for (int k = -1; k < 2; k++) {
				for (int l = -1; l < 2; l++) {
					if ((i + k >= 0) && (i + k < SIZE) && (j + l >= 0) && (j + l < SIZE)) {
						alive_neighbours = alive_neighbours + GRID[i + k][j + l];
					}
				}
			}

			// Descontar la celda actual ya que no cuenta para los vecinos
			alive_neighbours = alive_neighbours - GRID[i][j];

			// Reglas de Game of Life
			// Si la celda esta sola, muere
			if ((GRID[i][j] == Alive) && (alive_neighbours < 2)) future[i][j] = Dead;

			// La celda muere por sobrepoblacion
			else if ((GRID[i][j] == Alive) && (alive_neighbours > 3)) future [i][j] = Dead;

			// La celda nace
			else if ((GRID[i][j] == Dead) && (alive_neighbours == 3)) future[i][j] = Alive;

			// El resto deja la celda igual
			else future[i][j] = GRID[i][j];
		}
	}

	set_initial_conditions(future);
	xil_printf("Ended cycle\r\n");
}

void print_game_state() {
	for (int i = 0; i < SIZE; i++) {
		for (int j = 0; j < SIZE; j++) {
			if (GRID[i][j] == Alive) {
				LCD_SetPointlColor(2*j, 2*i, 0xFFFF);
				LCD_SetPointlColor(2*j + 1, 2*i, 0xFFFF);
				LCD_SetPointlColor(2*j, 2*i + 1, 0xFFFF);
				LCD_SetPointlColor(2*j + 1, 2*i + 1, 0xFFFF);
			}
			else {
				LCD_SetPointlColor(2*j, 2*i, 0x0000);
				LCD_SetPointlColor(2*j + 1, 2*i, 0x0000);
				LCD_SetPointlColor(2*j, 2*i + 1, 0x0000);
				LCD_SetPointlColor(2*j + 1, 2*i + 1, 0x0000);
			}
		}
	}
}

void print_alive_cells() {
	for (int i = 0; i < SIZE; i++) {
			for (int j = 0; j < SIZE; j++) {
				if (GRID[i][j] == Alive) {
					LCD_SetPointlColor(2*j, 2*i, 0xFFFF);
					LCD_SetPointlColor(2*j + 1, 2*i, 0xFFFF);
					LCD_SetPointlColor(2*j, 2*i + 1, 0xFFFF);
					LCD_SetPointlColor(2*j + 1, 2*i + 1, 0xFFFF);
				}
			}
		}
}

void set_cell_state(uint16_t x, uint16_t y, enum Condition state) {
	GRID[y][x] = state;
}

void toggle_cell_state(uint16_t x, uint16_t y) {
	if (GRID[y][x] == Alive) GRID[y][x] = Dead;
	else GRID[y][x] = Alive;
}

int check_empty_grid() {
	for (int i = 0; i < SIZE; i++) {
		for (int j = 0; j < SIZE; j++) {
			if (GRID[i][j] == Alive) return 0;
		}
	}

	return 1;
}
