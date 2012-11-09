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

//Method Declarations
bool kill(string message);
bool error(string message);

int main(int argc, char **argv) {
	void *addr;

		shmem_fd = shm_open("/villwocj_shmem", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
		if( shmem_fd == -1)
			kill("Can't open shmem object");

		if( ftruncate(shmem_fd, 1024 * 1024 * sizeof(double) * 16) == -1)
			kill("Failed to resize shmem object");

		addr = mmap(NULL, 1024 * 1024 * sizeof(double) * 16, PROT_READ | PROT_WRITE, MAP_SHARED, shmem_fd, 0);

		if(MAP_FAILED == addr)
			kill("Map failed");

		/* at this point, everything is based on pointer arithmetic -- relative to addr */
		int* size = (int*)addr;
		*size = 10;

		double *value = (double*)(addr + sizeof(int));

		unsigned char *bitmap = (unsigned char*)(addr + sizeof(int) + sizeof(double) * *size);

		value[0] = 17.32;
		value[1] = M_PI;
		value[2] = M_E;
		value[3] = HUGE_VAL;
		value[4] = FLT_EPSILON;
		value[5] = FLT_MAX;
		value[6] = INFINITY;
		value[7] = NAN;

		// bitmap[0] represents 0-7
		// bitmap[1] represents 8-15

		if(bitmap[0] & 1 << 3)
			//bit 3 is set
			1;
		else
			//bit 3 is off
			2;

		bitmap[0] |= 1 << 3; //turns on bit 3

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
