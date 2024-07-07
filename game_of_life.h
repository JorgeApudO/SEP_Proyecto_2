void set_initial_conditions(enum Condition *new_state[]);
void reset_grid();
void update_game_state();
void print_game_state();
void print_alive_cells();
void set_cell_state(uint16_t x, uint16_t y, enum Condition state);
void toggle_cell_state(uint16_t x, uint16_t y);
int check_empty_grid();
