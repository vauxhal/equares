/*
EquaRes - a tool for numerical analysis of equations.
Copyright (C) 2014 Stepan Orlov

This file is part of EquaRes.

Full text of copyright notice can be found in file copyright_notice.txt in the EquaRes root directory.
License agreement can be found in file LICENSE.md in the EquaRes root directory.
*/

#ifndef EQUARES_CORE_GLOBAL_H
#define EQUARES_CORE_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(EQUARES_CORE_LIBRARY)
#  define EQUARES_CORESHARED_EXPORT Q_DECL_EXPORT
#else
#  define EQUARES_CORESHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // EQUARES_CORE_GLOBAL_H
