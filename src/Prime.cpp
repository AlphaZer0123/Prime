//============================================================================
// Name        : Prime.cpp
// Author      : Joshua Villwock
// Version     :
// Copyright   : Your copyright notice
// Description : Finds Primes, and stores in a bitmap.
//============================================================================

#include "stdinc.h"		//global includes here

using namespace std;	//namespace for ease of use

//declare global variables
int shmem_fd;

//BOOST thread group for ease of use.
thread_group threadList;

//Command line options
bool proc;
int workers;

//Pointers for the Bitmap
void *addr;
unsigned char *bitmap;
unsigned int *size;

//Method Declarations
bool kill(string message);
bool error(string message);
void initializeStuff();
void closeStuff();
void childProc(int childNum);
void threadFindPrimes(unsigned int from, const unsigned int to);
void turnOnBit(unsigned int number);
bool isBitOn(unsigned int whichNum);
void turnBitOff(unsigned int bit);
void turnBitOn(unsigned int);
unsigned int countPrimes();
void printAllPrimes();

//Takes the command line arguments, interprets them,
//Initializes all shared objects, and closes them once the
//Main program is finished executing.
int main(int argc, char **argv) {
	proc = false;
	//User Ran program without specifying number
	//Or tpe of children.
	//Assume 8 worker Threads
	if (argc < 2) {
		cout << "Quantity or Type Not Specified, So Assuming 8 Working Threads" << endl;
		workers = 8;
	}
	//User Provided Number of workers,
	//but not type.  Assume threads.
	else if (argc == 2) {
		cout << "You provided number, but not type, So assuming " << atoi (argv[1]) << " Working Threads" << endl;
		workers = atoi (argv[1]);
	}
	//User provided both Number of workers,
	//And type, so set the variables for that
	else if (argc == 3) {
		workers = atoi (argv[1]);
		if (argv[2] == "Processes" || argv[2] == "Process" || argv[2] == "processes" || argv[2] == "process")
			proc = true;
	}
	//User provided too many arguments.
	//Tell them such.
	else {
		cout << HELP << endl << "I can't handle all this awesomness!\n" << "Too many arguments!" << endl;
		kill("Invalid use of Prime.");
	}

	//There is where we switch to Processes if we are doing that, rather than Threads.
	if (proc == true)
		kill("processes not implemented yet!");

	//Set up The Shared Memory & semaphore
	initializeStuff();

	//Fire up the Threads!
	//Boost makes it really easy to create a ton of threads.
	for (int i = 0; i < workers; i++) {
		threadList.create_thread(boost::bind(childProc, i));
	}

	//Wait for all Threads to finish.
	//Boost makes it really easy to wait on a ton of threads.
	threadList.join_all();

	//Count The primes from the bitmap!
	cout << "Found " << countPrimes() << endl;
	//printAllPrimes();

	//Make sure we close all the shared stuff.
	closeStuff();
	return 0;
}

//Ends program with error.
bool kill(string message) {
	perror(message.c_str());
	exit(-1);
	return false;
}

//outputs a string to stderror
bool error(string message) {
	perror(message.c_str());
	return false;
}

//Sets up the Shared Memory and associated pointers
void initializeStuff() {
	//Create Shared Memory Object
	shmem_fd = shm_open("/villwocj_shmem", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
	if( shmem_fd == -1)
		kill("Can't open shmem object");

	//Expand Shared Memory Object
	if( ftruncate(shmem_fd, sizeof(int) + sizeof(unsigned char) * 536870912) == -1)
		kill("Failed to resize shmem object");

	//Set initial Pointer & Memory Map it.
	addr = mmap(NULL, sizeof(int) + sizeof(unsigned char) * 536870912, PROT_READ | PROT_WRITE, MAP_SHARED, shmem_fd, 0);
	if(MAP_FAILED == addr)
		kill("Memory Mapping failed");

	//at this point, everything is based on pointer arithmetic -- relative to addr
	size = (unsigned int*)addr;
	*size = 10;

	bitmap = (unsigned char*)(addr + sizeof(unsigned int));
}

//Closes all the shared objects we set up in initializeStuff()
void closeStuff() {
	//Make sure we remove our shm object.
	close(shmem_fd);
	shm_unlink("/villwocj_shmem");
}

//This is what each child will run.
void childProc(int childNum) {
	unsigned int numEach = (MAXNUMBER / workers) + 1;
	unsigned int startNum = childNum * numEach;
	unsigned int endNum = startNum + numEach;
	if (endNum > MAXNUMBER)
		endNum = MAXNUMBER;

	//Debug code for if values seem to be wrong:
	cout << "Thread " << childNum << " will find from " << startNum << " to " << endNum << endl;
	threadFindPrimes(startNum, endNum);
}

//This is the code each child will run to actually find their chunk of primes.
//This Code Originally From:
//http://create.stephan-brumme.com/eratosthenes
//Totally Changed.
void threadFindPrimes(unsigned int from, const unsigned int to) {
	for (unsigned int i = 3; i <= sqrt(to); i+=2) {
		//Makes code significantly faster:
		//skip multiples of 3, 5, 7, 11, 15, 17, ...
		/*
 		if (i % 3 && i > 5 == 0)
			continue;
		// skip multiples of five
		if (i % 5 && i > 7 == 0)
			continue;
		// skip multiples of seven
		if (i % 7 && i > 9 == 0)
			continue;
		// skip multiples of eleven
		if (i % 11 && i > 13 == 0)
			continue;
		// skip multiples of thirteen
		if (i % 13 && i > 15 == 0)
			continue;
		// skip multiples of seventeen
		if (i % 17 && i > 19 == 0)
			continue;
		*/

		//Minimum number to start working at:
		unsigned int minJ = i*i;
		if (minJ < from) {
			minJ = from;
			while (minJ % i != 0) {
				minJ++;
			}
		}

		// find all odd non-primes
		for (unsigned int j = minJ; j <= to; j += i) {
			//cout << "adding non-prime: " << j << " from/to: " << from << " " << to << endl;
		 	turnOnBit(j);
		}
	}
}

//Adds the number passed to it to the bitmap.
void turnOnBit(unsigned int number) {
	int primeByteLoc = (number - 1) / 8;
	int primeBitLoc = number - (primeByteLoc * 8);
	bitmap[primeByteLoc] |= 128 >> primeBitLoc;
}

//Not strictly checking if bit is on.
//Added code to make it considerably faster,
//Since we're only checking primes.
bool isBitOn(unsigned int whichNum) {
	//Even numbers are never Primes, so don't bother checking
	if (whichNum == 1)
		return true;
	if (whichNum == 2)
		return false;
	if (whichNum % 2 == 0)
		return true;
	//Prevents problems with negative numbers and such
	if (whichNum < 1)
		return false;

	//Which byte is this number located in?
	unsigned int whichByte = (whichNum - 1) / 8;

	//Byte was empty.  break.
	if(bitmap[whichByte] == 0)
		return false;

	//Which Bit are we looking for inside this byte?
	unsigned int whichBit = whichNum - (whichByte * 8);

	if(whichBit == 0) {
		if(bitmap[whichByte] & 128)
			return true;
	}
	else if(whichBit == 1) {
		if(bitmap[whichByte] & 64)
			return true;
	}
	else if(whichBit == 2) {
		if(bitmap[whichByte] & 32)
			return true;
	}
	else if(whichBit == 3) {
		if(bitmap[whichByte] & 16)
			return true;
	}
	else if(whichBit == 4) {
		if(bitmap[whichByte] & 8)
			return true;
	}
	else if(whichBit == 5) {
		if(bitmap[whichByte] & 4)
			return true;
	}
	else if(whichBit == 6) {
		if(bitmap[whichByte] & 2)
			return true;
	}
	else if(whichBit == 7) {
		if(bitmap[whichByte] & 1)
			return true;
	}
return false;
}

unsigned int countPrimes() {
	int found = 0;
	for (unsigned int i = 0; i <= MAXNUMBER; i++) {
		if (!isBitOn(i))
			found ++;
	}
	return found;
}

void printAllPrimes() {
	for (unsigned int i = 1; i < MAXNUMBER; i++) {
		if (!isBitOn(i))
			cout << i << endl;
	}
}
