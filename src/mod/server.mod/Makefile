# Makefile for src/mod/server.mod/
# $Id: Makefile,v 1.2 2004/08/26 03:21:15 wcc Exp $

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
	$(CC) $(CFLAGS) $(CPPFLAGS) -MM $(srcdir)/server.c > .depend

clean:
	@rm -f .depend *.o *.$(MOD_EXT) *~

distclean: clean

#safety hash
