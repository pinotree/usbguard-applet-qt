How to generate translation files for USBGuard Qt Applet
========================================================

To generate the translation file, you'll need the Qt Linguist tools.
On Fedora, these tools are provides by the *qt5-linguist* package.

Steps:

 1. Run the lupdate tool (lupdate-qt5) to generate a .ts file:

    `$ lupdate -source-language en_US -target-language <LANG> . -ts translations/<LANG>.ts`

 2. Use the Qt linguist tool (linguist-qt5) to translate the .ts file into
    `<LANG>`.

 3. If you wish to test your translation at run-time, you'll have to add the new
    translation file (.ts) to the *translations* subdirectory and re-compile
    from scratch.

