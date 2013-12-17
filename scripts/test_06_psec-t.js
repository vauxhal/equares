names = function(x) { for(n in x) print(n) }

function run(a, omegaFactor) {
    odeParam = new Param
    solverParam = new Param
    counterParam = new Param

    initState = new Param

    ode = new VibratingPendulum
    solver = new Rk4
    stroboscope = new IntervalFilter
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
        [solver.nextState, stroboscope.input],
        [stroboscope.output, proj.input],
        [proj.output, canvas.input],
        [proj.output, counter.input],
        [counter.output, solver.stop],
        [solver.finish, canvas.flush],
        [canvas.output, dump.input]
    ])

    l = 1
    g = 9.8
    omega = Math.sqrt(g/l)*2*omegaFactor

    odeParam.data = [
        l,      // l
        g,      // g
        a,      // a
        omega   // omega
    ]

    solverParam.data = [
        0.01,   // h
        0,      // n (0 means max. int)
        1       // nout (1 means each step)
    ]

    // Degrees to radians
    D2R = Math.PI/180

    initState.data = [
        1*D2R,  // q
        0,      // dq
        0       // t
    ]

    stroboscope.param = {index: 2, offset: 0, interval: 2*Math.PI/omega}

    canvas.param = {
        x: {vmin: -Math.PI, vmax: Math.PI, resolution: 500},
        y: {vmin: -Math.PI, vmax: Math.PI, resolution: 500}
    }

    counterParam.data = [
        10000   // Counter value
    ]

    dump.fileName = "dump.png"

    s.run();
}
run(0.01, 1)


