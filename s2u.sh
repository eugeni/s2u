#!/bin/sh
#---------------------------------------------------------------
# Project         : Mandrakelinux
# Module          : alert_applet-0.13
# File            : s2u.sh
# Version         : $Id$
# Author          : Frederic Lepied
# Created On      : Sat Jul 31 01:03:37 2004
# Purpose         : 
#---------------------------------------------------------------

vars() {
    CURR_DISPLAY=`LC_ALL=C xdpyinfo | grep 'display' | awk '{print $4}'`

    if [ -f "/var/tmp/dbus-xsession$CURR_DISPLAY" -a -O "/var/tmp/dbus-xsession$CURR_DISPLAY" ]; then
	. "/var/tmp/dbus-xsession$CURR_DISPLAY"
    fi
}

vars

if [ -z "$DBUS_SESSION_BUS_PID" -o ! -d "/proc/$DBUS_SESSION_BUS_PID" ]; then
    F=`mktemp /var/tmp/dbus.XXXXXX`
    chmod 600 $F
    dbus-launch --sh-syntax --exit-with-session > $F
    rm -f "/var/tmp/dbus-xsession$CURR_DISPLAY"
    mv -f $F "/var/tmp/dbus-xsession$CURR_DISPLAY"
    vars
fi

s2u --daemon=yes


# s2u.sh ends here
