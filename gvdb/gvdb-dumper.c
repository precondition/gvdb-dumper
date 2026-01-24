#include <string.h>
#include "gvdb-reader.h"

/**
 * Comparison function for paths that orders keys before dirs.
 */
static gint
path_compare (const void *a,
              const void *b)
{
  const gchar *as = *(const gchar **)a;
  const gchar *bs = *(const gchar **)b;

  const gboolean a_is_dir = !!g_str_has_suffix (as, "/");
  const gboolean b_is_dir = !!g_str_has_suffix (bs, "/");

  if (a_is_dir != b_is_dir)
    return a_is_dir - b_is_dir;
  else
    return strcmp (as, bs);
}

static gchar **
gvdb_list_dir (GvdbTable *table,
               const gchar *dir,
               gint *out_len)
{
    gchar **names = gvdb_table_get_names (table, NULL);
    GPtrArray *items = g_ptr_array_new_with_free_func (g_free);
    gsize dir_len = strlen (dir);

    for (gchar **n = names; *n; n++) {
        const gchar *path = *n;

        if (!g_str_has_prefix (path, dir))
            continue;

        const gchar *rest = path + dir_len;
        if (*rest == '\0')
            continue;

        const gchar *slash = strchr (rest, '/');

        if (slash) {
            /* directory */
            gsize len = slash - rest + 1;
            gchar *entry = g_strndup (rest, len);
            g_ptr_array_add (items, entry);
        } else {
            /* key */
            g_ptr_array_add (items, g_strdup (rest));
        }
    }

    g_strfreev (names);

    g_ptr_array_sort (items, path_compare);
    g_ptr_array_add (items, NULL);

    if (out_len)
        *out_len = items->len - 1;

    return (gchar **) g_ptr_array_free (items, FALSE);
}

static void
add_to_keyfile (GKeyFile    *kf,
                GvdbTable *table,
                const gchar *dir_src,
                const gchar *dir_dst)
{
  g_autofree gchar *group = NULL;
  g_auto(GStrv) items = NULL;
  gint length;
  gsize n;

  /* Key-file group names are formed by removing initial and trailing slash
   * from dir name, with the singular exception of root dir whose group name
   * is just "/". */

  n = strlen (dir_dst);
  g_assert (n >= 1 && dir_dst[n - 1] == '/');

  if (g_str_equal (dir_dst, "/"))
    group = g_strdup ("/");
  else
    group = g_strndup (dir_dst + 1, n - 2);

  items = gvdb_list_dir(table, dir_src, &length);

  for (gchar **item = items; *item; ++item)
    {
      g_autofree gchar *path = g_strconcat (dir_src, *item, NULL);

      if (g_str_has_suffix (*item, "/"))
        {
          g_autofree gchar *subdir = g_strconcat (dir_dst, *item, NULL);
          add_to_keyfile (kf, table, path, subdir);
        }
      else
        {
          g_autoptr(GVariant) value = gvdb_table_get_value (table, path);
          if (value != NULL)
            {
              g_autofree gchar *value_str = g_variant_print (value, TRUE);
              g_key_file_set_value (kf, group, *item, value_str);
            }
        }
    }
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

    add_to_keyfile (kf, table, "/", "/");

    gvdb_table_free(table);

    gsize length;
    gchar *data = g_key_file_to_data(kf, &length, NULL);
    g_key_file_unref(kf);

    g_print("%s", data);
    g_free(data);

    return 0;
}

