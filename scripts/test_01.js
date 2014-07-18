/*
EquaRes - a tool for numerical analysis of equations.
Copyright (C) 2014 Stepan Orlov

This file is part of EquaRes.

Full text of copyright notice can be found in file copyright_notice.txt in the EquaRes root directory.
License agreement can be found in file LICENSE.md in the EquaRes root directory.
*/

names = function(x) { for(var n in x) print(n) }
param = new Param
ode = new Pendulum
s = new Simulation
s.setLinks([[param.output, ode.parameters]])
