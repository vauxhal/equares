#include "JsRunner.h"
#include "JsInputSplitter.h"
#include "equares_core/EquaresException.h"
#include "PrintUtil.h"

#include <QScriptEngine>
#include <QRegExp>
#include <QDebug>

#include <string>
#include <fstream>

using namespace std;

void JsRunner::runFile(QScriptEngine& engine, istream& is)
{
    JsInputSplitter jsis;
    QRegExp rxExit("^\\s*exit\\s*$");
    forever {
        string line;
        getline(is, line);
        if(is.fail())
            break;
        try {
            jsis << QString::fromUtf8(line.c_str());
        }
        catch (const EquaresException& e) {
            cout << e.what() << endl;
            jsis.clear();
            continue;
        }

        if (!jsis.hasInput())
            continue;
        QString input = jsis.input();
        if (rxExit.exactMatch(input)) {
            cout << "exiting equares" << endl;
            break;
        }
        jsis.clear();
        QScriptValue result = engine.evaluate(input);
        if (engine.hasUncaughtException()) {
            int lineNo = engine.uncaughtExceptionLineNumber();
            cerr << "ERROR: line " << lineNo << ": " << result.toString() << endl;
        }
        else
            cout << result.toString() << endl;
    }
}

void JsRunner::runFiles(QScriptEngine& engine, const QStringList& inputFileNames)
{
    if (inputFileNames.isEmpty())
        runFile(engine, cin);
    else
        foreach (const QString& fileName, inputFileNames) {
            // TODO better: handle unicode file names
            ifstream is(fileName.toLatin1().constData());
            if (is.fail()) {
                cerr << "WARNING: Failed to open input file '" << fileName.toLatin1().constData() << "', skipping" << endl;
            }
            else
                runFile(engine, is);
        }
}

void JsRunner::runServer(QScriptEngine& engine)
{
    QRegExp cmdStart("^==\\[(\\d+)\\]==\\{$");
    QRegExp cmdEnd("^==\\[(\\d+)\\]==\\}$");
    QRegExp rxExit("^\\s*exit\\s*$");

    QStringList cmd;
    int cmdId = -1;
    forever {
        string line;
        getline(cin, line);
        if(cin.fail())
            break;
        QString s = QString::fromUtf8(line.c_str());
        s.remove('\r');
        if (cmdId == -1) {
            // Waiting for a command
            if (cmdStart.exactMatch(s)) {
                cmdId = cmdStart.capturedTexts()[1].toInt();
                Q_ASSERT(cmdId != -1);
            }
            else
                cerr << "WARNING: Command was expected, ignoring input line '" << s << "'" << endl;
        }
        else {
            // Collecting command input and wait for end-of-command
            if (cmdEnd.exactMatch(s)) {
                int cmdIdAtEnd = cmdEnd.capturedTexts()[1].toInt();
                if (cmdIdAtEnd != cmdId) {
                    cerr << "WARNING: Ignoring termination of command " << cmdIdAtEnd << ", waiting for termination of command " << cmdId << endl;
                    continue;
                }

                // Process command and provide reply
                cout << "==[" << cmdId << "]=={" << endl;
                if (!cmd.isEmpty()) {
                    JsInputSplitter jsis;
                    foreach (QString str, cmd)
                        jsis << str;
                    if (jsis.hasInput()) {
                        QString input = jsis.input();
                        if (rxExit.exactMatch(input)) {
                            cout << "exiting equares" << endl;
                            break;
                        }
                        QScriptValue result = engine.evaluate(input);
                        if (engine.hasUncaughtException()) {
                            int lineNo = engine.uncaughtExceptionLineNumber();
                            cerr << "ERROR: line " << lineNo << ": " << result.toString() << endl;
                        }
                        else
                            cout << result.toString() << endl;
                    }
                    else
                        cerr << "WARNING: Ignoring command " << cmdId << " because it contains unbalanced braces and/or quotes" << endl;
                }
                cout << "==[" << cmdId << "]==}" << endl;

                // Prepare for next command
                cmd.clear();
                cmdId = -1;
            }
            else
                // Collect command input
                cmd << s;
        }
    }
}
