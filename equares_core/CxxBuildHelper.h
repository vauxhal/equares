/*
EquaRes - a tool for numerical analysis of equations.
Copyright (C) 2014 Stepan Orlov

This file is part of EquaRes.

Full text of copyright notice can be found in file copyright_notice.txt in the EquaRes root directory.
License agreement can be found in file LICENSE.md in the EquaRes root directory.
*/

#ifndef CXXBUILDHELPER_H
#define CXXBUILDHELPER_H

#include <QSharedPointer>

class Box;
class QLibrary;
typedef QSharedPointer<QLibrary> QLibraryPtr;

class CxxBuildHelper
{
public:
    CxxBuildHelper(
        const Box *user,
        const QString& src,
        const QString& libBaseName,
        const QString& footerFileName,
        bool useQmake);

    QString buildDirName() const throw();
    QString className() const throw();
    QString hashString() const throw();
    QLibraryPtr library() const;

private:
    const Box *m_box;
    QString m_src;
    QString m_libBaseName;
    QString m_footerFileName;
    bool m_useQmake;

    mutable QString m_className;
    mutable QString m_hashString;
    mutable bool m_hasNames;
    mutable bool m_namesExtracted;
    mutable QString m_error;

    bool extractNames() const;
    void throwBoxException(const QString& what) const;
    bool libUpToDate(const QString &libName, const QString& hashString) const;
    void checkLib(const QString& libName) const;
};

#endif // CXXBUILDHELPER_H
