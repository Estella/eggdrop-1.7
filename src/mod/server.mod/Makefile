# Makefile for src/mod/server.mod/
# $Id: Makefile,v 1.3 2004/08/27 09:34:11 wcc Exp $

srcdir = .


doofus:
	@echo ""
	@echo "Let's try this from the right directory..."
	@echo ""
	@cd ../../../ && make

static: ../server.o

modules: ../../../server.$(MOD_EXT)

../server.o:
	$(CC) $(CFLAGS) $(CPPFLAGS) -DMAKING_MODS -c $(srcdir)/server.c
	@rm -f ../server.o
	mv server.o ../

../../../server.$(MOD_EXT): ../server.o
	$(LD) -o ../../../server.$(MOD_EXT) ../server.o $(XLIBS) $(MODULE_XLIBS)
	$(STRIP) ../../../server.$(MOD_EXT)

depend:
	$(CC) $(CFLAGS) -DMAKING_DEPEND -MM server.c > .depend

clean:
	@rm -f .depend *.o *.$(MOD_EXT) *~

distclean: clean

#safety hash
