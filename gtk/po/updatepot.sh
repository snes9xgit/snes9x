#!/bin/sh

cd $(dirname "${0}")/..
xgettext -f po/POTFILES.in \
         --from-code 'UTF-8' --package-name 'Snes9x' \
         --copyright-holder 'Brandon Wright and Snes9x contributors' \
         --msgid-bugs-address 'https://github.com/snes9xgit/snes9x/issues' \
         -w 80 -j -o ../i18n/snes9x.pot -kmaybegettext -k_
cd -
