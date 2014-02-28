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
    var findFirst = equaresBox.findFirst = function(array, match) {
        for (var i=0; i<array.length; ++i)
            if (match(array[i]))
                return array[i]
        throw {message: "element is not found"}
    }

    var EventEmitter = equaresBox.EventEmitter = function() {
        this.eventHandlers = {}
    }
    EventEmitter.prototype.handlers = function(event) {
        var result = this.eventHandlers[event]
        return result || []
    }
    EventEmitter.prototype.on = EventEmitter.prototype.addListener = function(event, handler) {
        var h = this.handlers(event)
        h.push(handler)
        this.eventHandlers[event] = h
    }
    EventEmitter.prototype.removeListener = function(event, handler) {
        var h = this.handlers(event)
        try {
            var i = findFirst(h, function(item) { return item === handler })
            h.splice(i, 1)
            this.eventHandlers[event] = h
        }
        catch(e) {}
    }
    EventEmitter.prototype.emit = function(event) {
        var args = cloneArray(arguments)
        args.splice(0, 1)
        var h = this.handlers(event)
        for (var i=0; i<h.length; ++h)
            h[i].apply(this, args)
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
    PortFormat.prototype.toHtml = function() {
        if (this.format instanceof Array) {
            var text = ""
            var count = 1;
            for (var i=0; i<this.format.length; ++i) {
                if (i > 0)
                    text += " x ";
                text += this.format[i];
                count *= this.format[i];
            }
            text += count == 1? " element": " elements";
            if (this.hints) {
                text += " ("
                for (i=0; i<this.hints.length; ++i) {
                    if (i > 0)
                        text += ", "
                    text += "<i>" + this.hints[i] + "</i>"
                }
                text += ")"
            }
            return text
        }
        else
            return "unspecified"
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
    Port.prototype.getFormat = function(arg) {
        if (arg instanceof PortFormat)
            arg.accum(this)
        else {
            arg = new PortFormat(arg === true? {}: this)
            ++timestamp
        }
        this.timestamp = timestamp
        var c = this.connectedPorts()
        for (var i=0; i<c.length; ++i) {
            var p = c[i]
            if (p.timestamp === timestamp)
                continue
            p.getFormat(arg)
        }
        return arg
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

    var Box = equaresBox.Box = function(name, type, options) {
        if (arguments.length == 0)
            return
        this.name = name
        this.type = type
        var opt = options || {}
        var info = $.extend(true, {}, equaresBox.boxInfo[type])
        if (opt.info)
            $.extend(info, opt.info)
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
            p.critical = pi.critical
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
        this.status = { level: "ok", text: "Ok" }
        this.stateChanged("init")
    }
    Box.prototype = new EventEmitter()
    Box.prototype.prop = function(name, value) {
        if (arguments.length == 2) {
            this.props[name].value = value
            this.stateChanged("prop", name)
            return this
        }
        else
            return this.props[name].value
    }
    Box.prototype.boxprop = function(name) {
        var prop = this.props[name]
        if (prop.toBoxProp instanceof Function)
            return prop.toBoxProp.call(this, prop, name)
        else
            return prop.value
    }
    Box.prototype.propType = function(name) {
        var p = this.props[name]
        return p? p.userType: undefined
    }
    Box.prototype.criticalPropCount = function() {
        var result = 0
        for (var propName in this.props)
            if (this.props[propName].critical)
                ++result
        return result
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
        $.get("equaresRequestInfo.cmd", {cmd: request})
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

    equaresBox.infoEx = function(request, callback, errorCallback) {
        $.post("equaresRequestInfoEx.cmd", request)
            .done(function(data) {
                var reply = JSON.parse(data);
                if (reply.error) {
                    if (errorCallback instanceof Function)
                        errorCallback(reply)
                    else
                        alert("equaresRequestInfoEx.cmd error: \n" + (reply.stderr || reply.message));
                }
                else
                    callback(eval("(function(){return " + reply + "})()"));
            })
            .fail(function() {
                if (errorCallback instanceof Function)
                    errorCallback("Ajax error")
                else
                    alert("equaresRequestInfoEx.cmd: Ajax error");
            });
    }

    equaresBox.init = function(callback, progressCallback) {
        if (equaresBox.boxTypes)
            callback()
        else equaresBox.info("boxTypes", function(boxTypes) {
            equaresBox.boxTypes = boxTypes
            equaresBox.boxInfo = {}
            var n = 0
            function loadBox(i) {
                var boxType = boxTypes[i]
                equaresBox.info(boxType, function(boxInfo) {
                    equaresBox.boxInfo[boxType] = boxInfo
                    ++n
                    if (progressCallback instanceof Function) {
                        var percent = Math.round(100*n / boxTypes.length)
                        progressCallback(percent)
                    }
                    if (n == boxTypes.length)
                        callback()
                    // dbg-loading-progress
                    // else setTimeout(function(){ loadBox(i+1) }, 100)
                })
            }
            // dbg-loading-progress
            // setTimeout(function(){loadBox(0)}, 500)
            for (var i=0; i<boxTypes.length; ++i)
                loadBox(i)
        })
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

function port2value(value, port) {
    var f = port.getFormat()
    var result = value
    if (f.valid()) {
        if (f.format.length == 0)
            result = [];
        if (f.format.length == 1) {
            var n = f.format[0]
            if (f.hints instanceof Array) {
                result = []
                for (var i=0; i<n; ++i)
                    result[i] = value[f.hints[i]];
            }
        }
    }
    return result;
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

function setGoodStatus(box) { box.status = { level: "ok", text: "Ok" } }
function setUnspecPortStatus(box, arg) {
    var msg
    switch (typeof arg) {
    case "string":
        msg = arg
        break
    case "number":
        msg = "Unknown format of port '" + box.ports[arg].info.name + "'"
        break
    case "object":
        var portNames = []
        for (var i in arg)
            portNames.push("'" + box.ports[arg[i]].info.name + "'")
        msg = "Unknown format of ports " + portNames.join(", ")
        break
    default:
        msg = "Unknown port format"
    }
    box.status = { level: "warning", text: msg }
}
function setBadPortStatus(box, arg, explanation) {
    var msg
    switch (typeof arg) {
    case "string":
        msg = arg
        break
    case "number":
        msg = "Bad format of port '" + box.ports[arg].info.name + "'"
        break
    case "object":
        var portNames = []
        for (var i in arg)
            portNames.push("'" + box.ports[arg[i]].info.name + "'")
        msg = "Bad format of ports " + portNames.join(", ")
        break
    default:
        msg = "Bad port format"
    }
    if (typeof explanation == "string")
        msg += ": " + explanation
    box.status = { level: "error", text: msg }
}

function projectionOutFormat() {
    var indices = this.prop("indices")
    var fin = this.ports[0].getFormat()
    var hints
    if (fin.valid()) {
        var err
        if (fin.format.length != 1)
            err = "1D data was expected at port 'input'"
        if (fin.hints) {
            hints = []
            for (var i=0; i<indices.length; ++i)
                hints[i] = fin.hints[indices[i]] || "?"
        }
        for (i=0; !err && i<indices.length; ++i) {
            if (indices[i] >= fin.format[0])
                err = "One or more index is out of range"
        }
        if (err)
            this.status = { level: "error", text: err }
        else
            setGoodStatus(this)
        }
    else {
        setUnspecPortStatus(this, 0)
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
        if (f1.valid()) {
            if (f1.equals(f2))
                setGoodStatus(this)
            else
                setBadPortStatus(this, [iin, iout], "the same format was exoected")
        }
        else
            setUnspecPortStatus(this, [iin, iout])
    }
    else {
        if (f1.valid()) {
            setFormat(port1, {})
            setFormat(port2, f1)
        }
        else {
            setFormat(port1, f2)
            setFormat(port2, {})
        }
        setGoodStatus(this)
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

function setCxxOdeDefaultStatus() {
    this.status = { level: "warning", text: "No source code is specified, see 'src' property" }
}

function restoreCxxOdeDefaultPortFormat() {
    // Restore default port info & formats
    for (var i=0; i<this.ports.length; ++i) {
        var port = this.ports[i]
        port.info = port.info0
        setFormat(port, port.info)
    }
}

function checkRk4Ports(box)
{
    var f = []
    for (var i=0; i<7; ++i) f[i] = box.ports[i].getFormat()
    var criticalPorts = [2,5]
    for (i=0; i<criticalPorts.length; ++i) {
        var n = criticalPorts[i], fn = f[n]
        if (fn.format === undefined) {
            setUnspecPortStatus(box, n)
            return
        }
        if (fn.format.length != 1) {
            setBadPortStatus(box, n, "expected 1D port data")
            return
        }
    }
    if (f[2].format[0]+1 != f[5].format[0]) {
        box.status = { level: "error", text: "Incompatible formats of ports 'rhs' and 'rhsState'" }
        return
    }
    setGoodStatus(box)
}

$.extend(equaresBox.rules, {
    Param: {
        init: function() {
            this.props["data"].toBoxProp = function(prop) {
                return port2value(prop.value, this.ports[0])
            }
            setUnspecPortStatus(this)
        },
        port: function(port) {
            var t = this.props["data"].userType = port2type(port)
            ;(t === undefined ? setUnspecPortStatus : setGoodStatus)(this)
            this.prop("data", defaultValue(t))
            this.stateChanged("propset")
        }
    },
    CrossSection: {
        init: function() { setUnspecPortStatus(this, [0, 1]) },
        port: function(port) { propagateFormat.call(this, port, 0, 1) }
    },
    CountedFilter: {
        init: function() { setUnspecPortStatus(this, [1, 2]) },
        port: function(port) { propagateFormat.call(this, port, 1, 2) }
    },
    IntervalFilter: {
        init: function() { setUnspecPortStatus(this, [0, 1]) },
        port: function(port) { propagateFormat.call(this, port, 0, 1) }
    },
    Valve: {
        init: function() { setUnspecPortStatus(this, [1, 2]) },
        port: function(port) { propagateFormat.call(this, port, 1, 2) }
    },
    Projection: {
        init: projectionOutFormat,
        prop: function(name) {
            if (name === "indices") {
                projectionOutFormat.call(this)
                this.editor.update()
            }
        },
        port: function(port) {
            if (port.info.name === "input") {
                projectionOutFormat.call(this)
                this.editor.update()
            }
        }
    },
    Rk4: {
        init: function() { checkRk4Ports(this) },
        port: function(port) {
            propagateFormatDirected.call(this, port, 5, [1,4])
            checkRk4Ports(this)
        }
    },
    Canvas: {
        init: function() { setUnspecPortStatus(this) },
        prop: function(name) {
            if (name === "param") {
                var v = this.prop(name)
                setFormat(this.ports[2], {format: [v.x.resolution, v.y.resolution]})
            }
        },
        port: function(port) {
            if (port.info.name === "input") {
                var f = port.getFormat()
                if (f.valid()) {
                    if (f.format.length == 1) {
                        if (f.format[0] == 2)
                            setGoodStatus(this)
                        else
                            setBadPortStatus(this, 0, "expected 1D vector of 2 elements")
                    }
                    else
                        setBadPortStatus(this, 0, "expected 1D port data")
                }
                else
                    setUnspecPortStatus(this)
            }
        }
    },
    Bitmap: {
        init: function() { setUnspecPortStatus(this) },
        port: function(port) {
            var f = port.getFormat()
            if (f.valid()) {
                if (f.format.length == 2)
                    setGoodStatus(this)
                else
                    setBadPortStatus(this, 0, "expected 2D port data")
            }
            else
                setUnspecPortStatus(this)
        }
    },
    CxxOde: {
        init: function() {
            // Backup port info
            for (var i=0; i<this.ports.length; ++i) {
                var port = this.ports[i]
                port.info0 = port.info
            }
            setCxxOdeDefaultStatus.call(this)
        },

        prop: function(name) {
            var box = this
            if (name === "src") {
                if (box.prop(name).length == 0) {
                    setCxxOdeDefaultStatus.call(box)
                    restoreCxxOdeDefaultPortFormat.call(box)
                    box.editor.update()
                    box.emit('critical')
                    return
                }
                box.status = { level: "waiting", text: "Compiling..." }
                box.editor.update()
                equaresBox.infoEx({
                    options: "ports",
                    type: "CxxOde",
                    props: { src: box.prop(name) }
                }, function(info) {
                    // Update port format
                    box.info.inputs = info.inputs
                    box.info.outputs = info.outputs
                    var i=0
                    function upd(ports) {
                        for (var j=0; j<ports.length; ++j, ++i) {
                            box.ports[i].info = ports[j]
                            setFormat(box.ports[i], ports[j])
                        }
                    }
                    upd(info.inputs)
                    upd(info.outputs)
                    setGoodStatus(box)
                    box.editor.update()
                    box.emit('critical')
                }, function(reply) {
                    restoreCxxOdeDefaultPortFormat.call(box)
                    box.status = { level: "error", text: reply.stderr || reply.message }
                    box.editor.update()
                    box.emit('critical')
                })
            }
        }
    },
    JsOde: {
        prop: function(name) {
            var box = this
            if (name === "ode") {
                // TODO
                box.emit('critical')
            }
        }
    }
})
})()
