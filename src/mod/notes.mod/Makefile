# Makefile for src/mod/notes.mod
# $Id: Makefile,v 1.3 2004/08/27 09:34:11 wcc Exp $

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
	$(CC) $(CFLAGS) -DMAKING_DEPEND -MM notes.c > .depend

clean:
	@rm -f .depend *.o *.$(MOD_EXT) *~

distclean: clean

#safety hash
