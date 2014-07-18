/*
EquaRes - a tool for numerical analysis of equations.
Copyright (C) 2014 Stepan Orlov

This file is part of EquaRes.

Full text of copyright notice can be found in file copyright_notice.txt in the EquaRes root directory.
License agreement can be found in file LICENSE.md in the EquaRes root directory.
*/

#include "equares_script.h"

ScriptCustomInit::ScriptInitFuncList *ScriptCustomInit::m_initializers;

void ScriptCustomInit::init(QScriptEngine *e) {
    foreach (ScriptInitFunc func, initializers())
        func(e);
}

void ScriptCustomInit::registerScriptInitFunc(ScriptInitFunc func) {
    initializers() << func;
}

ScriptCustomInit::ScriptInitFuncList& ScriptCustomInit::initializers() {
    if (!m_initializers)
        m_initializers = new ScriptInitFuncList;
    return *m_initializers;
}
