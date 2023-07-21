#include "environment.h"
#include "configurations.h"
Environment env;

int env_getSlaveState(int slave_id){
    return env.states[slave_id-1];
}

void env_setSlaveState(int slave_id, int state){
    env.states[slave_id-1] = state;
}

void env_setNumSlaves(int n){
    env.n_slaves = n;
}

void env_setSequenceMode(int mode){
    env.mode_select = mode;
}

void env_setGlobalTimers(int *global_timers_array) { 
  for(int i=0;i<env.n_slaves;i++)    
    env.global_fsm_timers[i] = global_timers_array[i];
}

void env_setSlavesGreenTimers(int *slave_timers_array) {   
  for(int i=0;i<env.n_slaves;i++)  
    env.slaves_green_timers[i] = slave_timers_array[i];
}

void env_setSlavesRedTimers(int *slave_timers_array) { 
  for(int i=0;i<env.n_slaves;i++)  
    env.slaves_red_timers[i] = slave_timers_array[i];
}

void env_calcSetSlaveTimers(){

  for(int i=0;i< env.n_slaves; i++)
  {
    //Red timers 
    for(int j=0;j<env.n_slaves+1;j++)
    {
      if(j==i) continue;
      env.slaves_red_timers[i] += env.global_fsm_timers[j];
    }

    //Green timers
    env.slaves_green_timers[i]= env.global_fsm_timers[i];
  }
}

int env_getGlobalTimer(int state_id) { 
  return env.global_fsm_timers[state_id];
}

int env_getSlaveTimer(int slave_id, int slave_state){
    if(slave_state == SlaveStates::RED)  
      return env.slaves_red_timers[slave_id-1];
    else 
      return env.slaves_green_timers[slave_id-1];
}

int env_getNumSlaves(){ 
  return env.n_slaves;
}

int env_getSequenceMode() { 
  return env.mode_select;
}