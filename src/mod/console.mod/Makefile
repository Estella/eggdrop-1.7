# Makefile for src/mod/console.mod/
# $Id: Makefile,v 1.3 2004/08/27 09:34:11 wcc Exp $

srcdir = .


doofus:
	@echo ""
	@echo "Let's try this from the right directory..."
	@echo ""
	@cd ../../../ && make

static: ../console.o

modules: ../../../console.$(MOD_EXT)

../console.o:
	$(CC) $(CFLAGS) $(CPPFLAGS) -DMAKING_MODS -c $(srcdir)/console.c
	@rm -f ../console.o
	mv console.o ../

../../../console.$(MOD_EXT): ../console.o
	$(LD) -o ../../../console.$(MOD_EXT) ../console.o $(XLIBS) $(MODULE_XLIBS)
	$(STRIP) ../../../console.$(MOD_EXT)

depend:
	$(CC) $(CFLAGS) -DMAKING_DEPEND -MM console.c > .depend

clean:
	@rm -f .depend *.o *.$(MOD_EXT) *~

distclean: clean

#safety hash
