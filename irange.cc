#include <iostream.h>
#include "irange.h"

int main(int argc, char* argv[])
{
	IntRange ir;

	if(!ir.Set(argv[1]))
	{
		cout << "failed to Set()" << endl;
		return 1;
	}

	int n;
	while(ir.Next(&n))
	{
		cout << "next: " << n << endl;
	}

	return 0;
}

