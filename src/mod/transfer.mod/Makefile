# Makefile for src/mod/transfer.mod/
# $Id: Makefile,v 1.3 2004/08/27 09:34:11 wcc Exp $

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
	$(CC) $(CFLAGS) -DMAKING_DEPEND -MM transfer.c > .depend

clean:
	@rm -f .depend *.o *.$(MOD_EXT) *~

distclean: clean

#safety hash
