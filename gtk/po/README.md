# Adding a New Language or Updating Existing Ones

1. Generate the template file. Run the shell script `updatepot.sh` or change to the `gtk` directory and run:
   ```sh
   xgettext -f po/POTFILES.in --from-code=UTF-8 -o po/snes9x-gtk.pot -kmaybegettext -k_
   ```
   This will update the [`snes9x-gtk.pot`](https://github.com/snes9xgit/snes9x/blob/master/gtk/po/snes9x-gtk.pot) file in this directory with the latest strings.

2. * Use the program “[poedit](https://poedit.net)” with this template or your favorite text editor with a spell checker to create a new translation or,
   * merge the template’s new strings into an existing `.po` file with [`msgmerge`](https://www.gnu.org/software/gettext/manual/html_node/msgmerge-Invocation.html "Invoking the msgmerge Program") first and then edit the new `.po` file.

3. In order to test your translations, you will have to build the message catalog and install Snes9x. Add the language code for your language to the list on the line in [`gtk/CMakeLists.txt`](https://github.com/snes9xgit/snes9x/blob/master/gtk/CMakeLists.txt) beginning with:
   ```cmake
   # Add any new language to this foreach loop
   foreach(lang es fr …
   ```
# How to Add Language Files and What to Translate
Please avoid suffixing your language code with a region code unless it is absolutely necessary. In other words, if you know of any region specific distinctions in your language then and only then add the affected messages exclusively in a `xx_XX.po` file. Do not duplicate all messages from the base `xx.po` language file in a `xx_XX.po` language file but only translate region specific messages.

# Source File Encoding
Make sure all source files are UTF-8 encoded.
