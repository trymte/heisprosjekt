#include "fsm.h"
#include "driver/timer.h"

bool requests_above(Elevator *my_elevator, Queue &my_queue,int current_floor){
	for(int i=current_floor+1;i<N_FLOORS;i++){
		for(int j=0;j<N_BUTTONS;j++){
			if ((my_queue.get_order_matrix()[i][j].active_button == 1) && (my_queue.get_order_matrix()[i][j].elevator_ID == my_elevator->get_elevator_status().elevator_ID))
				return 1;
		}
	}
	return 0;
}

bool requests_below(Elevator *my_elevator, Queue &my_queue, int current_floor){
	for(int i=0;i<current_floor;i++){
		for(int j=0;j<N_BUTTONS;j++){
			if ((my_queue.get_order_matrix()[i][j].active_button == 1) && (my_queue.get_order_matrix()[i][j].elevator_ID == my_elevator->get_elevator_status().elevator_ID))
				return 1;
		}
	}
	return 0;
}


int requests_should_stop(Elevator *my_elevator, Queue &my_queue){
	int current_floor = my_elevator->get_elevator_status().floor;
	switch(my_elevator->get_elevator_status().dir){
	case D_Down:
		return
			((my_queue.get_order_matrix()[current_floor][(int)B_HallDown].active_button) && 
			 (my_queue.get_order_matrix()[current_floor][(int)B_HallDown].elevator_ID ==  my_elevator->get_elevator_status().elevator_ID)) ||
			
			((my_queue.get_order_matrix()[current_floor][(int)B_Cab].active_button) &&
			 (my_queue.get_order_matrix()[current_floor][(int)B_Cab].elevator_ID ==  my_elevator->get_elevator_status().elevator_ID))      ||

			!(requests_below(my_elevator, my_queue,current_floor));

	case D_Up:
		return
			((my_queue.get_order_matrix()[current_floor][(int)B_HallUp].active_button) && 
			 (my_queue.get_order_matrix()[current_floor][(int)B_HallUp].elevator_ID ==  my_elevator->get_elevator_status().elevator_ID)) ||
			
			((my_queue.get_order_matrix()[current_floor][(int)B_Cab].active_button) &&
			 (my_queue.get_order_matrix()[current_floor][(int)B_Cab].elevator_ID ==  my_elevator->get_elevator_status().elevator_ID))    ||

			!(requests_above(my_elevator, my_queue,current_floor));

	default:
		return 0;
	}
}

void open_door(){
	std::cout << "Door open" << std::endl;
	elev_set_door_open_lamp(1);
	timer_start(DOOR_TIME_S, TIMER_DOOR_ID);
}



void fsm_execute_order(Elevator *my_elevator, Queue &my_queue, Order &order){ //Status &status,Order &order) 
	int current_floor = my_elevator->get_elevator_status().floor;
	State current_state = my_elevator->get_elevator_status().current_state;

	switch(current_state){
		case MOVING:
			if(current_floor == order.floor){
				fsm_on_floor_arrival(my_elevator,my_queue,current_floor);
			}
			break;

   		case IDLE:
   			if(current_floor == order.floor){
   				fsm_on_floor_arrival(my_elevator,my_queue,current_floor);
   			}

   			else if(current_floor > order.floor){
   				timer_start(TIME_CONDITION_S,TIMER_CONDITION_ID);
   				elev_set_motor_direction(DIRN_DOWN);
   				my_elevator->set_elevator_dir(D_Down);
	   			my_elevator->set_elevator_current_state(MOVING);
   			}
   			
   			else{
   				timer_start(TIME_CONDITION_S,TIMER_CONDITION_ID);
   				elev_set_motor_direction(DIRN_UP);
   				my_elevator->set_elevator_dir(D_Up);
	   			my_elevator->set_elevator_current_state(MOVING);
   			}
   			
   			break;


   		default:
   			break;
	}
}


bool fsm_on_floor_arrival(Elevator *my_elevator,Queue &my_queue, int current_floor){
	bool stopped;
	elev_set_floor_indicator(current_floor);
	my_elevator->set_elevator_floor(current_floor);
	switch(my_elevator->get_elevator_status().current_state){
	case MOVING:
		if (requests_should_stop(my_elevator, my_queue)){
			std::cout << "Requst should not stop" << std::endl;
			stopped = true;			
			elev_set_motor_direction(DIRN_STOP);
			if (get_timer_id() == TIMER_CONDITION_ID)
				timer_stop();

			if (get_timer_id() == 0){
				open_door();
			}
			
			my_elevator->set_elevator_dir(D_Stop);
   			my_elevator->set_elevator_current_state(DOOR_OPEN);
   			my_elevator->set_elevator_out_of_order(0);
   			
			for (int j=0;j<N_BUTTONS;j++){
				Order order_to_be_removed;
	   			order_to_be_removed.floor = current_floor;
	   			order_to_be_removed.btn = (Button)j;
	   			my_queue.remove_order(order_to_be_removed);

			}
		}
		break;
	case IDLE:
			for(int i=0;i<N_BUTTONS;i++){
				if (my_queue.get_order_matrix()[current_floor][i].active_button == 1){
					stopped = true;
					if (get_timer_id() == TIMER_CONDITION_ID)
						timer_stop();

					if (get_timer_id() == 0)
						open_door();
					
					my_elevator->set_elevator_dir(D_Stop);
		   			my_elevator->set_elevator_current_state(DOOR_OPEN);
		   			my_elevator->set_elevator_out_of_order(0);

					Order order_to_be_removed;
		   			order_to_be_removed.floor = current_floor;
		   			order_to_be_removed.btn = (Button)i;
		   			my_queue.remove_order(order_to_be_removed);
				}
			}	
		break;

	default:
		break;
	}
	return stopped;
}


void fsm_on_door_timeout(Elevator *my_elevator,Queue &my_queue){
	std::cout << "Door timed out " << std::endl;
	int current_floor = elev_get_floor_sensor_signal();
	my_elevator->set_elevator_current_state(IDLE);
	elev_set_door_open_lamp(0);
	if(get_timer_id() == TIMER_DOOR_ID)
		timer_stop();

	Order order_to_be_removed;
	for(int i=0;i<N_BUTTONS;i++){
		if((my_queue.get_order_matrix()[current_floor][i].active_button == 1) && (my_queue.get_order_matrix()[current_floor][i].elevator_ID == my_elevator->get_elevator_status().elevator_ID)){
			order_to_be_removed.floor = current_floor;
			order_to_be_removed.btn = (Button)i;
			my_queue.remove_order(order_to_be_removed);
		}
	}
}

