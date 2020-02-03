#include <unistd.h>
#include <stdio.h>
#include <stdlib.h> 
#include <pthread.h>
#include <string.h>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
using namespace std;

struct PatronDetails {
	string name;
	int delay;
	int duration;
}; //PatronDetails

static int nAhead[2];
static int notFree[2] = {0};
	/* 0 means they're free
	   1 means they're busy*/
int numberOfPatrons = 0;
int current_time = 0;


int quickestLane() { 
	if (nAhead[0] > nAhead[1]) 
		return 1;
	else if (nAhead[1] > nAhead[0]) 
		return 0;
	else if (nAhead[0] == nAhead[1])
		return 0;
} //quickestLane


void *patron(void *arg) {
	struct PatronDetails *thisPatron = (PatronDetails*) arg;
	int myClerk = 0;
	/*
	The mutex class is a synchronization primitive that can be used to protect 
	shared data from being simultaneously accessed by multiple threads.
	*/
	static pthread_mutex_t entry    = PTHREAD_MUTEX_INITIALIZER;	// access
	static pthread_mutex_t clerk[2] = {PTHREAD_MUTEX_INITIALIZER};	// controls access to the clerks

	/*
	The condition_variable class is a synchronization primitive that can 
	be used to block a thread, or multiple threads at the same time, until
	another thread both modifies a shared variable (the condition), and 
	notifies the condition_variable.
	*/
	static pthread_cond_t clear[2] =  PTHREAD_COND_INITIALIZER;		// free, signals if the customer is being served by a clerk. 


	cout << thisPatron->name << " is ready to check out." << endl;	

	if (nAhead[0] <= nAhead[1])
		myClerk = 0;
	else
		myClerk = 1;
	nAhead[myClerk]++;

	cout << thisPatron->name << " selects lane " << myClerk << "." << endl;

	
	pthread_mutex_lock(&clerk[myClerk]);
	while (notFree[myClerk] != 0) {
		/*Clerk is not free and patron has to wait*/
		pthread_cond_wait(&clear[myClerk], &clerk[myClerk]);
	}
	
	pthread_cond_signal(&clear[myClerk]);			// Signal that patron is being helped.
	/*Clerk is free and patron can get help.*/
	cout << thisPatron->name << " is getting helped by clerk working lane " << myClerk << "." << endl;
	notFree[myClerk] = 1;
	
	pthread_mutex_unlock(&clerk[myClerk]);
	sleep(thisPatron->duration);
	pthread_mutex_lock(&clerk[myClerk]);

	nAhead[myClerk]--;
	cout << thisPatron->name << " leaves the market." << endl;
	notFree[myClerk] = 0;
	pthread_cond_signal(&clear[myClerk]);			// Signal that the patron has left the market
	pthread_mutex_unlock(&clerk[myClerk]);
	
	pthread_exit(0);
}
int main(int argc, char const *argv[]) {
	PatronDetails patrons_list[512];
	pthread_t tid[512];

	string line = "", customerName = "";
	int arrivalTime = 0, serviceTime = 0;

	ifstream inFS(argv[1]);
	if (!inFS) {
		cout << "Couldn't open the input file!" << endl;
		return 0;
	}

	nAhead[0] = 0;
	nAhead[1] = 0;

	while (getline(inFS,line)) {
		stringstream ss(line);
		ss >> customerName >> arrivalTime >> serviceTime;
		patrons_list[numberOfPatrons].name = customerName;
		patrons_list[numberOfPatrons].delay = arrivalTime;
		patrons_list[numberOfPatrons++].duration = serviceTime;
	}//while

	
	// cout << "There are " << numberOfPatrons << " customers in the supermarket." << endl;
	// cout << "Creating pThreads" << endl;
	for (int i = 0; i < numberOfPatrons; ++i) {
		sleep(patrons_list[i].delay);
		pthread_create(&tid[i], NULL, patron, (void *) &patrons_list[i]);
		// current_time += patrons_list[i].delay;
	}

	// cout << "Joining pThreads" << endl;
	for (int i = 0; i < numberOfPatrons; ++i) {
		pthread_join(tid[i], NULL);
	}


	cout << "-- A total of " << numberOfPatrons << " patrons completed their purchases." << endl;
	pthread_exit(0);

	
	return 0;
} //main
