#include <QCoreApplication>
#include "equares_core/equares_script.h"
#include "equares_core/initBoxFactory.h"
#include "JsRunner.h"
#include "EquaresPrintUtil.h"

using namespace std;

void describeSystem(QScriptEngine& engine)
{
    Q_UNUSED(engine);
    foreach (const QString& name, BoxFactory::boxTypes()) {
        Box::Ptr box(BoxFactory::newBox(name));
        cout << name << " = {" << endl;
        cout << "  inputs: [\n    ";
        printContainer(cout, box->inputPorts(), PortPrinter(), ",\n    ");
        cout << "\n  ],\n  outputs: [\n    ";
        printContainer(cout, box->outputPorts(), PortPrinter(), ",\n    ");
        cout << "\n  ],\n  properties: [\n    ";
        printContainer(cout, box->boxProperties(), BoxPropPrinter(), ",\n    ");
        cout << "\n  ]";
        if (!box->helpString().isEmpty())
            cout << ",\n  help: '" << escapeString(box->helpString()) << "'";
        cout << "\n}" << endl;
    }
    cout << "\nboxTypes = [\n  ";
    printContainer(cout, BoxFactory::boxTypes(), SimplePrinter<QString>(), ",\n  " );
    cout << "\n]\n";
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

        QScriptEngine engine;

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

