/*===========================================================================
 * Project         : Mandrakelinux
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
 * Copyright (C) 2004 Mandrakesoft
 * Stew Benedict, <sbenedict@mandrakesoft.com>
 * Frederic Lepied, <flepied@mandrakesoft.com>
 *
 * code borrowed/adapted from the hal project - 
 * 					http://www.freedesktop.org/Software/hal
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

#include <dbus/dbus.h>
#include <dbus/dbus-glib.h>

static DBusConnection *dbus_connection;
static gchar *cookie = NULL;

/** Print out program usage.
 *
 */
static void
usage ()
{
	fprintf (stderr, "\n" "usage : s2u [--daemon=yes|no] [--help]\n");
	fprintf (stderr,
		 "\n"
		 "        --daemon=yes|no    Become a daemon\n"
		 "        --help             Show this information and exit\n"
		 "\n"
		 "s2u monitors messages through D-BUS,\n"
		 "taking appropraite actions.\n");
}


/** If #TRUE, we will daemonize */
static dbus_bool_t opt_become_daemon = FALSE;

/** Run as specified username if not #NULL */
static char *opt_run_as = NULL;

static void
die (const char *message)
{
  fprintf (stderr, "*** %s", message);
  exit (1);
}

static int
drak_echo_message (DBusConnection * connection,
	      DBusMessage * message)
{
  DBusError error;
  DBusMessage *reply;
  char *s;

  dbus_error_init (&error);
  
  if (!dbus_message_get_args (message,
                              &error,
                              DBUS_TYPE_STRING, &s,
                              DBUS_TYPE_INVALID))
    {
      reply = dbus_message_new_error (message,
                                      error.name,
                                      error.message);

      if (reply == NULL)
        die ("No memory\n");

      if (!dbus_connection_send (connection, reply, NULL))
        die ("No memory\n");

      dbus_message_unref (reply);

      return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
    }

  reply = dbus_message_new_method_return (message);
  if (reply == NULL)
    die ("No memory\n");
  
  if (!dbus_message_append_args (reply,
                                 DBUS_TYPE_STRING, s,
                                 DBUS_TYPE_INVALID))
    die ("No memory");
  
  if (!dbus_connection_send (connection, reply, NULL))
    die ("No memory\n");

  fprintf (stderr, "Mandrakesoft_user_message: %s\n", s);
  
  dbus_free (s);
  
  dbus_message_unref (reply);
    
  return DBUS_HANDLER_RESULT_HANDLED;
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
	if (dbus_message_is_method_call (message,
					 "com.mandrakesoft.user",
					 "message") &&
	    strcmp (dbus_message_get_path (message),
		    "/com/mandrakesoft/user") == 0) {
	  gchar* args[] = 
	    {
	      "/usr/X11R6/bin/xauth",
	      "add",
	      NULL,
	      ".",
	      NULL,
	      NULL
	    };

	  args[2] = getenv("DISPLAY");
	  args[4] = cookie;

	  printf("setting cookie to %s\n", cookie);
	  
	  g_spawn_async("/", args, NULL, 0, NULL, NULL, NULL, NULL);

	  return drak_echo_message (connection, message);
	} else
		return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}


DBusConnection *
service_dbus_init (void)
{
	DBusError dbus_error;

	dbus_connection_set_change_sigpipe (TRUE);

	dbus_error_init (&dbus_error);
	dbus_connection = dbus_bus_get (DBUS_BUS_SESSION, &dbus_error);
	if (dbus_connection == NULL) {
		printf("dbus_bus_get(): %s", dbus_error.message);
		exit (1);
	}

	dbus_connection_setup_with_g_main (dbus_connection, NULL);

	/* service, object, interface, member */
	dbus_message_new_method_call ("com.mandrakesoft.user.message", "/com/mandrakesoft/user",
                                                "com.mandrakesoft.user",
                                                "message");
 
	dbus_bus_acquire_service (dbus_connection, "com.mandrakesoft.user.message",
				  0, &dbus_error);
	if (dbus_error_is_set (&dbus_error)) {
		printf("dbus_bus_acquire_service(): %s",
		       dbus_error.message);
		exit (1);
	}

	dbus_connection_add_filter (dbus_connection, filter_function, NULL,
				    NULL);

	return dbus_connection;
}

int
main (int argc, char *argv[])
{
	DBusConnection *dbus_connection;
	GMainLoop *loop;
	gchar* args[] = 
	  {
	    "/usr/X11R6/bin/xauth",
	    "list",
	    NULL,
	    NULL
	  };
	gint in;
	gchar result[255];
	gchar* idx;

	memset(result, 0, sizeof(result));
	
	args[2] = getenv("DISPLAY");
	
	g_spawn_async_with_pipes("/", args, NULL, 0, NULL, NULL, NULL,
				 NULL, &in, NULL, NULL);
	if (read(in, result, sizeof(result)) <= 0) {
	  die("unable to read X11 cookie");
	} else {
	  close(in);
	  
	  idx = rindex(result, ' ');
	  cookie = g_strdup(idx+1);
	  cookie[strlen(cookie) - 1] = '\0';
	}
	
	while (1) {
		int c;
		int option_index = 0;
		const char *opt;
		static struct option long_options[] = {
			{"daemon", 1, NULL, 0},
			{"help", 0, NULL, 0},
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
			}
			break;

		default:
			usage ();
			return 1;
			break;
		}
	}

	if (opt_become_daemon) {
		int child_pid;
		int dev_null_fd;

		if (chdir ("/") < 0) {
			printf("Could not chdir to /, errno=%d", errno);
			return 1;
		}

		child_pid = fork ();
		switch (child_pid) {
		case -1:
			printf("Cannot fork(), errno=%d", errno);
			break;

		case 0:
			/* child */

			dev_null_fd = open ("/dev/null", O_RDWR);
			/* ignore if we can't open /dev/null */
			if (dev_null_fd > 0) {
				/* attach /dev/null to stdout, stdin, stderr */
				dup2 (dev_null_fd, 0);
				dup2 (dev_null_fd, 1);
				dup2 (dev_null_fd, 2);
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

	if (opt_run_as != NULL) {
		uid_t uid;
		gid_t gid;
		struct passwd *pw;


		if ((pw = getpwnam (opt_run_as)) == NULL) {
			printf("Could not lookup user %s, errno=%d",
				    opt_run_as, errno);
			exit (1);
		}

		uid = pw->pw_uid;
		gid = pw->pw_gid;

		if (setgid (gid) < 0) {
			printf("Failed to set GID to %d, errno=%d", gid, errno);
			exit (1);
		}

		if (setuid (uid) < 0) {
			printf("Failed to set UID to %d, errno=%d", uid, errno);
			exit (1);
		}

	}

	g_type_init ();

	/* set up the dbus services */
	dbus_connection = service_dbus_init ();

	loop = g_main_loop_new (NULL, FALSE);

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
