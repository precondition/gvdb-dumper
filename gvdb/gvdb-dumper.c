#include <string.h>
//#include <gio/gio.h>

#include "gvdb-reader.h"

static void
add_key (GKeyFile *kf, const gchar *full_key, GVariant *value)
{
    /* full_key looks like: /org/gnome/desktop/interface/gtk-theme */

    const gchar *last_slash = strrchr(full_key, '/');
    if (!last_slash || last_slash == full_key)
        return;

    gchar *group = g_strndup(full_key + 1, last_slash - full_key - 1);
    const gchar *key = last_slash + 1;

    gchar *printed = g_variant_print(value, TRUE);
    g_key_file_set_value(kf, group, key, printed);

    g_free(group);
    g_free(printed);
    g_variant_unref(value);
}

int
main (int argc, char **argv)
{
    if (argc != 2) {
        g_printerr("usage: %s <dconf-gvdb-file>\n", argv[0]);
        return 1;
    }

    GError *error = NULL;
    GvdbTable *table = gvdb_table_new(argv[1], TRUE, &error);
    if (!table) {
        g_printerr("Failed to open GVDB: %s\n", error->message);
        g_error_free(error);
        return 1;
    }

    GKeyFile *kf = g_key_file_new();

    gchar **names = gvdb_table_get_names(table, NULL);
    for (gchar **n = names; *n; n++) {
        /* dconf keys are always absolute paths */
        if (**n != '/')
            continue;

        GVariant *value = gvdb_table_get_value(table, *n);
        if (!value)
            continue;

        add_key(kf, *n, value);
        g_variant_unref(value);
    }

    g_strfreev(names);
    gvdb_table_free(table);

    gsize length;
    gchar *data = g_key_file_to_data(kf, &length, NULL);
    g_key_file_unref(kf);

    g_print("%s", data);
    g_free(data);

    return 0;
}

