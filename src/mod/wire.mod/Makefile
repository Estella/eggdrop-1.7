# Makefile for src/mod/wire.mod/
# $Id: Makefile,v 1.3 2004/08/27 09:34:11 wcc Exp $

srcdir = .


doofus:
	@echo ""
	@echo "Let's try this from the right directory..."
	@echo ""
	@cd ../../../ && make

static: ../wire.o

modules: ../../../wire.$(MOD_EXT)

../wire.o:
	$(CC) $(CFLAGS) $(CPPFLAGS) -DMAKING_MODS -c $(srcdir)/wire.c
	@rm -f ../wire.o
	mv wire.o ../

../../../wire.$(MOD_EXT): ../wire.o
	$(LD) -o ../../../wire.$(MOD_EXT) ../wire.o $(XLIBS) $(MODULE_XLIBS)
	$(STRIP) ../../../wire.$(MOD_EXT)

depend:
	$(CC) $(CFLAGS) -DMAKING_DEPEND -MM wire.c > .depend

clean:
	@rm -f .depend *.o *.$(MOD_EXT) *~

distclean: clean

#safety hash
