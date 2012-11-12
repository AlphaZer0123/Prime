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

//Method Declarations
bool kill(string message);
bool error(string message);
void threadFindPrimes(const unsigned int from, const unsigned int to);
void addPrimeToBitMap(unsigned int prime, char[] *map);

int main(int argc, char **argv) {

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

		double *value = (double*)(addr + sizeof(int));

		unsigned char *bitmap = (unsigned char*)(addr + sizeof(int) + sizeof(double) * *size);



		threadFindPrimes(1, 1000000000); //1 billion for testing



		/*
		value[0] = 17.32;
		value[1] = M_PI;
		value[2] = M_E;
		value[3] = HUGE_VAL;
		value[4] = FLT_EPSILON;
		value[5] = FLT_MAX;
		value[6] = INFINITY;
		value[7] = NAN;
		*/

		// bitmap[0] represents 0-7
		// bitmap[1] represents 8-15

		/*
		if(bitmap[0] & 1 << 3)
			//bit 3 is set
			1;
		else
			//bit 3 is off
			2;

		bitmap[0] |= 1 << 3; //turns on bit 3
		*/

		/*
		  ~ bitwise not
		  ^ bitwise xor
		  << left shift
		  >> right shift

		  assignment operators
		  &=
		  |=
		  ~=
		  ^=
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

//This Code Originally From:
//http://create.stephan-brumme.com/eratosthenes
//Ever so slightly optomized and changed
void threadFindPrimes(const unsigned int from, const unsigned int to) {
	const int memorySize = (to - from + 1) / 2;

	//Setup
	bool* isPrime = new bool[memorySize];
	for (unsigned int i = 0; i < memorySize; i++)
		isPrime[i] = 1;

	for (unsigned int i = 3; i*i <= to; i+=2) {
		//skip multiples of three: 9, 15, 21, 27, ...
		//More than 13 doesn't give any improvement (test outweights its gain)
		if (i > 3 && i % 3 == 0)
			continue;
		// skip multiples of five
		if (i > 5 && i % 5 == 0)
			continue;
		// skip multiples of seven
		if (i > 7 && i % 7 == 0)
			continue;
		// skip multiples of eleven
		if (i > 11 && i % 11 == 0)
			continue;
		// skip multiples of thirteen
		if (i > 13 && i % 13 == 0)
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
	 unsigned int found = 0;
	 for (unsigned int i = 0; i < memorySize; i++)
		 found += isPrime[i];
	 // 2 is not odd => include on demand
	 if (from <= 2)
		 found++;

	 delete[] isPrime;
	 cout << found << endl;
}

//Adds the number passed to it to the bitmap.
void addPrimeToBitMap(unsigned int prime, char[] *map) {
	unsigned int indexLocOfNum = 0;
	unsigned int numLocInIndex = 0;
	indexLocOfNum = (prime - 1) / 8;
	numLocInIndex = prime - (indexLocOfNum * 8);
	map[indexLocOfNum] |= 1 << numLocInIndex; //turns on bit 3
}
