var equaresBox = {};

(function(){
    var rules = equaresBox.rules = {}
    var timestamp = 0

    var cloneArray = function(a) { return $.merge([], a) }
    var arraysEqual = function(a, b) {
         if (a === undefined && b === undefined)
             return true
        if (!((a instanceof Array) && (b instanceof Array)))
            return false
        var n = a.length
        if (b.length != n)
            return false
        for (--n; n>=0; --n)
            if (a[n] !== b[n])
                return false
        return true
    }

    var PortFormat = equaresBox.PortFormat = function(port)
    {
         this.format = port.format
         this.hints = port.hints
         this.bad = false
    }
    PortFormat.prototype.valid = function() {
        return this.format !== undefined && !this.bad
    }
    PortFormat.prototype.accum = function(port) {
        if (this.bad)
            return
        if (!port.format)
            return
        if (this.valid()) {
            if (!arraysEqual(this.format, port.format))
                this.bad = true
            else if (port.hints instanceof Array) {
                if (this.hints instanceof Array) {
                    if (!arraysEqual(this.hints, port.hints))
                        this.bad = true
                }
                else
                    this.hints = port.hints
            }
        }
        else {
            this.format = port.format
            this.hints = port.hints
        }
    }
    PortFormat.prototype.equals = function(that) {
        if (this.bad != that.bad)
            return false
        if (this.bad)
            return true
        if (!arraysEqual(this.format, that.format))
            return false
        var h1 = this.hints instanceof Array,
            h2 = that.hints instanceof Array
        if (h1 != h2)
            return false
        return h1? arraysEqual(this.hints, that.hints): true
    }

    var Port = equaresBox.Port = function (info, box, index) {
        if (arguments.length == 0)
            return
        this.info = info
        this.box = box
        this.index = index
        this.format = info.format
        this.hints = info.hints
        this.pos = info.pos
    }
    Port.prototype.getFormat = function(excludeSelf) {
        var f = arguments[0]
        if (f instanceof PortFormat)
            f.accum(this)
        else {
            f = new PortFormat(excludeSelf? {}: this)
            ++timestamp
        }
        this.timestamp = timestamp
        var c = this.connectedPorts()
        for (var i=0; i<c.length; ++i) {
            var p = c[i]
            if (p.timestamp === timestamp)
                continue
            p.getFormat(f)
        }
        return f
    }

    var InputPort = equaresBox.InputPort = function() {
        this.type = "in"
        Port.apply(this, arguments)
        this.connection = null
    }
    InputPort.prototype = new Port()
    InputPort.prototype.canConnect = function(thatPort) {
        if (this.connection != null)
            return false
        return arguments.length == 0 ?   true :
            this.box === thatPort.box ?   false :
            thatPort instanceof OutputPort;
    }
    InputPort.prototype.connect = function(thatPort) {
        this.connection = thatPort
    }
    InputPort.prototype.disconnect = function(thatPort) {
        this.connection = null
    }
    InputPort.prototype.connectedPorts = function() {
        return this.connection? [this.connection]: []
    }

    var OutputPort = equaresBox.OutputPort = function() {
        this.type = "out"
        Port.apply(this, arguments)
        this.connections = []
    }
    OutputPort.prototype = new Port()
    OutputPort.prototype.findConnection = function(port) {
        for (var i=0; i<this.connections.length; ++i)
            if (this.connections[i] === port)
                return i
        return -1
    }
    OutputPort.prototype.canConnect = function(thatPort) {
        return arguments.length == 0 ?   true :
            this.box === thatPort.box ?   false :
            (thatPort instanceof InputPort)   &&   this.findConnection(thatPort) == -1
    }
    OutputPort.prototype.connect = function(thatPort) {
        this.connections.push(thatPort)
    }
    OutputPort.prototype.disconnect = function(thatPort) {
        this.connections.splice(this.findConnection(thatPort), 1)
    }
    OutputPort.prototype.connectedPorts = function() {
        return this.connections
    }

    function importFunc(dst, src, name) {
        var f = src[name]
        if (typeof(f) != "string"   ||   f.length == 0)
            return
        dst[name] = eval("(function(){return " + f + "})()")
    }

    var Box = equaresBox.Box = function(name, type, info) {
        if (arguments.length == 0)
            return
        this.name = name
        this.type = type
        this.info = info
        this.ports = []
        function addPorts(box, ports, PortCtor) {
            for (var i=0, n=ports.length; i<n; ++i)
                box.ports.push(new PortCtor(ports[i], box, box.ports.length))
        }
        addPorts(this, info.inputs, InputPort)
        addPorts(this, info.outputs, OutputPort)
        this.props = {}
        for(var i in info.properties) {
            var pi = info.properties[i]
            var p = this.props[pi.name] = {}
            // p.name = pi.name
            if (pi.userType.length > 0)
                p.userType = JSON.parse(pi.userType)
            importFunc(p, pi, "toBoxType")
            importFunc(p, pi, "toUserType")
            importFunc(p, pi, "resolveUserType")
            var d = pi.defaultValue
            if (d !== undefined) {
                if (d instanceof Array)
                    p.value = $.merge([], d)
                else if (d instanceof Object)
                    p.value = $.extend(true, {}, d)
                else
                    p.value = d
            }
        }
        this.callbacks = []
        this.stateChanged("init")
    }
    Box.prototype.prop = function(name, value) {
        if (arguments.length == 2) {
            this.props[name].value = value
            this.stateChanged("prop", name)
            return this
        }
        else
            return this.props[name].value
    }
    Box.prototype.propType = function(name) {
        var p = this.props[name]
        return p? p.userType: undefined
    }
    Box.prototype.stateChanged = function(kind, id) {
        var rule = rules[this.type]
        if (!rule)
            return
        var func = rule[kind]
        if (func instanceof Function)
            func.call(this, id)
        for (var i=0; i<this.callbacks.length; ++i)
            this.callbacks[i].call(this, kind, id)
    }
    Box.prototype.links = function() {
        var result = []
        var ports = this.ports
        for (var i=0; i<ports.length; ++i) {
            var port = ports[i]
            var c = port.connectedPorts()
            for (var j=0; j<c.length; ++j)
                result.push({source: port, target: c[j]})
        }
        return result
    }
    // Arguments:
    // a) port having name and type that we are looking for
    // b) name [, type]
    Box.prototype.findPort = function() {
        if (arguments[0] instanceof Port) {
            var p = arguments[0]
            return this.findPort(p.info.name, p.type)
        }
        else {
            var name = arguments[0], type = arguments[1]
            var ports = this.ports
            for (var i=0; i<ports.length; ++i) {
                var port = ports[i]
                if (port.info.name !== name)
                    continue
                if (type && port.type !== type)
                    continue
                return port
            }
            return null
        }
    }

    equaresBox.canConnect = function(port1, port2) {
        return port1.canConnect(port2) && port2.canConnect(port1)
    }
    function wrapConnect(port1, port2, func) {
        var f1b = port1.getFormat()
        var f2b = port2.getFormat()
        func()
        var f1a = port1.getFormat()
        var f2a = port2.getFormat()
        if (!f1b.equals(f1a))
            port1.box.stateChanged("port", port1)
        if (!f2b.equals(f2a))
            port2.box.stateChanged("port", port2)
    }

    equaresBox.connect = function(port1, port2) {
        wrapConnect(port1, port2, function() {
            port1.connect(port2)
            port2.connect(port1)
        })
    }
    equaresBox.disconnect = function(port1, port2) {
        wrapConnect(port1, port2, function() {
            port1.disconnect(port2)
            port2.disconnect(port1)
        })
    }

    equaresBox.info = function(request, callback) {
        if (typeof request == "string")
            request = {cmd: request}
        $.ajax("equaresRequestInfo.cmd", {data: request, type: "GET"})
            .done(function(data) {
                var reply = JSON.parse(data);
                if (reply.error)
                    alert("equaresRequestInfo.cmd error: \n" + reply.stderr);
                else
                    callback(eval("(function(){return " + reply.stdout + "})()"));
            })
            .fail(function() {
                // equaresDebug.html("equaresExec.cmd: Ajax error");
                alert("equaresRequestInfo.cmd: Ajax error");
            });
    }
})()

;(function(){

var PortFormat = equaresBox.PortFormat,
    Port = equaresBox.Port,
    InputPort = equaresBox.InputPort,
    OutputPort = equaresBox.OutputPort

function port2type(port) {
    var f = port.getFormat()
    if (f.valid()) {
        if (f.format.length == 0)
            return undefined;
        if (f.format.length == 1) {
            var n = f.format[0]
            var result, i
            if (f.hints instanceof Array) {
                result = {}
                for (i=0; i<n; ++i)
                    result[f.hints[i]] = 'd';
            }
            else {
                result = []
                for (i=0; i<n; ++i)
                    result[i] = 'd'
            }
            return result;
        }
        else
            return "TODO: multi-dim array"
    }
    else
        return undefined;
}

function defaultValue(type) {
    var result
    if (type instanceof Array) {
        result = []
        for (var i=0; i<type.length; ++i)
            result[i] = defaultValue(type[i])
    }
    else if (type instanceof Object) {
        result = {}
        for (var name in type)
            result[name] = defaultValue(type[name])
    } else if (typeof(type) == "string") switch (type) {
        case 'i': case 'd':
            result = 0
            break
        case 's':
            result = ""
            break
        case 'i:*':
            result = []
            break
    }
    return result
}

function setFormat(port, format) {
    var f0 = port.getFormat()
    port.format = format.format
    port.hints = format.hints

    // deBUG
    // console.log(port.box.name + ":" + port.info.name + ", format=" + (format.format === undefined? "undefined": format.format.toString()))

    if (!f0.equals(port.getFormat())) {
        var c = port.connectedPorts()
        for (var i=0; i<c.length; ++i) {
            var p = c[i]
            // if (p.timestamp === timestamp)
            //     continue
            p.box.stateChanged("port", p)
        }
    }
}

function projectionOutFormat() {
    var indices = this.prop("indices")
    var fin = this.ports[0].getFormat()
    var hints
    if (fin.valid() && fin.hints) {
        hints = []
        for (var i=0; i<indices.length; ++i)
            hints[i] = fin.hints[indices[i]]
    }
    setFormat(this.ports[1], {format: [indices.length], hints: hints})
}

function propagateFormat(port1, iin, iout) {
    var port2
    if (port1 instanceof InputPort) {
        if (port1.index != iin)
            return
        port2 = this.ports[iout]
    }
    else {
        if (port1.index != iout)
            return
        port2 = this.ports[iin]
    }
    var f1 = port1.getFormat(true),
        f2 = port2.getFormat(true)
    if (f1.valid() == f2.valid()) {
        setFormat(port1, {})
        setFormat(port2, {})
    }
    else if (f1.valid()) {
        setFormat(port1, {})
        setFormat(port2, f1)
    }
    else {
        setFormat(port1, f2)
        setFormat(port2, {})
    }
}

function propagateFormatDirected(port1, ifrom, ito)
{
    if (port1.index != ifrom)
        return
    if (!(ito instanceof Array))
        ito = [ito]
    for (var i=0; i<ito.length; ++i) {
        var port2 = this.ports[ito[i]]
        var f1 = port1.getFormat(true)
        setFormat(port2, f1.valid()? f1: {})
    }
}

$.extend(equaresBox.rules, {
    Param: {
        port: function(port) {
            var t = this.props["data"].userType = port2type(port)
            this.prop("data", defaultValue(t))
            this.stateChanged("propset")
        }
    },
    CrossSection: {
        port: function(port) { propagateFormat.call(this, port, 0, 1) }
    },
    CountedFilter: {
        port: function(port) { propagateFormat.call(this, port, 1, 2) }
    },
    IntervalFilter: {
        port: function(port) { propagateFormat.call(this, port, 0, 1) }
    },
    Valve: {
        port: function(port) { propagateFormat.call(this, port, 1, 2) }
    },
    Projection: {
        init: projectionOutFormat,
        prop: function(name) {
            if (name === "indices")
                projectionOutFormat.call(this)
        },
        port: function(port) {
            if (port.info.name === "input")
                projectionOutFormat.call(this)
            }
    },
    Rk4: {
        port: function(port) {
            propagateFormatDirected.call(this, port, 5, [0,1])
        }
    },
    CxxOde: {
        prop: function(name) {
            if (name === "src")
                equaresBox.info({
                    cmd: "box -i",
                    input: "box = new CxxOde\nbox.src = '\n" + this.prop(name) + "'\n"
                }, function(info) {
                    // Update port format
                    this.info = info
                    var i=0, j
                    for (j=0; j<info.inputs.length; ++j, ++i) {
                        this.ports[i].format = info.inputs[j].format
                        this.ports[i].hints = info.inputs[j].hints
                    }
                    for (j=0; j<info.outputs.length; ++j, ++i) {
                        this.ports[i].format = info.outputs[j].format
                        this.ports[i].hints = info.outputs[j].hints
                    }
                })
            }
        }
})
})()
