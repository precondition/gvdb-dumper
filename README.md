gvdb-dumper (or dconf dump-file)
====

The `gvdb-dumper` (or `dconf dump-file`) is a CLI program for dumping a binary GVariant Database file into a human-readable ini format to stdout.

The internal database for `dconf` and `gsettings` uses the GVDB (GVariant Database) format, and is typically stored at `$XDG_CONFIG_HOME/dconf/user`.

Thanks to this tool, you can achieve the same effect as `dconf dump /` without having to jump through hoops in order to load a given dconf database file into your currently running dconf server, as described in “[How can I view the content of a backup of the dconf database file? [Unix & Linux StackExchange]](https://unix.stackexchange.com/questions/199836/how-can-i-view-the-content-of-a-backup-of-the-dconf-database-file#199864)”

The repository is modeled after the official [GVDB repository](https://gitlab.gnome.org/GNOME/gvdb), with the `gvdb-dumper` branch adding `gvdb/gvdb-dumper.c` and modifying `meson.build` to add the `gvdp-dumper` target.

The dumping logic is closely modeled after [dconf/bin/dconf.c](https://gitlab.gnome.org/GNOME/dconf/-/blob/main/bin/dconf.c), with the dependency on DConfClient replaced by a simple `GvdTable` loaded in memory.

## Install

Simply download the compiled binary executables from the [Releases](https://github.com/precondition/gvdb-dumper/releases) and make it executable (`chmod +x gvdb-dumper`). Place the program somewhere in your `$PATH` if you want to easily access it by name from anywhere.

## Uninstall

Simply delete the executable `gvdb-dumper` file. This is a simple self-contained and stateless program that does not leave anything behind, so no particular cleanup is needed.

## Examples

Inspect current dconf gsettings:

```sh
$ gvdb-dumper ~/.config/dconf/user | head
[ca/desrt/dconf-editor]
saved-pathbar-path='/com/github/stunkymonkey/nautilus-open-any-terminal/terminal'
saved-view='/com/github/stunkymonkey/nautilus-open-any-terminal/terminal'
show-warning=false
window-height=500
window-is-maximized=false
window-width=540

[com/github/stunkymonkey/nautilus-open-any-terminal]
terminal='alacritty'
```

The above command should be the same as running `dconf dump /`:

```bash
$ sha256sum <(dconf dump /) <(gvdb-dumper ~/.config/dconf/user)
28dd96f4163e26765e598f519986e572735f3c8c22a88cd6e58a356b27519d74  /proc/self/fd/12
28dd96f4163e26765e598f519986e572735f3c8c22a88cd6e58a356b27519d74  /proc/self/fd/13
```

Compare current gsettings with a backup:

```diff
$ diff --unified <(gvdb-dumper ~/.config/dconf/user) <(gvdb-dumper /run/media/username/BACKUPS/2026-01/.config/dconf/user)
@@ -1,11 +1,14 @@
 [ca/desrt/dconf-editor]
-saved-pathbar-path='/org/gnome/nautilus/window-state/'
-saved-view='/org/gnome/nautilus/'
+saved-pathbar-path='/com/github/stunkymonkey/nautilus-open-any-terminal/terminal'
+saved-view='/com/github/stunkymonkey/nautilus-open-any-terminal/terminal'
 show-warning=false
 window-height=500
 window-is-maximized=false
 window-width=540

+[com/github/stunkymonkey/nautilus-open-any-terminal]
+terminal='alacritty'
+
 [desktop/ibus/general]
 preload-engines=['xkb:us::eng', 'xkb:jp::jpn', 'mozc-off', 'mozc-on', 'mozc-jp']
 use-system-keyboard-layout=true
@@ -41,7 +44,7 @@ height=940
 width=1648

 [org/gnome/nautilus/icon-view]
-default-zoom-level='large'
+default-zoom-level='medium'

 [org/gnome/nautilus/preferences]
 default-folder-viewer='icon-view'
```

Use `gvdb-dumper` as a textconv filter for diffing `dconf/user` in case you want to track it with git:

```diff
$ tail --lines 2 ~/.gitconfig
[diff "gvdb-dumper"]
	textconv = gvdb-dumper

$ tail --lines 2 .gitattributes
dconf/user	diff=gvdb-dumper

$ git log -1 -p dconf/user
commit 79f8750
Author: precondition <57645186+precondition@users.noreply.github.com>
Date:   Sat Jan 24 22:08:30 2026 +0100

    Update last folder path

diff --git a/dconf/user b/dconf/user
index a1166bd..97b3d14 100644
--- a/dconf/user
+++ b/dconf/user
@@ -54,6 +54,9 @@ migrated-gtk-settings=true
 initial-size=(936, 1017)
 initial-size-file-chooser=(890, 550)

+[org/gnome/portal/filechooser/firefox]
+last-folder-path='/home/username/temp'
+
 [org/gnome/portal/filechooser/org.gnome.Nautilus]
 last-folder-path='/usr/share/icons/MoreWaita/scalable/places'
```

<!--
  SPDX-FileCopyrightText: 2021 Endless OS Foundation, LLC
  SPDX-License-Identifier: LGPL-2.1-or-later
-->
