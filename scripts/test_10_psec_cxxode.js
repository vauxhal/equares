names = function(x) { for(var n in x) print(n) }

odeParam = new Param
solverParam = new Param
counterParam = new Param

initState = new Param

ode = new CxxOde
ode.name = "code"
// ode.useQmake = false;
ode.src = "
struct X {
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
        L1L2,
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
        L1L2 = L1*L2;
        L1mg = L1*M1*G;
        L2mg = L2*M2*G;
    }

    void rhs(double *out, const double *param, const double *state) const {
        out[0] = state[2];
        out[1] = state[3];
        double
            PHI = state[0],
            PSI = state[1],
            DPHI = state[2],
            DPSI = state[3],
            sphi = sin(PHI),
            spsi = sin(PSI),
            sdif = sin(PSI-PHI),
            cdif = cos(PSI-PHI),
            A = L12m + L22m,
            B = L1L2*M2*cdif,
            C = L22m,
            c1 = L1L2*M2*sdif,
            F1 = c1*DPSI*DPSI - (L1mg+L2mg)*sphi,
            F2 = -c1*DPHI*DPHI - L2mg*spsi,
            D = A*C - B*B,
            D1 = F1*C - F2*B,
            D2 = A*F2 - B*F1;
        out[2] = D1/D;
        out[3] = D2/D;
    }
};
"

solver = new Rk4
psec = new CrossSection
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
    0.005,  // h
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

psec.param = {index: 2, pos: 0, flags: ["positive"]}

canvas.param = {
    x: {vmin: -Math.PI, vmax: Math.PI, resolution: 500},
    y: {vmin: -Math.PI, vmax: Math.PI, resolution: 500}
}

counterParam.data = [
    10000   // Counter value
]

dump.fileName = "dump.png"

s.run();

