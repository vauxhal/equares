#include <QCoreApplication>
#include "equares_core/equares_script.h"
#include "equares_core/initBoxFactory.h"
#include <QRegExp>

#include <QDebug>

#include <iostream>
#include <string>

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

int main(int argc, char **argv)
{
    Q_INIT_RESOURCE(equares_core);

    QCoreApplication app(argc, argv);
    QScriptEngine engine;

    initBoxFactory();
    registerEquaresScriptTypes(&engine);

    JsInputSplitter jsis;
    QRegExp rxExit("^\\s*exit\\s*$");
    forever {
        using namespace std;
        string line;
        getline(cin, line);
        if(cin.fail())
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

    return 0;
}

