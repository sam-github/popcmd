# Makefile: build popcmd
#
# $Id$

VERSION	= popcmds-$(shell cat Version)

prefix	= /usr/local/stow/popcmd

LDFLAGS   = -lsocket++ -lsocket -L/usr/local/lib
CXXFLAGS  = -g -I/usr/local/include

EXE = popcmd popstat irange

V = #-vv
S = guest:guest@localhost

.PHONY: build clean empty deps run

build: $(EXE)

run: build
	mailfetch -d localhost guest guest

popstat: popcmd
	ln -s popcmd popstat

install: $(EXE)
	mkdir -p $(prefix)
	mkdir -p $(prefix)/bin
	cp $(EXE) $(prefix)/bin/

release:
	mkdir -p $(VERSION)
	pax -w -f - < Manifest | (cd $(VERSION); tar -xf-)
	tar -cvf $(VERSION).tar $(VERSION)
	gzip -f $(VERSION).tar
	mv $(VERSION).tar.gz $(VERSION).tgz
	rm -Rf $(VERSION)

test:
	popcmd $V $S stat
	popcmd $V $S list
	popcmd $V $S list 2
	popcmd $V $S uidl
	popcmd $V $S uidl 2
	popcmd $V $S retr 2
	popcmd $V $S rset
	popcmd $V $S noop

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

