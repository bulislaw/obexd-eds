/*
 *
 *  OBEX Client
 *
 *  Copyright (C) 2007-2009  Intel Corporation
 *  Copyright (C) 2007-2009  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib.h>
#include <gdbus.h>

#include "session.h"

#define SYNC_INTERFACE	"org.openobex.Synchronization"
#define ERROR_INF SYNC_INTERFACE ".Error"

struct sync_data {
	char *phonebook_path;
};

static DBusMessage *sync_setlocation(DBusConnection *connection,
			DBusMessage *message, void *user_data)
{
	struct session_data *session = user_data;
	struct sync_data *syncdata = session_get_data(session);
	const char *location;
	char *path = NULL, *tmp;

	if (dbus_message_get_args(message, NULL,
			DBUS_TYPE_STRING, &location,
			DBUS_TYPE_INVALID) == FALSE)
		return g_dbus_create_error(message,
			ERROR_INF ".InvalidArguments", NULL);

	if (!g_ascii_strcasecmp(location, "INT") ||
			!g_ascii_strcasecmp(location, "INTERNAL"))
		path = g_strdup("telecom/pb.vcf");
	else if (!g_ascii_strncasecmp(location, "SIM", 3)) {
		tmp = g_ascii_strup(location, 4);
		path = g_build_filename(tmp, "telecom/pb.vcf", NULL);
		g_free(tmp);
	} else
		return g_dbus_create_error(message,
			ERROR_INF ".InvalidArguments", "InvalidPhonebook");

	g_free(syncdata->phonebook_path);
	syncdata->phonebook_path = path;

	return dbus_message_new_method_return(message);
}

static DBusMessage *sync_getphonebook(DBusConnection *connection,
			DBusMessage *message, void *user_data)
{
	return g_dbus_create_error(message,
			ERROR_INF ".Failed", "Not implemented");
}

static DBusMessage *sync_putphonebook(DBusConnection *connection,
			DBusMessage *message, void *user_data)
{
	return g_dbus_create_error(message,
			ERROR_INF ".Failed", "Not implemented");
}

static GDBusMethodTable sync_methods[] = {
	{ "SetLocation", "s", "", sync_setlocation },
	{ "GetPhonebook", "", "s", sync_getphonebook,
			G_DBUS_METHOD_FLAG_ASYNC },
	{ "PutPhonebook", "s", "", sync_putphonebook,
			G_DBUS_METHOD_FLAG_ASYNC },
	{}
};

gboolean sync_register_interface(DBusConnection *connection, const char *path,
				void *user_data, GDBusDestroyFunction destroy)
{
	struct session_data *session = user_data;
	void *priv;

	priv = g_try_malloc0(sizeof(struct sync_data));
	if (!priv)
		return FALSE;

	session_set_data(session, priv);

	return g_dbus_register_interface(connection, path, SYNC_INTERFACE,
				sync_methods, NULL, NULL, user_data, destroy);
}

void sync_unregister_interface(DBusConnection *connection, const char *path,
							void *user_data)
{
	struct session_data *session = user_data;
	struct sync_data *syncdata = session_get_data(session);

	g_dbus_unregister_interface(connection, path, SYNC_INTERFACE);

	if (syncdata) {
		g_free(syncdata->phonebook_path);
		g_free(syncdata);
	}
}