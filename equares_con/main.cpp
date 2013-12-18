#include <QCoreApplication>
#include "equares_core/equares_script.h"
#include "equares_core/initBoxFactory.h"
#include <QRegExp>

#include <QDebug>

#include <iostream>
#include <fstream>
#include <string>

using namespace std;

class JsInputSplitter
{
public:
    JsInputSplitter() {}
    bool hasInput() const {
        return !m_input.isEmpty() && m_braces.isEmpty();
    }
    bool isEmpty() const {
        return m_input.isEmpty();
    }
    QString input() const {
        Q_ASSERT(hasInput());
        QString result;
        foreach (const QString& line, m_input)
            result += line;
        return result;
    }
    void clear() {
        m_input.clear();
        m_braces.clear();
    }

    JsInputSplitter& operator<<(const QString& line) {
        countBraces(line);
        m_input << line;
        if (state() == InStringLiteral)
            m_input << "\\n";
        else
            m_input << "\n";
        return *this;
    }

private:
    QStringList m_input;
    QString m_braces;
    enum State {
        Normal,
        InComment,
        InStringLiteral
    };
    State state() const {
        if (m_braces.isEmpty())
            return Normal;
        else {
            switch (m_braces[m_braces.size()-1].toLatin1()) {
            case '*':
                return InComment;
            case '"':
            case '\'':
                return InStringLiteral;
            default:
                return Normal;
            }
        }
    }

    void countBraces(const QString& line) {
        QChar prevChar = 0;
        foreach (QChar c, line) {
            switch (state()) {
            case Normal:
                switch (c.toLatin1()) {
                case '(':
                case '[':
                case '{':
                    m_braces += c;
                    break;
                case ')':
                case ']':
                case '}':
                    matchBrace(c);
                    break;
                case '*':
                    if (prevChar == '/')
                        // C comment
                        m_braces += c;
                    break;
                case '/':
                    if (prevChar == '/')
                        // C++ comment
                        return;
                    break;
                case '"':
                case '\'':
                    m_braces += c;
                }
                break;
            case InComment:
                if (c == '/' && prevChar == '*')
                    m_braces.chop(1);
                break;
            case InStringLiteral: {
                QChar match = m_braces[m_braces.size()-1];
                if (c == match && prevChar != '\\')
                    m_braces.chop(1);
                break;
            }
            default:
                Q_ASSERT(false);
            }
            prevChar = c;
        }
    }
    void matchBrace(QChar c) {
        bool ok = true;
        if (m_braces.isEmpty())
            ok = false;
        else {
            QChar c0 = m_braces[m_braces.size()-1];
            switch (c0.toLatin1()) {
            case '(':   ok = c == ')';   break;
            case '[':   ok = c == ']';   break;
            case '{':   ok = c == '}';   break;
            default:
                ok = false;
            }
        }
        if (ok)
            m_braces.chop(1);
        else
            throw EquaresException(QString("Unmatched brace: '%1'").arg(c));
    }
};

void runFile(QScriptEngine& engine, istream& is)
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
            qWarning() << "line" << lineNo << ":" << result.toString();
        }
        else {
            cout << result.toString().toStdString() << endl;
        }
    }
}

void runFiles(QScriptEngine& engine, const QStringList& inputFileNames)
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

inline ostream& operator<<(ostream& os, const QString& s) {
    os << s.toUtf8().constData();
    return os;
}

template<class Container, class Printer>
inline void printContainer(ostream& os, const Container& c, const Printer& p, const QString& delimiter) {
    bool first = true;
    foreach (typename Container::value_type x, c) {
        if (first)
            first = false;
        else
            os << delimiter;
        p(os, x);
    }
}

inline QString escapeString(const QString& str)
{
    QByteArray resultu8;
    foreach (char c, str.toUtf8())
    {
        switch (c)
        {
        case '\'':    resultu8 += "\\'";   break;
        case '\\':   resultu8 += "\\\\";   break;
        case '\a':    resultu8 += "\\a";   break;
        case '\b':    resultu8 += "\\b";   break;
        case '\f':    resultu8 += "\\f";   break;
        case '\n':    resultu8 += "\\n";   break;
        case '\r':    resultu8 += "\\r";   break;
        case '\t':    resultu8 += "\\t";   break;
        case '\v':    resultu8 += "\\v";   break;
        default:
            if (c >= 0   &&   c < 0x20)
            {
                resultu8 += "\\x";
                if (c < 0x10)
                    resultu8 += "0";
                resultu8 += QString::number(c, 16);
            }
            else
                resultu8 += c;
        }
    }
    return QString::fromUtf8(resultu8);
}

template< class X >
struct SimplePrinter {
    void operator()(ostream& os, const X& x) const {
        os << x;
    }
};
template<>
struct SimplePrinter<QString> {
    void operator()(ostream& os, const QString& x) const {
        os << "'" << escapeString(x) << "'";
    }
};

struct NamePrinter {
    void operator()(ostream& os, const Named *x) const {
        os << x->name();
    }
    void operator()(ostream& os, const Named& x) const {
        os << x.name();
    }
};

struct PortPrinter {
    void operator()(ostream& os, const Port *port) const {
        os << "{name: '" << port->name() + "'";
        if (port->format().isFixed()) {
            os << ", format: [";
            printContainer(os, port->format().size(), SimplePrinter<int>(), ", ");
            os << "]";
        }
        if (!port->helpString().isEmpty())
            os << ", help: '" << escapeString(port->helpString()) << "'";
        if (port->entryHints().hasHints()) {
            os << ", hints: [";
            printContainer(os, port->entryHints().hints(), SimplePrinter<QString>(), ", ");
            os << "]";
        }
        os << "}";
    }
};

struct BoxPropPrinter {
    void operator()(ostream& os, const BoxProperty& boxProp) const {
        os << "{name: '" << boxProp.name
           << "', help: '" << escapeString(boxProp.helpString) << "'}";
    }
};

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
}

int main(int argc, char **argv)
{
#ifdef __linux__
    Q_INIT_RESOURCE(equares_core);
#endif // __linux__

    QCoreApplication app(argc, argv);

    try {
        // Parse command line arguments
        enum Mode { RunMode, DescribeMode } mode = RunMode;
        QStringList inputFileNames;
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
                default:
                    throw EquaresException(QString("Unrecognized option '%1'").arg(arg));
                }
            }
            inputFileNames << arg;
        }

        QScriptEngine engine;

        initBoxFactory();
        registerEquaresScriptTypes(&engine);

        switch (mode) {
        case RunMode:
            runFiles(engine, inputFileNames);
            break;
        case DescribeMode:
            describeSystem(engine);
            break;
        default:
            Q_ASSERT(false);
        }

        return 0;
    }
    catch(const exception& e) {
        cerr << "ERROR: " << e.what() << endl;
        return -1;
    }
}

