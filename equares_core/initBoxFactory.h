/*
EquaRes - a tool for numerical analysis of equations.
Copyright (C) 2014 Stepan Orlov

This file is part of EquaRes.

Full text of copyright notice can be found in file copyright_notice.txt in the EquaRes root directory.
License agreement can be found in file LICENSE.md in the EquaRes root directory.
*/

#ifndef INITBOXFACTORY_H
#define INITBOXFACTORY_H

#ifdef EQUARES_CORE_STATIC
void initBoxFactory();
#else // EQUARES_CORE_STATIC
inline void initBoxFactory() {}
#endif // EQUARES_CORE_STATIC

#endif // INITBOXFACTORY_H
