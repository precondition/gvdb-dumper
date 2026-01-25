#include <string.h>
#include "gvdb-reader.h"

/**
 * Hierarchical lexicographic order.
 * @param a first path
 * @param b second path
 * @return negative value if a < b, 0 if a == b, positive value if a > b
 */
static gint
path_compare (const void *a,
              const void *b)
{
    const gchar *as = *(const gchar **)a;
    const gchar *bs = *(const gchar **)b;

    const gchar *pa = as;
    const gchar *pb = bs;

    while (*pa && *pb) {
        /* find next component */
        const gchar *a_slash = strchr(pa, '/');
        const gchar *b_slash = strchr(pb, '/');

        gsize a_len = a_slash ? (gsize)(a_slash - pa) : strlen(pa);
        gsize b_len = b_slash ? (gsize)(b_slash - pb) : strlen(pb);

        const bool a_terminal = a_slash == NULL;
        const bool b_terminal = b_slash == NULL;
        if (a_terminal && !b_terminal)
            return -1;
        else if (!a_terminal && b_terminal)
            return 1;

        gint cmp = strncmp(pa, pb, MIN(a_len, b_len));
        if (cmp != 0)
            return cmp;

        if (a_len != b_len)
            return (a_len < b_len) ? -1 : 1;

        /* advance to next component */
        pa = a_slash ? a_slash + 1 : pa + a_len;
        pb = b_slash ? b_slash + 1 : pb + b_len;
    }

    /* one path ended */
    if (*pa)
        return 1;
    if (*pb)
        return -1;

    return 0;
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

        /* key */
        g_ptr_array_add (items, g_strdup (rest));
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
                const gchar *dir_src)
{
  g_auto(GStrv) items = NULL;
  gint length;

  items = gvdb_list_dir(table, dir_src, &length);

  for (gchar **item = items; *item; ++item)
    {
      g_autofree const gchar *path = g_strconcat (dir_src, *item, NULL);
      g_autofree const gchar *group = g_path_get_dirname (*item);
      g_autofree const gchar *field = g_path_get_basename (*item);

      g_autoptr(GVariant) value = gvdb_table_get_value (table, path);
      if (value != NULL)
        {
          g_autofree const gchar *value_str = g_variant_print (value, TRUE);
          g_key_file_set_value (kf, group, field, value_str);
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

    add_to_keyfile (kf, table, "/");

    gvdb_table_free(table);

    gsize length;
    gchar *data = g_key_file_to_data(kf, &length, NULL);
    g_key_file_unref(kf);

    g_print("%s", data);
    g_free(data);

    return 0;
}

