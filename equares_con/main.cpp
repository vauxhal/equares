#include <QCoreApplication>
#include "equares_core/equares_script.h"
#include "equares_core/initBoxFactory.h"
#include "JsRunner.h"
#include "EquaresPrintUtil.h"

using namespace std;

void describeSystem(QScriptEngine& engine)
{
    Q_UNUSED(engine);
    QTextStream& os = EQUARES_COUT;
    foreach (const QString& name, BoxFactory::boxTypes()) {
        Box::Ptr box(BoxFactory::newBox(name));
        os << name << " = {" << endl;
        os << "  inputs: [\n    ";
        printContainer(os, box->inputPorts(), PortPrinter(), ",\n    ");
        os << "\n  ],\n  outputs: [\n    ";
        printContainer(os, box->outputPorts(), PortPrinter(), ",\n    ");
        os << "\n  ],\n  properties: [\n    ";
        printContainer(os, box->boxProperties(), BoxPropPrinter(), ",\n    ");
        os << "\n  ]";
        if (!box->helpString().isEmpty())
            os << ",\n  help: '" << escapeString(box->helpString()) << "'";
        os << "\n}" << endl;
    }
    os << "\nboxTypes = [\n  ";
    printContainer(os, BoxFactory::boxTypes(), SimplePrinter<QString>(), ",\n  " );
    os << "\n]\n";
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
        QStringList inputFileNames;
        bool forceInteractive = false;
        QStringList args = app.arguments();
        args.removeFirst();
        foreach (QString arg, args) {
            if (arg.isEmpty())
                continue;
            if (arg[0] == '-' && arg.size() == 2) {
                switch (arg[1].toLatin1()) {
                case 'd':
                    mode = DescribeMode;
                    break;
                case 's':
                    mode = ServerMode;
                case 'i':
                    forceInteractive = true;
                    break;
                default:
                    throw EquaresException(QString("Unrecognized option '%1'").arg(arg));
                }
            }
            else
                inputFileNames << arg;
        }

        // Create script engine
        QScriptEngine engine;

        // Create default thread manager
        DefaultThreadManager threadManager;

        // Initialize equares core
        initBoxFactory();
        registerEquaresScriptTypes(&engine);

        switch (mode) {
        case RunMode:
            JsRunner::runFiles(engine, inputFileNames);
            if (inputFileNames.isEmpty())
                // Standard input is already processed
                forceInteractive = false;
            break;
        case DescribeMode:
            describeSystem(engine);
            break;
        case ServerMode:
            JsRunner::runServer(engine);
            forceInteractive = false;
            break;
        default:
            Q_ASSERT(false);
        }

        if (forceInteractive)
            // Process standard input
            JsRunner::runFiles(engine, QStringList());

        return 0;
    }
    catch(const exception& e) {
        cerr << "ERROR: " << e.what() << endl;
        return -1;
    }
}

