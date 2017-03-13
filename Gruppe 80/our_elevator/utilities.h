#pragma once

#include <cmath>
#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <stdlib.h>

const unsigned int N_FLOORS = 4;
const unsigned int N_BUTTONS = 3;
const unsigned int N_ELEVATORS = 3;
const unsigned int DOOR_TIME_S = 1;
const unsigned int TIME_CONDITION_S = 10;
const unsigned int TIMER_DOOR_ID = 1;
const unsigned int TIMER_CONDITION_ID = 2;
const unsigned int TIMER_NETWORK = 1;
const unsigned int MIN_MESSAGE_LENGTH = 69;



typedef enum { 
    D_Down  = -1,
    D_Stop  = 0, 
    D_Up    = 1 
} Dirn;

typedef enum { 
    B_HallUp = 0,
    B_HallDown = 1,
    B_Cab = 2
} Button;

typedef enum {
    SLAVE = 0,
    MASTER = 1
} Role;

typedef enum{
    MASTER_IP_INIT = 0,
    SLAVE_IP_INIT = 1,
    HANDSHAKE = 2,
    SLAVE_ORDER_COMPLETE = 3,
    SLAVE_ORDER_INCOMPLETE = 4,
    SLAVE_SEND_ELEVATOR_INFORMATION = 5,
    MASTER_DISTRIBUTE_ORDER_MATRIX = 6
} Message;


typedef enum { 
    MOVING = 0, 
    IDLE = 1, 
    DOOR_OPEN = 2
}State;


struct Queue_element{
    bool active_button;
    int elevator_ID;
};

struct Order{
    bool active_order;
    int floor;
    Button btn;
};


struct Status{
    std::string ip;
	Dirn dir;
	int floor;
    int last_floor;
    State current_state;
	int elevator_ID;
	bool out_of_order;
	bool online;
    Role role;
};

Status init_elev_status();

std::vector<std::vector<Queue_element> > twoD_vector_init(); 

std::vector<std::vector <Queue_element> > string_to_order_matrix(std::string &order_matrix_string);

std::string order_matrix_to_string(std::vector<std::vector <Queue_element> > *order_matrix_ptr);

void print_order_matrix(std::vector<std::vector <Queue_element> > *order_matrix_ptr);
