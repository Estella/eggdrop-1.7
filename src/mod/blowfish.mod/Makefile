# Makefile for src/mod/blowfish.mod/
# $Id: Makefile,v 1.2 2004/08/26 03:21:14 wcc Exp $

srcdir = .


doofus:
	@echo ""
	@echo "Let's try this from the right directory..."
	@echo ""
	@cd ../../../ && make

static: ../blowfish.o

modules: ../../../blowfish.$(MOD_EXT)

../blowfish.o:
	$(CC) $(CFLAGS) $(CPPFLAGS) -DMAKING_MODS -c $(srcdir)/blowfish.c
	@rm -f ../blowfish.o
	mv blowfish.o ../

../../../blowfish.$(MOD_EXT): ../blowfish.o
	$(LD) -o ../../../blowfish.$(MOD_EXT) ../blowfish.o $(XLIBS) $(MODULE_XLIBS)
	$(STRIP) ../../../blowfish.$(MOD_EXT)

depend:
	$(CC) $(CFLAGS) $(CPPFLAGS) -MM $(srcdir)/blowfish.c > .depend

clean:
	@rm -f .depend *.o *.$(MOD_EXT) *~

distclean: clean

#safety hash
