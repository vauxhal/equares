#ifndef JSRUNNER_H
#define JSRUNNER_H

#include <iostream>

class QScriptEngine;
class QStringList;

class JsRunner
{
public:
    static void runFile(QScriptEngine& engine, std::istream& is);
    static void runFiles(QScriptEngine& engine, const QStringList& inputFileNames);
    static void runServer(QScriptEngine& engine);
};

#endif // JSRUNNER_H
