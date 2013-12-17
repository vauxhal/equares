names = function(x) { for(var n in x) print(n) }
param = new Param
initstate = new Param
rk4h = new Param
rk4T = new Param
rk4c = new Param
xcn = new Param
rk4adj = new Rk4ParamAdjust
ode = new DoublePendulum
rk4 = new Rk4
xs = new CrossSection
xc = new CountedFilter
proj = new Projection
canvas = new Canvas
dump = new Dump

xs.param = {index: 0, pos: 0, flags: ["positive"]}
// dump.fileName = "dump.txt"
proj.indices = [0,1]
canvas.param = {
    x: {vmin: -1, vmax: 1, resolution: 10},
    y: {vmin: -1, vmax: 1, resolution: 10}
}
s = new Simulation
s.setLinks([
        [param.output, ode.parameters],
        [rk4h.output, rk4adj.h],
        [rk4T.output, rk4adj.T],
        [rk4c.output, rk4adj.nout],
        [rk4adj.rk4param, rk4.parameters],
        [initstate.output, rk4.initState],
        [rk4.rhsState, ode.state],
        [ode.oderhs, rk4.rhs],
        [rk4.nextState, dump.input],
/*
        [rk4.nextState, xs.input],
        [xcn.output, xc.count],
        [xs.output, xc.input],
        [xc.output, proj.input],
        [proj.output, canvas.input],
        [rk4.finish, canvas.flush],
        [canvas.output, dump.input]
*/
])
param.data = [
    1,      // l1
    1,      // m1
    0.6,    // l2
    1,      // m2
    9.8     // g
]

initstate.data = [
    0.1,    // q1
    0,      // q2
    0,      // dq1
    0,      // dq2
    0       // t
]

rk4h.data = [1e-2]
rk4T.data = [0.1]
rk4c.data = [1]

xcn.data = [2]

s.run();

