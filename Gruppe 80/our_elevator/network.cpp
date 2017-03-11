#include "network.h"


Network::Network(){
	Elevator* elev_temp = new Elevator;
	std::vector<std::vector<Queue_element> > temp = twoD_vector_init();
	elev_temp->set_elevator_order_matrix(&temp);
	for(unsigned int i = 0; i < N_ELEVATORS ; i++){
		this->elevators.push_back(elev_temp);
		elevators[i]->set_elevator_ID(i);
	}
	this->master_ip = "0";
}

Network::Network(Status elevator_status, std::vector<std::vector<Queue_element> > *order_matrix_ptr, int elevator_ID){
	
	Elevator* elev_temp_this = new Elevator(elevator_status, order_matrix_ptr);
	Elevator* elev_temp_others = new Elevator(elevator_status);
	if(elevator_status.role == MASTER){	
		this->master_ip = elevator_status.ip;
	}
	else{
		this->master_ip = "0";
	}
	
	for(unsigned int i = 0; i < N_ELEVATORS ; i++){
		if(i != elevator_ID){
			this->elevators.push_back(elev_temp_others);
		} else{
			this->elevators.push_back(elev_temp_this);
		}
		elevators[i]->set_elevator_ID(i);
	}

}

Network::~Network(){
	if (elevators.size() != 0)
		for(unsigned int i = 0; i < N_ELEVATORS; i++){
			delete elevators[i];
		}
		this->elevators.erase(this->elevators.begin(),this->elevators.end());
}

//--------------------------------------------------------------------------------------------------
//Private functions
//----------------------------------------------------------------------------------------------------

Elevator Network::messagestring_to_elevator_object(std::string &messagestring){
	Elevator temp_elevator;
	std::vector<std::string> result;
	std::string order_matrix_string;
	std::stringstream ss1(messagestring);
	std::string word;
	while(std::getline(ss1,word, ':')){
		result.push_back(word);
	}
	temp_elevator.set_elevator_ip(result[0]);
	switch(atoi(result[1].c_str())){
		case 0:
			temp_elevator.set_elevator_role(SLAVE);
		break;
		case 1:
			temp_elevator.set_elevator_role(MASTER);
		break;
	}
	temp_elevator.set_elevator_ID(atoi(result[2].c_str()));
	switch(atoi(result[3].c_str())){
		case 0:
			temp_elevator.set_elevator_dir(D_Stop);
			break;
		case 1:
			temp_elevator.set_elevator_dir(D_Up);
			break;
		case -1:
			temp_elevator.set_elevator_dir(D_Down);
			break;
	}
	temp_elevator.set_elevator_floor(atoi(result[4].c_str()));
	temp_elevator.set_elevator_last_floor(atoi(result[5].c_str()));
	temp_elevator.set_elevator_out_of_order((bool)result[6].compare("0"));
	temp_elevator.set_elevator_online((bool)result[7].compare("0")); 

	switch(atoi(result[8].c_str())){
		case 0:
			temp_elevator.set_elevator_current_state(MOVING);
			break;
		case 1:
			temp_elevator.set_elevator_current_state(IDLE);
			break;
		case 2:
			temp_elevator.set_elevator_current_state(DOOR_OPEN);
			break;
	}
	order_matrix_string = result[9];
	std::vector<std::vector <Queue_element> > order_matrix_temp = string_to_order_matrix(order_matrix_string);
	temp_elevator.set_elevator_order_matrix(&order_matrix_temp);
	return temp_elevator;
}

std::string Network::elevator_object_to_messagestring(Elevator &elevator){
	std::stringstream ss;
	Status elev_status = elevator.get_elevator_status();
	std::string order_matrix_string = order_matrix_to_string(elevator.get_order_matrix_ptr());
	ss << elev_status.ip << ":" << elev_status.role << ":" << elev_status.elevator_ID << ":" << elev_status.dir << ":" << elev_status.floor << ":" << 
		elev_status.last_floor << ":" << elev_status.out_of_order  << ":" << elev_status.online << ":" << elev_status.current_state << ":" << order_matrix_string;

	return ss.str();
}

//----------------------------------------------------------------------------------------------------
//Public functions
//----------------------------------------------------------------------------------------------------


void Network::handle_message(Message message, int foreign_elevator_ID, int this_elevator_ID){
	switch(message){
		//Slave receives
		case MASTER_IP_INIT:
//			std::cout << "------------------------------------------------------------------------"<< std::endl;
//			std::cout << "Initializing MASTER IP of elevator:" << foreign_elevator_ID << std::endl;
//			std::cout << "------------------------------------------------------------------------"<< std::endl;
			this->master_ip = elevators[foreign_elevator_ID]->get_elevator_ip();
			break;
		//Master receives
		case SLAVE_IP_INIT:
//			std::cout << "------------------------------------------------------------------------"<< std::endl;
//			std::cout << "Initializing SLAVE IP of elevator:" << foreign_elevator_ID << std::endl;
//			std::cout << "------------------------------------------------------------------------"<< std::endl;
			break;
		//Master and slave can receive
		case HANDSHAKE:
			std::cout << "------------------------------------------------------------------------"<< std::endl;
			std::cout << "I recieved your HANDSHAKE, acknowledging:" << std::endl;
			std::cout << "------------------------------------------------------------------------"<< std::endl;
			send_message_packet(HANDSHAKE, this_elevator_ID, elevators[foreign_elevator_ID]->get_elevator_ip());
			break;
		//Master receive
		case SLAVE_ORDER_COMPLETE:
			elevators[foreign_elevator_ID]->print_elevator();
			sv_manage_completed_order(elevators[foreign_elevator_ID]);
			std::cout << "------------------------------------------------------------------------"<< std::endl;
			std::cout << "Slave order complete: " << std::endl;
			std::cout << "------------------------------------------------------------------------"<< std::endl;
			sv_manage_order_matrix(elevators, foreign_elevator_ID);
			send_message_packet(MASTER_DISTRIBUTE_ORDER_MATRIX, this_elevator_ID, "");
			break;
		case SLAVE_ORDER_INCOMPLETE:
			std::cout << "------------------------------------------------------------------------"<< std::endl;
			std::cout << "Slave order incomplete: " << std::endl;
			std::cout << "------------------------------------------------------------------------"<< std::endl;
			//sv_manage_completed_order(elevators[elevator_ID]);
			send_message_packet(MASTER_DISTRIBUTE_ORDER_MATRIX, this_elevator_ID, "");
			break;
		case SLAVE_SEND_ELEVATOR_INFORMATION:
			std::cout << "------------------------------------------------------------------------"<< std::endl;
			std::cout << "I recieved your message: SLAVE_SEND_ELEVATOR_INFORMATION: " << std::endl;
			std::cout << "------------------------------------------------------------------------"<< std::endl;
			elevators[foreign_elevator_ID]->print_elevator();
			sv_manage_order_matrix(elevators, foreign_elevator_ID);
			send_message_packet(MASTER_DISTRIBUTE_ORDER_MATRIX, this_elevator_ID, "");
			break;

		//Slave receives
		case MASTER_DISTRIBUTE_ORDER_MATRIX: 
			for(unsigned int i= 0; i < N_ELEVATORS; i++){
				elevators[i]->set_elevator_order_matrix(elevators[foreign_elevator_ID]->get_order_matrix_ptr());
			}
			std::cout << "------------------------------------------------------------------------"<< std::endl;
			std::cout << "Slave receive: Master elevator order matrix: " << std::endl;
			std::cout << "------------------------------------------------------------------------- " <<std::endl;
			elevators[foreign_elevator_ID]->print_elevator();
			std::cout << "------------------------------------------------------------------------"<< std::endl;
			std::cout << "Slave receive: My elevator order matrix: " << std::endl;
			std::cout << "------------------------------------------------------------------------- " <<std::endl;
			elevators[this_elevator_ID]->print_elevator();

			break;
		default:
			std::cout << "Invalid message, but i will accept your elevator" << std::endl;
			break;
	}
}


void Network::recieve_message_packet(int this_elevator_ID){
	Message message;
	std::string datastring;
	std::string messagestring;
	struct code_message packet;
	switch(elevators[this_elevator_ID]->get_elevator_role()){
		case MASTER:
			packet = udp_reciever();
			break;
		case SLAVE:
			packet = udp_recieve_broadcast();
			break;
	}
	datastring.assign(packet.data);
//	std::cout << "datastring: " << datastring << std::endl;
	bool b = (datastring.length() !=0) && (datastring[1] == ':');
//	std::cout << "b: " << b << std::endl;
	if(b){
		message = message_id_string_to_enum(datastring.substr(0,1));
		messagestring = datastring.substr(datastring.find_first_of(":")+1,datastring.npos);
		Elevator temp_elevator = messagestring_to_elevator_object(messagestring);
		Status temp_status = temp_elevator.get_elevator_status();
		elevators[temp_status.elevator_ID]->set_elevator_status(temp_status);
		elevators[temp_status.elevator_ID]->set_elevator_order_matrix(temp_elevator.get_order_matrix_ptr());
		handle_message(message, temp_status.elevator_ID, this_elevator_ID);
	}
}

void Network::recieve_handshake_message(int this_elevator_ID){
	std::string datastring;
	std::string messagestring;
	struct code_message packet;
	packet = udp_handshake_reciever();
	datastring.assign(packet.data);
	bool b = (datastring.length() !=0) && (datastring[1] == ':');
//	std::cout << "b: " << b << std::endl;
	if(b){
		Message message = message_id_string_to_enum(datastring.substr(0,1));
		messagestring = datastring.substr(datastring.find_first_of(":")+1,datastring.npos);
		Elevator temp_elevator = messagestring_to_elevator_object(messagestring);
		Status temp_status = temp_elevator.get_elevator_status();
		handle_message(message, temp_status.elevator_ID, this_elevator_ID);
	}	

}

void Network::send_message_packet(Message message, int this_elevator_ID, std::string reciever_ip){
	std::string message_string;
	char * ip = new char[reciever_ip.size() + 1];
	std::copy(reciever_ip.begin(), reciever_ip.end(), ip);
	ip[reciever_ip.size()] = '\0'; // don't forget the terminating 0
	switch(message){
		case MASTER_IP_INIT:
			std::cout << "Send message packet Master ip init" << std::endl;
			message_string = "0:";
			udp_broadcaster(message_string + elevator_object_to_messagestring(*elevators[this_elevator_ID]));
			break;
		case SLAVE_IP_INIT:
			message_string = "1:";
			udp_sender(message_string + elevator_object_to_messagestring(*elevators[this_elevator_ID]), MASTERPORT, ip);
			break;
		case HANDSHAKE:
			message_string = "2:";
			udp_handshake_sender(message_string + elevator_object_to_messagestring(*elevators[this_elevator_ID]), PINGPORT, ip);
			break;
		case SLAVE_ORDER_COMPLETE:
			message_string = "3:";
			udp_sender(message_string + elevator_object_to_messagestring(*elevators[this_elevator_ID]), MASTERPORT, ip);
			break;
		case SLAVE_ORDER_INCOMPLETE:
			message_string = "4:",
			udp_sender(message_string + elevator_object_to_messagestring(*elevators[this_elevator_ID]),MASTERPORT, ip);
			break;
		case SLAVE_SEND_ELEVATOR_INFORMATION:
			message_string = "5:";
			udp_sender(message_string + elevator_object_to_messagestring(*elevators[this_elevator_ID]), MASTERPORT, ip);
			break;
		case MASTER_DISTRIBUTE_ORDER_MATRIX:
			message_string = "6:"; 
			udp_broadcaster(message_string + elevator_object_to_messagestring(*elevators[this_elevator_ID]));
			break;	
		default:
			std::cout << "Not a valid message" << std::endl;
	}
	delete[] ip;
}


bool Network::is_node_responding(int this_elevator_ID, int foreign_elevator_ID){
	struct code_message code;
	std::cout << std::endl;
	std::cout << "------------------------------------------------------------------------"<< std::endl;
	std::cout << "Send HANDSHAKE, w8 for response: ";
	std::cout << "my ip:  " << this->elevators[this_elevator_ID]->get_elevator_ip() << std::endl;
	std::cout << "Foreign elev ip: " << this->elevators[foreign_elevator_ID]->get_elevator_ip() << std::endl;
	std::cout << "Elevator " << this_elevator_ID << " online : " << elevators[this_elevator_ID]->get_elevator_status().online << std::endl;
	std::cout << "Elevator " << foreign_elevator_ID << " online: " << elevators[foreign_elevator_ID]->get_elevator_status().online << std::endl;
	std::cout << "Elevator " << this_elevator_ID << " role : " << elevators[this_elevator_ID]->get_elevator_status().role << std::endl;
	std::cout << "Elevator " << foreign_elevator_ID << " role: " << elevators[foreign_elevator_ID]->get_elevator_status().role << std::endl;

	send_message_packet(HANDSHAKE, this_elevator_ID, this->elevators[foreign_elevator_ID]->get_elevator_ip()); // elevators[foreign_elevator_ID]->get_elevator_ip()); 
	

	code = udp_handshake_reciever();
	std::cout << "code.data_ " << code.data << std::endl;
	std::cout << "Responding = " << code.responding << std::endl;
	std::cout << "------------------------------------------------------------------------"<< std::endl;
	return code.responding;
}


void Network::check_responding_elevators(int this_elevator_ID){
	for(unsigned int i = 0; i < N_ELEVATORS; i++){
		if(i != this_elevator_ID){
			std::cout << "this elev id: " << this_elevator_ID << std::endl;

			if(!is_node_responding(this_elevator_ID, i)){
				elevators[i]->set_elevator_online(false);
			}
			else{
				elevators[i]->set_elevator_online(true);
			}
		}	
	}
}

void Network::check_my_role(int this_elevator_ID){
	int master_ID = 0;
	std::cout << "Role: " << this->elevators[this_elevator_ID]->get_elevator_status().role << "\t Master ip: " << this->master_ip << "My ip: " << this->elevators[this_elevator_ID]->get_elevator_ip() << std::endl;
	for(unsigned int i = 0; i < N_ELEVATORS; i++){
		if(this->elevators[i]->get_elevator_status().online){
			master_ID = this->elevators[i]->get_elevator_ID();
			std::cout << i << " Master id: " << master_ID << " <-> online: " << this->elevators[i]->get_elevator_status().online << std::endl;
			break;
		}
		
	}
	if(this_elevator_ID == master_ID){
		this->elevators[this_elevator_ID]->set_elevator_role(MASTER);
	}
	else{
		this->elevators[this_elevator_ID]->set_elevator_role(SLAVE);
	}
	this->master_ip = this->elevators[master_ID]->get_elevator_ip();
}

// ------------------------------------------------------------------------------------------------
//                 NETWORK THREAD FOR PING AND LISTENING
// ------------------------------------------------------------------------------------------------

void network_communication(Elevator* my_elevator, Network &my_network){
	while(1){
		switch(my_elevator->get_elevator_role()){
			case MASTER:
				std::cout << "network_communication case Master: " << my_elevator->get_elevator_role() << std::endl;
				my_network.send_message_packet(MASTER_IP_INIT, my_elevator->get_elevator_ID(),"");
				my_network.recieve_message_packet(my_elevator->get_elevator_ID());
				break;
			case SLAVE:
				std::cout << "network_communication case Slave: " << my_elevator->get_elevator_role() << std::endl;
				my_network.send_message_packet(SLAVE_IP_INIT, my_elevator->get_elevator_ID(), my_network.get_master_ip());
				my_network.recieve_message_packet(my_elevator->get_elevator_ID());
				break;
		}
	}
}

void network_ping(Elevator* my_elevator, Network &my_network){
	while(1){
		usleep(25000);
		my_network.check_responding_elevators(my_elevator->get_elevator_ID());
		my_network.check_my_role(my_elevator->get_elevator_ID());
		my_network.recieve_handshake_message(my_elevator->get_elevator_ID());
	}
}