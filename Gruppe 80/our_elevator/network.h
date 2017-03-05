#pragma once
#include "const_struct_def.h"
//#include "supervisor.h"
#include "elevator.h"


class Network{
private:
	std::vector<Elevator> elevators;

	Elevator messagestring_to_elevator_object(std::string &messagestring);

	std::string elevator_object_to_messagestring(Elevator &elevator);

public:
	Network();

	Network(Status elevator_status, std::vector<std::vector<Queue_element> > *order_matrix_ptr);

	std::vector<Elevator> get_elevators(){return elevators;}

	Elevator* get_elevator_ptr(int elevator_ID){return &elevators[elevator_ID];}

	void handle_message(Message message_ID, int foreign_elevator_ID, int this_elevator_ID);

	void recieve_message_packet(int this_elevator_ID);

	void send_message_packet(Message message_ID, int elevator_ID);

};
