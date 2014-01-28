var ctmEquaresSchemeEditor = {};

(function(E) {
    var nameBox             // Generates default box name
    var unuseBoxName
    var boxNameExists
    var renameBox
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
        unuseBoxName = function(name) {
            delete boxNames[name]
        }
        boxNameExists = function(name) {
            return boxNames[name] === true
        }
        renameBox = function(box, newName) {
            delete boxNames[box.name]
            boxNames[box.name = newName] = true
        }
    })()

    // Some helper functions

    // Creates new SVG element
    // function newSvg(tag) { return document.createElementNS(d3.ns.prefix.svg, tag) }

    // Synonim for d3.select
    function d$() { return d3.select.apply(this, arguments) }

    // Constructors for types used as the D3 data for box ports
    var Port = equaresBox.Port,
        InputPort = equaresBox.InputPort,
        OutputPort = equaresBox.OutputPort

    // Constructor for type used as the D3 data for a box
    function Box(boxType, boxInfo, options) {
        var opt = options || {}
        var name = opt.name || nameBox(boxType)
        equaresBox.Box.call(this, name, boxType, boxInfo)
        var pos = opt.pos || {}
        this.x = pos.x || 0
        this.y = pos.y || 0
        this.iid = opt.iid || 0
        this.editor = opt.editor || null
        this.index = typeof(opt.index) == "number"? opt.index: -1
        this.selected = false
    }
    Box.prototype = new equaresBox.Box()

    var clickTimer = {};
    (function() {
    var mouseDownTime
        clickTimer.start = function() {
            mouseDownTime = (new Date()).valueOf()
        }
        clickTimer.clicked = function() {
            return (new Date()).valueOf() - mouseDownTime < 350
        }
    })()

    var DragHelper
    (function() {
        DragHelper = function (editor) {
            // Store reference to the editor
            this.editor = editor

            // line displayed when dragging new nodes
            this.dragLine = editor.maingroup.append("line")
                .attr("class", "scheme-drag-line-hidden")
                .attr("id", "scheme-drag-line")
            var thisDragHelper = this
            editor.svg
                .on("mousemove", function() { thisDragHelper.onMouseMove(editor.maingroup.node()) })
                .on("mouseup", function() { thisDragHelper.onMouseUpMainRect(editor.maingroup.node()) })

        }
        var currentElement = null
        var prevCursorPos = null
        var draggingLine = false
        var point
        (function() {
            var ptcache = null
            point = function(dragHelper) {
                if (!ptcache)
                    ptcache = dragHelper.editor.svg.node().createSVGPoint();
                return ptcache
            }
        })()
        DragHelper.prototype.elpos = function(el, ax, ay) {
            var m = el.getCTM()
            var pt = point(this)
            var elsel = d$(el)
            pt.x = elsel.attr(ax)
            pt.y = elsel.attr(ay)
            pt = pt.matrixTransform(m)
            return pt
        }

        DragHelper.prototype.beginDragPort = function(element, port, index) {
            d3.event.preventDefault()
            if (!port.canConnect())
                return
            currentElement = element
            var pt = this.elpos(element, "cx", "cy")
            var t = this.editor.maingroup.myTransform
            pt.x -= t.x
            pt.y -= t.y
            var mp = d3.mouse(this.editor.maingroup.node())
            this.dragLine
                .attr("class", "scheme-drag-line")
                .attr("x1", pt.x)
                .attr("y1", pt.y)
                .attr("x2", mp[0])
                .attr("y2", mp[1])
            draggingLine = true
            prevCursorPos = null
        }
        DragHelper.prototype.beginDragBox = function(element, box, index) {
            currentElement = element
            prevCursorPos = d3.mouse(this.editor.maingroup.node())
            d3.event.preventDefault()
        }
        DragHelper.prototype.beginPan = function() {
            prevCursorPos = d3.mouse(this.editor.svg.node())
            currentElement = null
            clickTimer.start()
            d3.event.preventDefault()
        }
        DragHelper.prototype.stopDragging = function() {
            if (draggingLine) {
                this.dragLine.attr("class", "scheme-drag-line-hidden")
                draggingLine = false
            }
            currentElement = null
            prevCursorPos = null
        }
        DragHelper.prototype.endDragPort = function(element, port, index) {
            if (!draggingLine   ||   !(currentElement.__data__ instanceof Port))
                return
            // Add new link if it is possible
            var p1 = currentElement.__data__,   p2 = port
            if (!p1.canConnect(p2))
                return
            this.editor.newLink(p1, p2)
        }
        DragHelper.prototype.onMouseUpMainRect = function(el) {
            if (!currentElement && clickTimer.clicked()) {
                // Clear box selection
                for(var i in this.editor.boxes)
                    this.editor.boxes[i].selected = false
                this.editor.update()
                equaresui.selectBox(null)
            }
            this.stopDragging()
        }

        DragHelper.prototype.onMouseMove = function(el) {
            if (draggingLine) {
                // update drag line
                var mp = d3.mouse(el)
                this.dragLine
                    .attr("x2", mp[0])
                    .attr("y2", mp[1])
            }
            if (prevCursorPos) {
                var cursorPos = d3.mouse(currentElement? this.editor.maingroup.node(): this.editor.svg.node());
                var dx = cursorPos[0] - prevCursorPos[0],
                    dy = cursorPos[1] - prevCursorPos[1]
                if (currentElement) {
                    var d = currentElement.__data__
                    d.x += dx
                    d.y += dy
                    this.editor.update()
                }
                else {
                    var t = this.editor.maingroup.myTransform
                    t.x += dx
                    t.y += dy
                    this.editor.maingroup.attr("transform", "translate(" + t.x + "," + t.y + ")")
                }
                prevCursorPos = cursorPos
            }
        }
    })()

    function Editor(root) {
        var thisEditor = this
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

        // Main group transformation controls panning and zooming
        this.maingroup = this.svg.append("g")
        this.maingroup.myTransform = { x: 0, y: 0, zoom: 1 }

        root.ctmDock.addResizeHandler(function() {
            var r = $(root), w = r.width(), h = r.height()
            thisEditor.svg.attr("width", w).attr("height", h)
            thisEditor.mainrect.attr("width", w-3).attr("height", h-3)
        })

        // Unique box identifier
        this.iid = 0

        // Create element that acts as a separator between boxes and links
        // line displayed when dragging new nodes
        // Important: do it before adding drag line in DragHelper ctor
        this.maingroup.append("line")
             .attr("class", "scheme-drag-line-hidden")
             .attr("id", "scheme-box-link-separator")

        // Create drag helper
        this.dragHelper = new DragHelper(this)

        // Enable scene panning
        this.mainrect.on("mousedown", function() { thisEditor.dragHelper.beginPan() })
    }
    Editor.prototype.newBox = function(boxType, boxInfo, options) {
        var opt = options || {}

        if (opt.offset) {
            var rootOffset = $(this.root).offset()
            var t = this.maingroup.myTransform
            opt.pos = {
                x: opt.offset.left - rootOffset.left - t.x,
                y: opt.offset.top - rootOffset.top - t.y
            }
        }
        opt.iid = ++this.iid
        opt.editor = this
        opt.index = this.boxes.length
        this.boxes.push(new Box(boxType, boxInfo, opt))
        this.visualize()
    }

    function updateArrayIndices(a, start) {
        for (var i=(typeof start == "number")? start: 0; i<a.length; ++i)
            a[i].index = i
    }

    Editor.prototype.deleteBox = function(boxIndex, dontVisualize) {
        var boxes = this.boxes, links = this.links, box = boxes[boxIndex]
        unuseBoxName(box.name)
        if (box.selected)
            equaresui.selectBox(null)
        boxes.splice(boxIndex, 1)
        updateArrayIndices(boxes, boxIndex)
        for (i=links.length-1; i>=0; --i) {
            var l = links[i]
            if (l.source.box === box   ||   l.target.box === box)
                this.deleteLink(i, true)
        }
        if (!dontVisualize)
            this.visualize()
    }
    Editor.prototype.newLink = function(port1, port2, dontVisualize) {
        port1.connect(port2)
        port2.connect(port1)
        this.links.push({ source: port1, target: port2, iid: this.iid++, index: this.links.length })
        if (!dontVisualize)
            this.visualize()
    }
    Editor.prototype.deleteLink = function(linkIndex, dontVisualize) {
        var links = this.links, link = links[linkIndex]
        link.source.disconnect(link.target)
        link.target.disconnect(link.source)
        links.splice(linkIndex, 1)
        updateArrayIndices(links, linkIndex)
        this.visualize()
        if (!dontVisualize)
            this.visualize()
    }
    Editor.prototype.dataKey = function(d) {
        return "k" + d.iid;
    }
    function portPos(d) {
        return { x: d.box.x + d.x,   y: d.box.y + d.y }
    }
    function positionLink(d) {
        // Obtain port positions
        var p1 = portPos(d.source),
            p2 = portPos(d.target),
        // Compute link length
            dx = p2.x - p1.x,
            dy = p2.y - p1.y,
            L = Math.sqrt(dx*dx + dy*dy)
         if (L > 0) {
             // Offset link line such that it starts and ends at port boundary circumference
             dx /= L
             dy /= L
             p1.x += dx*5
             p1.y += dy*5
             p2.x -= dx*5
             p2.y -= dy*5
         }
         // Specify line end positions
         d$(this)
            .attr("x1", p1.x)
            .attr("y1", p1.y)
            .attr("x2", p2.x)
            .attr("y2", p2.y)
    }
    Editor.prototype.visualize = function() {
        var thisEditor = this

        var boxUpd = this.maingroup.selectAll(".scheme-box")
            .data(this.boxes, this.dataKey)

        // Create group for incoming boxes
        var boxGroup = boxUpd.enter().insert("g", "#scheme-box-link-separator")
            .attr("class", "scheme-box")

        // Stuff box group with content
        boxGroup.each(function(d) {
            var g = d$(this)

            // Define box position
            g.attr("transform", "translate(" + d.x + "," + d.y + ")")

            // Add text, rectangle, and image acting as the close buttion
            var hmargin = 8, vmargin = 8, sep = 4, wclose = 8
            var txt = g.append("text")
                .text(d.name)
            var bbox = txt.node().getBBox()
            txt
                .attr("x", hmargin - bbox.x)
                .attr("y", vmargin - bbox.y)
            var mouseDownTime
            g.insert("rect", "text")
                .attr("class", "scheme-box-shape")
                .attr("width", bbox.width + sep + wclose + 2*hmargin)
                .attr("height", bbox.height + 2*vmargin)
                .attr("rx", 5)
                .attr("ry", 5)
                .on("mousedown", function(d, i) {
                    clickTimer.start()
                    thisEditor.dragHelper.beginDragBox(this.parentNode, d, i) })
                .on("mouseup", function(d, i) {
                    if (clickTimer.clicked()) {
                        var newsel = !d.selected
                        for (var i in thisEditor.boxes)
                            thisEditor.boxes[i].selected = false
                        d.selected = newsel
                        thisEditor.update()
                        equaresui.selectBox(newsel? d: null)
                    }
                })
            g.append("image")
                .attr("xlink:href", "close.png")
                .attr("x", hmargin + bbox.width + sep)
                .attr("y", vmargin)
                .attr("width", wclose)
                .attr("height", wclose)
                .on("click", function(d) { d.editor.deleteBox(d.index) })

            // Add ports
            var portSel = g.selectAll(".scheme-port")
            var portUpd = portSel.data(d.ports)
            var center = {
                x: 0.5*(bbox.width + sep + wclose) + hmargin,
                y: 0.5*bbox.height + vmargin
            }
            var t0 = center.y / center.x
            var eps = 1e-5
            function clampPortPos(pos) {
                var t = pos/4
                return 4*(t - Math.floor(t))
            }
            portUpd.enter().append("circle")
                .attr("class", "scheme-port")
                .classed("scheme-inport", function(d) { return d instanceof InputPort })
                .classed("scheme-outport", function(d) { return d instanceof OutputPort })
                .attr("r", 5)
                .on("mousedown", function(d, i) { thisEditor.dragHelper.beginDragPort(this, d, i) })
                .on("mouseup", function(d, i) { thisEditor.dragHelper.endDragPort(this, d, i) })
                .on("mouseover", equaresui.hpPortHelp.enter)
                .on("mouseout", equaresui.hpPortHelp.leave)
                .attr("cx", function(d, i) {
                    var t = clampPortPos(d.pos)
                    if (t<1)
                        return d.x = 2*center.x
                    if (--t < 1)
                        return d.x = 2*center.x*(1-t);
                    if (--t < 1)
                        return d.x = 0;
                    return d.x = 2*center.x*--t;
                })
                .attr("cy", function(d) {
                    var t = clampPortPos(d.pos)
                    if (t < 1)
                        return d.y = 2*center.y*(1-t);
                    if (--t < 1)
                        return d.y = 0;
                    if (--t < 1)
                        return d.y = 2*center.y*t;
                    return d.y = 2*center.y;
                });
        })

        boxUpd.exit()
            .transition()
            .duration(350)
            .style("opacity", 1e-6)
            .remove();



        // Create group for incoming links
        var linkUpd = this.maingroup.selectAll(".scheme-link")
            .data(this.links, this.dataKey)

        linkUpd.enter().insert("line", "#scheme-drag-line")
            .attr("class", "scheme-link")
            .on("click", function(d) { thisEditor.deleteLink(d.index) })
            .each(positionLink)
        linkUpd.exit()
            .transition()
            .duration(350)
            .style("opacity", 1e-6)
            .remove();
    }

    Editor.prototype.update = function() {
        var boxUpd = this.maingroup.selectAll(".scheme-box")
            .data(this.boxes, this.dataKey)
        boxUpd.each(function(d) {
            d$(this)
                .attr("transform", "translate(" + d.x + "," + d.y + ")")
                .select("rect")
                    .classed("selected", d.selected)
        })

        var linkUpd = this.maingroup.selectAll(".scheme-link")
            .data(this.links, this.dataKey)
        linkUpd.each(positionLink)
    }

    E.newEditor = function(root) {
        return new Editor(root)
    }

})(ctmEquaresSchemeEditor)
