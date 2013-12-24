names = function(x) { for(var n in x) print(n) }

odeParam = new Param
solverParam = new Param
counterParam = new Param

initState = new Param

ode = new CxxOde
solver = new Rk4
psec = new CrossSection
proj = new Projection
canvas = new Canvas
counter = new CountedFilter
dump = new Bitmap

proj.indices = [0,2]

ode.src = "
struct DoublePendulum {
    int paramCount() const {
        return 5;   // l1, m1, l2, m2, g
    }
    int varCount() const {
        return 4;   // q1, q2, dq1, dq2
    }

    // Auxiliary parameters
    double
        L1,
        M1,
        L2,
        M2,
        G,
        L12,
        L22,
        L12m,
        L22m,
        L1mg,
        L2mg;
    void prepare(const double *param) {
        L1 = param[0];
        M1 = param[1];
        L2 = param[2];
        M2 = param[3];
        G = param[4];
        L12 = L1*L1;
        L22 = L2*L2;
        L12m = L12*M1;
        L22m = L22*M2;
        L1mg = L1*M1*G;
        L2mg = L2*M2*G;
    }

    void rhs(double *out, const double *param, const double *state) const {
        out[0] = state[2];
        out[1] = state[3];
	double
            phi = state[0],
            psi = state[1],
            dphi = state[2],
            dpsi = state[3],
            sphi = sin(phi),
            spsi = sin(psi),
            cphi = cos(phi),
            cpsi = cos(psi),

            c = L1+L2*cpsi,

            aphi = L12m + M2*c*c,
            daphi = -2*M2*c*L2*dpsi*spsi,

            fphi = -daphi*dphi - G*sphi*((M1+M2)*L1 + M2*L2*cpsi),
            ddpsi = (-c*spsi*dphi*dphi - G*cphi*spsi)/L2;
	out[2] = fphi/aphi;
	out[3] = ddpsi;
    }
};
"

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
    10*D2R, // q1
    10*D2R, // q2
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

