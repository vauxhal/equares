var runSimulation

;(function(){

function newx(type) { return new this[type] }

runSimulation = function(sim) {
    var i
    var boxes = {}
    for (i=0; i<sim.boxes.length; ++i) {
        var bx = sim.boxes[i]
        var b = boxes[bx.name] = newx(bx.type)
        b.name = bx.name
        if (bx.props instanceof Object )
            for (var propName in bx.props)
                b[propName] = bx.props[propName]
    }
    var links = []
    for (i=0; i<sim.links.length; ++i) {
        var lx = sim.links[i]
        var l = [
            boxes[lx.source.box][lx.source.port],
            boxes[lx.target.box][lx.target.port]
        ]
        links.push(l)
    }
    this.s = new Simulation
    s.setLinks(links)
    s.run()
}

})()
