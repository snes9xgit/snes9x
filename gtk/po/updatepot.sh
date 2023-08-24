#!/bin/sh

cd $(dirname "${0}")/..
xgettext -f po/POTFILES.in -o po/snes9x-gtk.pot -kmaybegettext
cd -
