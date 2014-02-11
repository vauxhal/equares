var ctmEquaresSchemeEditor = {};

(function(E) {
    function removeBoxVisualizer(box) {
        box.editor.maingroup.selectAll(".scheme-box").datum(function(d) { return d.iid == box.iid? {iid: -1}: d })
    }

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
            if (box.name == newName)
                return
            if (boxNames[newName])
                return false
            removeBoxVisualizer(box)
            delete boxNames[box.name]
            boxNames[box.name = newName] = true
            box.editor.visualize().update()
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

    function onChangeBoxState(kind, id) {
        if (this.selected && kind == "propset")
            equaresui.selectBox(this)
    }

    // Constructor for type used as the D3 data for a box
    function Box(boxType, options) {
        var opt = options || {}
        var name = opt.name || nameBox(boxType)
        equaresBox.Box.call(this, name, boxType, opt)
        var pos = opt.pos || {}
        this.x = pos.x || 0
        this.y = pos.y || 0
        this.iid = opt.iid || 0
        this.editor = opt.editor || null
        this.index = typeof(opt.index) == "number"? opt.index: -1
        this.selected = opt.selected || false
        this.callbacks.push(onChangeBoxState)
    }
    Box.prototype = new equaresBox.Box()
    Box.prototype.rename = function(newName) { return renameBox(this, newName) }
    Box.prototype.changeType = function(boxType) {
        // Replace box with a new one
        removeBoxVisualizer(this)
        var e = this.editor
        var newBox = new Box(boxType, {
            name: this.name,
            pos: {x: this.x, y: this.y},
            iid: this.iid,
            editor: e,
            index: this.index,
            selected: this.selected
        })
        e.boxes[this.index] = newBox

        // Try to apply old links to new box
        var oldLinks = this.links()
        for (var i=0; i<oldLinks.length; ++i) {
            var link = oldLinks[i]
            e.deleteLink(e.findLink(link.source, link.target), true)
            var port = newBox.findPort(link.source)
            if (port && equaresBox.canConnect(port, link.target))
                e.newLink(port, link.target, true)
        }

        // Copy compatible properties
        for (var pname in newBox.props) {
            var t = newBox.propType(pname)
            if (t.length == 0)
                continue    // Property value is not available
            if (this.propType(pname) === t)
                newBox.prop(pname, this.prop(pname))
        }

        e.visualize().update()
        if (this.selected)
            equaresui.selectBox(newBox)
        return newBox
    }
    Box.prototype.select = function(newsel) {
        if (arguments.length == 0)
            newsel = !this.selected
        var e = this.editor
        for (var i in e.boxes)
            e.boxes[i].selected = false
        this.selected = newsel
        e.update()
        equaresui.selectBox(newsel? this: null)
    }

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
            if (!equaresBox.canConnect(p1, p2))
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
    Editor.prototype.newBox = function(boxType, options) {
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
        var result = new Box(boxType, opt)
        this.boxes.push(result)
        this.visualize()
        return result
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
        equaresBox.connect(port1, port2)
        this.links.push({ source: port1, target: port2, iid: this.iid++, index: this.links.length })
        if (!dontVisualize)
            this.visualize().update()
    }
    Editor.prototype.findLink = function(port1, port2) {
        var links = this.links
        for (var i=0; i<links.length; ++i) {
            var link = links[i], p1 = link.source, p2 = link.target
            if (p1 === port1 && p2 === port2   ||   p1 === port2 && p2 === port1)
                return i
        }
        return -1
    }
    Editor.prototype.deleteLink = function(linkIndex, dontVisualize) {
        var links = this.links, link = links[linkIndex]
        equaresBox.disconnect(link.source, link.target)
        links.splice(linkIndex, 1)
        updateArrayIndices(links, linkIndex)
        if (!dontVisualize)
            this.visualize().update()
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
    function updateLink(d) {
        positionLink.apply(this, arguments)
        var f1 = d.source.getFormat(), f2 = d.target.getFormat()
        var good = !f1.bad && f1.equals(f2)
        d$(this)
            .classed("scheme-link-good", good)
            .classed("scheme-link-bad", !good)
    }

    function setBoxStatus(node, d) {
        var g = d$(node)
        var stat = g.select(".status")
        stat
            .attr("xlink:href", "pix/status-"+d.status.level+".png")
            .on("mouseover", equaresui.hpStatusHelp.enter)
            .on("mouseout", equaresui.hpStatusHelp.leave)
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
            var hmargin = 10, vmargin = 8, wstat = 12, sep = 4, wclose = 8
            var xitem = hmargin
            var txt = g.append("text")
                .text(d.name)
            var bbox = txt.node().getBBox()
            var wtotal = bbox.width + wstat + 2*sep + wclose + 2*hmargin

            // Add status image
            g.append("image")
                .attr("class", "status")
                .attr("x", xitem)
                .attr("y", vmargin)
                .attr("width", wstat)
                .attr("height", wstat)
            xitem += wstat + sep

            // Add box caption
            txt
                .attr("x", xitem - bbox.x)
                .attr("y", vmargin - bbox.y)
            xitem += bbox.width + sep
            var mouseDownTime

            // Add box rectangle
            g.insert("rect", "text")
                .attr("class", "scheme-box-shape")
                .attr("width", wtotal)
                .attr("height", bbox.height + 2*vmargin)
                .attr("rx", 5)
                .attr("ry", 5)
                .on("mousedown", function(d, i) {
                    clickTimer.start()
                    thisEditor.dragHelper.beginDragBox(this.parentNode, d, i) })
                .on("mouseup", function(d, i) {
                    if (clickTimer.clicked()) {
                        d.select()
                    }
                })

            // Add close button image
            g.append("image")
                .attr("xlink:href", "pix/close.png")
                .attr("class", "close")
                .attr("x", xitem)
                .attr("y", vmargin)
                .attr("width", wclose)
                .attr("height", wclose)
                .style("cursor", "pointer")
                .on("click", function(d) { d.editor.deleteBox(d.index) })
            xitem += wclose

            // Add ports
            var portSel = g.selectAll(".scheme-port")
            var portUpd = portSel.data(d.ports)
            var center = {
                x: 0.5*wtotal,
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
            setBoxStatus(this, d)
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
            .each(updateLink)
        linkUpd.exit()
            .transition()
            .duration(350)
            .style("opacity", 1e-6)
            .remove();
        return this
    }

    Editor.prototype.update = function() {
        var boxUpd = this.maingroup.selectAll(".scheme-box")
            .data(this.boxes, this.dataKey)
        boxUpd.each(function(d) {
            d$(this)
                .attr("transform", "translate(" + d.x + "," + d.y + ")")
                .select("rect")
                    .classed("selected", d.selected)
            setBoxStatus(this, d)
        })

        var linkUpd = this.maingroup.selectAll(".scheme-link")
            .data(this.links, this.dataKey)
        linkUpd.each(updateLink)
        return this
    }

    function allProps(obj, suffix) {
        var result = []
        suffix = suffix || ""
        for (var name in obj)
            result.push(name + suffix)
        return result
    }

    function copyProps(dst, src, paths) {
        if (paths === undefined)
            paths = allProps(src)
        for (var i=0; i<paths.length; ++i) {
            var path = paths[i].split("/")
            var name = path[0]
            var suffix = "", subpaths
            if (path.length > 1) {
                path.splice(0, 1)
                subpaths = [path.join("/")]
                suffix = "/" + subpaths[0]
            }
            if (name === "*") {
                var names = allProps(src, suffix)
                copyProps(dst, src, names)
            }
            else {
                var ps = src[name]
                if (ps === undefined)
                    continue
                if (ps instanceof Array) {
                    if (!(dst[name] instanceof Array))
                        dst[name] = []
                    copyProps(dst[name], ps, subpaths)
                }
                else if (ps instanceof Object) {
                    if (!(dst[name] instanceof Object))
                        dst[name] = {}
                    copyProps(dst[name], ps, subpaths)
                }
                else
                    dst[name] = src[name]
            }
        }
    }
    Editor.prototype.export = function() {
        var result = { boxes: [], links: [] }
        var i
        for (i=0; i<this.boxes.length; ++i) {
            var b = this.boxes[i],   bx = result.boxes[i] = {}
            copyProps(bx, b, [ "name", "info/inputs", "info/outputs", "type", "props/*/value", "status", "x", "y" ])
        }
        for (i=0; i<this.links.length; ++i) {
            var l = this.links[i],   lx = result.links[i] = {}
            lx.source = {box: l.source.box.name, port: l.source.info.name}
            lx.target = {box: l.target.box.name, port: l.target.info.name}
        }
        return JSON.stringify(result)
    }
    Editor.prototype.exportSimulation = function() {
        var simulation = { boxes: [], links: [] }
        var i
        for (i=0; i<this.boxes.length; ++i) {
            var b = this.boxes[i],   bx = simulation.boxes[i] = {}
            copyProps(bx, b, [ "name", "type" ])
            if (b.props instanceof Object) {
                bx.props = {}
                for (var propName in b.props)
                    bx.props[propName] = b.boxprop(propName)
            }
        }
        for (i=0; i<this.links.length; ++i) {
            var l = this.links[i],   lx = simulation.links[i] = {}
            lx.source = {box: l.source.box.name, port: l.source.info.name}
            lx.target = {box: l.target.box.name, port: l.target.info.name}
        }
        return simulation
    }
    var findFirst = equaresBox.findFirst
    Editor.prototype.findBox = function (name) {
        return findFirst(this.boxes, function(box) { return box.name === name })
    }
    Editor.prototype.import = function(text, callback, progressCallback) {
        try {
            var data = JSON.parse(text)
            var boxes = data.boxes, links = data.links, i, j, b, box
            if (!(boxes instanceof Array))
                throw { message: "boxes is missing or is not an array" }
            if (!(links instanceof Array))
                throw { message: "boxes is missing or is not an array" }
            var editor = this
            editor.boxes = []
            editor.links = []

            // Create boxes (no param values so far)
            var rootOffset = $(editor.root).offset()
            var t = editor.maingroup.myTransform
            for (i=0; i<boxes.length; ++i) {
                b = boxes[i]
                var opt = {
                    offset: {left: b.x + rootOffset.left + t.x, top: rootOffset.top + b.y + t.y},
                    name: b.name,
                    info: b.info
                }
                editor.newBox(b.type, opt)
            }

            // Count the total number of critical parameters in all boxes;
            // bind listeners to the 'critical' box event
            var ncp = 0,   icp = 0
            function onSetCriticalProp() {
                ++icp
                if (progressCallback instanceof Function) {
                    var percent = Math.round(100*icp / ncp)
                    progressCallback(percent)
                }
                if (icp == ncp)
                    continueLoading();
            }
            for (i=0; i<editor.boxes.length; ++i) {
                box = editor.boxes[i]
                box.on('critical', onSetCriticalProp)
                ncp += box.criticalPropCount()
            }

            function setProps(critical) {
                for (var i=0; i<boxes.length; ++i) {
                    var b = boxes[i], box = editor.findBox(b.name)
                    for (var propName in b.props)
                        if (box.props[propName].critical == critical)
                            box.prop(propName, b.props[propName].value)
                }
            }

            // This is done as soon as all critical props are set
            function continueLoading() {
                // Remove critical event listener from all boxes
                var box, i
                for (i=0; i<editor.boxes.length; ++i) {
                    box = editor.boxes[i]
                    box.removeListener('critical', onSetCriticalProp)
                }

                // Create links
                function findPort(portData) {
                    var box = editor.findBox(portData.box)
                    return findFirst(box.ports, function(port) { return port.info.name === portData.port })
                }
                for (i=0; i<links.length; ++i) {
                    var l = links[i], p1 = findPort(l.source), p2 = findPort(l.target)
                    editor.newLink(p1, p2, true)
                }

                // Now set non-critical box properties
                setProps(false)

                // Visualize scene
                editor.visualize().update()
                if (callback instanceof Function)
                    callback()
            }

            // Set critical properties; do the rest as soon as they are all set
            if (ncp > 0)
                setProps(true)
            else
                continueLoading()
        }
        catch(e) {
            alert(e.message)
            if (callback instanceof Function)
                callback()
        }
    }
    E.newEditor = function(root) {
        return new Editor(root)
    }

})(ctmEquaresSchemeEditor)
