var equaresBox = {}

(function(){

    var Port = equaresBox.Port = function (type, info, box) {
        if (arguments.length == 0)
            return
        this.type = type
        this.info = info
        this.box = box
        this.format=[]
        if (info.format instanceof Array)
            $.merge(this.format, info.format)
        this.pos = info.pos
    }
    var InputPort = equaresBox.InputPort = function() {
        Port.apply(this, arguments)
        this.connection = null
    }
    InputPort.prototype = new Port()
    InputPort.prototype.canConnect = function(thatPort) {
        if (this.connection != null)
            return false
        return arguments.length == 0 ?   true :
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

    var OutputPort = equaresBox.OutputPort = function() {
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
            (thatPort instanceof InputPort)   &&   this.findConnection(port) == -1
    }
    OutputPort.prototype.connect = function(thatPort) {
        this.connections.push(thatPort)
        this.box.stateChanged()
    }
    OutputPort.prototype.disconnect = function(thatPort) {
        this.connections.splice(this.findConnection(thatPort), 1)
        this.box.stateChanged()
    }

    var Box = equaresBox.Box = function(name, type, info) {
        if (arguments.length == 0)
            return
        this.name = name
        this.type = type
        this.info = info
        this.ports = []
        function addPorts(box, ports, type) {
            for (var i=0, n=ports.length; i<n; ++i)
                box.ports.push(new Port(type, ports[i], {box: box}))
        }
        addPorts(this, info.inputs, "in")
        addPorts(this, info.outputs, "out")
        $.extend(this.props={}, info.properties)
        // TODO: default value
    }
    Box.connect = function(thisPort, thatPort) {
        thisPort.box.connect(thatPort)
        thatPort.box.connect(thisPort)
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
})()
