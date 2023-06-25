#ifndef LANES_H_INCLUDED
#define LANES_H_INCLUDED


/*

Signal states 
0 → red
1 → yellow
2 → forward green
3 → left green
4 → right green 
*/

/*
This method is used to initialize the lanes fsm by setting the first lane as the go and stopping traffic on other lanes 
*/
void lanes_start_signals();  

/*
This method is used to update the lanes fsm to change state from allow traffic on one lane to the other depending on the timer values 
*/
void lanes_update();

void lanes_set_state(int state);

int lanes_get_state();

#endif

