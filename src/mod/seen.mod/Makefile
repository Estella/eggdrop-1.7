# Makefile for src/mod/seen.mod/
# $Id: Makefile,v 1.2 2004/08/26 03:21:15 wcc Exp $

srcdir = .


doofus:
	@echo ""
	@echo "Let's try this from the right directory..."
	@echo ""
	@cd ../../../ && make

static: ../seen.o

modules: ../../../seen.$(MOD_EXT)

../seen.o:
	$(CC) $(CFLAGS) $(CPPFLAGS) -DMAKING_MODS -c $(srcdir)/seen.c
	@rm -f ../seen.o
	mv seen.o ../

../../../seen.$(MOD_EXT): ../seen.o
	$(LD) -o ../../../seen.$(MOD_EXT) ../seen.o $(XLIBS) $(MODULE_XLIBS)
	$(STRIP) ../../../seen.$(MOD_EXT)

depend:
	$(CC) $(CFLAGS) $(CPPFLAGS) -MM $(srcdir)/seen.c > .depend

clean:
	@rm -f .depend *.o *.$(MOD_EXT) *~

distclean: clean

#safety hash
