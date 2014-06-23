#ifndef BOX_UTIL_H
#define BOX_UTIL_H

#include "equares_core.h"

bool propagateCommonFormat(Port& port1, Port& port2);
bool propagateCommonFormat(const QList<Port*>& ports);
QString readFile(const QString& fileName, const Box *box);

#endif // BOX_UTIL_H
