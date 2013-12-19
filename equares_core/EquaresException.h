#ifndef EQUARESEXCEPTION_H
#define EQUARESEXCEPTION_H

#include <exception>
#include <QString>

class EquaresException : public std::exception
{
public:
    ~EquaresException() throw() {}
    explicit EquaresException(const QString& what) throw() : m_what(what.toUtf8()) {}
    const char *what() const throw() {
        return m_what.constData();
    }
private:
    QByteArray m_what;
 };

#endif // EQUARESEXCEPTION_H
