# Makefile for src/mod/notes.mod
# $Id: Makefile,v 1.2 2004/08/26 03:21:15 wcc Exp $

srcdir = .


doofus:
	@echo ""
	@echo "Let's try this from the right directory..."
	@echo ""
	@cd ../../../ && make

static: ../notes.o

modules: ../../../notes.$(MOD_EXT)

../notes.o:
	$(CC) $(CFLAGS) $(CPPFLAGS) -DMAKING_MODS -c $(srcdir)/notes.c
	@rm -f ../notes.o
	mv notes.o ../

../../../notes.$(MOD_EXT): ../notes.o
	$(LD) -o ../../../notes.$(MOD_EXT) ../notes.o $(XLIBS) $(MODULE_XLIBS)
	$(STRIP) ../../../notes.$(MOD_EXT)

depend:
	$(CC) $(CFLAGS) $(CPPFLAGS) -MM $(srcdir)/notes.c > .depend

clean:
	@rm -f .depend *.o *.$(MOD_EXT) *~

distclean: clean

#safety hash
