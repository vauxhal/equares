/*
EquaRes - a tool for numerical analysis of equations.
Copyright (C) 2014 Stepan Orlov

This file is part of EquaRes.

Full text of copyright notice can be found in file copyright_notice.txt in the EquaRes root directory.
License agreement can be found in file LICENSE.md in the EquaRes root directory.
*/

#ifndef JSRUNNER_H
#define JSRUNNER_H

#include <iostream>

class QScriptEngine;
class QStringList;

class JsRunner
{
public:
    static bool runFile(QScriptEngine& engine, std::istream& is, bool printResults, bool stopOnError);
    static bool runFiles(QScriptEngine& engine, const QStringList& inputFileNames, bool printResults);
    static void runServer(QScriptEngine& engine);
};

#endif // JSRUNNER_H
