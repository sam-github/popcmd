#include <iostream.h>
#include "irange.h"

int main(int argc, char* argv[])
{
	if(argv[1] == 0 || strcmp(argv[1], "-h") == 0)
	{
		cout <<
			"Usage: irange <-h|range>"
			"\n"
			"driver for testing the IntRange class\n"
			;

		exit(0);
	}

	IntRange ir;

	if(!ir.Set(argv[1]))
	{
		cerr << "failed to Set()" << endl;
		return 1;
	}

	int n;
	while(ir.Next(&n))
	{
		cout << "next: " << n << endl;
	}

	return 0;
}

