/*	James Daniel Brisky
	CS144 Final Project (Traffic version)
	12/9/14

	Program used to simulate directing of traffic.  Written in C++ with the posix
	thread library.

	Each car is its own thread.  Cars pull up to a 4-way intersection.  The
	intersection is only one lane per direction.  Each direction has a queue,
	and only the front of the queue is allowed to go at any given time.  Further,
	the entire intersection is protected by a mutex, and only one car can be in 
	the intersection at a time.
				
						Southbound Queue
							|	|	|
							|	|	|
							|	|	|
							|	|	|
			________________		___________________
Eastbound	______________Intersection_________________ Westbound
Queue		________________		___________________ Queue
							|	|	|
							|	|	|
							|	|	|
							|	|	|
						Northbound Queue

	Each queue and the intersection are protected by a mutex.  Additionally,
	there is an ID counter to assign each car an ID, and this counter is also
	protected by a mutex.

	In testing, program was able to run on a relatively high-end Windows
	desktop running an Ubuntu Virtual Box with 2000 total threads, and completed
	without deadlock or any car crashes in approximately 30 seconds.
	*/

#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <unistd.h>
#include <queue>

using namespace std;

#define NUM_CARS 500 //number of cars in each direction; so 4x this number is the total number of threads
#define TIME_TO_RUN 60 //max time to allow the entire program to run, in seconds

//queues.  one queue for each direction
queue <int> southboundQ;
queue <int> northboundQ;
queue <int> westboundQ;
queue <int> eastboundQ;

//each car is assigned an ID in sequential order.  first car is car(0), last car is car(NUM_CARS -1)
int id_counter;

//mutexes to protect the global variables, as well as to make sure only one car is allowed
//in the intersection at a time.
pthread_mutex_t sb_lock;
pthread_mutex_t nb_lock;
pthread_mutex_t wb_lock;
pthread_mutex_t eb_lock;
pthread_mutex_t intersection_lock;
pthread_mutex_t id_counter_lock;

pthread_t threadID; //Thread ID
pthread_attr_t attr; //Set of thread attributes

//each thread runs one of these functions, depending on where the car is coming from.
//all of these functions are essentially the same.
void *southbound(void *param);
void *northbound(void *param);
void *westbound(void *param);
void *eastbound(void *param);

//initializes all the mutexes and sets the counter to 0.
void initializeData ()
{
	pthread_mutex_init(&sb_lock, NULL);
	pthread_mutex_init(&nb_lock, NULL);
	pthread_mutex_init(&wb_lock, NULL);
	pthread_mutex_init(&eb_lock, NULL);
	pthread_mutex_init(&intersection_lock, NULL);
	pthread_mutex_init(&id_counter_lock, NULL);

	pthread_attr_init(&attr);
	id_counter = 0;
}

//comments explain this function.  other directions will be left uncommented; all
//other directions are essentially the same
void *southbound(void *param)
{
	//first assigns the car an ID, which it gets from the global id_counter variable
	int id;
	pthread_mutex_lock(&id_counter_lock);
	id = id_counter++;
	pthread_mutex_unlock(&id_counter_lock);

	//next pushes the car onto the appropriate queue
	pthread_mutex_lock(&sb_lock);
	southboundQ.push(id);
	pthread_mutex_unlock(&sb_lock);

	//next tries to see if the car is at the head of its respective queue.  if it is,
	//break out of the loop and try to enter the intersection.  if not, releases the mutex
	//and voluntarily interrupts self before trying again. 
	while (true)
	{
		pthread_mutex_lock(&sb_lock);
		if (id != southboundQ.front())
		{
			pthread_mutex_unlock(&sb_lock);
			usleep(1);
			continue;
		}
		else
			break;
	}

	//at this point, you are at the head of your queue.  try and enter the intersection.
	//if the two cout lines are ever divided during run time, that means a crash occured.
	pthread_mutex_lock(&intersection_lock);
	cout << "Southbound car with ID (" << id << ") entering intersection.\n";
	cout << "Southbound car with ID (" << id << ") leaving intersection.\n";
	pthread_mutex_unlock(&intersection_lock);

	//after leaving the intersection, also need to release the mutex on the queue
	pthread_mutex_unlock(&sb_lock);

	//finally, after crossing, pop self out of queue
	pthread_mutex_lock(&sb_lock);
	southboundQ.pop();
	pthread_mutex_unlock(&sb_lock);

	//return;
}

//see southbound() for comments
void *northbound(void *param)
{
	int id;
	pthread_mutex_lock(&id_counter_lock);
	id = id_counter++;
	pthread_mutex_unlock(&id_counter_lock);

	pthread_mutex_lock(&nb_lock);
	northboundQ.push(id);
	pthread_mutex_unlock(&nb_lock);

	while (true)
	{
		pthread_mutex_lock(&nb_lock);
		if (id != northboundQ.front())
		{
			pthread_mutex_unlock(&nb_lock);
			usleep(1);
			continue;
		}
		else
			break;
	}

	pthread_mutex_lock(&intersection_lock);
	cout << "Northbound car with ID (" << id << ") entering intersection.\n";
	cout << "Northbound car with ID (" << id << ") leaving intersection.\n";
	pthread_mutex_unlock(&intersection_lock);

	pthread_mutex_unlock(&nb_lock);

	pthread_mutex_lock(&nb_lock);
	northboundQ.pop();
	pthread_mutex_unlock(&nb_lock);

	//return;
}

//see southbound() for comments
void *westbound(void *param)
{
	int id;
	pthread_mutex_lock(&id_counter_lock);
	id = id_counter++;
	pthread_mutex_unlock(&id_counter_lock);

	pthread_mutex_lock(&wb_lock);
	westboundQ.push(id);
	pthread_mutex_unlock(&wb_lock);

	while (true)
	{
		pthread_mutex_lock(&wb_lock);
		if (id != westboundQ.front())
		{
			pthread_mutex_unlock(&wb_lock);
			usleep(1);
			continue;
		}
		else
			break;
	}

	pthread_mutex_lock(&intersection_lock);
	cout << "Westbound car with ID (" << id << ") entering intersection.\n";
	cout << "Westbound car with ID (" << id << ") leaving intersection.\n";
	pthread_mutex_unlock(&intersection_lock);

	pthread_mutex_unlock(&wb_lock);

	pthread_mutex_lock(&wb_lock);
	westboundQ.pop();
	pthread_mutex_unlock(&wb_lock);

	//return;
}

//see southbound() for comments
void *eastbound(void *param)
{
	int id;
	pthread_mutex_lock(&id_counter_lock);
	id = id_counter++;
	pthread_mutex_unlock(&id_counter_lock);

	pthread_mutex_lock(&eb_lock);
	eastboundQ.push(id);
	pthread_mutex_unlock(&eb_lock);

	while (true)
	{
		pthread_mutex_lock(&eb_lock);
		if (id != eastboundQ.front())
		{
			pthread_mutex_unlock(&eb_lock);
			usleep(1);
			continue;
		}
		else
			break;
	}

	pthread_mutex_lock(&intersection_lock);
	cout << "Eastbound car with ID (" << id << ") entering intersection.\n";
	cout << "Eastbound car with ID (" << id << ") leaving intersection.\n";
	pthread_mutex_unlock(&intersection_lock);

	pthread_mutex_unlock(&eb_lock);

	pthread_mutex_lock(&eb_lock);
	eastboundQ.pop();
	pthread_mutex_unlock(&eb_lock);

	//return;
}

int main ()
{
	initializeData();

	//creates the threads. equal amount of threads in each direction, multiplied
	//by NUM_CARS.  set to 500 by default, or 2000 total threads.
	for (int i = 0; i < NUM_CARS; i++)
	{
		pthread_create(&threadID,&attr,southbound,NULL);
		pthread_create(&threadID,&attr,northbound,NULL);
		pthread_create(&threadID,&attr,westbound,NULL);
		pthread_create(&threadID,&attr,eastbound,NULL);
	}

	//main() sleeps, allowing the threads to run for a given amount of time, then returns
	sleep(TIME_TO_RUN);
	cout << "Exiting...\n";
	return 0;
}
