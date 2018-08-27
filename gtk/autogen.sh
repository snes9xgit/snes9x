#!/bin/sh

glib-gettextize -f -c
intltoolize --copy --force --automake
aclocal
automake --add-missing --copy
autoconf

rm -f intltool-extract.in intltool-merge.in intltool-update.in
    
