#ifndef PRINTUTIL_H
#define PRINTUTIL_H

#include <QString>
#include <QTextStream>

template<class Container, class Printer>
inline void printContainer(QTextStream& os, const Container& c, const Printer& p, const QString& delimiter)
{
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
    void operator()(QTextStream& os, const X& x) const {
        os << x;
    }
};
template<>
struct SimplePrinter<QString> {
    void operator()(QTextStream& os, const QString& x) const {
        os << "'" << escapeString(x) << "'";
    }
};

struct VerbatimStringPrinter {
    void operator()(QTextStream& os, const QString& x) const {
        os << x;
    }
};

#endif // PRINTUTIL_H
