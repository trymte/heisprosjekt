#include <iostream>
#include <unistd.h>

#include "elev.h"
#include "io.h"
#include "channels.h"

#include "../queue.cpp"
#include "../const_struct_def.cpp"
#include "timer.h"

#include "../fsm.cpp"
//#include "../network.cpp"
#include "../elevator.cpp"

/////////////////////////////////////////////////////////////////////////////



bool check_buttons(Queue &my_queue){
	Order new_order;
	bool new_button_press = 0;
	for(int i=0; i<N_FLOORS;i++){
		for(int j=0;j<N_BUTTONS;j++){
			if (elev_get_button_signal((elev_button_type_t)j,i) && (my_queue.get_order_matrix()[i][j].active_button == 0)){
				new_order.floor = i;
				new_order.btn = (Button)j;
				new_button_press = 1;
				
				my_queue.add_order(new_order,1); //Skal være -1
				my_queue.print_order_matrix(); //Kan fjernes
			}
		}
	}
	return new_button_press;
}

void set_all_lights(Queue &my_queue){
	//Må lages, alternativt tenne lysene underveis?
	//Kan lese fra queue hvilke betjeningsknapper som skal lyse.


	for(int i =0;i<N_FLOORS;i++){
		for(int j=0;j<N_BUTTONS;j++){
			if(my_queue.get_order_matrix()[i][j].active_button == 1){
				elev_set_button_lamp((elev_button_type_t)j,i,1);
			}
			else
				elev_set_button_lamp((elev_button_type_t)j,i,0);
		}
	}
}



int main(){
///////////////////////////////////////////////////////
// For testing, skal i main
//	Network my_network;
	Elevator my_elevator; //Settes til å peke på elevator(elevator_ID) i elevators. Alle endringer på my_elevator i etterkant vil da endres i network. GUNSTIG!
	Queue my_queue;

	my_elevator.set_elevator_current_state(IDLE);
	my_elevator.set_elevator_floor(0);
	my_elevator.set_elevator_dir(D_Stop);
	my_elevator.set_elevator_role(MASTER);
	my_elevator.set_elevator_out_of_order(0);
	my_elevator.set_elevator_ID(1); 
	elev_set_floor_indicator(0);



 
//////////////////////////////////////////////////////////////////////////////// 
//Initializing
////////////////////////////////////////////////////////////////////////////////     
	std::cout << "Event manager initializing..." << std::endl; 
	elev_init();
	int input_poll_rate_ms = 25;
	while(elev_get_floor_sensor_signal() != 0){
		elev_set_motor_direction(DIRN_DOWN);
	}
	elev_set_motor_direction(DIRN_STOP); 
	elev_set_floor_indicator(0);


	
	std::cout << "Event manager initialized" << std::endl;
/////////////////////////////////////////////////////////////////////////////////


	while(1){
		if (check_buttons(my_queue)){

			//Update elevators in network --> NEI, fordi når queue blir oppdatert, blir my_elevator og elevators oppdatert siden de peker på ordrematrisen.
						
			switch(my_elevator.get_elevator_status().role){
				case MASTER:
					std::cout << "Supervisor" << std::endl;
					//sv_manage_order_matrix(network.get_elevators());
					//Kall supervisor funksjon.
					break;

				case SLAVE:
					std::cout << "Send_message_packet" << std::endl;
					//send_message_packet;
					break;
			}
	
		}

		Order next_order = my_queue.get_next_order(my_elevator.get_elevator_status().elevator_ID);
		if (next_order.active_order == 1){ 
			fsm_execute_order(my_elevator,my_queue,next_order);
		}


		int current_floor = elev_get_floor_sensor_signal();
		my_elevator.set_elevator_floor(current_floor);
		if (elev_get_floor_sensor_signal() != -1){
			fsm_on_floor_arrival(my_elevator,my_queue,current_floor);
		}


		if((timer_timedOut()) && (get_timer_id() == TIMER_DOOR_ID)){ 
			fsm_on_door_timeout(my_elevator,my_queue);
		}


		if((timer_timedOut())&& (get_timer_id() == TIMER_CONDITION_ID)){ 
			std::cout << "Elevator is out of order" << std::endl;
			my_elevator.set_elevator_out_of_order(1);
			timer_stop(); 
		}

//		std::cout << my_elevator.get_elevator_status().current_state << std::endl;      
		set_all_lights(my_queue); 
	 
		usleep(input_poll_rate_ms*1000);   
	}  
}
/*
 
To do:


- Role --> Assign elevators to orders etc (inform_supervisor) -->Når network er ferdig. Litt mye feilmeldinger nå.
- Supervisor funksjoner
*/








