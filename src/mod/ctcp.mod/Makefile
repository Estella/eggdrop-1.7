# Makefile for src/mod/ctcp.mod/
# $Id: Makefile,v 1.3 2004/08/27 09:34:11 wcc Exp $

srcdir = .


doofus:
	@echo ""
	@echo "Let's try this from the right directory..."
	@echo ""
	@cd ../../../ && make

static: ../ctcp.o

modules: ../../../ctcp.$(MOD_EXT)

../ctcp.o:
	$(CC) $(CFLAGS) $(CPPFLAGS) -DMAKING_MODS -c $(srcdir)/ctcp.c
	@rm -f ../ctcp.o
	mv ctcp.o ../

../../../ctcp.$(MOD_EXT): ../ctcp.o
	$(LD) -o ../../../ctcp.$(MOD_EXT) ../ctcp.o $(XLIBS) $(MODULE_XLIBS)
	$(STRIP) ../../../ctcp.$(MOD_EXT)

depend:
	$(CC) $(CFLAGS) -DMAKING_DEPEND -MM ctcp.c > .depend

clean:
	@rm -f .depend *.o *.$(MOD_EXT) *~

distclean: clean

#safety hash
