# Makefile for src/mod/assoc.mod/
# $Id: Makefile,v 1.2 2004/08/26 03:21:14 wcc Exp $

srcdir = .


doofus:
	@echo ""
	@echo "Let's try this from the right directory..."
	@echo ""
	@cd ../../../ && make

static: ../assoc.o

modules: ../../../assoc.$(MOD_EXT)

../assoc.o:
	$(CC) $(CFLAGS) $(CPPFLAGS) -DMAKING_MODS -c $(srcdir)/assoc.c
	@rm -f ../assoc.o
	mv assoc.o ../

../../../assoc.$(MOD_EXT): ../assoc.o
	$(LD) -o ../../../assoc.$(MOD_EXT) ../assoc.o $(XLIBS) $(MODULE_XLIBS)
	$(STRIP) ../../../assoc.$(MOD_EXT)

depend:
	$(CC) $(CFLAGS) $(CPPFLAGS) -MM $(srcdir)/assoc.c > .depend

clean:
	@rm -f .depend *.o *.$(MOD_EXT) *~

distclean: clean

#safety hash
