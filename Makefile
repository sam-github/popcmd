# Makefile: build mailfetch
# $Id$
# $Log$
# Revision 1.1  1999/01/23 05:25:01  sam
# Initial revision
#
# Revision 1.1  1998/09/20 07:18:58  sroberts
# Initial revision
#

srcdir = .

LDFLAGS   = -l socket++ -lsocket -L$(srcdir)/..
CXXFLAGS  = -I$(srcdir)/.. -g #-D__STDC__

DEPEND_SOURCES = $(srcdir)/*.cc

EXE = popcmd popstat

.PHONY: build clean empty deps run

build: $(EXE)

usage:
	@mailfetch -h | usemsg mailfetch -

run: build
	mailfetch -d localhost guest guest

popcmd: ../libsocket++.a

popstat: popcmd
	ln -s popcmd popstat

clean:
	-rm -f *~ *.o core *.err

empty: clean
	-rm -f $(EXE) *.map

deps:
	makedeps -- $(CXXFLAGS) -D__cplusplus -- -f - *.cc > depends.mak

-include depends.mak

# build rules
%: %.cc
	$(LINK.cc) $^ $(LOADLIBES) $(LDLIBS) -o $@
	$@ -h | usemsg $@ -

%: %.o
	$(LINK.cc) $^ $(LOADLIBES) $(LDLIBS) -o $@
	$@ -h | usemsg $@ -

