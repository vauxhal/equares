/*
EquaRes - a tool for numerical analysis of equations.
Copyright (C) 2014 Stepan Orlov

This file is part of EquaRes.

Full text of copyright notice can be found in file copyright_notice.txt in the EquaRes root directory.
License agreement can be found in file LICENSE.md in the EquaRes root directory.
*/

#include "JsInputSplitter.h"
#include "equares_core/EquaresException.h"

bool JsInputSplitter::hasInput() const {
    return !m_input.isEmpty() && m_braces.isEmpty();
}

bool JsInputSplitter::isEmpty() const {
    return m_input.isEmpty();
}

QString JsInputSplitter::input() const
{
    Q_ASSERT(hasInput());
    QString result;
    foreach (const QString& line, m_input)
        result += line;
    return result;
}

void JsInputSplitter::clear()
{
    m_input.clear();
    m_braces.clear();
}

JsInputSplitter& JsInputSplitter::operator<<(const QString& line)
{
    QString l = line;

    // Remove '\r' (if line has CRLF at end, as it is on Windows)
    l.remove('\r');

    // Ignore empty lines
    if (l.isEmpty())
        return *this;

    countBraces(l);
    m_input << l;
    if (state() == InStringLiteral)
        m_input << "\\n";
    else
        m_input << "\n";
    return *this;
}

JsInputSplitter::State JsInputSplitter::state() const
{
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

void JsInputSplitter::countBraces(const QString& line)
{
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

void JsInputSplitter::matchBrace(QChar c)
{
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
