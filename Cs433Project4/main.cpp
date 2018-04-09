
#include <stdlib.h> /* required for rand() */
#include "buffer.h"
#include <thread>
#include <iostream>
#include <vector>
#include <mutex>
#include <condition_variable>

/* the buffer */
buffer_item buffer[BUFFER_SIZE];
int n, first, last;

bool running = true;

std::mutex mtx; // mutex for critical section
std::condition_variable empty; //consumer check
std::condition_variable full;

/* insert item into buffer
return 0 if successful, otherwise
return -1 indicating an error condition */
int insert_item(buffer_item item) {

	if (n != BUFFER_SIZE)
	{
		buffer[last] = item;
		std::cout << "item "  << item << " inserted by producer" << std::endl;
		last = (last + 1) % BUFFER_SIZE; //loop around
		n++;
		//success
		return 0;
	}
	//failed
	return -1;
}

/* remove an object from buffer
placing it in item
return 0 if successful, otherwise
return -1 indicating an error condition */
int remove_item(buffer_item *item) {
	// remove an object from buffer and placing it in item
	if (n > 0)
	{

		item = &buffer[first];
		std::cout << "item " << *item << " removed by consumer" << std::endl;
		first = (first + 1) % BUFFER_SIZE; //loop around
		n--;
		return 0;
	}
	return -1;
}

//Print that buffer
void printBuffer()
{
	std::cout << "The current content of the buffer is [ ";

	if(n == 0)
	{
		std::cout << "]" << std::endl;
		return;
	}

	if (last > first)
	{
		for (int cnt = first; cnt< last; cnt++)
		{
			std::cout << buffer[cnt] << " ";

		}
	}
	else if (last <= first)
	{
		//loops around
		for (int cnt = first; cnt < BUFFER_SIZE; cnt++)
		{
			std::cout << buffer[cnt] << " ";
		}
		for (int cnt = 0; cnt < last; cnt++)
		{
			std::cout << buffer[cnt] << " ";
		}
	}

	std::cout << "]" << std::endl;
}

//producer thread with rand seed
void producer(unsigned seed) {
	buffer_item item;
	srand(seed);

	while (running) { /* sleep for a random period of time */
		std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 5 + 1));
		
		std::unique_lock<std::mutex> lck(mtx);
		while (n == BUFFER_SIZE) {
			//buffer checks, waits, etc
			if (running == false) {
				//dump all, end condition cleanup
				empty.notify_all();
				return;
			}
			full.wait(lck);	//wait until not full
		}

		/* generate a random number */
		item = rand();
		if (insert_item(item) == -1)
		{
			std::cout << "Buffer is full" << std::endl;
		}
		else
		{
			printBuffer();
			
		}

		empty.notify_all();
	}
}

//consumer thread with rand seed
void consumer(unsigned seed) {
	buffer_item item;
	srand(seed);
	while (running) { /* sleep for a random period of time */
		std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 5 + 1));

		std::unique_lock<std::mutex> lck(mtx);
		while (n == 0) {
			//buffer checks, waits, etc
			if (running == false) {
				//dump all, end condition cleanup
				full.notify_all();
				return;
			}
			empty.wait(lck);	//wait until not full
		}

		if (remove_item(&item) == -1) {
			std::cout << "Buffer is empty" << std::endl;
		}
		else
		{
			printBuffer();
			
		}

		full.notify_all();
	}
}

//the main
int main(int argc, char *argv[]) { 

	std::cout << "Judah Perez, CS 433, Project 4" << std::endl;
	std::cout << "4/2/2018" << std::endl;
	std::cout << "Multi-threaded Programing for the Producer - Consumer Problem" << std::endl;

	n = 0;
	first = 0;
	last = 0;

	/* 1. Get command line arguments argv[1],argv[2],argv[3] */

	int sleep= atoi(argv[1]);
	int producerNum = atoi(argv[2]);
	int consumerNum = atoi(argv[3]);

	if ((argc < 4) || (sleep <= 0) || (producer <= 0) || (consumer <= 0))
	{
		std::cout << ("Please add arguments for sleep time, producers and consumers") << std::endl;
		return -1;
	}

	std::cout << "Producers " << producerNum << ", Consumers " << consumerNum << std::endl;

	/* 2. Initialize buffer */

	std::vector<std::thread> producers(producerNum);
	std::vector<std::thread> consumers(consumerNum);

	/* 3. Create producer thread(s) */
	for (int cnt = 0; cnt < producerNum; cnt++)
	{
		std::thread thrd = std::thread(producer, time(NULL) + cnt);//thread and seed param
		//thrd.detach();
		producers[cnt] = std::move(thrd);
	}

	/* 4. Create consumer thread(s) */
	for (int cnt = 0; cnt < consumerNum; cnt++)
	{
		consumers[cnt] = std::thread(consumer, time(NULL) + cnt); //thread and seed param
		//consumers[cnt].detach();
	}

	/* 5. Sleep */
	std::this_thread::sleep_for(std::chrono::milliseconds(sleep));
	/* 6. Exit */

	running = false;

	/* Clean Up */
	for (int cnt = 0; cnt < producerNum; cnt++)
	{
		producers[cnt].join();
	}

	for (int cnt = 0; cnt < consumerNum; cnt++)
	{
		consumers[cnt].join();
	}

	return 0;
}