/*
EquaRes - a tool for numerical analysis of equations.
Copyright (C) 2014 Stepan Orlov

This file is part of EquaRes.

Full text of copyright notice can be found in file copyright_notice.txt in the EquaRes root directory.
License agreement can be found in file LICENSE.md in the EquaRes root directory.
*/

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
