#ifndef STDINC_H_
#define STDINC_H_

#define _BSD_SOURCE

#define HELP "\nCorrect Program Usage:\nPrime [<number of workers >] [<type of workers>]\nNumber of workers will be assumed to be 10 if not specified\nType will be assumed to be Threads if not specified\n\nExamples of correct usage:\nPrime 10 threads\nPrime 5 processes\n"
#define MAXNUMBER 100000000
//4294967295  full 32 bit

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <semaphore.h>

#include <fcntl.h>
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <math.h>
#include <limits.h>
#include <float.h>
#include <string>

#include <boost/thread.hpp>
#include <boost/foreach.hpp>

using boost::thread;
using boost::thread_group;

#endif /* STDINC_H_ */
