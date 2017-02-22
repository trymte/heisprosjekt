#include <iostream>
#include <thread>

pthread_mutex_t lock;

void* state_machine_main(void*){
	while(1){
		pthread_mutex_lock(&lock);
		//If something needs to be locket, this is the syntax
		pthread_mutex_unlock(&lock);
	}
}

void* network_main(void*){
	while(1){
		pthread_mutex_lock(&lock);
		//If something needs to be locket, this is the syntax
		pthread_mutex_unlock(&lock);
	}
}




int main(){
//Initializing threads
	std::cout << "Initializing threads" << std::endl;
	pthread_t state_machine_thread, network_thread;
	int error_thread_1,error_thread_2;
	error_thread_1 = pthread_create(&state_machine_thread, NULL, state_machine_main, NULL);
	if(error_thread_1)
		std::cout << "Error createing state_machine_thread" << std::endl;
	
	error_thread_2 = pthread_create(&network_thread, NULL, network_main, NULL);
	if(error_thread_2)
		std::cout << "Error creating network_thread" << std::endl;

//Joining threads
	std::cout << "Joining threads" << std::endl;
	pthread_join(state_machine_thread, NULL);
	pthread_join(network_thread, NULL);
}