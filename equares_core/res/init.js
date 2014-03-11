var runSimulation

;(function(){

function newx(type) { return new this[type] }

function evalProp(src, dst)
{
    switch (typeof dst)
    {
    case 'string': case 'boolean':
        return src
    case 'number':
        try {
            return eval(src)
        } catch(err) {
            err.value = src
            throw err
        }
    case 'object':
        var result = src instanceof Array? []: {}
        var s
        for (var i in src)
        {
            if (dst[i] !== undefined)
                s = dst[i]
            if (s === undefined)
            {
                if (i === 'flags')
                    // Assume string type for element of flags array
                    s = ''
                else
                    // Assume numeric type for elements of other arrays
                    s = 0
            }
            result[i] = evalProp(src[i], s)
        }
        return result
    default: throw 'Unsupported type of destination property (' + (typeof dst) + ')'
    }
}

// deBUG
function printVal(v, pad)
{
    if (!pad)
        pad = '=='
    switch (typeof v) {
    case 'number': case 'boolean':
        print( pad + v + ' (' + (typeof v) + ')' )
        break
    case 'string':
        print( pad + "'" + v + "'" + ' (' + (typeof v) + ')' )
        break
    case 'object':
        var a = v instanceof Array
        print (pad + (a?'[':'{'))
        for (var i in v)
            printVal(v[i], pad+'==')
        print (pad + (a?']':'}'))
        break;
    case 'undefined':
        print (pad + 'undefined')
        break
    default: throw 'printVal: unsupported value type ' + (typeof v)
    }
}

runSimulation = function(sim) {
    var i
    var boxes = {}
    for (i=0; i<sim.boxes.length; ++i) {
        var bx = sim.boxes[i]
        var b = boxes[bx.name] = newx(bx.type)
        b.name = bx.name
        if (bx.props instanceof Object )
            for (var propName in bx.props)
                try {
                    //*
                    b[propName] = evalProp(bx.props[propName], b[propName])
                    /*/
                    // deBUG
                    {
                        print(bx.name + '.' + propName + ' (' + typeof(b[propName]) + ')')
                        var v = evalProp(bx.props[propName], b[propName])
                        print('-- before eval')
                        printVal(bx.props[propName])
                        print('-- after eval')
                        printVal(v)
                        b[propName] = v
                    }
                    //*/
                } catch (err) {
                    err.message = 'Unable to set property ' + bx.name + '.' + propName + ': ' + err.message + ' (value: ' + err.value + ')'
                    throw err
                }
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
