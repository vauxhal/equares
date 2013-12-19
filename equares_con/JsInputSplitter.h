#ifndef JSINPUTSPLITTER_H
#define JSINPUTSPLITTER_H

#include <QString>
#include <QStringList>

class JsInputSplitter
{
public:
    bool hasInput() const;
    bool isEmpty() const;
    QString input() const;
    void clear();
    JsInputSplitter& operator<<(const QString& line);

private:
    QStringList m_input;
    QString m_braces;
    enum State {
        Normal,
        InComment,
        InStringLiteral
    };
    State state() const;
    void countBraces(const QString& line);
    void matchBrace(QChar c);
};

#endif // JSINPUTSPLITTER_H
