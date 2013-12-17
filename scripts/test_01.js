names = function(x) { for(n in x) print(n) }
param = new Param
ode = new Pendulum
s = new Simulation
s.setLinks([[param.output, ode.parameters]])
