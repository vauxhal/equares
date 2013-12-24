names = function(x) { for(var n in x) print(n) }

odeParam = new Param
solverParam = new Param
counterParam = new Param

initState = new Param

ode = new DoublePendulum
solver = new Rk4
psec = new CrossSection
proj = new Projection
canvas = new Canvas
counter = new CountedFilter
dump = new Bitmap

proj.indices = [0,2]

s = new Simulation
s.setLinks([
    [odeParam.output, ode.parameters],
    [solverParam.output, solver.parameters],
    [counterParam.output, counter.count],
    [initState.output, solver.initState],
    [solver.rhsState, ode.state],
    [ode.oderhs, solver.rhs],
    [solver.nextState, psec.input],
    [psec.output, proj.input],
    [proj.output, canvas.input],
    [proj.output, counter.input],
    [counter.output, solver.stop],
    [solver.finish, canvas.flush],
    [canvas.output, dump.input]
])

odeParam.data = [
    1,      // l1
    1,      // m1
    0.6,    // l2
    1,      // m2
    9.8     // g
]

solverParam.data = [
    0.002,  // h
    0,      // n (0 means max. int)
    1       // nout (1 means each step)
]

// Degrees to radians
D2R = Math.PI/180

initState.data = [
    50*D2R, // q1
    0,      // q2
    0,      // dq1
    0,      // dq2
    0       // t
]

psec.param = {index: 1, pos: 0, flags: ["positive"]}

canvas.param = {
    x: {vmin: -0.5*Math.PI, vmax: 0.5*Math.PI, resolution: 500},
    y: {vmin: -2*Math.PI, vmax: 1.8*Math.PI, resolution: 500}
}
canvas.refreshInterval = 1000;

counterParam.data = [
    10000000   // Counter value
]

dump.fileName = "dump.png"

s.run();

