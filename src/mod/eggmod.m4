dnl eggmod.m4
dnl
dnl $Id: eggmod.m4,v 1.1 2004/08/25 01:02:06 wcc Exp $

dnl EGG_REMOVE_MOD(MODULE-NAME)
dnl
dnl Removes a module from the list of modules to be compiled.
define(EGG_REMOVE_MOD,
[
  ${srcdir}/../../../misc/modconfig -q --top_srcdir=${srcdir}/../../.. --bindir=../../.. del $1
])
