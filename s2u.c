/*===========================================================================
 * Project         : Mandriva Linux
 * Module          : s2u
 * File            : s2u.c
 * Version         : $Id$
 * Authors         : Stew Benedict and Frederic Lepied
 * Created On      : Mon Aug  2 08:28:19 2004
 * Purpose         : read dbus messages addressed to the user and take
 *                 appropriate actions.
 *===========================================================================*/

/***************************************************************************
 *
 * Copyright (C) 2004,2005,2009 Mandrakesoft
 * Copyright (C) 2005 Mandriva
 *
 * Stew Benedict, <sbenedict@mandriva.com>
 * Frederic Lepied, <flepied@mandriva.com>
 * Eugeni Dodonov <eugeni@mandriva.com>
 *
 * code borrowed/adapted from the hal project -
 *                  http://www.freedesktop.org/Software/hal
 *
 * Licensed under the GNU General Public License version 2.0
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 **************************************************************************/

static const char rcs_id[] = "$Id$";
static const char compile_id[] = "$Compile: " __FILE__ " " __DATE__ " " __TIME__ " $";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <gdk/gdk.h>
#include <libnotify/notify.h>

#include <dbus/dbus.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-lowlevel.h>

static DBusConnection *dbus_connection;
static gchar *cookie = NULL;
static NotifyNotification *n;

/** Print out program usage.
 *
 */
static void
usage ()
{
    g_printerr ("\n" "usage : s2u [--daemon=yes|no] [--help] [--debug]\n");
    g_printerr (
         "\n"
         "        --daemon=yes|no    Become a daemon\n"
         "        --debug            Print debug informations\n"
         "        --help             Show this information and exit\n"
         "\n"
         "s2u monitors messages through D-BUS,\n"
         "taking appropraite actions.\n");
}


/** If #TRUE, we will daemonize */
static dbus_bool_t opt_become_daemon = FALSE;

static void
die (const char *message)
{
  g_printerr ("*** %s", message);
  exit (1);
}

/** Message handler for method invocations. All invocations on any object
 *  or interface is routed through this function.
 *
 *  @param  connection          D-BUS connection
 *  @param  message             Message
 *  @param  user_data           User data
 *  @return                     What to do with the message
 */

static DBusHandlerResult
filter_function (DBusConnection * connection,
         DBusMessage * message, void *user_data)
{

/*
    INFO (("obj_path=%s interface=%s method=%s",
    dbus_message_get_path(message),
    dbus_message_get_interface(message),
    dbus_message_get_member(message)));
*/

    /* message, interface, method */
    if (dbus_message_is_signal (message,
                     "com.mandriva.user",
                     "message") &&
        strcmp (dbus_message_get_path (message),
            "/com/mandriva/user") == 0) {
      gchar* args[] =
        {
          "/usr/bin/xauth",
          "add",
          NULL,
          ".",
          NULL,
          NULL
        };

      args[2] = getenv("DISPLAY");
      args[4] = cookie;

      g_print("s2u: detected hostname change; setting cookie to '%s' for %s.\n", cookie, args[2]);

      g_spawn_async("/", args, NULL, 0, NULL, NULL, NULL, NULL);

      return DBUS_HANDLER_RESULT_HANDLED;
    } else if (dbus_message_is_signal (message,
                     "com.mandriva.user",
                     "updatemenu")) {

          g_print("s2u: updatemenu signal received.\n");

      g_spawn_command_line_async("/etc/X11/xinit.d/menu", NULL);

      return DBUS_HANDLER_RESULT_HANDLED;
    } else if (dbus_message_is_signal (message,
                     "com.mandriva.user",
                     "security_notification")) {
        /* msec */
        char *string;
        DBusError error;
        dbus_error_init(&error);
        if (dbus_message_get_args (message,
                    &error,
                    DBUS_TYPE_STRING, &string,
                    DBUS_TYPE_INVALID)) {
            n = notify_notification_new("MSEC", string, GTK_STOCK_INFO, NULL);
            if (!notify_notification_show (n, NULL)) {
                g_printerr("notify_notification_show: failed to show notification\n");
            }
            g_object_unref(G_OBJECT(n));
        }
        else {
            fprintf (stderr, "an error occurred: %s\n", error.message);
        }
        dbus_error_free(&error);
        return DBUS_HANDLER_RESULT_HANDLED;
    } else if (dbus_message_is_signal (message,
                     "com.mandriva.user",
                     "custom_notification")) {
        /* msec */
        char *string, *title;
        DBusError error;
        dbus_error_init(&error);
        if (dbus_message_get_args (message,
                    &error,
                    DBUS_TYPE_STRING, &title,
                    DBUS_TYPE_STRING, &string,
                    DBUS_TYPE_INVALID)) {
            n = notify_notification_new(title, string, GTK_STOCK_INFO, NULL);
            if (!notify_notification_show (n, NULL)) {
                g_printerr("notify_notification_show: failed to show notification\n");
            }
            g_object_unref(G_OBJECT(n));
        }
        else {
            fprintf (stderr, "an error occurred: %s\n", error.message);
        }
        dbus_error_free(&error);
        return DBUS_HANDLER_RESULT_HANDLED;
    } else {
        return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
    }
}


static void
service_dbus_init (void)
{
    GError *error = NULL;

    dbus_connection = dbus_g_connection_get_connection (dbus_g_bus_get (DBUS_BUS_SYSTEM, &error));
    if (dbus_connection == NULL) {
        g_printerr ("dbus_bus_get(): %s", error->message);
        exit (1);
    }

         dbus_bus_add_match (dbus_connection,
                             "type='signal',"
                             "interface='com.mandriva.user'",
                NULL);


    dbus_connection_add_filter (dbus_connection, filter_function, NULL,
                    NULL);

}

int
main (int argc, char *argv[])
{
    GMainLoop *loop;
    gchar* args[] = {
        "/usr/bin/xauth",
        "list",
        NULL,
        NULL
    };
    gint in;
    gchar result[255];
    gchar* idx;
    dbus_bool_t opt_debug = FALSE;

    while (1) {
        int c;
        int option_index = 0;
        const char *opt;
        static struct option long_options[] = {
            {"daemon", 1, NULL, 0},
            {"help", 0, NULL, 0},
            {"debug", 0, NULL, 0},
            {NULL, 0, NULL, 0}
        };

        c = getopt_long (argc, argv, "",
                 long_options, &option_index);
        if (c == -1)
            break;

        switch (c) {
        case 0:
            opt = long_options[option_index].name;

            if (strcmp (opt, "help") == 0) {
                usage ();
                return 0;
            } else if (strcmp (opt, "daemon") == 0) {
                if (strcmp ("yes", optarg) == 0) {
                    opt_become_daemon = TRUE;
                } else if (strcmp ("no", optarg) == 0) {
                    opt_become_daemon = FALSE;
                } else {
                    usage ();
                    return 1;
                }
            } else if (strcmp (opt, "debug") == 0) {
                opt_debug = TRUE;
            }
            break;

        default:
            usage ();
            return 1;
            break;
        }
    }

    memset(result, 0, sizeof(result));

    args[2] = getenv("DISPLAY");

    g_spawn_async_with_pipes("/", args, NULL, 0, NULL, NULL, NULL,
                 NULL, &in, NULL, NULL);
    if (read(in, result, sizeof(result)) <= 0) {
        die("unable to read X11 cookie");
    } else {
        close(in);

        idx = rindex(result, ' ');
        if (idx == NULL) {
            die ("unable to read X11 cookie");
        }
        cookie = g_strdup(idx+1);
        cookie[strlen(cookie) - 1] = '\0';
        if (opt_debug) {
            g_print("%s: cookie for %s = '%s'\n", argv[0], args[2], cookie);
        }
    }

    if (opt_become_daemon) {
        int child_pid;
        int dev_null_fd;

        if (chdir ("/") < 0) {
            g_printerr ("Could not chdir to /, errno=%d", errno);
            return 1;
        }

        child_pid = fork ();
        switch (child_pid) {
        case -1:
            g_printerr ("Cannot fork(), errno=%d", errno);
            break;

        case 0:
            /* child */

            dev_null_fd = open ("/dev/null", O_RDWR);
            /* ignore if we can't open /dev/null */
            if (dev_null_fd > 0) {
                /* attach /dev/null to stdout, stdin, stderr */
                dup2 (dev_null_fd, 0);
                if (opt_debug == FALSE) {
                        dup2 (dev_null_fd, 1);
                    dup2 (dev_null_fd, 2);
                }
            }

            umask (022);
            break;

        default:
            /* parent */
            exit (0);
            break;
        }

        /* Create session */
        setsid ();
    }

    gdk_init(&argc, &argv);

    /* init libnotify */
    notify_init("s2u");

    loop = g_main_loop_new (NULL, FALSE);

    /* set up the dbus services */
    service_dbus_init ();

    /* run the main loop and serve clients */

    g_main_loop_run (loop);

    return 0;
}

/*
 * Local variables:
 * mode: c
 * End:
 *
 * s2u.c ends here
 */
