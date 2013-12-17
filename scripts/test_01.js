names = function(x) { for(var n in x) print(n) }
param = new Param
ode = new Pendulum
s = new Simulation
s.setLinks([[param.output, ode.parameters]])
