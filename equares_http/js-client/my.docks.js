
// ======== Exports
var ctmDock = {};

(function() {

// ======== Utility functions

function newElement( elementType, elementClass ) {
    var j = $( document.createElement(elementType) );
    if( elementClass == undefined )
        return j[0];
    if( elementClass instanceof Array ) {
        for( var i in elementClass )
            j.addClass( elementClass[i] );
    }
    else
        j.addClass( elementClass );
    return j[0];
}
function newDiv( divClass ) { return newElement( 'DIV', divClass ); }
function newImg( imgClass ) { return newElement( 'IMG', imgClass ); }

function orientation( vertical )    { return vertical? "vertical": "horizontal"; }
function widthOrHeight( width )     { return width? "width": "height"; }
function outerWidthOrHeight( width ){ return width? "outerWidth": "outerHeight"; }
function leftOrTop( left )          { return left? "left": "top"; }
function xOrY( x )                  { return x? "x": "y"; }
function prevOrNext( dir )          { return dir > 0? "next": "prev"; }
function firstOrLast( first )       { return first? "first": "last"; }
function beforeOrAfter( before )    { return before? "before": "after"; }

function generator(factory, type) {
    var result = factory[type];
    if( !( result instanceof Function ) )
        throw "Unknown type";
    return result;
}

function safeValue(value, defaultValue) {
    return value == undefined? defaultValue: value;
}



// ======== Utility types

// -------- Point (2D point)
function Point( x, y ) {
    this.x = x;
    this.y = y;
}
Point.prototype.toOffset = function() {
    return { left: x, top: y };
}
Point.fromOffset = function( offset ) {
    return new Point( offset.left, offset.top );
}
Point.fromEvent = function( ev ) {
    return new Point( ev.pageX, ev.pageY );
}

// -------- Range with optional min & max and a validation method
function Range() {
    var ok = true;
    if( arguments.length == 0 )
        this.min = 10;
    else if( arguments.length == 1 ) {
        if( arguments[0] == undefined )
            this.min = 10;
        else if( arguments[0] instanceof Object ) {
            this.min = safeValue( arguments[0].min, 10 );
            this.max = arguments[0].max;
            }
        else if( typeof(arguments[0]) == "number" )
            this.min = this.max = arguments[0];
        else
            ok = false;
        }
    else if( arguments.length == 2 &&
        typeof(arguments[0]) == "number" &&
        typeof(arguments[1]) == "number" )
    {
        this.min = arguments[0];
        this.max = arguments[0];
    }
    else
        ok = false;
    if( !ok )
        throw "Range() failed: unable to understand arguments";
}
Range.prototype.validate = function(size) {
    if( this.min != undefined   &&   size < this.min )
        return this.min;
    if( this.max != undefined   &&   size > this.max )
        return this.max;
    return size;
}

// -------- Drag helper
var DragHelper = {}
DragHelper.handler = undefined;
DragHelper.start = function(ev, handler, dock) {
    DragHelper.handler = handler;
    DragHelper.dock = dock;
    DragHelper.tpos0 = Point.fromOffset( $(dock.dom).offset() );
    DragHelper.mpos0 = Point.fromEvent( ev );
    DragHelper.mpos = DragHelper.mpos0;
    DragHelper.mdelta = new Point(0, 0);
    DragHelper.mdelta0 = new Point(0, 0);
}
DragHelper.moveHelper = function(ev, stop) {
    if( DragHelper.handler ) {
        var mpos = Point.fromEvent( ev );
        DragHelper.mdelta.x = mpos.x - DragHelper.mpos.x;
        DragHelper.mdelta.y = mpos.y - DragHelper.mpos.y;
        DragHelper.mdelta0.x = mpos.x - DragHelper.mpos0.x;
        DragHelper.mdelta0.y = mpos.y - DragHelper.mpos0.y;
        DragHelper.mpos = mpos;
        DragHelper.handler.call( DragHelper.dock, stop );
    }
}
DragHelper.move = function(ev) {
    DragHelper.moveHelper( ev, false );
}
DragHelper.stop = function(ev) {
    DragHelper.moveHelper( ev, true );
    DragHelper.handler = undefined;
    DragHelper.dock = undefined;
}
// Drag helper initialization
$(window).mousemove(DragHelper.move);
$(window).mouseup(DragHelper.stop);



// ======== Dock - the base for other types

function Dock() {}
Dock.prototype.parentDock = function() {
    return $(this.dom).parent()[0].ctmDock;
}
Dock.prototype.attach = function(domElement) {
    domElement.ctmDock = this;
    this.dom = domElement;
}
Dock.prototype.orientation = function() {
    return orientation(this.vertical);
}
Dock.prototype.moveBy = function( delta, vertical ) {
    var lt = leftOrTop( !vertical );
    var j = $(this.dom);
    var offset = j.offset();
    offset[lt] += delta;
    j.offset( offset );
}
Dock.prototype.setPos = function( pos ) {
    var j = $(this.dom);
    if( pos.width )
        j.width( pos.width );
    if( pos.height )
        j.height( pos.height );
    if( pos.left != undefined   ||   pos.top != undefined )
        j.offset( pos );
}
Dock.prototype.highlight = function() {
    if( this.highlightCount == undefined )
        this.highlightCount = 0;
    ++this.highlightCount;
    if( this.highlightCount == 1 )
        $(this.dom).addClass("highlight");
}
Dock.prototype.unhighlight = function() {
    --this.highlightCount;
    if( this.highlightCount == 0 )
        $(this.dom).removeClass("highlight");
}



// ======== Layouts

// -------- Layout - base type for layouts
function Layout() {}
Layout.prototype = new Dock();
Layout.prototype.setOptions = function( options ) {
    this.options = {};
    if( !options instanceof Object )
        return;
    this.options.fixed = options.fixed;
    this.options.master = options.master;
}
Layout.prototype.containerCount = function() {
    return this.containers().length;
}
// Returns object that has the following properties:
// size     container size in the layout direction
// asize    container size in the direction adjacent to the layout direction
// fsize    size occupied by fixed containers
// rsize    size occupied by resizable containers
// ssize    size occupied by splitters
// nr       number of resizable containers
// n        number of containers
// Note: hidden containers are not counted!
Layout.prototype.sizeInfo = function() {
    var result = {};
    
    // Obtain parent container
    var container = this.parentDock();

    var vertical = this.vertical;

    // Generate orientation-specific prop names
    var hw = widthOrHeight( vertical );
    var wh = widthOrHeight( !vertical );
    var owh = outerWidthOrHeight( !vertical );

    // Obtain jQuery wrappers for parent container and this layout
    var jc = $(container.dom);
    var jl = $(this.dom);

    // Obtain container size and position
    result.size = jc[wh]();
    result.asize = jc[hw]();

    // Compute sizes occupied by fixed containers, resizable containers, and splitters;
    // count resizable containers
    result.fsize = 0;
    result.rsize = 0;
    result.nr = 0;
    result.n = 0;
    jl.children( ".container:visible" ).each(function() {
        var s = $(this)[owh]();
        if( this.ctmDock.fixedSize(vertical) )
            result.fsize += s;
        else {
            result.rsize += s;
            ++result.nr;
        }
        ++result.n;
    });
    
    // Compute size occupied by splitters
    result.ssize = 0;
    jl.children( ".splitter:visible" ).each(function() { result.ssize += $(this)[owh](); });

    return result;
}
Layout.prototype.resize = function() {
    // Obtain layout size info
    var si = this.sizeInfo();

    // Obtain parent container
    var container = this.parentDock();

    var vertical = this.vertical;

    // Generate orientation-specific prop names
    var hw = widthOrHeight( vertical );
    var wh = widthOrHeight( !vertical );
    var owh = outerWidthOrHeight( !vertical );
    var tl = leftOrTop( vertical );
    var lt = leftOrTop( !vertical );

    // Obtain jQuery wrappers for parent container and this layout
    var jc = $(container.dom);
    var jl = $(this.dom);

    var pos = jc.offset();
    pos[hw] = si.asize;
    pos[wh] = si.size;

    // Resize this layout to container
    this.setPos(pos);

    // Position and resize children
    var containers = jl.children( ".container:visible" );

    // Resize and position child items
    var rsizeNew = si.size - si.fsize - si.ssize;
    containers.add(jl.children(".splitter:visible")).each(function() {
        var d = this.ctmDock;
        var j = $(this);
        var s = 0;
        if( d instanceof Container ) {
            var s0 = j[wh]();
            var delta = j[owh]() - s0;
            if( d.fixedSize(vertical) )
                s = d[wh].min;
            else {
                if( rsizeNew > 0   &&   si.rsize > 0 )
                    s = s0 * rsizeNew / si.rsize;
                else if( si.nr > 0 )
                    s = rsizeNew / si.nr;
                s = d[wh].validate( s );
                rsizeNew -= s;
                si.rsize -= s0;
                --si.nr;
            }
            pos[wh] = s;
            s += delta;
        }
        else if( d instanceof Splitter ) {
            pos[wh] = undefined;
            s = j[owh]();
        }

        // As a special case, consider header of vertical layout - it sould always be at the bottom.
        if( vertical   &&   ( d instanceof HeaderContainer )   &&   this === containers.last()[0] )
            pos[lt] = jc.offset()[lt] + si.size - s;

        d.setPos( pos );
        pos[lt] += s;
    });

    // Resize nested containers
    containers.each(function() {
        this.ctmDock.resize();
    });
}
Layout.prototype.add = function( options ) {
    var container;
    var needRegister;
    if( options.container instanceof Container )
        container = options.container;
    else {
        if( !options.tools   &&   options.toolhandle !== false )
            options.tools = ctmDock.toolSets.free;
        container = new NormalContainer( options );
        // If container has a variable size, give it a reasonable initial size
        if( !container.fixedSize(this.vertical) ) {
            var si = this.sizeInfo();
            if( si.nr > 0 ) {
                var wh = widthOrHeight( !this.vertical );
                $(container.dom)[wh]( container[wh].validate( si.rsize / si.nr ) );
            }
        }
    }
    if( !( container instanceof HeaderContainer ) )
        needRegister = this.hasHeader();
    var containers = this.containers();
    if( container instanceof HeaderContainer )
        $(this.dom)[this.vertical? "append": "prepend"]( $(container.dom) );
    else if( containers.length == 0 )
        $(this.dom)[this.vertical? "prepend": "append"]( $(container.dom) );
    else {
        var splitter = new Splitter( !this.vertical, !options.nosplit );
        if( options.before instanceof Container )
            $(options.before.dom).before( $(container.dom), $(splitter.dom) );
        else {
            var after = options.after;
            if( after instanceof Container )
                after = $(after.dom);
            else
                after = containers.last();
            after.after( $(splitter.dom), $(container.dom) );
        }
        if( $(container.dom).is(":hidden") )
            $(splitter.dom).hide();
    }
    if( needRegister )
        this.header().register( container );
    this.updateContainerIndices();
    if( this.options.tabbed   &&
        !( container instanceof HeaderContainer )   &&
        $(container.dom).is(":visible") )
    {
        // For tabbed layout, we need to make sure there's at most one visible container.
        var vcontainers = this.containers().filter(":visible");
        var header = this.header();
        vcontainers.not( $(container.dom) ).each(function() {
            header.minimize( this.ctmDock, true );
        });
    }
    if( !options.noresize )
        this.resize();
    return container;
}
Layout.prototype.remove = function( container ) {
    if( !( container instanceof HeaderContainer )   &&   container.containerHandle )
        this.header().unregister( container );
    var jc = $(container.dom);
    var hidden = jc.is(":hidden");
    var s1 = container.splitter(-1),   s2 = container.splitter(1);
    if( !s1 && !s2 ) {
        // No more containers in the layout
        jc.remove();
        return false;
    }
    // There are more containers in the layout
    // Decide which splitter to remove
    if( s1 && s2 ) {
        if( $(s1.dom).is(":hidden") != hidden )
            s1 = s2;
        }
    else if( !s1 )
        s1 = s2;
    if( !hidden   &&   $(s1.dom).is(":hidden") ) {
        var js = container.wvisibleSplitter(-1);
        if( js.length == 0 )
            js = container.wvisibleSplitter(1);
        js.hide();
    }
    jc.remove();
    $(s1.dom).remove();
    this.updateContainerIndices();
    
    if( this.options.tabbed ) {
        // For tabbed layout, we might need to restore some visible container
        var containers = this.containers();
        if( containers.filter(":visible").length == 0 )
            this.header().restore( containers[0].ctmDock );
    }

    this.resize();
    return true;
}
Layout.prototype.tryGetHeader = function() {
    var fl = firstOrLast( !this.vertical );
    var jh = $(this.dom).children(".container")[fl]();
    if( jh.length > 0   &&   jh[0].ctmDock instanceof HeaderContainer )
        return jh[0].ctmDock;
    return undefined;
}
Layout.prototype.hasHeader = function() {
    return this.tryGetHeader() != undefined;
}
Layout.prototype.header = function( options ) {
    var hdr = this.tryGetHeader();
    if( !hdr ) {
        hdr = new HeaderContainer( options );
        $(hdr.dom).addClass(orientation(!this.vertical));
        this.add( { container: hdr } );
        var parentContainer = this.parentDock();
        if( parentContainer instanceof NormalContainer   &&
            parentContainer.parentLayout().tryGetHeader() instanceof HeaderContainer )
            $(hdr.dom).children(".toolhandle").html( parentContainer.index().toString() );
    }
    return hdr;
}
Layout.prototype.containers = function() {
    return $(this.dom).children(".container").filter(":not(.header_container)");
}
Layout.prototype.updateContainerIndices = function() {
    var index = 1;
    var hasHeader = this.tryGetHeader() instanceof HeaderContainer;
    this.containers().each(function() {
        // Set index to:
        // - container's tool handle
        var c = this.ctmDock;
        var s = hasHeader ?   index.toString() :   "";
        $(c.dom).children(".toolhandle").html(s);

        // - container handle stored in the header
        if( c.containerHandle )
            $(c.containerHandle.dom).html(s);

        // - container nested layout header's tool handle
        var layout = c.layout();
        if( layout ) {
            var header = layout.tryGetHeader();
            if( header )
                $(header.dom).children(".toolhandle").html(s);
            }
        ++index;
    });
}
Layout.prototype.toggleTabbed = function() {
    if( !this.options.tabbed ) {
        var c = this.containers();
        if( c.length > 0 ) {
            var cv = c.filter( ":visible" );
            var h = this.header();
            if( cv.length > 0 ) {
                var d = cv[0].ctmDock;
                if( cv.length > 1 )   cv.each(function() {
                    if( this.ctmDock != d )
                        h.minimize( this.ctmDock );
                });
            }
            else
                h.restore( c[0].ctmDock );
        }
    }
    this.options.tabbed = !this.options.tabbed;
}
Layout.prototype.reduce = function() {
    var containers = this.containers();
    if( containers.length == 0 ) {
        // The layout is empty
        
        // Obtain parent container
        var parentContainer = this.parentDock();
        
        //  Remove layout
        $(this.dom).remove();
        
        if( parentContainer instanceof NormalContainer ) {
            if( parentContainer.master ) {
                if( ctmDock.toolSets.free ) {
                    // Create new tool handle in the container
                    parentContainer.makeToolHandle( ctmDock.toolSets.free );
                    parentContainer.setPos( {} );   // Do this to position tool handle
                    parentContainer.parentLayout().updateContainerIndices();
                }
            }
            else {
                // Also close parent container, unless it's master
                var parentLayout = parentContainer.parentLayout();
                parentLayout.remove( parentContainer );
                parentLayout.reduce();
            }
        }
        return true;
    }
    
    if( this.options.master ) {
        if( containers.length == 1 ) {
            var subc = containers[0].ctmDock;
            var subl = subc.layout();
            if( subl instanceof Layout )
                return subl.reduce();
        }
        // Don't be a smartass, don't kill this layout if it is master.
        // The only case where we do that is empty master layout.
        return false;
    }

    if( containers.length == 1 ) {
        var subc = containers[0].ctmDock;
        var subl = subc.layout();
        if( subl ) {
            var supc = this.parentDock();
            var supl = supc.parentDock();
            if( supl instanceof Layout && supl.vertical === subl.vertical ) {
                // Merge layouts
                var c = subl.containers();
                c.each(function() {
                    supl.add( { container: this.ctmDock, before: supc, noresize: true } );
                });
                supl.remove( supc );
                return true;
            }
        }
        else {
            var supc = this.parentDock();
            var supl = supc.parentDock();
            if( supl instanceof Layout ) {
                supl.add( { container: subc, before: supc, noresize: true } );
                supl.remove( supc );
                return true;
            }
        }
    }
    else {
        var parentContainer = this.parentDock();
        var parentLayout = parentContainer.parentDock();
        if( parentLayout   &&   parentLayout.vertical == this.vertical ) {
            // Merge this layout to parent
            var after = parentContainer;
            containers.each(function() {
                parentLayout.add( {
                    container: this.ctmDock,
                    noresize: true,
                    after: after
                } );
                after = this.ctmDock;
            } );
            // Remove parent container
            parentLayout.remove( parentContainer );
            parentLayout.resize();
            return true;
        }
    }
    return false;
}

// -------- Vertical layout
function VerticalLayout( options ) {
    this.attach( newDiv("vertical layout") );
    this.vertical = true;
    this.setOptions( options );
}
VerticalLayout.prototype = new Layout();

// -------- Horizontal layout
function HorizontalLayout( options ) {
    this.attach( newDiv("horizontal layout") );
    this.vertical = false;
    this.setOptions( options );
}
HorizontalLayout.prototype = new Layout();



// ******** Layout factory
Layout.factory = {
    vertical: VerticalLayout,
    horizontal: HorizontalLayout
};
Layout.generator = function( type ) {
    return generator( Layout.factory, type );
}



// ======== Containers

// -------- Container - base type for containers
function Container() {
    this.resizeHandlers = [];
}
Container.prototype = new Dock();
Container.prototype.layout = function() {
    var layoutElement = $(this.dom).children(".layout");
    if( layoutElement.length != 1 )
        return undefined;
    if( layoutElement[0].ctmDock instanceof Layout )
        return layoutElement[0].ctmDock;
    else
        return undefined;
}
Container.prototype.setLayout = function( options ) {
    var layoutType;
    var hasOptions;
    if( typeof(options) == 'string' ) {
        layoutType = options;
        hasOptions = false;
    }
    else {
        layoutType = options.type;
        hasOptions = true;
    }
    var generator = Layout.generator( layoutType );
    var layout = hasOptions ?   new generator( options ) :   new generator({});
    if( this.master )
        layout.options.master = true;
    $(this.dom).empty().append($(layout.dom));
    var toolSet = options.tools;
    if( toolSet === undefined )
        toolSet = ctmDock.toolSets.free
    if( toolSet instanceof Object )
        layout.header( { tools: ctmDock.toolSets.free, toolhandle: options.toolhandle } );
    return layout;
}
Container.prototype.resize = function() {
    var layout = this.layout();
    if( layout )
        layout.resize();

    // Call resize handlers, if any
    if( this.resizeHandlers instanceof Array )
        for( var idx in this.resizeHandlers )
            this.resizeHandlers[idx].call(this);
}
Container.prototype.addResizeHandler = function( resizeHandler ) {
    this.resizeHandlers.push( resizeHandler )
}
Container.prototype.isRoot = function() {
    return this instanceof RootContainer;
}
Container.prototype.splitter = function( dir ) {
    if( dir == undefined )
        return this.splitter(-1) || this.splitter(1);
    else {
        jc = $(this.dom);
        var js = jc[prevOrNext(dir)]();
        if( js.length == 0   ||   !( js[0].ctmDock instanceof Splitter ) )
            return undefined;
        return js[0].ctmDock;
    }
}
Container.prototype.wsplitter = function() {
    var s = this.splitter.apply( this, arguments );
    return s? $(s.dom): $();
}
Container.prototype.wvisibleSplitter = function( dir ) {
    if( dir == undefined ) {
        var result = this.wvisibleSplitter( -1 );
        if( result.length == 1 )
            return result;
        return this.wvisibleSplitter( 1 );
    }
    var pn = prevOrNext( dir );
    var jns = $(this.dom)[pn]();
    while( jns.is(":hidden") )
        jns = jns[pn]()[pn]();
    if( jns.length == 1   &&   jns[0].ctmDock instanceof Splitter )
        return jns;
    else
        return $();
}
Container.prototype.init = function( options ) {
    this.attach( newDiv( "container" ) );
    this.width = new Range( options.width );
    this.height = new Range( options.height );
    var j = $(this.dom);
    j.width( this.width.min ).height( this.height.min );
    if( options.tools && options.toolhandle !== false)
        this.makeToolHandle( options.tools );
    if( typeof(options.title) == "string" )
        this.title = options.title;
}
Container.prototype.parentLayout = function() {
    var result = this.parentDock();
    if( !result instanceof Layout )
        throw "Parent of a container is not a layout"
    return result;
}
Container.prototype.vertical = function() {
    return !this.parentLayout().vertical;
}
Container.prototype.sizeRange = function( vertical ) {
    var wh = widthOrHeight( !vertical );
    return this[wh];
}
Container.prototype.fixedSize = function( vertical ) {
    var sr = this.sizeRange( vertical );
    return sr.max != undefined && sr.max <= sr.min;
}
Container.prototype.setPos = function( pos ) {
    Dock.prototype.setPos.call( this, pos );
    var j = $(this.dom);
    var jh = j.children(".toolhandle");
    if( jh.length == 1 ) {
        var o = j.offset();
        jh.offset( {
            left: o.left + j.width() - jh.outerWidth(),
            top:  o.top
            } );
    }
}
Container.prototype.makeToolHandle = function( tools ) {
    this.toolHandle = new ToolHandle( this, tools );
    $(this.dom).prepend( $(this.toolHandle.dom) );
}
Container.prototype.setTitle = function( title ) {
    if( typeof(title) == "string" )
        this.title = title;
    else
        this.title = "";
    if( this.containerHandle )
        this.containerHandle.dom.title = title;
}

// -------- Root container - the one that occupies the entire window and that isn't nested into another container
function RootContainer() {
    this.attach( newDiv( "root_container" ) );
    ctmDock.root = this;
    this.init();
}
RootContainer.prototype = new Container();
RootContainer.prototype.init = function() {
    var root = $("#root");
    if( root.length == 1 )
        root.replaceWith( $(ctmDock.root.dom) );
    else
        $("body").empty().append( $(ctmDock.root.dom) );
    $(window).resize( RootContainer.resize );
    RootContainer.resize();
}
RootContainer.resize = function() {
    $(ctmDock.root.dom).height( window.innerHeight );
    var layout = ctmDock.root.layout();
    if( layout )
        layout.resize();
}

// -------- Normal container (can be fixed or variable size in either direction)
function NormalContainer( options ) {
    if( arguments.length == 0 )
        return;
    this.init( options );
    $(this.dom).addClass("normal_container");
}
NormalContainer.prototype = new Container();
NormalContainer.prototype.header = function() {
    return this.parentLayout().header();
}
NormalContainer.prototype.index = function() {
    var result = 1;
    var containers = this.parentLayout().containers();
    var i; 
    for( i=0; i<containers.length; ++i )
        if( containers[i] == this.dom )
            return i + 1;
    return -1;
}
NormalContainer.prototype.split = function( options ) {
    // Save all current content in a var
    var content = $(this.dom).contents();

    var layout = this.layout();
    var master = layout ?   layout.options.master :   this.master;

    // Create new layout
    var generator = Layout.generator( orientation( options.vertical ) );
    var newLayout = new generator( {} );

    // Add layout to the container
    $(this.dom).append($(newLayout.dom));
    
    // Add header to new layout, if necessary
    if( ctmDock.toolSets.free )
        newLayout.header( { tools: ctmDock.toolSets.free } );

    // Create new container in the new layout
    // Note: intentionally don't set noresize option, that leads to incorrect resizing later
    var subc = newLayout.add( { toolhandle: false, noresize: true } );
    
    // Move all contents to the new container
    var th = content.filter(".toolhandle");
    if( th.length == 1 )
        th[0].ctmDock.container = subc;
    $(subc.dom).append( content );
    if( typeof(this.title) == "string" ) {
        subc.setTitle( this.title );
        this.setTitle( "" );
    }

    // Add another new container to the new layout
    newContainerOptions = {};
    if( ctmDock.toolSets.free )
        newContainerOptions.toolhandle = true;
    if( options.addNewBefore )
        newContainerOptions.before = subc;
    newLayout.add( newContainerOptions );

    // manage master layout state
    if( master ) {
        newLayout.options.master = true;
        if( layout )
            layout.options.master = false;
    }

    // Reduce new layout, if not asked not to do it
    if( !options.noreduce )
        newLayout.reduce();
}
NormalContainer.prototype.clear = function() {
    $(this.dom).empty();
    this.setTitle( "" );
    if( ctmDock.toolSets.free ) {
        // Create new tool handle in the container
        this.makeToolHandle( ctmDock.toolSets.free );
        this.setPos( {} );   // Do this to position tool handle
        this.parentLayout().updateContainerIndices();
    }
}

// -------- Header container - for storing minimized containers of the parent layout
function HeaderContainer( options ) {
    var hdrOptions = { width: 20, height: 20 };
    if( options instanceof Object ) {
        hdrOptions.tools = options.tools;
        hdrOptions.toolhandle = options.toolhandle;
    }
    this.init( hdrOptions );
    $(this.dom)
        .addClass( "header_container" )
        .append( $(newDiv( "buttonphantom" )) );
}
HeaderContainer.prototype = new Container();
HeaderContainer.prototype.register = function( container ) {
    if( !container.containerHandle )
        container.containerHandle = new ContainerHandle( container );
    var j = $(this.dom);
    var jhh = j.children(".containerhandleholder");
    if( jhh.length == 0 ) {
        jhh = $(newDiv( [ "containerhandleholder", orientation(this.vertical()) ] ));
        j.append(jhh);
    }
    var jch = jhh.children(".containerhandle");
    var i = container.index() - 1;
    if( i >= jch.length )
        jhh.append($(container.containerHandle.dom));
    else
        $(container.containerHandle.dom).insertBefore( $(jch[i]) );
}
HeaderContainer.prototype.unregister = function( container ) {
    $(container.containerHandle.dom).remove();
    container.containerHandle = undefined;
}
HeaderContainer.prototype.minimize = function( container, recursiveCall ) {
    if( !recursiveCall   &&   this.parentLayout().options.tabbed )
        return;
    if( !container.containerHandle ) {
        this.register( container );
        this.parentLayout().updateContainerIndices();
    }
    // Change container size in order it to be restored the same size
    var layout = container.parentLayout();
    if( !container.fixedSize(layout.vertical) ) {
        var wh = widthOrHeight( !layout.vertical );
        var si = layout.sizeInfo();
        var s = $(container.dom)[wh]();
        if( si.nr > 1   &&   si.size > s ) {
            s *= si.size / ( si.size - s );
            $(container.dom)[wh]( s );
        }
    }
    $(container.dom).add(container.wvisibleSplitter()).hide();
    container.containerHandle.update();
    this.parentLayout().resize();
}
HeaderContainer.prototype.restore = function( container, recursiveCall ) {
    // Decide whether we should restore a splitter and which one of the two
    var layout = this.parentLayout();
    var thisHeader = this;
    if( !recursiveCall   &&   layout.options.tabbed )
        layout.containers().filter(":visible").each(function() {
            thisHeader.minimize( this.ctmDock, true );
        });
    if( !container.containerHandle ) {
        this.register( container );
        this.parentLayout().updateContainerIndices();
    }
    var jc = $(container.dom);
    var js = $();
    for( var dir=-1; dir<2; dir+=2 ) {
        var pn = prevOrNext( dir );
        var j = jc[pn]();
        while( j.length == 1   &&   ( j[0].ctmDock instanceof Splitter ) ) {
            if( !j.is(":hidden") )
                break;              // Visible splitter - no need to set visible
            var jcn = j[pn]();
            if( jcn.length == 0   ||   ( jcn[0].ctmDock instanceof HeaderContainer ) )
                break;              // No more 'real' containers
            if( !jcn.is(":hidden") ) {
                js = js.add( j );   // This splitter should be made visible
                break;
            }
            j = jcn[pn]();          // Skip hidden container
        }
    }
    jc.add(js).show();
    container.containerHandle.update();
    layout.resize();
}
HeaderContainer.prototype.toggle = function( container ) {
    if( $(container.dom).is(":visible") )
        this.minimize( container );
    else
        this.restore( container );
}
HeaderContainer.prototype.makeToolHandle = function( tools ) {
    this.headerToolHandle = new HeaderToolHandle( this, tools );
    $(this.dom).append( $(this.headerToolHandle.dom) );
}



// ======== Splitter

function Splitter( vertical, draggable ) {
    this.attach( newDiv( ["splitter", orientation(vertical)] ) );
    this.vertical = vertical;
    if( draggable ) {
        var this_ = this;
        $(this.dom)
            .mousedown(function(ev) {
                ev.preventDefault();
                DragHelper.start( ev, Splitter.prototype.move, this_ );
            })
            .dblclick(function(ev) {
                var splitter = this.ctmDock;
                if( splitter.parentDock().options.fixed )
                    return;
                var layout = splitter.parentDock();
                var options = {
                    toolhandle: true,
                    before: splitter.wvisibleContainer(1)[0].ctmDock
                };
                layout.add( options );
            });
        
    }
    else
        $(this.dom).hide();
}
Splitter.prototype = new Dock();
Splitter.prototype.wcontainer = function( dir ) {
    var pn = prevOrNext( dir );
    var jnc = $(this.dom)[pn]();
    return jnc;
}
Splitter.prototype.wvisibleContainer = function( dir ) {
    var pn = prevOrNext( dir );
    var jnc = $(this.dom)[pn]();
    while( jnc.is(":hidden") )
        jnc = jnc[pn]()[pn]();
    return jnc;
}
Splitter.prototype.tryMove = function( delta, dir, moveContainers, moveThisSplitter ) {
    var wh = widthOrHeight( this.vertical );
    
    // Obtain prev/next visible container
    var jnc = this.wvisibleContainer( dir );
    var dnc = jnc[0].ctmDock;

    // Compute new valid container size
    var csize = jnc[wh]();
    var csizeNew = dnc[wh].validate( csize - delta*dir );
    
    // Compute limited delta allowed by new size
    var deltaThis = (csize - csizeNew)*dir;
    
    // Probably remaining delta can be realized by moving prev/next splitters
    var deltaRemaining = delta - deltaThis;
    if( moveContainers ) {
        // Resize and move container and splitter
        jnc[wh]( csizeNew );
        if( dir > 0 )
            dnc.moveBy( deltaThis, !this.vertical );
        if( moveThisSplitter )
            this.moveBy( deltaThis, !this.vertical );
    }
    if( deltaRemaining == 0 )
        // Done
        return delta;
    else {
        // Try to move prev/next visible splitter in order to let this splitter move by deltaRemaining
        var jns = dnc.wvisibleSplitter( dir );
        if( jns.length != 1 )
            // No more visible splitters
            return delta - deltaRemaining;
        var delta2 = jns[0].ctmDock.tryMove( deltaRemaining, dir, moveContainers, true );
        if( moveContainers ) {
            // Adjust container and splitter position
            dnc.moveBy( delta2, !this.vertical );
            if( moveThisSplitter )
                this.moveBy( delta2, !this.vertical );
            }
        // Return how much we could advance splitter
        return (delta - deltaRemaining) + delta2;
    }
}
Splitter.prototype.move = function( stop ) {
    // Compute splitter delta
    var xy = xOrY(this.vertical);
    var delta = DragHelper.mdelta[xy];
    if( delta == 0 )
        return;
        
    // Compute delta satisfying container size constraints
    var d1 = this.tryMove( delta, -1, false, false );
    var d2 = this.tryMove( delta,  1, false, false );
    delta = Math.abs(d1) < Math.abs(d2) ?   d1 :   d2;
    if( delta == 0 )
        return;
        
    // Move splitter and affected containers
    this.tryMove( delta, -1, true, false );
    this.tryMove( delta,  1, true, true );
    
    // Resize the entire layout containing this splitter
    this.parentDock().resize();
}



// ======== Toolset support

// -------- General tool
function Tool( options ) {
    if( !( options instanceof Object ) )
        return;
    this.title = options.title;
    this.src = options.src;
    this.alt = options.alt;
    this.handler = options.handler;
    this.updateUi = options.updateUi;
}
Tool.prototype.makeUi = function() {
    var ui = newImg( "tool" )
    ui.title = this.title;
    ui.src = this.src;
    ui.alt = this.alt;
    return ui;
}

function MenuTool( options ) {
    if( !( options instanceof Object ) )
        return;
    Tool.apply( this, arguments );
    this.toolSet = options.toolSet;
}
MenuTool.prototype = new Tool();
MenuTool.prototype.makeUi = function() {
    var toolHandle = new MenuToolHandle( this.toolSet );
    var j = $(toolHandle.dom);
    if( this.src ) {
        var img = newImg("tool");
        img.src = this.src;
        img.alt = this.alt;
        img.title = this.title;
        j.append( $(img) );
        j.width(20).height(20); // TODO better
        }
    else {
        j.addClass("tool");
        j[0].title = this.title;
    }
    return toolHandle.dom;
}


// -------- Tool handle
function ToolHandle( container, toolSet ) {
    if( !toolSet )
        return;
    this.attach( newDiv("toolhandle") );
    this.container = container;
    this.toolSet = toolSet;
    $(this.dom).hover( this.onEnter, this.onLeave );
}
ToolHandle.prototype = new Dock();
ToolHandle.prototype.target = function() {
    return this.container;
}
ToolHandle.prototype.onEnter = function() {
    if( this.ctmDock.toolSet.activate )
        this.ctmDock.toolSet.activate( this.ctmDock );
}
ToolHandle.prototype.onLeave = function() {
    var thisToolHandle = this.ctmDock;
    setTimeout(
        function() {
            var t = thisToolHandle.toolSet;
            if( t.deactivate   &&   t.cachedContext   &&   !$(t.dom).is(":hover") )
                t.deactivate();
        },
        300 );
}
ToolHandle.prototype.positionPopupToolSet = function( toolSet ) {
    var j = $(this.dom);
    var t = $(toolSet.dom)
    var pos = j.offset();
    pos.left += j.outerWidth() - t.outerWidth();
    var maxLeft = window.innerWidth-1 - t.outerWidth();
    if( pos.left > maxLeft )
        pos.left = maxLeft;
    t.show();
    t.offset( pos );
}

// -------- Tool handle for header container
function HeaderToolHandle() {
    ToolHandle.apply( this, arguments );
}
HeaderToolHandle.prototype = new ToolHandle();
HeaderToolHandle.prototype.target = function() {
    return this.container.parentLayout().parentDock();
}
HeaderToolHandle.prototype.positionPopupToolSet = function( toolSet ) {
    var j = $(this.dom);
    var t = $(toolSet.dom);
    var pos = j.offset();
    var layout = j.parent()[0].ctmDock.parentLayout();
    if( layout.vertical ) {
        pos.left += j.outerWidth() - t.outerWidth();
        pos.top += j.outerHeight() - t.outerHeight();
    }
    var maxLeft = window.innerWidth-1 - t.outerWidth();
    if( pos.left > maxLeft )
        pos.left = maxLeft;
    t.show();
    t.offset(pos);
}

// -------- Tool handle for menu
function MenuToolHandle() {
    ToolHandle.apply( this, [undefined, arguments[0]] );
}
MenuToolHandle.prototype = new ToolHandle();
MenuToolHandle.prototype.target = function() {
    return this.parentDock().toolHandle.target();
}
MenuToolHandle.prototype.positionPopupToolSet = function( toolSet ) {
    var parentToolSet = this.parentDock();
    var parentVertical = $(parentToolSet.dom).is(".v");
    var j = $(this.dom);
        // Turns out that the div might have wrong size and position,
        // so better rely on nested image!
        j = j.children("img");
    var t = $(toolSet.dom);
    var pos = j.offset();
    var z = $(parentToolSet.dom).css("z-index");
    ++z;
    t.css( { zIndex: z } );
    var jw = j.outerWidth(),   jh = j.outerHeight();
    var tw = t.outerWidth(),   th = t.outerHeight();
    if( parentVertical ) {
        pos.top -= 1;
        if( pos.left + jw + tw < window.innerWidth )
            pos.left += jw;
        else
            pos.left -= tw;
        }
    else {
        pos.left -= 1;
        if( pos.top + jh + th < window.innerHeight )
            pos.top += jh;
        else
            pos.top -= th;
        }
    t.show();
    t.offset(pos);
}

// -------- General tool set
function ToolSet( options ) {
    if( !( options instanceof Object ) )
        return;
    this.attach( newDiv() );
    this.toolHandle = this;
    this.container = options.container;
    this.addTools( options );
}
ToolSet.prototype = new Dock();
ToolSet.prototype.target = function() {
    return this.container;
}
ToolSet.prototype.addTools = function( options ) {
    if( !( options instanceof Object ) )
        throw "ToolSet.prototype.addTools failed: Unexpected type of options argument";
    var tools = options instanceof Array? options: options.tools;
    for( i in tools )
        this.add( tools[i] );
}
ToolSet.prototype.add = function( tool ) {
    var ui = $(tool.makeUi());
    if( !( ui[0].ctmDock instanceof Object ) )
        ui[0].ctmDock = {};
    ui[0].ctmDock.tool = tool;
    ui[0].ctmDock.parentToolSet = this;
    $(this.dom).append( ui );
    var thisToolSet = this;
    if( tool.handler instanceof Function )
        ui.click(function() {
            var context = thisToolSet.cachedContext;
            tool.handler.call( context, this );
            thisToolSet.deactivate( tool, context, this );
        });
}
ToolSet.prototype.highlightTarget = function( toolHandle ) {
    var container = toolHandle.target();
    var j = $(container.dom);
    if( container.containerHandle )
        j = j.add( $(container.containerHandle.dom) );
    j.each(function() { this.ctmDock.highlight(); } );
    this.highlighted = j;
}
ToolSet.prototype.unhighlightTarget = function() {
    if( this.highlighted ) {
        this.highlighted.each(function() { this.ctmDock.unhighlight(); } );
        this.highlighted = undefined;
    }
}
ToolSet.prototype.activate = function( toolHandle ) {
    this.cachedContext = toolHandle.target();
}
ToolSet.prototype.deactivate = function( tool, context, ui ) {
    if( tool.updateUi instanceof Function )
        tool.updateUi.call( context, ui );
    this.cachedContext = undefined;
}

// -------- Popup tool set
function PopupToolSet( options ) {
    if( !( options instanceof Object ) )
        return;
    this.attach( newDiv(["tools", options.vertical? "v": "h"]) );
    this.vertical = options.vertical;
    var j = $(this.dom);
    $("body").append( j );
    j.hover( function() {}, function() { this.ctmDock.deactivate(); } );
    this.addTools( options );
}
PopupToolSet.prototype = new ToolSet();
PopupToolSet.prototype.calcSize = function() {
    var length = 0;
    var lng = outerWidthOrHeight( !this.vertical );
    $(this.dom).children().each(function() {
        var j = $(this);
        if( j.css("display") == "none" )
            return;
        length += j[lng]();
    });
    length += 2;  // border (?)
    var j = $(this.dom);
    j[lng]( length + 2 );
}
PopupToolSet.prototype.activate = function( toolHandle ) {
    if( this.cachedContext )
        // Don't allow multiple activation
        return;
    this.toolHandle = toolHandle;
    this.highlightTarget( toolHandle );
    var context = this.cachedContext = toolHandle.target();
    $(this.dom).children().each(function() {
        var tool = this.ctmDock.tool;
        if( tool instanceof Tool   &&   tool.updateUi instanceof Function )
            tool.updateUi.call( context, this );
    });
    this.calcSize();
    toolHandle.positionPopupToolSet( this );
    $(this.dom).show();
}
PopupToolSet.prototype.deactivate = function( toolHandle ) {
    this.unhighlightTarget();
    this.toolHandle = undefined;
    $(this.dom).hide();
    this.cachedContext = undefined;
}

// -------- Tools

Tools = {};
Tools.closeContainer = function() {
    var layout = this.parentLayout();
    layout.remove( this );
    layout.reduce();
}
Tools.closeContainer.updateUi = function( ui ) {
    var show = this.master !== true;
    if( show ) {
        var layout = this.parentDock();
        show = layout instanceof Layout   &&   !layout.options.fixed;
    }
    $(ui)[show? "show": "hide"]();
}
Tools.minimizeContainer = function() {
    this.header().minimize( this );
}
Tools.minimizeContainer.updateUi = function( ui ) {
    var show = this.master !== true;
    if( show ) {
        var layout = this.parentDock();
        show = layout instanceof Layout;
    }
    $(ui)[show? "show": "hide"]();
}
Tools.splitContainer = function( ui, options ) {
    var sender = ui.ctmDock.parentToolSet.toolHandle.parentDock();
    var layout = this.layout();
    if( sender instanceof HeaderContainer   &&
        !layout.options.fixed   &&
        layout.vertical == options.vertical )
    {
        // Don't really split, add container instead
        var containerOptions = { toolhandle: true };
        if( options.addNewBefore ) {
            var c = layout.containers();
            if( c.length > 0 )
                containerOptions.before = c[0].ctmDock;
        }
        layout.add( containerOptions );
    }
    else
        // Split container
        this.split( options );
}
Tools.splitContainer.updateUi = function( ui ) {
    var layout = this.parentDock();
    var show = layout instanceof Layout   &&   !layout.options.fixed;
    $(ui)[show? "show": "hide"]();
}
Tools.toggleTabbedView = function( ui ) {
    var layout = this.layout();
    layout.toggleTabbed();
}
Tools.toggleTabbedView.updateUi = function( ui ) {
    var layout = this.layout();
    if( layout ) {
        $(ui).show();
        ui.src = layout.options.tabbed ?   "css/splitview.png" :   "css/tabview.png";
    }
    else
        $(ui).hide();
}

ctmDock.tools = {
    splitVB: new Tool( {
        title: "Add new split on the top",
        src: "css/layout-split-vb.png",
        alt: "vertical split at start",
        handler: function( ui ) { Tools.splitContainer.call( this, ui, { vertical: true, addNewBefore: true } ); },
        updateUi: Tools.splitContainer.updateUi
        } ),
    splitVE: new Tool( {
        title: "Add new split on the bottom",
        src: "css/layout-split-ve.png",
        alt: "vertical split at end",
        handler: function( ui ) { Tools.splitContainer.call( this, ui, { vertical: true, addNewBefore: false } ); },
        updateUi: Tools.splitContainer.updateUi
        } ),
    splitHB: new Tool( {
        title: "Add new split on the left",
        src: "css/layout-split-hb.png",
        alt: "horizontal split at start",
        handler: function( ui ) { Tools.splitContainer.call( this, ui, { vertical: false, addNewBefore: true } ); },
        updateUi: Tools.splitContainer.updateUi
        } ),
    splitHE: new Tool( {
        title: "Add new split on the right",
        src: "css/layout-split-he.png",
        alt: "horizontal split at end",
        handler: function( ui ) { Tools.splitContainer.call( this, ui, { vertical: false, addNewBefore: false } ); },
        updateUi: Tools.splitContainer.updateUi
        } ),

    closeContainer: new Tool( {
        title: "Close container",
        src: "css/close.png",
        alt: "close",
        handler: Tools.closeContainer,
        updateUi: Tools.closeContainer.updateUi
        } ),
    minimizeContainer: new Tool( {
        title: "Minimize container",
        src: "css/minimize.png",
        alt: "minimize",
        handler: Tools.minimizeContainer,
        updateUi: Tools.minimizeContainer.updateUi
        } ),
    toggleTabbedView: new Tool( {
        title: "Toggle tabbed / split view",
        src: "css/splitview.png",
        alt: "tabbed/split",
        handler: Tools.toggleTabbedView,
        updateUi: Tools.toggleTabbedView.updateUi
        } )
};

    

function ContainerHandle( container ) {
    this.attach( newDiv("containerhandle") );
    this.container = container;
    $(this.dom)
        .click( ContainerHandle.click )
        .hover( ContainerHandle.highlightTarget, ContainerHandle.unhighlightTarget );
    if( typeof(container.title) == "string" )
        this.dom.title = container.title;
    this.update();
}
ContainerHandle.prototype = new Dock();
ContainerHandle.prototype.update = function() {
    var j = $(this.dom);
    // Set handle icon
    if( $(this.container.dom).is(":visible") )
        j.addClass("minimize").removeClass("maximize");
    else
        j.addClass("maximize").removeClass("minimize");
}
ContainerHandle.click = function() {
    this.ctmDock.container.header().toggle( this.ctmDock.container );
}
ContainerHandle.highlightTarget = function() {
    this.ctmDock.container.highlight();
}
ContainerHandle.unhighlightTarget = function() {
    this.ctmDock.container.unhighlight();
}

// Initialization. Note: creating popup toolsets requires
// addition of elements to body, so we have to create
// toolsets after document is ready.
$(document).ready(function() {
    var t = ctmDock.tools;
    
    ctmDock.toolSets = {};

    ctmDock.toolSets.sourceMenuTools = new PopupToolSet( {
        tools: [
            new Tool( {
                title: "Clear",
                src: "pix/clear.png",
                alt: "clear",
                handler: function() { this.clear(); }
            } )
        ],
        vertical: true
    } );

    t.sourceMenu = new MenuTool( {
        title: "Select source",
        src: "pix/menu.png",
        alt: "select source",
        toolSet: ctmDock.toolSets.sourceMenuTools,
        updateUi: Tools.splitContainer.updateUi
    } );

    ctmDock.toolSets.free = new PopupToolSet( [
        t.splitVB,
        t.splitVE,
        t.splitHB,
        t.splitHE,
        t.toggleTabbedView,
        t.minimizeContainer,
        t.sourceMenu,
        t.closeContainer
    ] );
});




// ======== Exports

ctmDock.makeRoot = function() {
    return new RootContainer();
};
ctmDock.newToolSet = function( options ) {
    return new ToolSet( options );
}
ctmDock.newPopupToolSet = function( options ) {
    return new PopupToolSet( options );
}
ctmDock.newTool = function( options ) {
    return new Tool( options );
}
ctmDock.newMenu = function( options ) {
    return new MenuTool( options );
}
;(function() {
    var id = 0;
    ctmDock.newId = function() {
        ++id;
        return "id_" + id;
    }
})();

})();
