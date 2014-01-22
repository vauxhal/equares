var ctmEquaresSchemeEditor = {};

(function(E) {
    var nameBox;    // Generates default box name
    (function(){
        var boxNames = {}   // Set of used box names
        var counters = {}   // Key = box type, value = index of last generated instance name
        nameBox = function(boxType) {
            var i = counters[boxType]
            if (i == undefined)
                i = 0;
            for (++i; ; ++i) {
                var name = boxType + " " + i
                if (!boxNames[name]) {
                    boxNames[name] = true
                    counters[boxType] = i
                    return name
                }
            }
        }
    })()

    // Some helper functions

    // Creates new SVG element
    function newSvg(tag) { return document.createElementNS(d3.ns.prefix.svg, tag) }

    // Synonim for d3.select
    function d$() { return d3.select.apply(this, arguments) }

    // Constructor for type used as the D3 data for a box port
    function BoxPortData(portType, portInfo, options) {
        this.type = portType
        this.info = portInfo
        var opt = options || {}
        this.parent = opt.box || null
        this.phi = opt.phi || 0
    }

    // Constructor for type used as the D3 data for a box
    function BoxData(boxType, boxInfo, options) {
        this.type = boxType
        this.info = boxInfo
        var opt = options || {}
        this.name = opt.name || nameBox(boxType)
        var pos = opt.pos || {}
        this.x = pos.x || 0
        this.y = pos.y || 0
        this.ports = []
        function addPorts(box, ports, type, phi0) {
            var n = ports.length;
            if (n == 0)
                return;
            var dphi = Math.PI / n,   phi = phi0 + 0.5*dphi
            for (var i=0; i<n; ++i, phi+=dphi)
                box.ports.push(new BoxPortData(type, ports[i], {box: box, phi: phi}))
        }
        // Note: the y direction is down!
        addPorts(this, boxInfo.inputs, "in", 0)
        addPorts(this, boxInfo.outputs, "out", Math.PI)
        this.iid = opt.iid || 0
        this.editor = opt.editor || null
        this.index = typeof(opt.index) == "number"? opt.index: -1
    }

    function Editor(root) {
        // Array of box instances
        this.boxes = []

        // Array of links between box ports
        this.links = []

        this.root = root

        var width = $(root).width(),
            height = $(root).height()

        // SVG element serving as the container for the scheme
        this.svg = d$(root).append("svg")
            .attr("class", "svg-main")
            .attr("width", width)
            .attr("height", height)

        // Main rectangle occupying all available space
        this.mainrect = this.svg.append("rect")
            .attr("class", "mainrect")
            .attr("id", "mainrect")
            .attr("width", width)
            .attr("height", height)

        // line displayed when dragging new nodes
        this.drag_line = this.svg.append("line")
            .attr("class", "drag_line")
            .attr("id", "drag_line")
            .attr("x1", 0)
            .attr("y1", 0)
            .attr("x2", 0)
            .attr("y2", 0)

        // Unique box identifier
        this.iid = 0
    }
    Editor.prototype.newBox = function(boxType, boxInfo, options) {
        var opt = options || {}

        if (opt.offset) {
            var rootOffset = $(this.root).offset()
            opt.pos = {
                x: opt.offset.left - rootOffset.left,
                y: opt.offset.top - rootOffset.top
            }
        }
        opt.iid = ++this.iid
        opt.editor = this
        opt.index = this.boxes.length
        this.boxes.push(new BoxData(boxType, boxInfo, options))
        this.visualize()
    }
    Editor.prototype.dataKey = function(d) {
        return "k" + d.iid;
    }
    Editor.prototype.visualize = function() {
        var boxSel = this.svg.selectAll(".scheme-box")
        var boxUpd = boxSel.data(this.boxes, this.dataKey)

        // Create group for incoming box
        var boxGroup = boxUpd.enter().insert("g", "#drag_line")
            .attr("class", "scheme-box");

        // Stuff box group with content
        boxGroup.each(function(d) {
            var g = d$(this)

            // Define box position
            g.attr("transform", "translate(" + d.x + "," + d.y + ")")

            // Add text, rectangle, and image acting as the close buttion
            var hmargin = 8, vmargin = 8, sep = 4, wclose = 8
            var bbox = g.append("text")
                .text(d.name)
                .attr("x", hmargin)
                .attr("y", vmargin)
                .node().getBBox()
            g.insert("rect", "text")
                .attr("class", "scheme-box-shape")
                .attr("width", bbox.width + sep + wclose + 2*hmargin)
                .attr("height", bbox.height + 2*vmargin)
                .attr("rx", 5)
                .attr("ry", 5)
            g.append("image")
                .attr("xlink:href", "close.png")
                .attr("x", hmargin + bbox.width + sep)
                .attr("y", vmargin)
                .attr("width", wclose)
                .attr("height", wclose)
                .on("click", function(d, i) {
                    var boxes = d.editor.boxes
                    boxes.splice(d.index, 1)
                    for (var j=d.index; j<boxes.length; ++j)
                        boxes[j].index = j;
// TODO: Remove dead links
//                    for (var il=links.length-1; il>=0; --il) {
//                        var l = links[il]
//                        if (l.source === d   ||   l.target === d)
//                            links.splice(il, 1);
//                    }
                    d.editor.visualize();
                })

            // Add ports
            var portSel = g.selectAll(".scheme-port")
            var portUpd = portSel.data(d.ports)
            var center = {
                x: 0.5*(bbox.width + sep + wclose) + hmargin,
                y: 0.5*bbox.height + vmargin
            }
            var t0 = center.y / center.x
            var eps = 1e-5
            portUpd.enter().append("circle")
                .attr("class", "scheme-port")
                .classed("scheme-inport", function(d) {
                   return d.type === "in"
                 })
                .classed("scheme-outport", function(d) {
                  return d.type === "out"
                 })
                .attr("r", 5)
//                .on("mousedown", mousedownAtPort)
//                .on("mouseup", mouseupAtPort)
                .on("mouseover", equaresui.enterPort)
                .on("mouseout", equaresui.leavePort)
                .attr("cx", function(d, i) {
                    if (Math.abs(d.phi-0.5*Math.PI) < eps || Math.abs(d.phi-1.5*Math.PI) < eps)
                        return center.x
                    var t = Math.tan(d.phi)
                    if (Math.abs(t) <= t0)
                        return center.x + (d.phi < 0.5*Math.PI || d.phi > 1.5*Math.PI ? 1: -1) * center.x
                    else
                        return center.x + (d.phi < Math.PI ? 1: -1) * center.y/t
                })
                .attr("cy", function(d) {
                    if (Math.abs(d.phi-0.5*Math.PI) < eps)
                        return 0
                    if (Math.abs(d.phi-1.5*Math.PI) < eps)
                        return 2*center.y;
                    var t = Math.tan(d.phi)
                    if (Math.abs(t) <= t0)
                        return center.y + (d.phi < 0.5*Math.PI || d.phi > 1.5*Math.PI ? -1: 1) * center.x*t
                    else
                        return center.y + (d.phi < Math.PI? -1: 1) * center.y
                });
        })

        boxUpd.exit()
            .transition()
            .duration(350)
            .style("opacity", 1e-6)
            .remove();
    }

    E.newEditor = function(root) {
        return new Editor(root)
    }

})(ctmEquaresSchemeEditor)
