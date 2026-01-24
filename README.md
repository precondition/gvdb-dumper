GVDB-DUMPER (or dconf dump-file)
====

The `gvdb-dumper` (or `dconf dump-file`) is a CLI program for dumping a binary GVariant Database file into a human-readable ini format to stdout.

The internal database for `dconf` and `gsettings` uses the GVDB (GVariant Database) format, and is typically stored at `$XDG_CONFIG_HOME/dconf/user`.

Thanks to this tool, you can achieve the same effect as `dconf dump /` without having to jump through hoops in order to load a given dconf database file into your currently running dconf server, as described in “[How can I view the content of a backup of the dconf database file? [[Unix & Linux StackExchange]]](https://unix.stackexchange.com/questions/199836/how-can-i-view-the-content-of-a-backup-of-the-dconf-database-file#199864)”

The repository is modeled after the official [GVDB repository](https://gitlab.gnome.org/GNOME/gvdb), with the `gvdb-dumper` branch adding `gvdb/gvdb-dumper.c` and modifying `meson.build` to add the `gvdp-dumper` target.

The dumping logic is closely modeled after [dconf/bin/dconf.c](https://gitlab.gnome.org/GNOME/dconf/-/blob/main/bin/dconf.c), with the dependency on DConfClient replaced by a simple `GvdTable` loaded in memory.

<!--
  SPDX-FileCopyrightText: 2021 Endless OS Foundation, LLC
  SPDX-License-Identifier: LGPL-2.1-or-later
-->
