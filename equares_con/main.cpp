#include <QCoreApplication>
#include "equares_core/equares_script.h"
#include "equares_core/initBoxFactory.h"
#include "JsRunner.h"
#include "EquaresPrintUtil.h"

using namespace std;

void describeSystem(QScriptEngine& engine, const QStringList& args)
{
    Q_UNUSED(engine);
    QTextStream& os = EQUARES_COUT;
    if (args.empty()) {
        describeSystem(engine, BoxFactory::boxTypes() << "boxTypes");
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
        else if (boxTypes.contains(arg)) {
            Box::Ptr box(newBox(arg));
            if (namedMode)
                os << "\nvar " << arg << " = ";
            os << "{" << endl;
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
        else
            throw EquaresException(QString("Unrecognized argument '%1'").arg(arg));
    }
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
                nonFlagArgs << arg;
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
            JsRunner::runFiles(engine, nonFlagArgs);
            if (nonFlagArgs.isEmpty())
                // Standard input is already processed
                forceInteractive = false;
            break;
        case DescribeMode:
            describeSystem(engine, nonFlagArgs);
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

