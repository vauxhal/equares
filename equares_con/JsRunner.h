#ifndef JSRUNNER_H
#define JSRUNNER_H

#include <iostream>

class QScriptEngine;
class QStringList;

class JsRunner
{
public:
    static bool runFile(QScriptEngine& engine, std::istream& is, bool printResults);
    static bool runFiles(QScriptEngine& engine, const QStringList& inputFileNames, bool printResults);
    static void runServer(QScriptEngine& engine);
};

#endif // JSRUNNER_H
