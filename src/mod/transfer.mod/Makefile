# Makefile for src/mod/transfer.mod/
# $Id: Makefile,v 1.2 2004/08/26 03:21:15 wcc Exp $

srcdir = .


doofus:
	@echo ""
	@echo "Let's try this from the right directory..."
	@echo ""
	@cd ../../../ && make

static: ../transfer.o

modules: ../../../transfer.$(MOD_EXT)

../transfer.o:
	$(CC) $(CFLAGS) $(CPPFLAGS) -DMAKING_MODS -c $(srcdir)/transfer.c
	@rm -f ../transfer.o
	mv transfer.o ../

../../../transfer.$(MOD_EXT): ../transfer.o
	$(LD) -o ../../../transfer.$(MOD_EXT) ../transfer.o $(XLIBS) $(MODULE_XLIBS)
	$(STRIP) ../../../transfer.$(MOD_EXT)

depend:
	$(CC) $(CFLAGS) $(CPPFLAGS) -MM $(srcdir)/transfer.c > .depend

clean:
	@rm -f .depend *.o *.$(MOD_EXT) *~

distclean: clean

#safety hash
