# Makefile for src/mod/uptime.mod/
# $Id: Makefile,v 1.2 2004/08/26 03:21:15 wcc Exp $
srcdir = .


doofus:
	@echo ""
	@echo "Let's try this from the right directory..."
	@echo ""
	@cd ../../../ && make

static: ../uptime.o

modules: ../../../uptime.$(MOD_EXT)

../uptime.o:
	$(CC) $(CFLAGS) $(CPPFLAGS) -DMAKING_MODS -c $(srcdir)/uptime.c
	@rm -f ../uptime.o
	mv uptime.o ../

../../../uptime.$(MOD_EXT): ../uptime.o
	$(LD) -o ../../../uptime.$(MOD_EXT) ../uptime.o $(XLIBS) $(MODULE_XLIBS)
	$(STRIP) ../../../uptime.$(MOD_EXT)

depend:
	$(CC) $(CFLAGS) $(CPPFLAGS) -MM $(srcdir)/uptime.c > .depend

clean:
	@rm -f .depend *.o *.$(MOD_EXT) *~

distclean: clean

#safety hash
