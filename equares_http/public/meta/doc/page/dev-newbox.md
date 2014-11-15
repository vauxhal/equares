# Developing new box

1. Provide two classes:
   * one derived from ```Box``` (say, ```MyBox```)
   * one derived from ```RuntimeBox``` (say, ```MyRuntimeBox```)
2. Make sure that ```MyBox::newRuntimeBox()``` returns a pointer to new instance of ```MyRuntimeBox```
3. Make sure to register your box using the ```REGISTER_BOX``` macro
4. If there are properties of custom types, available to JS, then
   * Register them in the Qt meta-type system using the ```Q_DECLARE_METATYPE``` macro
   * Provide ```toScriptValue()``` and ```fromScriptValue()``` conversion functions for your types
   * Write a ```scriptInit()``` static function that registers the above conversion functions in the QtScript module using the ```qScriptRegisterMetaType()``` function
   * Register your ```scriptInit()``` function using the ```REGISTER_SCRIPT_INIT_FUNC``` macro
5. Provide a section in file ```equares_core/res/BoxSettings.ini``` for your box
6. Add box logics to file ```equares_http/public/js/equares-box.js```
7. Add box documentation &mdash; it will be read from file ```equares_http/public/meta/box/BoxName.md```, where ```BoxName``` is the name of the box visible to the end users.
8. Build, restart server, enjoy!
