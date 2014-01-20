// ======== Exports
var ctmEquaresSchemeEditor = {};

ctmEquaresSchemeEditor.init = function(root) {

// Stub, TODO
ctmEquaresSchemeEditor.addBox = function(box, info, options) {
    var nodeData = {
            x: 0, y: 0,
            name: box,
            ports: cloneArray(boxdata[0].ports),
            iid: iid++,
            index: nodes.length
            };
    nodes.push(nodeData);
    restart();
    }

//return; // deBUG
// TODO better: select from root
var schemeContainer = d3.select(root);
var width = $(root).width(),
    height = $(root).height();
var svg = schemeContainer.append("svg")
        .attr("class", "svg-main")
        .attr("width", width)
        .attr("height", height)

var mainrect = svg.append("rect")
    .attr("class", "mainrect")
    .attr("id", "mainrect")
    .attr("width", width)
    .attr("height", height);

// line displayed when dragging new nodes
var drag_line = svg.append("line")
    .attr("class", "drag_line")
    .attr("id", "drag_line")
    .attr("x1", 0)
    .attr("y1", 0)
    .attr("x2", 0)
    .attr("y2", 0);
    
var mousedown_port = null,
    mousedown_node = null;

svg
    .on("mousemove", mousemove)
    .on("mouseup", mouseup)

var boxdata = [
    { name: "Box 1", inports: ["i1"], outports: ["o1"] },
    { name: "Box 2", inports: ["i1", "i2"], outports: ["i1", "i2"] },
    { name: "Box 3", inports: ["i1", "i2", "i3"], outports: ["i1", "i2", "i3"] }
    ];

function merge(dst, src) {
    for (var name in src)
        dst[name] = src[name];
    return dst;
}
function wrap(x, name) {
  var result = {};
  result[name] = x;
  return result;
  }
function bind2nd(func, b) { return function(a) { return func(a,b); }; }
function superpos(f2, f1) { return function(a) { return f2(f1(a)); }; }
function transformArray(array, func) {
    for (var i in array)
        array[i] = func(array[i]);
}
transformArray(boxdata, function(bd) {
    function transformPorts(type) {
        return superpos(
            bind2nd(merge, {type: type}),
            bind2nd(wrap, "name"));
    };
    transformArray(bd.inports, transformPorts("in"));
    transformArray(bd.outports, transformPorts("out"));
    bd.ports = bd.inports.concat(bd.outports);
    var nports = bd.ports.length;
    if (nports > 0) {
        var dphi = 2*Math.PI / (bd.ports.length);
        for (var i=0; i<nports; ++i)
            bd.ports[i].phi = i*dphi;
    }
    return bd;
});
function cloneArray(a) {
    var result = [];
    for (var i in a)
        result[i] = a[i] instanceof Object? $.extend(true, {}, a[i]): a[i];
    return result;
}

var force = d3.layout.force()
    .size([width, height])
    .linkDistance(200)
    .charge(-800)
    .on("tick", tick);

root.ctmDock.addResizeHandler( function() {
    var r = $(root), w = r.width(), h = r.height();
    svg.attr("width", w).attr("height", h);
    mainrect.attr("width", w-3).attr("height", h-3);
    force.size([w, h]).start();
})

var fnodes = force.nodes(),
    flinks = force.links(),
    nodes = [],
    links = [];

nodes = fnodes;
links = flinks;

restart();

var iid = 0;
$(document).contextmenu({
		delegate: "#mainrect",
		preventSelect: true,
		menu: (function() {
		    var result = [];
		    for (var i in boxdata) {
		        var bd = boxdata[i];
		        result[i] = { title: bd.name, cmd: i } //, uiIcon: "ui-icon-scissors"},
		    }
		    return result;
		  })(),
		select: function(event, ui) {
			var $target = ui.target;
			$("#test1").text(ui.cmd);
			var bd = boxdata[ui.cmd];
			var rcpos = $("#mainrect").offset();
            var nodeData = {
                    x: event.pageX-rcpos.left, y: event.pageY-rcpos.top,
                    name: bd.name,
                    ports: cloneArray(bd.ports),
                    iid: iid++,
                    index: nodes.length
                    };
            nodes.push(nodeData);
            restart();
		}
	});

function dataKey(d) { return "k" + d.iid; }

function portX(node, port) { return node.x + 50*Math.cos(port.phi); }
function portY(node, port) { return node.y + 20*Math.sin(port.phi); }
function linkDir(d) {
    var x1 = portX(d.source, d.sourcePort),
        y1 = portY(d.source, d.sourcePort),
        x2 = portX(d.target, d.targetPort),
        y2 = portY(d.target, d.targetPort),
        dx = x2 - x1,
        dy = y2 - y1,
        L = Math.sqrt(dx*dx + dy*dy);
    return L > 0 ?   [dx/L, dy/L] :   [0,0];
}

function tick() {
  var nodeSel = svg.selectAll(".node"),
      linkSel = svg.selectAll(".link");
  var link = linkSel.data(links, dataKey);
  link.attr("x1", function(d) { return portX(d.source, d.sourcePort) + linkDir(d)[0]*5; })
      .attr("y1", function(d) { return portY(d.source, d.sourcePort) + linkDir(d)[1]*5; })
      .attr("x2", function(d) { return portX(d.target, d.targetPort) - linkDir(d)[0]*5; })
      .attr("y2", function(d) { return portY(d.target, d.targetPort) - linkDir(d)[1]*5; });

  var node = nodeSel.data(nodes, dataKey);
  node
      .attr("transform", function(d, i) {
        return "translate(" + d.x + "," + d.y + ")";
      });

  node.selectAll(".port")
      .data(function(d) {return d.ports;})
      .attr("cx", function(d) { return Math.cos(d.phi)*50; })
      .attr("cy", function(d) { return Math.sin(d.phi)*20; });
}

function restart() {
  var nodeSel = svg.selectAll(".node"),
      linkSel = svg.selectAll(".link");

  var link = linkSel.data(links, dataKey);

//  link.enter().insert("line", ".node")
  link.enter().insert("line", "#drag_line")
      .attr("class", "link");

  link.on("click", function(d,i) {
    links.splice(i, 1);
    restart();
  });

  link.exit()
    .transition()
    .duration(350)
    .style("opacity", 1e-6)
    .remove();

  var node = nodeSel.data(nodes, dataKey);

  var box = node.enter().insert("g", "#drag_line")
      .attr("class", "node");

  node.exit()
    .transition()
    .duration(350)
    .style("opacity", 1e-6)
    .remove();

  var ell = box.append("ellipse")
      .attr("class", "box")
      .attr("rx", 50)
      .attr("ry", 20);
  ell.on("mousedown", function(d, i) {
      var node = d3.select(this.parentNode),
          m = this.parentNode.getCTM();
      dragBoxInfo = {
        box: d,
        cursorPos: d3.mouse(svg.node()),
        node: node,
        x: m.e,
        y: m.f
      };
  })
  var txt = box.append("text")
      .attr("x", -40)
      .attr("y", 5)

    box.append("image")
        .attr("xlink:href", "close.png")
        .attr("x", 20)
        .attr("y", -10)
        .attr("width", 8)
        .attr("height", 8)
        .on("click", function(d, i){
          if (i != d.index) {
              $("#test3").text("Unexpected node index " + i + " != " + d.index);
              setTimeout(function(){$("#test3").text("blah");}, 1000);
          }
          nodes.splice(d.index, 1);
          for (var j=d.index; j<nodes.length; ++j)
              nodes[j].index = j;
          for (var il=links.length-1; il>=0; --il) {
              var l = links[il]
              if (l.source === d   ||   l.target === d)
                  links.splice(il, 1);
          }
          restart();
        })

  node.select("text")
//      .attr("textLength", 80)
      .text(function(d, i) { return d.name + " " + d.iid; });

  var port = node.selectAll(".port")
      .data(function(d) {
        return d.ports;
      });
  port
      .enter().append("circle")
      .attr("class", "port")
      .classed("inport", function(d) {
         return d.type === "in"
       })
      .classed("outport", function(d) {
        return d.type === "out"
       })
      .attr("r", 5)
      .attr("cy", 10)
      .on("mousedown", mousedownAtPort)
      .on("mouseup", mouseupAtPort)
  port
      .attr("cx", function(d, i) {
        // return i*40/this.parentNode.__data__.ports.length;
        return Math.cos(d.phi)*50
      })
      .attr("cy", function(d) { return Math.sin(d.phi)*20; });

//  var ports = box.selectAll(".port");
//  d3.range(0, boxda

  force.start();
}

function mousedownAtPort(d, i) {
    mousedown_port = d;
    mousedown_node = this.parentNode.__data__;
    d3.event.preventDefault();
    var m = this.getCTM();
    var pt = svg.node().createSVGPoint();
    var thisSel = d3.select(this);
    pt.x = thisSel.attr("cx");
    pt.y = thisSel.attr("cy");
    pt = pt.matrixTransform(m);
    var mp = d3.mouse(svg.node());
    drag_line
      .attr("class", "drag_line")
      .attr("x1", pt.x)
      .attr("y1", pt.y)
      .attr("x2", mp[0])
      .attr("y2", mp[1]);
    $("#test1").text('mousedown (at a port)');
}

var dragBoxInfo = null;

function mouseMoveAtBox(d, i) {
  if (!dragBoxInfo)
    return;
  var cursorPos = d3.mouse(svg.node()),
      x = dragBoxInfo.x + cursorPos[0] - dragBoxInfo.cursorPos[0],
      y = dragBoxInfo.y + cursorPos[1] - dragBoxInfo.cursorPos[1];
  dragBoxInfo.box.x = x;
  dragBoxInfo.box.y = y;
  tick();
}

function mousemoveAtPort() {
    if (!mousedown_port)
        return;
    // update drag line
    var mp = d3.mouse(this);
    drag_line
      .attr("x2", mp[0])
      .attr("y2", mp[1]);

    $("#test2").text('mousemove');
}

function mousemove() {
    mousemoveAtPort.apply(this, arguments);
    mouseMoveAtBox.apply(this, arguments);
}

function mouseup() {
    mousedown_port = null;
    mousedown_node = null;
    drag_line
      .attr("class", "drag_line_hidden")
    $("#test1").text('mouseup');
    dragBoxInfo = null;
    force.start();
}

function mouseupAtPort(d, i) {
    if (mousedown_port) {
        var mouseup_port = d,
            mouseup_node = this.parentNode.__data__;
        function canAddLink(n1, p1, n2, p2) {
            if (n1 === n2 && p1 === p2)
                return false;
            for (var il in links) {
                var l = links[il];
                if (l.source === n1 && l.sourcePort === p1 && l.target === n2 && l.targetPort === p2)
                    return false;
                if (l.source === n2 && l.sourcePort === p2 && l.target === n1 && l.targetPort === p1)
                    return false;
            }
            return true;
        }
        if (canAddLink(mousedown_node, mousedown_port, mouseup_node, mouseup_port)) {
            links.push({
                source: mousedown_node,
                target: mouseup_node,
                sourcePort: mousedown_port,
                targetPort: mouseup_port,
                iid: iid++
            });
            restart();
        }
    }
    mouseup.apply(this, arguments);
}

function normalizeAngle(phi) {
    phi = (phi/(2*Math.PI) + 0.5);
    phi -= Math.floor(phi);
    phi = (phi - 0.5) * 2*Math.PI;
    return phi;
}

var myLayoutAlpha = 0,
    myLayoutAlphaFactor = 0.98,
    myLayoutAlphaThreshold = 0.01;

function doMyLayoutStep() {
    var n = nodes.length,   m = links.length,   i, j, np;

    // Set zero forces on all ports
    for (i=0; i<n; ++i) {
        var nd = nodes[i];
        for (j=0, np=nd.ports.length; j<np; ++j)
            nd.ports[j].Q = 0;
    }

    // Compute generalized forces acting on ports from links
    for (var i=0; i<m; ++i) {
        var l = links[i],
            dir = linkDir(l),
            phi1 = l.sourcePort.phi,
            phi2 = l.targetPort.phi,
            Q1 = -50*Math.sin(phi1)*dir[0] + 20*Math.cos(phi1)*dir[1],
            Q2 =  50*Math.sin(phi2)*dir[0] - 20*Math.cos(phi2)*dir[1];
        l.sourcePort.Q += Q1;
        l.targetPort.Q += Q2;
    }

    // Compute generalized forces acting on ports from heighboring ports
    for (i=0; i<n; ++i) {
        var nd = nodes[i];
        var dphinom = 2*Math.PI / nd.ports.length;
        var Q0 = 50, Qscale = 20;
        var nds = [];
        for (j=0, np=nd.ports.length; j<np; ++j) {
            nds[j] = { idx: j, phi: normalizeAngle(nd.ports[j].phi) };
        }
        nds.sort(function(a, b) { return a.phi-b.phi; });
        for (j=0; j<np; ++j) {
            var jn = (j+1)%np,
                p1 = nd.ports[nds[j].idx],
                p2 = nd.ports[nds[jn].idx],
                dphi = normalizeAngle(p2.phi - p1.phi);
            if (dphi == 0) {
                p1.Q += Q0;
                p2.Q -= Q0;
            } else {
                var Q = Qscale*(dphinom / Math.abs(dphi) - 1);
                if (Q > Q0)
                    Q = Q0;
                if (dphi < 0)
                    Q = -Q;
                p2.Q += Q;
                p1.Q -= Q;
            }
        }
    }

    // Apply forces at ports
    var f = 0.01 * myLayoutAlpha;
    for (i=0; i<n; ++i) {
        var nd = nodes[i];
        for (j=0, np=nd.ports.length; j<np; ++j) {
            var p = nd.ports[j];
            p.phi += f*p.Q;
        }
    }
    myLayoutAlpha *= myLayoutAlphaFactor;

    tick();
}

var myLayoutRunning = false;
var myLayoutInterval = 50;
var myLayoutIteration = 0;
function stepMyLayout() {
    if (!myLayoutRunning)
        return;
    doMyLayoutStep();
    $("#test4").text(myLayoutIteration);
    ++myLayoutIteration;
    if (myLayoutAlpha > myLayoutAlphaThreshold)
        setTimeout(stepMyLayout, myLayoutInterval);
    }
$("#test4").click(function() {
    myLayoutRunning = !myLayoutRunning;
    $(this).attr("class", myLayoutRunning? "test_running": "test");
    if (myLayoutRunning) {
        myLayoutIteration = 0;
        myLayoutAlpha = 1;
        setTimeout(stepMyLayout, myLayoutInterval);
    }
});

}
