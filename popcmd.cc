// popcmd.cc
//
// Command-line oriented utility to manipulate a pop3 server.
//
// $Id$
// $Log$
// Revision 1.3  1999/10/03 22:52:14  sam
// implemented the top command
//
// Revision 1.2  1999/01/24 02:07:14  sam
// all commands now take , and - delimited lists of msg ids
//
// Revision 1.1  1999/01/23 05:25:01  sam
// Initial revision
//

#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <strstream.h>
#include <fstream.h>

#include <pop3.h>

#include "irange.h"

inline void failed(char* cmd, pop3& p)
{
	// skip past the "-ERR " part of the response
	const char* reason = p->response() + strlen("-ERR ");

	cerr << "command \"" << cmd << "\" failed: " << reason << endl;
	exit(1);
}


char* name = 0;
int   verbosity = 1;
char* host = 0;
char* user = 0;
char* pass = 0;
void (*cmd)(pop3&);

IntRange msgid;

char* USAGE =
	"%s - command line tool to manipulate a pop3 server\n"
	"\n"
	"usage:\n"
	"  popcmd  [-hvq] <server> <cmd> [<msgid>]\n"
	"  pop<cmd> [-hvq] <server> [<msgid>]\n"
	"\n"
	"  -h    print this help message\n"
	"  -v    increase the verbosity (default is 1)\n"
	"  -q    set verbosity to 0\n"
	"\n"
	" If not called as pop<cmd>, where <cmd> is described below, then\n"
	" the command type must be specified explicitly\n"
	"\n"
	" server: username:password@popserver\n"
	"\n"
	" cmd:\n"
	"     stat        returns a pair of integers, the number of messages\n"
	"                 and total size in octets (bytes) of the mailbox\n"
	"     list        returns a list of msgids and message size pairs\n"
	"     list msgid  returns a size of the indicate message\n"
	"     dele msgid  deletes the designated message\n"
	"     retr msgid  retrieves the designated message to stdout\n"
	"     top  msgid  retrieves the header of the designated message to stdout\n"
	"\n"
	" See: RFC 1939 - Post Office Protocol - Version 3\n"
	;

void usage(ostream& os = cerr)
{
	char buf[1024];

	sprintf(buf, USAGE,
		name,
		name
		);

	os << buf;
}

void FormatError(const char* server)
{
	cerr << "failed: invalid server '" << server << "'" << endl;

	exit(1);
}

void SetMsgId(const char* idl, int optional = 0)
{
	if(!idl && optional) { return; }

	if(!msgid.Set(idl))
	{
		cerr << "failed: invalid msgid list" << endl;
		exit(1);
	}
}

void Stat(pop3& p)
{	
	int	count, size;

	if(!p->stat(&count, &size)) { failed("stat", p); }

	cout << count << " " << size << endl;
}

void List(pop3& p)
{
	int m;

	if(msgid.Next(&m))
	{
		int lookahead;

		// if there is only one msg, print its size and return
		if(!msgid.Next(&lookahead)) {
			int	size;
			if(!p->list(m, &size)) { failed("list", p); }
			cout << size << endl;
			return;
		}
		// otherwise we're dealing with a list, print the first since we looked
		// ahead and then loop
		int	size;
		if(!p->list(m, &size)) { failed("list", p); }
		cout << m << " " << size << endl;

		m = lookahead;
		do {
			int	size;
			if(!p->list(m, &size)) { failed("list", p); }
			cout << m << " " << size << endl;

		} while(msgid.Next(&m));
	}
	else
	{
		if(!p->list(&cout)) { failed("list", p); }
	}
}

void Dele(pop3& p)
{
	int m;

	while(msgid.Next(&m))
	{
		if(verbosity >= 2) { cout << "dele: " << m << endl; }

		if(!p->dele(m)) { failed("dele", p); }
	}
}

void Retr(pop3& p)
{
	int m;
	while(msgid.Next(&m))
	{
		if(!p->retr(m, &cout)) { failed("retr", p); }
	}
}

void Top(pop3& p)
{
	int m;
	while(msgid.Next(&m))
	{
		if(!p->top(m, 0, &cout)) { failed("top", p); }
	}
}

void main(int argc, char* argv[])
{
	name = argv[0];

	opterr = 0;
	for(int c; (c = getopt(argc, argv, "hvq")) != -1;)
	{
		switch(c)
		{
		case 'h':
			usage(cout);
			exit(0);
			break;

		case 'v': verbosity++;		break;
		case 'q': verbosity = 0;	break;

		default:
			cerr << "failed: unknown option '" << char(optopt) << "'" << endl;
			exit(1);
		}
	}

	if((optarg = argv[optind++]))
	{
		char* server = strdup(optarg);

		user = optarg;

		char* p = strpbrk(optarg, ":@");
		if(!p) { FormatError(server); }

		if(*p == ':')
		{
			*p = '\0';
			pass = p + 1;
			p = strpbrk(pass, "@");
			if(!p) { FormatError(server); }
		}

		if(*p == '@')
		{
			*p = '\0';
			host = p + 1;
		}
		else
		{
			FormatError(server);
		}

		// none of these can be null strings
		if(!*user && !*pass && !*host) { FormatError(server); }
	}
	else
	{
		cerr << "failed: missing command" << endl;
		usage();
		exit(1);
	}

	if(verbosity >= 2)
	{
		cout << name << ":"
			<< " user " << user
			<< " pass " << pass
			<< " host " << host
			<< endl;
	}

	if(optarg = argv[optind++])
	{
		if(!strcmp(optarg, "stat")) {
			cmd = Stat;

		} else if(!strcmp(optarg, "list")) {
			cmd = List;
			SetMsgId(argv[optind], 1);

		} else if(!strcmp(optarg, "dele")) {
			cmd = Dele;
			SetMsgId(argv[optind]);

		} else if(!strcmp(optarg, "retr")) {
			cmd = Retr;
			SetMsgId(argv[optind]);

		} else if(!strcmp(optarg, "top")) {
			cmd = Top;
			SetMsgId(argv[optind]);

		} else {
			if(!cmd) { cerr << "unknown command: " << optarg << endl; }
			exit(1);
		}
	}

	ostream *debug = 0;

	if(verbosity > 2) { debug = &cout; }

	pop3 p(debug);

	if((errno = p->connect(host)))
	{
		cerr << "connect failed: [" << errno << "] " << strerror(errno) << endl;
		exit(1);
	}

	if(!p->checkconnect()) { failed("connect", p); }

	if(!p->user(user)) { failed("user", p); }

	if(!p->pass(pass)) { failed("pass", p); }

	cmd(p);

	if(!p->quit()) { failed("quit", p); }

	exit(0);
}

