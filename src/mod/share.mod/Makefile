# Makefile for src/mod/share.mod/
# $Id: Makefile,v 1.3 2004/08/27 09:34:11 wcc Exp $

srcdir = .


doofus:
	@echo ""
	@echo "Let's try this from the right directory..."
	@echo ""
	@cd ../../../ && make

static: ../share.o

modules: ../../../share.$(MOD_EXT)

../share.o:
	$(CC) $(CFLAGS) $(CPPFLAGS) -DMAKING_MODS -c $(srcdir)/share.c
	@rm -f ../share.o
	mv share.o ../

../../../share.$(MOD_EXT): ../share.o
	$(LD) -o ../../../share.$(MOD_EXT) ../share.o $(XLIBS) $(MODULE_XLIBS)
	$(STRIP) ../../../share.$(MOD_EXT)

depend:
	$(CC) $(CFLAGS) -DMAKING_DEPEND -MM share.c > .depend

clean:
	@rm -f .depend *.o *.$(MOD_EXT) *~

distclean: clean

#safety hash
