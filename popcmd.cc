// popcmd.cc
//
// Command-line oriented utility to manipulate a pop3 server.
//
// $Id$
// $Log$
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

int  msgid = -1;

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

void GetMsgId(char* argv[])
{
	if(optarg = argv[optind++]) { msgid = atoi(optarg); }
}

void Stat(pop3& p)
{	
	int	count, size;

	if(!p->stat(&count, &size)) { failed("stat", p); }

	cout << count << " " << size << endl;
}

void List(pop3& p)
{
	if(msgid != -1)
	{
		int	size;

		if(!p->list(msgid, &size)) { failed("list", p); }

		cout << size << endl;
	}
	else
	{
		if(!p->list(&cout)) { failed("list", p); }
	}
}

void Dele(pop3& p)
{
	if(!p->dele(msgid)) { failed("dele", p); }
}

void Retr(pop3& p)
{
	if(!p->retr(msgid, &cout)) { failed("retr", p); }
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
			if(argv[optind]) GetMsgId(argv);

		} else if(!strcmp(optarg, "dele")) {
			cmd = Dele;
			GetMsgId(argv);

		} else if(!strcmp(optarg, "retr")) {
			cmd = Retr;
			GetMsgId(argv);

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

