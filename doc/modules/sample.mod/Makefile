# Makefile for sample.mod.
#
# This is an example Makefile for a module.
#
# $Id: Makefile,v 1.1 2004/12/02 22:15:52 wcc Exp $

MOD_NAME = sample
srcdir = .


doofus:
	@echo ""
	@echo "Let's try this from the right directory..."
	@echo ""
	@cd ../../../ && make

static: ../$(MOD_NAME).o

modules: ../../../$(MOD_NAME).$(MOD_EXT)

../$(MOD_NAME).o:
	$(CC) $(CFLAGS) -DMAKING_MODS -c $(srcdir)/$(MOD_NAME).c
	@rm -f ../$(MOD_NAME).o
	mv $(MOD_NAME).o ../

../../../$(MOD_NAME).$(MOD_EXT): ../$(MOD_NAME).o
	$(LD) -o ../../../$(MOD_NAME).$(MOD_EXT) ../$(MOD_NAME).o $(XLIBS) $(MODULE_XLIBS)
	$(STRIP) ../../../$(MOD_NAME).$(MOD_EXT)

depend:
	$(CC) $(CFLAGS) -MM $(MOD_NAME).c -MT ../$(MOD_NAME).o > .depend

clean:
	@rm -f .depend *.o *.$(MOD_EXT) *~

distclean: clean

#safety hash
