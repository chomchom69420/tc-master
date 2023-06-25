#include "slaves.h"
#include "configurations.h"
SlaveStates slave_states;

int get_slave_state(int slave_id){
    return slave_states.states[slave_id-1];
}

void set_slave_state(int slave_id, int state){
    slave_states.states[slave_id-1] = state;
}

void set_number_of_slaves(int n){
    slave_states.n_slaves = n;
}

void set_sequence_mode(int mode){
    slave_states.mode_select = mode;
}

void set_global_timers(int *global_timers_array, int n){ for(int i=0;i<n;i++)    slave_states.global_fsm_timers[i] = global_timers_array[i];}

void set_slaves_green_timers(int *slave_timers_array, int n){   for(int i=0;i<n;i++)  slave_states.slaves_green_timers[i] = slave_timers_array[i];}

void set_slaves_red_timers(int *slave_timers_array, int n){ for(int i=0;i<n;i++)  slave_states.slaves_red_timers[i] = slave_timers_array[i];}

void calc_set_slave_timers(){

  for(int i=0;i< slave_states.n_slaves; i++)
  {
    //Red timers 
    for(int j=0;j<slave_states.n_slaves+1;j++)
    {
      if(j==i) continue;
      slave_states.slaves_red_timers[i] += slave_states.global_fsm_timers[j];
    }

    //Green timers
    slave_states.slaves_green_timers[i]= slave_states.global_fsm_timers[i];
  }
}

int get_global_timer(int state_id){ return slave_states.global_fsm_timers[state_id];}

int get_slave_timer(int slave_id, int slave_state){
    if(slave_state == SLAVE_STATE_RED)  return slave_states.slaves_red_timers[slave_id-1];
    else return slave_states.slaves_green_timers[slave_id-1];
}

int get_number_of_slaves(){ return slave_states.n_slaves;}

int get_sequence_mode() { return slave_states.mode_select;}