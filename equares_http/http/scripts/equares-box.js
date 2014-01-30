var equaresBox = {};

(function(){

    var Port = equaresBox.Port = function (info, box) {
        if (arguments.length == 0)
            return
        this.info = info
        this.box = box
        this.format=[]
        if (info.format instanceof Array)
            $.merge(this.format, info.format)
        this.pos = info.pos
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
        this.box.stateChanged()
    }
    InputPort.prototype.disconnect = function(thatPort) {
        this.connection = null
        this.box.stateChanged()
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
        this.box.stateChanged()
    }
    OutputPort.prototype.disconnect = function(thatPort) {
        this.connections.splice(this.findConnection(thatPort), 1)
        this.box.stateChanged()
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
                box.ports.push(new PortCtor(ports[i], box))
        }
        addPorts(this, info.inputs, InputPort)
        addPorts(this, info.outputs, OutputPort)
        this.props = []
        for(var i in info.properties) {
            var pi = info.properties[i]
            var p = this.props[i] = {}
            p.name = pi.name
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
    }
    Box.prototype.prop = function(name, value) {
        if (arguments.length == 2) {
            this.props[name].value = value
            this.stateChanged()
            return this
        }
        else
            return this.props[name].value
    }
    Box.prototype.stateChanged = function() {}
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
    equaresBox.connect = function(port1, port2) {
        port1.connect(port2)
        port2.connect(port1)
    }
    equaresBox.disconnect = function(port1, port2) {
        port1.disconnect(port2)
        port2.disconnect(port1)
    }
})()
