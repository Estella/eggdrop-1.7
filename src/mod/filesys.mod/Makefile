# Makefile for src/mod/filesys.mod/
# $Id: Makefile,v 1.2 2004/08/26 03:21:14 wcc Exp $

srcdir = .


doofus:
	@echo ""
	@echo "Let's try this from the right directory..."
	@echo ""
	@cd ../../../ && make

static: ../filesys.o

modules: ../../../filesys.$(MOD_EXT)

../filesys.o:
	$(CC) $(CFLAGS) $(CPPFLAGS) -DMAKING_MODS -c $(srcdir)/filesys.c
	@rm -f ../filesys.o
	mv filesys.o ../

../../../filesys.$(MOD_EXT): ../filesys.o
	$(LD) -o ../../../filesys.$(MOD_EXT) ../filesys.o $(XLIBS) $(MODULE_XLIBS)
	$(STRIP) ../../../filesys.$(MOD_EXT)

depend:
	$(CC) $(CFLAGS) $(CPPFLAGS) -MM $(srcdir)/filesys.c > .depend

clean:
	@rm -f .depend *.o *.$(MOD_EXT) *~

distclean: clean

#safety hash
