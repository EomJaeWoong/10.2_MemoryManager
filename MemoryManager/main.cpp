#include "MemoryMsg.h"

void main()
{
	int *i = new int;
	char *c = new char[10];
	long *l = new long[30];
	float *f = new float[2];
	
	delete[] c;
	delete l;
	//delete c;
}