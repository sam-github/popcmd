// irange.h
//
// IntRange parses a series of intergers in a format such as:
//	1-4,,3,3,5-8
// 
// Its pretty tolerant of malformed ranges and such, and doesn't
// deal with negative numbers.
//
// $Id$
// $Log$
// Revision 1.2  1999/07/16 15:59:16  sam
// error in initialization of range, pointer not being zeroed
//
// Revision 1.1  1999/01/24 02:07:14  sam
// Initial revision
//

#include <stdlib.h>

class IntRange
{
private:
	char* list_;
	char* cur_;

	int num_;
	int eor_; // end of range
	char state_;

	void Delete()
	{
		delete [] list_;
	}

	void Clear()
	{
		Delete();

		list_ = 0;
		cur_ = 0;
		num_ = 0;
		eor_ = 0;
		state_ = ',';
	}

public:
	IntRange() : list_(0), cur_(0)
	{
		Clear();
	}

	IntRange(const char* list) : list_(0), cur_(0)
	{
		Set(list);
	}

	int Set(const char* list)
	{
		Clear();

		if(!list)
		{
			return 0;
		}

		if(strspn(list, "0123456789,-") != strlen(list))
		{
			return 0;
		}

		list_ = new char [strlen(list) + 1];

		if(!list_) { return 0; }

		strcpy(list_, list);

		cur_ = list_;

		return 1;
	}

	int IntRange::Next(int* number)
	{
		if(!list_) { return 0; }

		if(state_ == '-')
		{
			if(num_ < eor_)
			{
				// return next in range
				num_++;
				if(number)
				{
					*number = num_;
					return 1;
				}
			}
			// else leave '-' mode and find the next number
			state_ = ',';
		}

		if(!*cur_) { return 0; }

		char* m = cur_;

		int done = 0;

		while(!done)
		{
			switch(*cur_)
			{
			case ',':
			case '-':
				state_ = *cur_;
				*cur_ = '\0';
				cur_++;
				done = 1;
				break;

			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				cur_++;
				break;

			case '\0': // end of input
				done = 1;
				break;

			default:	// this shouldn't happen, call it end of string
				return 0;
			}
		}

		if(!strlen(m))
		{
			// a ",," or something, try for another number
			return Next(number);
		}

		// ok, got a number
		num_ = atoi(m);

		// if we are in the "-" state, get the end of range
		if(state_ == '-')
		{
			// can't just just call Next(), otherwise "1--,,,,-,4"
			// would be seen as "1-4", better to see it as "1,4"

			if(*cur_ == ',' || *cur_ == '-')
			{
				// invalid input, so return the begin of range as a single
				// number, and move on
				cur_++;
				state_ = ',';
			}
			else
			{

				int num = num_; // save 'cause the Next() will trash it
				state_ = ',';
				int ok = Next(&eor_);
				num_ = num;
				state_ = '-';

				if(!ok)
				{
					state_ = ','; // chop the range short
				}
				// switch the ranges endpoints if necessary
				if(num_ > eor_)
				{
					int n = num_;
					num_ = eor_;
					eor_ = n;
				}
			}
		}

		if(number) { *number = num_; }

		return 1;
	}
};

