#!/bin/sh
#---------------------------------------------------------------
# Project         : Mandrakelinux
# Module          : s2u
# File            : s2u.sh
# Version         : $Id$
# Author          : Frederic Lepied
# Created On      : Sat Jul 31 01:03:37 2004
# Purpose         : launch the dbus client attaching it to an
#             exisiting dbus session or launching a new one to
#             attache itself to.
#---------------------------------------------------------------

f="/tmp/dbus-$USER-xsession$CURR_DISPLAY"

vars() {
    CURR_DISPLAY=`LC_ALL=C xdpyinfo | grep 'display' | awk '{print $4}'`

    if [ -f "$f" -a -O "$f" ]; then
	. "$f"
    fi
}

vars

if [ -z "$DBUS_SESSION_BUS_PID" -o ! -d "/proc/$DBUS_SESSION_BUS_PID" ]; then
    F=`mktemp /tmp/dbus.XXXXXX`
    chmod 600 $F
    dbus-launch --sh-syntax --exit-with-session > $F
    rm -f "$f"
    mv -f $F "$f"
    vars
fi

exec s2u --daemon=yes

# s2u.sh ends here
