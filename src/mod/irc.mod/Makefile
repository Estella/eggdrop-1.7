# Makefile for src/mod/irc.mod/
# $Id: Makefile,v 1.3 2004/08/27 09:34:11 wcc Exp $

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
	$(CC) $(CFLAGS) -DMAKING_DEPEND -MM irc.c > .depend

clean:
	@rm -f .depend *.o *.$(MOD_EXT) *~

distclean: clean

#safety hash
