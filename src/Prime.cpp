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
void setBitInBitMap(unsigned int prime);
bool isBitOn(unsigned int whichNum);
void turnBitOff(unsigned int bit);
void turnBitOn(unsigned int);
unsigned int countPrimes();
void printAllPrimes();

int main(int argc, char **argv) {
	//just ran program.  assume 10 working children
	if (argc < 2) {
		cout << "Quantity or Type Not Specified, So Assuming 8 Working Threads" << endl;
		workers = 8;
		proc = false;
	}
	//Provided just number, but not type.  Assume threads
	else if (argc == 2) {
		cout << "You provided number, but not type, So assuming " << atoi (argv[1]) << " Working Threads" << endl;
		workers = atoi (argv[1]);
		proc = false;
	}
	//Provided both number and type.
	else if (argc == 3) {
		workers = atoi (argv[1]);
		if (argv[2] == "Processes" || argv[2] == "Process" || argv[2] == "processes" || argv[2] == "process")
			proc = true;
		else
			proc = false;
	}
	//Too many arguments
	else {
		cout << HELP << endl << "I can't handle all this awesomness!\n" << "Too many arguments!" << endl;
		kill("Invalid use of Prime.");
	}

	if (proc == true)
		kill("processes not implemented yet!");

	//Set up The Shared Memory & semaphore
	initializeStuff();

	//Fire up the Threads!
	for (int i = 0; i < workers; i++) {
		threadList.create_thread(boost::bind(childProc, i));
	}

	//Wait for all Threads to finish.
	threadList.join_all();

	//threadFindPrimes(51, 100);

	cout << "Found " << countPrimes() << endl;
	//printAllPrimes();

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

void closeStuff() {
	//Make sure we remove our shm object.
	close(shmem_fd);
	shm_unlink("/villwocj_shmem");
}

void childProc(int childNum) {
	unsigned int numEach = (sqrt(MAXNUMBER) / workers) + 1;
	unsigned int startNum = childNum * numEach;
	unsigned int endNum = startNum + numEach;
	if (endNum > MAXNUMBER)
		endNum = MAXNUMBER;

	//Debug code for if values seem to be wrong:
	cout << "Thread " << childNum << " will find from " << startNum << " to " << endNum << endl;
	threadFindPrimes(startNum, endNum);
}

//This Code Originally From:
//http://create.stephan-brumme.com/eratosthenes
//Totally Changed.
void threadFindPrimes(unsigned int from, const unsigned int to) {
	if (from < 3)
		from = 3;
	if (from % 2 == 0)
		from++;
	for (unsigned int i = from; i <= to; i+=2) {
		//skip multiples of three: 9, 15, 21, 27, ...
		if (i > 5 && i % 3 == 0)
			continue;
		// skip multiples of five
		if (i > 7 && i % 5 == 0)
			continue;
		// skip multiples of seven
		if (i > 9 && i % 7 == 0)
			continue;
		// skip multiples of eleven
		if (i > 13 && i % 11 == 0)
			continue;
		// skip multiples of thirteen
		if (i > 15 && i % 13 == 0)
			continue;
		// skip multiples of seventeen
		if (i > 19 && i % 17 == 0)
			continue;

		// find all odd non-primes
		for (unsigned int j = i*i; j <= MAXNUMBER; j += i) {
			//cout << "adding non-prime: " << j << " from/to: " << from << " " << to << endl;
		 	setBitInBitMap(j);
		}
	}
}

//Adds the number passed to it to the bitmap.
void setBitInBitMap(unsigned int prime) {
	int primeByteLoc = (prime - 1) / 8;
	int primeBitLoc = prime - (primeByteLoc * 8);
	bitmap[primeByteLoc] |= 128 >> primeBitLoc;
}

bool isBitOn(unsigned int whichNum) {
	if (whichNum % 2 == 0)
		return true;
	if (whichNum < 1)
		return false;

	unsigned int whichByte = 0;
	unsigned int whichBit = 0;
	whichByte = (whichNum - 1) / 8;

	//Byte was empty.  break.
	if(bitmap[whichByte] == 0)
		return false;

	//Which Bit are we looking for inside this byte?
	whichBit = whichNum - (whichByte * 8);

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
