# Makefile for src/mod/woobie.mod/
# $Id: Makefile,v 1.3 2004/08/27 09:34:11 wcc Exp $

srcdir = .


doofus:
	@echo ""
	@echo "Let's try this from the right directory..."
	@echo ""
	@cd ../../../ && make

static: ../woobie.o

modules: ../../../woobie.$(MOD_EXT)

../woobie.o:
	$(CC) $(CFLAGS) $(CPPFLAGS) -DMAKING_MODS -c $(srcdir)/woobie.c
	@rm -f ../woobie.o
	mv woobie.o ../

../../../woobie.$(MOD_EXT): ../woobie.o
	$(LD) -o ../../../woobie.$(MOD_EXT) ../woobie.o $(XLIBS) $(MODULE_XLIBS)
	$(STRIP) ../../../woobie.$(MOD_EXT)

depend:
	$(CC) $(CFLAGS) -DMAKING_DEPEND -MM woobie.c > .depend

clean:
	@rm -f .depend *.o *.$(MOD_EXT) *~

distclean: clean

#safety hash
