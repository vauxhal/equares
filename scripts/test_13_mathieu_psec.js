names = function(x) { for(var n in x) print(n) }

odeParam = new Param
solverParam = new Param
counterParam = new Param

initState = new Param

ode = new VibratingPendulum
solver = new Rk4
proj = new Projection
canvas = new Canvas
counter = new CountedFilter
dump = new Bitmap

proj.indices = [0,1]

s = new Simulation
s.setLinks([
    [odeParam.output, ode.parameters],
    [solverParam.output, solver.parameters],
    [counterParam.output, counter.count],
    [initState.output, solver.initState],
    [solver.rhsState, ode.state],
    [ode.oderhs, solver.rhs],
    [solver.nextState, proj.input],
    [proj.output, canvas.input],
    [proj.output, counter.input],
    [counter.output, solver.stop],
    [solver.finish, canvas.flush],
    [canvas.output, dump.input]
])

var
    l = 1,
    g = 9.8,
    a = 0.45,
    omega = Math.sqrt(g/l)*1.95,
    nStepsPerPeriod = 1000,
    nPeriods = 100000

odeParam.data = [
    l,
    g,
    a,
    omega
]

solverParam.data = [
    2*Math.PI/(omega*nStepsPerPeriod),  // h
    0,                                  // n (0 means max. int)
    nStepsPerPeriod                     // nout (1 means each step)
]

// Degrees to radians
D2R = Math.PI/180

initState.data = [
    10*D2R, // q1
    0,      // dq1
    0       // t
]

var sx = 1,
    sy = 1
canvas.param = {
    x: {vmin: -sx*Math.PI, vmax: sx*Math.PI, resolution: 500},
    y: {vmin: -sy*Math.PI, vmax: sy*Math.PI, resolution: 500}
}
canvas.refreshInterval = 1000;

counterParam.data = [
    10000000   // Counter value
]

dump.fileName = "dump.png"

s.run();

