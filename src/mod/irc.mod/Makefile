# Makefile for src/mod/irc.mod/
# $Id: Makefile,v 1.2 2004/08/26 03:21:14 wcc Exp $

srcdir = .


doofus:
	@echo ""
	@echo "Let's try this from the right directory..."
	@echo ""
	@cd ../../../ && make

static: ../irc.o

modules: ../../../irc.$(MOD_EXT)

../irc.o:
	$(CC) $(CFLAGS) $(CPPFLAGS) -DMAKING_MODS -c $(srcdir)/irc.c
	@rm -f ../irc.o
	mv irc.o ../

../../../irc.$(MOD_EXT): ../irc.o
	$(LD) -o ../../../irc.$(MOD_EXT) ../irc.o $(XLIBS) $(MODULE_XLIBS)
	$(STRIP) ../../../irc.$(MOD_EXT)

depend:
	$(CC) $(CFLAGS) $(CPPFLAGS) -MM $(srcdir)/irc.c > .depend

clean:
	@rm -f .depend *.o *.$(MOD_EXT) *~

distclean: clean

#safety hash
