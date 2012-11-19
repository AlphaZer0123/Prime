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

thread_group threadList;

//Command line options
bool proc;
int workers;
unsigned int maxNumber;

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
void addPrimeToBitMap(unsigned int prime);
bool isBitOn(unsigned int whichNum);
void turnBitOff(unsigned int bit);
void turnBitOn(unsigned int);
unsigned int countPrimes();
void printAllPrimes();

int main(int argc, char **argv) {
	maxNumber = 100;
	//4294967295  full 32 bit

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
	//Provided both number and type, but not max Workers
	else if (argc == 3) {
		workers = atoi (argv[1]);
		if (argv[2] == "Processes" || argv[2] == "Process" || argv[2] == "processes" || argv[2] == "process")
			proc = true;
		else
			proc = false;
	}
	//Provided full arguments
	else if (argc == 4) {
		workers = atoi (argv[1]);
			if (argv[2] == "Processes" || argv[2] == "Process" || argv[2] == "processes" || argv[2] == "process")
				proc = true;
			else
				proc = false;
		maxNumber = atoi (argv[3]);
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
	printAllPrimes();

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
	unsigned int numEach = (maxNumber / workers) + 1;
	unsigned int startNum = childNum * numEach;
	unsigned int endNum = startNum + numEach;
	if (endNum > maxNumber)
		endNum = maxNumber;

	//Debug code for if values seem to be wrong:
	cout << "Thread " << childNum << " will find from " << startNum << " to " << endNum << endl;
	threadFindPrimes(startNum, endNum);
}

//This Code Originally From:
//http://create.stephan-brumme.com/eratosthenes
//Ever so slightly optomized and changed
void threadFindPrimes(unsigned int from, const unsigned int to) {
	const unsigned int memorySize = (to - from + 1) / 2;

	//Setup
	char* isPrime = new char[memorySize];
	for (unsigned int i = 0; i < memorySize; i++)
		isPrime[i] = 1;
	//isPrime[0] = 0;

	for (unsigned int i = 3; i*i <= to; i+=2) {
		//skip multiples of three: 9, 15, 21, 27, ...
		//More than 13 doesn't give any improvement (test outweighs its gain)
		if (i >= 3*3 && i % 3 == 0)
			continue;
		// skip multiples of five
		if (i >= 5*5 && i % 5 == 0)
			continue;
		// skip multiples of seven
		if (i >= 7*7 && i % 7 == 0)
			continue;
		// skip multiples of eleven
		if (i >= 11*11 && i % 11 == 0)
			continue;
		// skip multiples of thirteen
		if (i >= 13*13 && i % 13 == 0)
			continue;
		// skip multiples of seventeen
		if (i >= 17*17 && i % 17 == 0)
			continue;

		// skip numbers before current slice
		unsigned int minJ = ((from+i-1)/i)*i;
		if (minJ < i*i)
			minJ = i*i;

		// start value must be odd
		if ((minJ & 1) == 0)
			minJ += i;

		// find all odd non-primes
		for (unsigned int j = minJ; j <= to; j += 2*i) {
			unsigned int index = j - from;
		 	isPrime[index/2] = 0;
		}
	}
	// count primes in this block

	int found = 0;
	for (unsigned int i = 0; i < memorySize; i++) {
		if (isPrime[i] == 1) {
			addPrimeToBitMap( (i * 2) + 1 + from);
			found++;
		}
	}
	// 2 is not odd => include on demand
	if (from <= 2) {
		found++;
		addPrimeToBitMap(2);
	}

	delete[] isPrime;
	cout << "found: " << found << " primes." << endl;
}

//Adds the number passed to it to the bitmap.
void addPrimeToBitMap(unsigned int prime) {
	int primeByteLoc = (prime - 1) / 8;
	int primeBitLoc = prime - (primeByteLoc * 8);
	bitmap[primeByteLoc] |= 128 >> primeBitLoc;
	cout << "adding prime: " << prime << " to byte: " << primeByteLoc << "(" << &primeByteLoc << ") and bit: " << primeBitLoc << endl;
}

bool isBitOn(unsigned int whichNum) {
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
	for (unsigned int i = 0; i <= maxNumber; i++) {
		if (isBitOn(i))
			found ++;
	}
	return found;
}

void printAllPrimes() {
	for (unsigned int i = 1; i < maxNumber; i++) {
		if (isBitOn(i))
			cout << i << endl;
	}
}
