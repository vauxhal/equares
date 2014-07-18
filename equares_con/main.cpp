/*
EquaRes - a tool for numerical analysis of equations.
Copyright (C) 2014 Stepan Orlov

This file is part of EquaRes.

Full text of copyright notice can be found in file copyright_notice.txt in the EquaRes root directory.
License agreement can be found in file LICENSE.md in the EquaRes root directory.
*/

#include <QCoreApplication>
#include <QVariantMap>
#include "equares_core/equares_script.h"
#include "equares_core/initBoxFactory.h"
#include "equares_core/PerTypeStorage.h"
#include "JsRunner.h"
#include "EquaresPrintUtil.h"

using namespace std;

static bool pickOption(QStringList& options, const QString& option) {
    if (options.contains(option)) {
        options.removeAll(option);
        return true;
    }
    else
        return false;
}

void describeSystem(QScriptEngine& engine, const QStringList& args, const QStringList& options_)
{
    QStringList options = options_;
    if (options.isEmpty())
        options << "ports" << "props" << "help";
    bool printPorts = pickOption(options, "ports");
    bool printProps = pickOption(options, "props");
    bool printHelp = pickOption(options, "help");
    if (!options.isEmpty())
        throw EquaresException(QString("Unknown describe system option(s): '%1'").arg(options.join(",")));
    Q_UNUSED(engine);
    QTextStream& os = EQUARES_COUT;
    if (args.empty()) {
        describeSystem(engine, BoxFactory::boxTypes() << "boxTypes", options);
        return;
    }
    QStringList boxTypes = BoxFactory::boxTypes();
    bool namedMode = args.size() > 1;
    foreach (const QString& arg, args) {
        if (arg == "boxTypes") {
            if (namedMode)
                os << "\nvar boxTypes = ";
            os << "[\n  ";
            printContainer(os, BoxFactory::boxTypes(), SimplePrinter<QString>(), ",\n  " );
            os << "\n]\n";
        }
        else {
            QScriptValue sbox;
            Box::Ptr boxptr;
            Box *box;
            if (boxTypes.contains(arg)) {
                boxptr = Box::Ptr(box = newBox(arg));
                sbox = engine.newQObject(box);
            }
            else {
                sbox = engine.evaluate(arg);
                if (sbox.isQObject())
                    box = qobject_cast<Box*>(sbox.toQObject());
                else
                    box = 0;
                if (!box)
                    throw EquaresException(QString("Failed to find box instance named '%1'").arg(arg));
            }
            if (namedMode)
                os << "\nvar " << arg << " = ";
            os << "{" << endl;
            bool needComma = false;
            if (printPorts) {
                os << "  inputs: [\n    ";
                printContainer(os, box->inputPorts(), PortPrinter(), ",\n    ");
                os << "\n  ],\n  outputs: [\n    ";
                printContainer(os, box->outputPorts(), PortPrinter(), ",\n    ");
                os << "\n  ]";
                needComma = true;
            }
            if (printProps) {
                if (needComma)
                    os << ",\n";
                os << "  properties: [\n    ";
                printContainer(os, box->boxProperties(), BoxPropPrinter(sbox), ",\n    ");
                os << "\n  ]";
                needComma = true;
            }
            if (printHelp) {
                if (!box->helpString().isEmpty()) {
                    if (needComma)
                        os << ",\n";
                    os << "  help: '" << escapeString(box->helpString()) << "'";
                    needComma = true;
                }
            }
            os << "\n}" << endl;
        }
    }
}

static QString readFile(const QString& fileName)
{
    QFile file(fileName);
    file.open(QIODevice::ReadOnly);
    if (!file.isOpen())
        throw EquaresException(QString("readFile(): failed to open file %1").arg(fileName));
    return QString::fromUtf8(file.readAll());
}

static QScriptValue print(QScriptContext *context, QScriptEngine *engine) {
    Q_UNUSED(engine);
    int n = context->argumentCount();
    QStringList args;
    for (int i=0; i<n; ++i)
        args << context->argument(i).toString();
    // args << "---";
    EQUARES_COUT << args.join(", ") << endl;
    return QScriptValue();
}

static QScriptValue readFile(QScriptContext *context, QScriptEngine *engine) {
    Q_UNUSED(engine);
    if (context->argumentCount() != 1)
        context->throwError("readFile(): one argument was expected");
    QString fileName = context->argument(0).toString();
    return readFile(fileName);
}

void registerUtilFunc(QScriptEngine *engine) {
    engine->globalObject().setProperty("print", engine->newFunction(print));
    if (PerTypeStorage::instance<QVariantMap>().value("allowReadFile").toBool())
        engine->globalObject().setProperty("readFile", engine->newFunction(readFile));
}

int main(int argc, char **argv)
{
#ifdef __linux__
    Q_INIT_RESOURCE(equares_core);
#endif // __linux__

    QCoreApplication app(argc, argv);

    try {
        // Parse command line arguments
        enum Mode { RunMode, DescribeMode, ServerMode } mode = RunMode;
        QStringList nonFlagArgs;
        bool forceInteractive = false;
        bool forceQuiet = false;
        QStringList args = app.arguments();
        args.removeFirst();
        QStringList describeSystemOptions;
        QString cacheDir;
        QString *flagArg = 0;
        QVariantMap& globalSettings = PerTypeStorage::instance<QVariantMap>();
        foreach (QString arg, args) {
            if (arg.isEmpty())
                continue;
            if (flagArg) {
                *flagArg = arg;
                flagArg = 0;
                continue;
            }
            if (arg[0] == '-' && arg.size() == 2) {
                switch (arg[1].toLatin1()) {
                case 'd':
                    mode = DescribeMode;
                    break;
                case 'f':
                    globalSettings["allowReadFile"] = true;
                    break;
                case 's':
                    mode = ServerMode;
                    // No break intentionally
                case 'i':
                    forceInteractive = true;
                    break;
                case 'c':
                    flagArg = &cacheDir;
                    break;
                case 'b':
                    globalSettings["denyBuild"] = true;
                    globalSettings["allowReadFile"] = false;
                    break;
                case 'q':
                    forceQuiet = true;
                    break;
                default:
                    throw EquaresException(QString("Unrecognized option '%1'").arg(arg));
                }
            }
            else if (arg.startsWith("-d")) {
                mode = DescribeMode;
                describeSystemOptions = arg.mid(2).split(",", QString::SkipEmptyParts);
            }
            else
                nonFlagArgs << arg;
        }

        // Stuff global settings
        if (!cacheDir.isEmpty())
            globalSettings["cacheDir"] = cacheDir;

        // Create script engine
        QScriptEngine engine;

        // Create default thread manager
        DefaultThreadManager threadManager;

        // Initialize equares core
        initBoxFactory();
        registerEquaresScriptTypes(&engine);
        registerUtilFunc(&engine);

        bool ok = true;

        // Execute initialization script
        engine.evaluate(readFile(":/js/init.js"));

        switch (mode) {
        case RunMode:
            ok = JsRunner::runFiles(engine, nonFlagArgs, !forceQuiet);
            if (nonFlagArgs.isEmpty())
                // Standard input is already processed
                forceInteractive = false;
            break;
        case DescribeMode:
            break;
        case ServerMode:
            JsRunner::runServer(engine);
            forceInteractive = false;
            break;
        default:
            Q_ASSERT(false);
        }

        if (forceInteractive)
            // Process standard input (note: only runs if was ok)
            ok = ok && JsRunner::runFiles(engine, QStringList(), forceQuiet? false: mode != DescribeMode);

        if (mode == DescribeMode && ok)
            describeSystem(engine, nonFlagArgs, describeSystemOptions);

        return ok? 0: -1;
    }
    catch(const exception& e) {
        cerr << "ERROR: " << e.what() << endl;
        return -1;
    }
}

