//This Code Originally From:
//http://create.stephan-brumme.com/eratosthenes
//Ever so slightly optomized and changed
void threadFindPrimes(const unsigned int from, const unsigned int to) {
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
		for (unsigned int j = i*i; j <= to; j += i) {
		 	setBitInBitMap(j);
		}
	}
	delete[] isPrime;
}