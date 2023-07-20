typedef struct {
    int n_slaves;
    int states[7];
    int mode_select;
    int global_fsm_timers[8]; //max is n_slaves + 1
    int slaves_green_timers[7];
    int slaves_red_timers[7];
} Environment;

void set_slave_state(int slave_id, int state);

void set_number_of_slaves(int n);

void set_sequence_mode(int mode);

void set_global_timers(int *global_timers_array, int n);

void set_slaves_green_timers(int *slave_timers_array, int n);

void set_slaves_red_timers(int *slave_timers_array, int n);

void calc_set_slave_timers();

int get_slave_state(int slave_id);

int get_global_timer(int state_id);

int get_slave_timer(int slave_id, int slave_state);

int get_number_of_slaves();

int get_sequence_mode();

