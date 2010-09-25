desktopdir = $(datadir)/applications
desktop_DATA = snes9x.desktop

icon16x16dir = $(datadir)/icons/hicolor/16x16/apps
icon16x16_DATA = snes9x_16x16.png

icon24x24dir = $(datadir)/icons/hicolor/24x24/apps
icon24x24_DATA = snes9x_24x24.png

icon32x32dir = $(datadir)/icons/hicolor/32x32/apps
icon32x32_DATA = snes9x_32x32.png

iconscalabledir = $(datadir)/icons/hicolor/scalable/apps
iconscalable_DATA = snes9x.svg

install-data-hook:
	mv -f $(DESTDIR)$(datadir)/icons/hicolor/16x16/apps/snes9x_16x16.png \
	      $(DESTDIR)$(datadir)/icons/hicolor/16x16/apps/snes9x.png
	mv -f $(DESTDIR)$(datadir)/icons/hicolor/24x24/apps/snes9x_24x24.png \
	      $(DESTDIR)$(datadir)/icons/hicolor/24x24/apps/snes9x.png
	mv -f $(DESTDIR)$(datadir)/icons/hicolor/32x32/apps/snes9x_32x32.png \
	      $(DESTDIR)$(datadir)/icons/hicolor/32x32/apps/snes9x.png
	touch -c $(DESTDIR)$(datadir)/icons/hicolor

