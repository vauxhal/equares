names = function(x) { for(n in x) print(n) }

odeParam = new Param
solverParam = new Param
initState = new Param

ode = new JsOde
solver = new Rk4
dump = new Dump

ode.ode = {
    varCount: 2,    // q, dq
    paramCount: 2,  // l, g
    prepare: function(param) {
        this.g_l = param[0]/param[1]
    },
    rhs: function(rhs, param, state) {
        rhs[0] = state[1]
        rhs[1] = -this.g_l*state[0]
    }
}

s = new Simulation
s.setLinks([
    [odeParam.output, ode.parameters],
    [solverParam.output, solver.parameters],
    [initState.output, solver.initState],
    [solver.rhsState, ode.state],
    [ode.oderhs, solver.rhs],
    [solver.nextState, dump.input]
])

odeParam.data = [
    1,      // l
    9.8     // g
]

solverParam.data = [
    0.01,   // h
    10,     // n (0 means max. int)
    1       // nout (1 means each step)
]

// Degrees to radians
D2R = Math.PI/180

initState.data = [
    10*D2R, // q
    0,      // dq
    0
]

// dump.fileName = "dump.txt"

s.run();

