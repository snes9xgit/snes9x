#!/bin/sh

cd $(dirname "${0}")/..
xgettext -f po/POTFILES.in --from-code=UTF-8 -o po/snes9x-gtk.pot -kmaybegettext -k_
cd -
