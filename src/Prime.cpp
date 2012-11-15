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
void *addr;
unsigned char *bitmap;
double *value;

//Method Declarations
bool kill(string message);
bool error(string message);
void initializeMemory();
void childProc(string childNum);
void threadFindPrimes(const unsigned int from, const unsigned int to);
void addPrimeToBitMap(unsigned int prime);
bool isBitOn(unsigned int whichNum);
void turnBitOff(unsigned int bit);
void turnBitOn(unsigned int);
unsigned int countPrimes();
bool isNumInPrimeList(unsigned int num);
void printPrimes();

int main(int argc, char **argv) {

	//Set up The Shared Memory
	initializeMemory();

	thread t1(childProc, "1");

	threadFindPrimes(1, 1000000);

	t1.join();

	threadFindPrimes(1, 1000000);

	//threadFindPrimes(1, 4294967295); //Unsigned 32 bit integer size.

	//for (int i = 1; i < 999999; i++) {
	//	if (isBitOn(i))
	//		cout << i << endl;
	//}

	/*
	bitmap[0] represents 0-7
	bitmap[1] represents 8-15
	if(bitmap[0] & 1 << 3)	//bit 3 is set
		1;
	else					//bit 3 is off
		2;
	bitmap[0] |= 1 << 3; //turns on bit 3
	*/

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
void initializeMemory() {
	//Create Shared Memory Object
	shmem_fd = shm_open("/villwocj_shmem", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
	if( shmem_fd == -1)
		kill("Can't open shmem object");

	//Expand Shared Memory Object
	if( ftruncate(shmem_fd, 1024 * 1024 * sizeof(double) * 16) == -1)
		kill("Failed to resize shmem object");

	//Set initial Pointer & Memory Map it.
	addr = mmap(NULL, 1024 * 1024 * sizeof(double) * 16, PROT_READ | PROT_WRITE, MAP_SHARED, shmem_fd, 0);
	if(MAP_FAILED == addr)
		kill("Memory Maping failed");

	//at this point, everything is based on pointer arithmetic -- relative to addr
	int* size = (int*)addr;
	*size = 10;

	value = (double*)(addr + sizeof(int));

	bitmap = (unsigned char*)(addr + sizeof(int) + sizeof(double) * *size);
}

void childProc(string childNum) {
	cout << "thread " << childNum << " ran!" << endl;
}

//This Code Originally From:
//http://create.stephan-brumme.com/eratosthenes
//Ever so slightly optomized and changed
void threadFindPrimes(const unsigned int from, const unsigned int to) {
	const unsigned int memorySize = (to - from + 1) / 2;

	//Setup
	char* isPrime = new char[memorySize];
	for (unsigned int i = 0; i < memorySize; i++)
		isPrime[i] = 1;
	isPrime[0] = 0;

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
			addPrimeToBitMap( (i * 2) + 1);
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
	int primeByteLoc = 0;
	int primeBitLoc = 0;
	primeByteLoc = (prime - 1) / 8;
	primeBitLoc = prime - (primeByteLoc * 8);
	bitmap[primeByteLoc] |= 128 >> primeBitLoc;
}

bool isBitOn(unsigned int whichNum) {
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
