# Makefile for src/mod/channels.mod/
# $Id: Makefile,v 1.2 2004/08/26 03:21:14 wcc Exp $

srcdir = .


doofus:
	@echo ""
	@echo "Let's try this from the right directory..."
	@echo ""
	@cd ../../../ && make

static: ../channels.o

modules: ../../../channels.$(MOD_EXT)

../channels.o:
	$(CC) $(CFLAGS) $(CPPFLAGS) -DMAKING_MODS -c $(srcdir)/channels.c
	@rm -f ../channels.o
	mv channels.o ../

../../../channels.$(MOD_EXT): ../channels.o
	$(LD) -o ../../../channels.$(MOD_EXT) ../channels.o $(XLIBS) $(MODULE_XLIBS)
	$(STRIP) ../../../channels.$(MOD_EXT)

depend:
	$(CC) $(CFLAGS) $(CPPFLAGS) -MM $(srcdir)/channels.c > .depend

clean:
	@rm -f .depend *.o *.$(MOD_EXT) *~

distclean: clean

#safety hash
