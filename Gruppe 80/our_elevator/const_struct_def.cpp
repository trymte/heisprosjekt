#include "const_struct_def.h"

std::vector<std::vector<Queue_element> > vector_init(){
    std::vector<std::vector<Queue_element> > temp;
    Queue_element init_element;
    init_element.active_button = 0;
    init_element.elevator_ID = -1;
    for (int i=0;i<N_FLOORS;i++){
        std::vector<Queue_element> rowvector;
        for(int j=0;j<N_BUTTONS;j++){
            rowvector.push_back(init_element);
        }
        temp.push_back(rowvector);
    }   
    return temp;
}


std::vector<std::vector <Queue_element> > string_to_order_matrix(std::string &order_matrix_string){
	std::vector<std::vector <Queue_element> > order_matrix = vector_init();
	std::stringstream ss2(order_matrix_string);
	int count = 0;
	int temp_floor = 0;
	Queue_element temp_queue_element;
	std::string queue_element_string;
	std::string act_btn;
	std::string elv_id;
	while(std::getline(ss2,queue_element_string, '&')){
		if(count == 3){
			temp_floor +=1;
			count = 0;
		}
		if(queue_element_string.size() == 2){
			act_btn = queue_element_string[0];
			elv_id = queue_element_string[1];
		} else if(queue_element_string.size() == 3){
			act_btn = queue_element_string[0];
			elv_id = queue_element_string.substr(1, 2);
		}
		if(act_btn == "0"){
			temp_queue_element.active_button = false;
		} else{
			temp_queue_element.active_button = true;
		}
		
		temp_queue_element.elevator_ID = atoi(elv_id.c_str());
		order_matrix[temp_floor][count] = temp_queue_element;
		count += 1;
	}
	return order_matrix;
}