/*
EquaRes - a tool for numerical analysis of equations.
Copyright (C) 2014 Stepan Orlov

This file is part of EquaRes.

Full text of copyright notice can be found in file copyright_notice.txt in the EquaRes root directory.
License agreement can be found in file LICENSE.md in the EquaRes root directory.
*/

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
    PortFormat.prototype.dataSize = function() {
        var result = 0
        for (var i=0; i<this.format.length; ++i)
            result += this.format[i]
        return result
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
        var snippetProp, multipleSnippetProp
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
            if (pi.snippet) {
                p.snippet = pi.snippet
                if (snippetProp) {
                    snippetProp = undefined
                    multipleSnippetProp = true
                }
                else
                    snippetProp = pi.name
            }
        }
        if (snippetProp)
            this.snippetProp = snippetProp
        this.callbacks = []
        this.status = { level: "ok", text: "Ok" }
        this.stateChanged("init")
    }
    Box.prototype = new EventEmitter()
    Box.prototype.prop = function(name, value) {
        if (arguments.length == 2) {
            this.props[name].value = value
            this.stateChanged("prop", name)
            this.editor.modify()
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
    Box.prototype.resizeInputPorts = function(nin, options) {
        var box = this
        options = options || {}
        if (!(options.portName instanceof Function))
            options.portName = function(info, idx) { return "in_" + (idx+1) }
        if (!(options.portHelp instanceof Function))
            options.portHelp = function(info, idx) { return info.help.match(/^(.*)\d+$/)[1] + (idx+1) }
        box.cache = box.cache || {}
        if (!box.cache.infoIn)
            box.cache.infoIn = box.info.inputs[0]
        var nin0 = box.info.inputs.length, nout = box.info.outputs.length
        if (box.info.inputs.length === nin)
            // It's likely the case when we are loading previously saved box
            return
        function positionPorts() {
            for (var i=0; i<nin; ++i)
                box.info.inputs[i].pos = box.ports[i].pos = 2 - (i+0.5)/nin
        }
        var d = nin - nin0, i
        if (d < 0) {
            var deadLinks = []
            for (i=nin; i<nin0; ++i) {
                var port = box.ports[i], pc = port.connection
                if (pc)
                    deadLinks.push(box.editor.findLink(port, pc))
            }
            for (i=0; i<deadLinks.length; ++i)
                box.editor.deleteLink(deadLinks[i])
            box.info.inputs.splice(nin, -d)
            box.ports.splice(nin, -d)
            positionPorts()
        }
        else if (d > 0) {
            var info0 = box.cache.infoIn
            for (i=0; i<d; ++i) {
                var n = nin0 + i,
                    info = {
                        help: options.portHelp(info0, n),
                        name: options.portName(info0, n),
                        pos: 0
                    }
                box.info.inputs.push(info)
                box.ports.splice(nin0+i, 0, new InputPort(info, box, n))
            }
            positionPorts()
        }
        box.redraw()
    }
    Box.prototype.resizeOutputPorts = function(nout, options) {
        var box = this
        options = options || {}
        if (!(options.portName instanceof Function))
            options.portName = function(info, idx) { return "out_" + (idx+1) }
        if (!(options.portHelp instanceof Function))
            options.portHelp = function(info, idx) { return info.help.match(/^(.*)\d+$/)[1] + (idx+1) }
        box.cache = box.cache || {}
        if (!box.cache.infoOut)
            box.cache.infoOut = box.info.outputs[0]
        var nout0 = box.info.outputs.length, nin = box.info.inputs.length
        if (nout0 === nout)
            // It's probably the case when we are loading previously saved box
            return
        function positionPorts() {
            for (var i=0; i<nout; ++i)
                box.info.outputs[i].pos = box.ports[nin+i].pos = 3 + (i+0.5)/nout
        }
        var d = nout - nout0, i, j
        if (d < 0) {
            var deadLinks = []
            for (i=nout; i<nout0; ++i) {
                var port = box.ports[nin+i], pc = port.connections
                for (j=0; j<pc.length; ++j)
                    deadLinks.push(box.editor.findLink(port, pc[j]))
            }
            for (i=0; i<deadLinks.length; ++i)
                box.editor.deleteLink(deadLinks[i])
            box.info.outputs.splice(nout, -d)
            box.ports.splice(nin+nout, -d)
            positionPorts()
        }
        else if (d > 0) {
            var info0 = box.cache.infoOut
            for (i=0; i<d; ++i) {
                var n = nout0 + i
                var info = {
                    help: options.portHelp(info0, n),
                    name: options.portName(info0, n),
                    pos: 0
                }
                box.info.outputs.push(info)
                box.ports.push(new OutputPort(info, box, nin + n))
            }
            positionPorts()
        }
        box.redraw()
    }
    Box.snippetDoc = function(value, propName) {
        if (typeof value != 'string')
            return ''
        var v = value.split('\n'), inDoc = false
        var doc = []
        for (var i=0; i<v.length; ++i) {
            var line = v[i]
            if (!inDoc && line === '/*#') {
                inDoc = true
                continue
            }
            if (inDoc) {
                if (line === '*/') {
                    inDoc = false
                    continue
                }
                if (line.match(/^\s*(keywords|title):/))
                    continue
                doc.push(line)
            }
        }
        if (doc.length < 1   &&   propName)
            return 'Advice: Add description for property ```' + propName + '``` by adding ?[markdown text](http://daringfireball.net/projects/markdown/)' +
                    ' (you can also use ?[gfm](https://help.github.com/articles/github-flavored-markdown) and ?[TeX](http://tug.org/) formulas) as follows:<br/>' +
                    '<pre>```/*#```\n' +
                    '```title:``` Snippet title\n' +
                    '```keywords:``` Snippet keywords\n' +
                    '```#``` Snippet title\n' +
                    'Snippet description\n' +
                    '...\n' +
                    '```*/```\n' +
                    'Rest of property text' +
                    '</pre>\n' +
                    'See also ?[simulation info property](/doc#page/editor-usage-text-info)'
        return doc.join('\n')
    }
    Box.prototype.snippetDoc = function(propName) {
        if (arguments.length < 1)
            propName = this.snippetProp
        if (!(typeof propName == 'string' && propName in this.props))
            return
        if (!this.props[propName].snippet)
            return
        return Box.snippetDoc(this.prop(propName), propName)
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
        $.get("cmd/requestInfo", {cmd: request})
            .done(function(data) {
                var reply = JSON.parse(data);
                if (reply.error)
                    errorMessage("requestInfo error: \n" + reply.stderr);
                else
                    callback(eval("(function(){return " + reply.stdout + "})()"));
            })
            .fail(function() {
                // equaresDebug.html("requestInfo: Ajax error");
                errorMessage("requestInfo: Ajax error");
            });
    }

    equaresBox.infoEx = function(request, callback, errorCallback) {
        $.post("cmd/requestInfoEx", request)
            .done(function(data) {
                var reply = JSON.parse(data);
                if (reply.error) {
                    if (errorCallback instanceof Function)
                        errorCallback(reply)
                    else
                        errorMessage("requestInfoEx error: \n" + (reply.stderr || reply.message));
                }
                else
                    callback(eval("(function(){return " + reply + "})()"));
            })
            .fail(function() {
                if (errorCallback instanceof Function)
                    errorCallback("Ajax error")
                else
                    errorMessage("requestInfoEx: Ajax error");
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
                setBadPortStatus(this, [iin, iout], "the same format was expected")
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

function propagateSameFormat() {
    var box = this
    function clearFormat() {
        for (var i=0; i<n; ++i)
            setFormat(box.ports[i], {})
    }
    // Find common port format
    var f = new PortFormat({}), i, n = box.ports.length, fp, allValid = true, port
    for (i=0; i<n; ++i) {
        fp = box.ports[i].getFormat(true)
        if (fp.valid()) {
            if (f.valid()) {
                if (!fp.equals(f)) {
                    // Incompatible formats, give up
                    clearFormat()
                    setBadPortStatus(box, 'Some ports have different formats')
                    return
                }
            }
            else
                f = fp
        }
        else
            allValid = false
    }

    if (allValid)
        return  // Nothing to do
    if (!f.valid()) {
        clearFormat()
        setUnspecPortStatus(box, 'Unknown port format')
        return
    }

    // Propagate common port format
    for (i=0; i<n; ++i) {
        port = box.ports[i]
        if (!port.getFormat(true).valid())
            setFormat(port, f)
    }
    setGoodStatus(box)
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

function checkFdeIteratorPorts(box)
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
    if (f[2].format[0] != f[5].format[0]) {
        box.status = { level: "error", text: "Incompatible formats of ports 'fdeOut' and 'fdeIn'" }
        return
    }
    setGoodStatus(box)
}

function odeOrFdeRules(type) {
    return {
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
                    type: type,
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
    }
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
    Const: {
        init: function() {
            // TODO better (equares_con -d Const returns incorrect default value for data property)
            this.info.properties[0].defaultValue = []
            this.props.data.value = []
        },
        prop: function(name) {
            if (name === "data") {
                setFormat(this.ports[0], {format: [this.prop("data").length]})
            }
        }
    },
    ParamArray: {
        init: function() {
            this.props["data"].toBoxProp = function(prop) {
                var result = [], port = this.ports[this.info.inputs.length]
                for (var i=0, n=prop.value.length; i<n; ++i)
                    result = result.concat(port2value(prop.value[i], port))
                return result
            }
            setUnspecPortStatus(this)
        },
        prop: function(name) {
            var box = this
            if (name === "withActivator") {
                var nin = box.props.withActivator.value? 1: 0
                box.resizeInputPorts(nin, {
                    portName: function() { return 'activator' },
                    portHelp: function() { return 'Activation port' }
                })
                box.emit('critical')
            }
        },
        port: function(port) {
            if (port.info.name == 'output') {
                var t = port2type(port)
                this.props["data"].userType = [t]
                ;(t === undefined ? setUnspecPortStatus : setGoodStatus)(this)
                this.prop("data", [])
                this.stateChanged("propset")
            }
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
    Interpolator: {
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
    FdeIterator: {
        init: function() { checkFdeIteratorPorts(this) },
        port: function(port) {
            propagateFormatDirected.call(this, port, 5, [1,4])
            checkFdeIteratorPorts(this)
        }
    },

    Canvas: {
        init: function() { setUnspecPortStatus(this) },
        prop: function(name) {
            var v = this.prop(name)
            switch(name) {
            case "param":
                setFormat(this.ports[4], {format: [v.x.resolution, v.y.resolution]})
                break
            case "withInputValue":
                setFormat(this.ports[0], {format: [v? 3: 2]})
                break
            }
        },
        port: function(port) {
            if (port.info.name === "input") {
                var f = port.getFormat()
                var nin = this.prop("withInputValue") ?   3:   2
                if (f.valid()) {
                    if (f.format.length == 1) {
                        if (f.format[0] == nin)
                            setGoodStatus(this)
                        else
                            setBadPortStatus(this, 0, 'expected 1D vector of ' + nin + ' elements')
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
    CxxOde: odeOrFdeRules("CxxOde"),
    CxxFde: odeOrFdeRules("CxxFde"),
    JsOde: {
        prop: function(name) {
            var box = this
            if (name === "ode") {
                // TODO
                box.emit('critical')
            }
        }
    },
    GridGenerator: {
        init: function() { setUnspecPortStatus(this, [0, 2]) },
        port: function(port) { propagateFormat.call(this, port, 0, 2) }
    },
    LinOdeStabChecker: {
        port: function(port) {
            propagateFormat.call(this, port, 1, 2)
        }
    },
    PointInput: {
        port: function(port) {
            propagateFormat.call(this, port, 1, 3)
        }
    },
    SimpleInput: {
        port: function(port) {
            propagateFormat.call(this, port, 1, 2)
        }
    },
    RangeInput: {
        port: function(port) {
            propagateFormat.call(this, port, 1, 2)
        }
    },
    Split: {
        init: function() {
            var portIds = [0]
            for (var i=0; i<this.info.outputs.length; ++i)
                portIds.push(i+1)
            setUnspecPortStatus(this, portIds)
        },
        prop: function(name) {
            var box = this
            if (name === "outputPortCount") {
                var MaxOutputPorts = 10
                var nout = Math.floor(+box.props[name].value), nout0 = box.info.outputs.length
                if (!(nout > 0 && nout <= MaxOutputPorts)) {
                    box.props[name].value = nout0
                    return errorMessage('Invalid number of output ports')
                }
                box.resizeOutputPorts(nout)
                box.emit('critical')
            }
        },
        port: function(port) {
            propagateSameFormat.call(this)
        }
    },
    Merge: {
        init: function() {
            var portIds = []
            for (var i=0; i<=this.info.inputs.length; ++i)
                portIds.push(i)
            setUnspecPortStatus(this, portIds)
        },
        prop: function(name) {
            var box = this
            if (name === "inputPortCount") {
                var MaxInputPorts = 10
                var nin = Math.floor(+box.props[name].value), nin0 = box.info.inputs.length
                if (!(nin > 0 && nin <= MaxInputPorts)) {
                    box.props[name].value = nin0
                    return errorMessage('Invalid number of input ports')
                }
                box.resizeInputPorts(nin)
                box.emit('critical')
            }
        },
        port: function(port) {
            propagateSameFormat.call(this)
        }
    },
    Replicator: {
        init: function() {
            setUnspecPortStatus(this)
        },
        port: function(port) {
            propagateFormat.call(this, port, 0, 2)
            propagateFormat.call(this, port, 1, 3)
        }
    },
    Join: {
        init: function() {
            setUnspecPortStatus(this)
        },
        port: function(port) {
            var fi = [this.ports[0].getFormat(true), this.ports[1].getFormat(true)],
                out = this.ports[2],
                fo = out.getFormat(true),
                i, dataSize = 0, hints = []
            for (i=0; i<2; ++i) {
                if (!fi[i].valid())
                    return setUnspecPortStatus(this, i)
                if (fi[i].format.length != 1)
                    return setBadPortStatus(this, i, '1D data was expected')
                dataSize += fi[i].format[0]
                if (hints)
                    hints = fi[i].hints? hints.concat(fi[i].hints): undefined
            }
            if (fo.valid()) {
                setFormat(out, {})
                if (fo.format.length != 1)
                    return setBadPortStatus(this, 2, '1D data was expected')
                if (fo.format[0] != dataSize)
                    return setBadPortStatus(this, 2, 'Invalid data size, must be ' + dataSize)
            }
            else
                setFormat(out, {format: [dataSize], hints: hints})
            setGoodStatus(this)
        }
    },
    Scalarize: {
        init: function() {
            setUnspecPortStatus(this, 0)
        },
        port: function(port) {
            var fi = this.ports[0].getFormat(true),
                fo = this.ports[1].getFormat(true)
            if (fi.valid() && fi.dataSize() < 1)
                return setBadPortStatus(this, 0, 'Data size must be positive')
            if (fo.valid() && !(fo.format.length == 1 && fo.format[0] == 1))
                return setBadPortStatus(this, 1, 'Must be scalar')
            if (!fi.valid())
                return setUnspecPortStatus(this, 0)
            setGoodStatus(this)
        }
    },
    ThresholdDetector: {
        port: function(port) {
            var f = port.getFormat(true)
            if (f.valid() && !(f.format.length == 1 && f.format[0] == 1))
                return setBadPortStatus(this, port.index, 'Must be scalar')
            setGoodStatus(this)
        }
    },
    Differentiate: {
        init: function() {
            setUnspecPortStatus(this, [0,1])
        },
        port: function(port) {
            propagateFormat.call(this, port, 0, 1)
        }
    },
    RectInput: {
        prop: function(name) {
            var box = this
            if (name === "withActivator") {
                var nin = box.props.withActivator.value? 1: 0
                box.resizeInputPorts(nin, {
                    portName: function() { return 'activator' },
                    portHelp: function() { return 'Activation port' }
                })
                box.emit('critical')
            }
        }
    },
    CxxTransform: odeOrFdeRules("CxxTransform"),
    Eigenvalues: {
        init: function() {
            setUnspecPortStatus(this, [0,1,2])
        },
        port: function(port) {
            // Propagate port format
            var fm = this.ports[0].getFormat(true),
                fr = this.ports[1].getFormat(true),
                fi = this.ports[2].getFormat(true),
                bm = fm.valid(),
                br = fr.valid(),
                bi = fi.valid(),
                allValid = bm && br && bi,
                allInvalid = !(bm || br || bi)
            if (!(allValid || allInvalid)) {
                if (bm && fm.format.length == 2 && fm.format[0] == fm.format[1]) {
                    var fe = {format: [fm.format[0]]}
                    if (!br) {
                        setFormat(this.ports[1], fe)
                        fr = this.ports[1].getFormat(false)
                        br = true
                    }
                    if (!bi) {
                        setFormat(this.ports[2], fe)
                        fi = this.ports[2].getFormat(false)
                        bi = true
                    }
                }
                if (!bm) {
                    var n = -1;
                    if (br && fr.format.length == 1)
                        n = fr.format[0]
                    else if (bi && fi.format.length == 1)
                        n = fi.format[0]
                    if (n != -1) {
                        setFormat(this.ports[0], {format: [n,n]})
                        fm = this.ports[0].getFormat(false)
                        bm = true
                    }
                }
                if (br != bi) {
                    if (br) {
                        setFormat(this.ports[2], fr)
                        fi = this.ports[2].getFormat(false)
                        br = true
                    }
                    if (bi) {
                        setFormat(this.ports[1], fi)
                        fr = this.ports[1].getFormat(false)
                        bi = true
                    }
                }
            }
            // Check port format
            var unspec = [];
            if (!bm)   unspec.push(0)
            if (!br)   unspec.push(1)
            if (!bi)   unspec.push(2)
            if (unspec.length > 0)
                return setUnspecPortStatus(this, unspec)
            if (fm.format.length != 2   ||   fm.format[0] < 1   ||   fm.format[0] != fm.format[1])
                return setBadPortStatus(this, 0, 'Must be 2D square nonempty matrix')
            if (fr.format.length != 1   ||   fr.format[0] != fm.format[0])
                return setBadPortStatus(this, 1, 'Must be 1D data matching matrix size')
            if (fi.format.length != 1   ||   fi.format[0] != fm.format[0])
                return setBadPortStatus(this, 1, 'Must be 1D data matching matrix size')
            setGoodStatus(this)
        }
    }
})
})()
