#include "JsRunner.h"
#include "JsInputSplitter.h"
#include "equares_core/EquaresException.h"
#include "ServerThreadManager.h"
#include "PrintUtil.h"

#include <QScriptEngine>
#include <QRegExp>
#include <QDebug>

#include <string>
#include <fstream>

using namespace std;

bool JsRunner::runFile(QScriptEngine& engine, istream& is, bool printResults, bool stopOnError)
{
    JsInputSplitter jsis;
    QRegExp rxExit("^\\s*exit\\s*$");
    QTextStream& os = EQUARES_COUT;
    forever {
        string line;
        getline(is, line);
        if(is.fail())
            break;
        try {
            jsis << QString::fromUtf8(line.c_str());
        }
        catch (const EquaresException& e) {
            os << e.what() << endl;
            jsis.clear();
            continue;
        }

        if (!jsis.hasInput())
            continue;
        QString input = jsis.input();
        if (rxExit.exactMatch(input)) {
            os << "exiting equares" << endl;
            break;
        }
        jsis.clear();
        QScriptValue result = engine.evaluate(input);
        if (engine.hasUncaughtException()) {
            int lineNo = engine.uncaughtExceptionLineNumber();
            EQUARES_CERR << "ERROR: line " << lineNo << ": " << result.toString() << endl;
            if (stopOnError)
                return false;
        }
        else if (printResults)
            EQUARES_COUT << result.toString() << endl;
    }
    return !ThreadManager::instance()->threadOutput()->hasErrors();
}

bool JsRunner::runFiles(QScriptEngine& engine, const QStringList& inputFileNames, bool printResults)
{
    if (inputFileNames.isEmpty())
        return runFile(engine, cin, printResults, false);
    else {
        foreach (const QString& fileName, inputFileNames) {
            // TODO better: handle unicode file names
            ifstream is(fileName.toLatin1().constData());
            if (is.fail()) {
                EQUARES_CERR << "ERROR: Failed to open input file '" << fileName.toLatin1().constData() << "', stopping\n";
                return false;
            }
            else if (!runFile(engine, is, printResults, true))
                return false;
        }
        return true;
    }
}

void JsRunner::runServer(QScriptEngine& engine)
{
    QRegExp cmdStart("^====\\{$");
    QRegExp cmdEnd("^====\\}$");
    QRegExp cmdSync("^==([0-9]+)==<$");
    QRegExp cmdTerm("^==([0-9]+)==x$");
    QRegExp cmdTermAll("^====x$");
    QRegExp rxExit("^\\s*exit\\s*$");
    QRegExp rxExitHard("^====X$");

    // Create server thread manager
    ServerThreadManager threadManager;

    QStringList cmd;
    bool waitForCommand = true;
    forever {
        string line;
        getline(cin, line);
        if(cin.fail())
            break;
        QString s = QString::fromUtf8(line.c_str());
        s.remove('\r');
        if (cmdSync.exactMatch(s)) {
            int jobId = cmdSync.capturedTexts()[1].toInt();
            threadManager.endSync(jobId);
        }
        else if (cmdTerm.exactMatch(s)) {
            int jobId = cmdTerm.capturedTexts()[1].toInt();
            threadManager.requestTermination(jobId);
        }
        else if (cmdTermAll.exactMatch(s))
            threadManager.requestTermination();
        else if (rxExitHard.exactMatch(s))
            break;
        else if (waitForCommand) {
            // Waiting for a command
            if (cmdStart.exactMatch(s))
                waitForCommand = false;
            else
                EQUARES_CERR << "WARNING: Command was expected, ignoring input line '" << s << "'" << endl;
        }
        else {
            // Collecting command input and wait for end-of-command
            if (cmdEnd.exactMatch(s)) {
                // Process command and provide reply
                EQUARES_COUT << "===={" << endl;
                if (!cmd.isEmpty()) {
                    JsInputSplitter jsis;
                    foreach (QString str, cmd)
                        jsis << str;
                    if (jsis.hasInput()) {
                        QString input = jsis.input();
                        if (rxExit.exactMatch(input))
                            break;
                        QScriptValue result = engine.evaluate(input);
                        if (engine.hasUncaughtException()) {
                            int lineNo = engine.uncaughtExceptionLineNumber();
                            EQUARES_CERR << "ERROR: line " << lineNo << ": " << result.toString() << endl;
                        }
                        else if (!result.isUndefined())
                            // Print result, unless it's undefined
                            EQUARES_COUT << result.toString() << endl;
                    }
                    else
                        EQUARES_CERR << "WARNING: Ignoring command because it contains unbalanced braces and/or quotes" << endl;
                }
                EQUARES_COUT << "====}" << endl;

                // Prepare for next command
                waitForCommand = true;
                cmd.clear();
            }
            else
                // Collect command input
                cmd << s;
        }
    }
    EQUARES_COUT << "end" << endl;
}
