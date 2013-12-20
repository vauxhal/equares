names = function(x) { for(var n in x) print(n) }

odeParam = new Param
solverParam = new Param
initState = new Param

ode = new CxxOde
solver = new Rk4
dump = new Dump

ode.src = "
struct Pendulum {
    int paramCount() const {
        return 2;   // l, g
    }
    int varCount() const {
        return 2;   // q, dq
    }

    // Auxiliary parameters
    double g_l;
    void prepare(const double *param) {
        g_l = param[1]/param[0];
    }

    void rhs(double *out, const double *param, const double *state) const {
        out[0] = state[1];
        out[1] = -g_l * sin(state[0]);
    }
};
"

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

