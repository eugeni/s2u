#!/bin/sh
#---------------------------------------------------------------
# Project         : Mandrakelinux
# Module          : s2u
# File            : s2u.sh
# Version         : $Id$
# Author          : Frederic Lepied
# Created On      : Sat Jul 31 01:03:37 2004
# Purpose         : launch the dbus client attaching it to 
#                   system bus
#---------------------------------------------------------------

CURR_DISPLAY=`LC_ALL=C xdpyinfo | grep 'display' | awk '{print $4}'`

exec s2u --daemon=yes

# s2u.sh ends here
