# Makefile for src/mod/server.mod/.
#
# $Id: Makefile,v 1.6 2004/09/10 01:10:51 wcc Exp $

MOD_NAME = server
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
../server.o: server.c ../../../src/mod/module.h ../../../src/main.h \
  ../../../config.h ../../../lush.h ../../../src/eggdrop.h \
  ../../../src/types.h ../../../src/debug.h ../../../src/logfile.h \
  ../../../src/lang.h ../../../src/flags.h ../../../src/language.h \
  ../../../src/tclegg.h ../../../src/tclhash.h ../../../src/chan.h \
  ../../../src/users.h ../../../src/compat/compat.h \
  ../../../src/compat/inet_aton.h ../../../src/compat/snprintf.h \
  ../../../src/compat/memset.h ../../../src/compat/memcpy.h \
  ../../../src/compat/strcasecmp.h ../../../src/compat/strftime.h \
  ../../../src/mod/modvals.h ../../../src/botmsg.h ../../../src/dcc.h \
  ../../../src/dccutil.h ../../../src/dns.h ../../../src/misc.h \
  ../../../src/net.h server.h servmsg.c cmdsserv.c tclserv.c
