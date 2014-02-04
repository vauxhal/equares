$(document).ready(function() {
    var rootLayout = ctmDock.makeRoot().setLayout( {
        type: "vertical",
        tools: false
    } );
    var c1, c2, c3;
    c1 = rootLayout.add( {height: 35, nosplit: true} );
    c2 = rootLayout.add( {nosplit: true} );
    c2.master = true;
    c3 = rootLayout.add( {height: 15, nosplit: true} );
    var header = $(c1.dom)
    header.attr("id", "header").html('<div id="projectname">Equares</div>');

    function wrap(tag) { return $("<" + tag + "></" + tag + ">") }
    function createMenu(root, items) {
        var m = wrap("ul").appendTo(root)
        function h0() {}
        for (var i=0; i<items.length; ++i) {
            var item = items[i]
            var li = wrap("li").appendTo(m)
            var a = wrap("a").html(item.text).attr("href", "#").appendTo(li)
            ;(function(){
                var h = item.handler instanceof Function? item.handler: h0
                a.click(function(e) {
                    e.preventDefault()
                    h()
                })
            })()
            if (item.menu)
                createMenu(li, item.menu)
        }
        return m
    }
    createMenu(wrap("div").appendTo(header), [
        {text: "Layout", menu: [
            {text: "Console", handler: function() { equaresui.setConsoleSource.call(c2) }},
            {text: "Source", handler: function() { equaresui.setWorkbenchSource.call(c2) }},
            {text: "Scheme", handler: function() { equaresui.setSceneSource.call(c2) }}
        ]},
        {text: "Scheme", menu: [
            {text: "Open ...", handler: function() { equaresui.openScheme() } },
            {text: "Save as...", handler: function() { equaresui.saveScheme() } },
            {text: "Run", handler: function() { equaresui.runScheme() } }
        ]}
    ]).attr("id", "mainmenu").menu({
        position: {
        my:'left top',
        at:'left bottom'
        }
    })

    $(c2.dom).addClass("mymain");
    $(c3.dom).addClass("myfooter").html('<a target="_blank" href="http://ctmech.ru/">Computer Technologies in Engineering</a>');
    rootLayout.resize();    // Because borders have changed

    // Set scene source by default
    equaresui.setSceneSource.call(c2)
});
