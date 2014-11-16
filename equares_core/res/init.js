/*
EquaRes - a tool for numerical analysis of equations.
Copyright (C) 2014 Stepan Orlov

This file is part of EquaRes.

Full text of copyright notice can be found in file copyright_notice.txt in the EquaRes root directory.
License agreement can be found in file LICENSE.md in the EquaRes root directory.
*/

var runSimulation, buildDirs, runDbSim, printTree

;(function(){

printTree = function(o, pad, name) {
    if (typeof pad != 'string')
        pad = ''
    var t = typeof o
    var s = pad
    if (name)
        s += name + ': '
    s += t
    if (t == 'object') {
        var tn = o.constructor.name || 'unknown type'
        s += ' (' + tn + ')'
        if (o instanceof Array)
            s += '[' + o.length + ']'
        print(s)
        var padd = pad + '    '
        for (var i in o)
            printTree(o[i], padd, i)
    }
    else {
        s += ' = ' + JSON.stringify(o)
        print(s)
    }
}

function newx(type) { return new this[type] }

function evalProp(src, dst)
{
    switch (typeof dst)
    {
    case 'string':
        return src
    case 'boolean':
        if (typeof src === 'string')
            return src === 'true'
        else
            return src? true: false
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
        for (var i in v) {
            print(pad + '[' + i + ']:')
            printVal(v[i], pad+'==')
        }
        print (pad + (a?']':'}'))
        break;
    case 'undefined':
        print (pad + 'undefined')
        break
    default: throw 'printVal: unsupported value type ' + (typeof v)
    }
}

function nativeBox(bx) {
    var b = newx(bx.type)
    b.name = bx.name
    if (bx.props instanceof Object )
        for (var propName in bx.props)
            try {
                // Note: sometimes we have to set property twice - to recognize exact types of array elements
                //*
                b[propName] = evalProp(bx.props[propName], b[propName])
                b[propName] = evalProp(bx.props[propName], b[propName])
                /*/
                // deBUG
                {
                print(bx.name + '.' + propName + ' (' + typeof(b[propName]) + ')')
                var v = evalProp(bx.props[propName], b[propName])
                b[propName] = v
                v = evalProp(bx.props[propName], b[propName])
                b[propName] = v
                print('-- before eval')
                printVal(bx.props[propName])
                print('-- after eval')
                printVal(v)
                }
                //*/
            } catch (err) {
                err.message = 'Unable to set property ' + bx.name + '.' + propName + ':\n' + err.message
                throw err
            }
    return b
}

function nativeSim(sim) {
    // deBUG, TODO: Remove
//    printTree(sim)

    var i,   n = sim.boxes.length
    var boxes = {}, boxArray = []
    for (i=0; i<n; ++i) {
        var bx = sim.boxes[i]
        boxArray.push(boxes[bx.name] = nativeBox(bx))
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
    var s = new Simulation
    s.setBoxes(boxArray);
    s.setLinks(links)
    return s
}

runSimulation = function(sim) {
    var s = nativeSim(sim)
    s.run()
}

// Returns build dirs for the specified simulation from database
// Note: The format of sim it not the same as in runSimulation():
// we need to replace props[*] with props[*].value
buildDirs = function(sim) {
    var dirs = [], i, n = sim.boxes.length
    for (i=0; i<n; ++i) {
        var bx = sim.boxes[i]
        var b = newx(bx.type)
        if (!(b.buildDir instanceof Function))
            continue

        // Note: The format of sim it not the same as in runSimulation():
        // we need to replace props[*] with props[*].value
        for (var propName in bx.props)
            bx.props[propName] = evalProp(bx.props[propName].value, b[propName])
            // TODO: Remove bx.props[propName] = bx.props[propName].value

        var d = b.buildDir(bx.props)
        if (d instanceof Array)
            dirs.concat(d)
        else
            dirs.push(d)
    }
    return dirs
}

function convertDbSim(dbsimdef) {
    var sim = JSON.parse(dbsimdef)
    // deBUG, TODO: Remove
//    printTree(sim)
    for (var i=0, n=sim.boxes.length; i<n; ++i) {
        var bx = sim.boxes[i]
        for (var propName in bx.props)
            bx.props[propName] = bx.props[propName].value
    }
    // deBUG, TODO: Remove
//    print('===')
//    printTree(sim)
    return sim
}

runDbSim = function(fileName) {
    var dbsim = JSON.parse(readFile(fileName))
    var sim = convertDbSim(dbsim.definition)
    eval(dbsim.script)
    runSimulation(sim)
}

})()
