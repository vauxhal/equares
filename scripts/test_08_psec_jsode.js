names = function(x) { for(var n in x) print(n) }

odeParam = new Param
solverParam = new Param
counterParam = new Param

initState = new Param

ode = new JsOde
ode.ode = {
    varCount: 4,    // q1, q2, dq1, dq2
    paramCount: 5,  // l1, m1, l2, m2, g
    prepare: function(param) {
        var
            L1 = param[0],
            M1 = param[1],
            L2 = param[2],
            M2 = param[3],
            G = param[4]
        this.L12 = L1*L1
        this.L22 = L2*L2
        this.L12m = this.L12*M1
        this.L22m = this.L22*M2
        this.L1L2 = L1*L2
        this.L1mg = L1*M1*G
        this.L2mg = L2*M2*G
    },
    rhs: function(rhs, param, state) {
        rhs[0] = state[2]
        rhs[1] = state[3]
        var
            PHI = state[0],
            PSI = state[1],
            DPHI = state[2],
            DPSI = state[3],
            M2 = param[3],
            sphi = Math.sin(PHI),
            spsi = Math.sin(PSI),
            sdif = Math.sin(PSI-PHI),
            cdif = Math.cos(PSI-PHI),
            A = this.L12m + this.L22m,
            B = this.L1L2*M2*cdif,
            C = this.L22m,
            c1 = this.L1L2*M2*sdif,
            F1 = c1*DPSI*DPSI - (this.L1mg+this.L2mg)*sphi,
            F2 = -c1*DPHI*DPHI - this.L2mg*spsi,
            D = A*C - B*B,
            D1 = F1*C - F2*B,
            D2 = A*F2 - B*F1
        rhs[2] = D1/D;
        rhs[3] = D2/D;
    }
}
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
    100   // Counter value
]

dump.fileName = "dump.png"

s.run();

