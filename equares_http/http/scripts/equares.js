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
    $(c1.dom).addClass("myheader").html("<h1>Equares server</h1>");
    $(c2.dom).addClass("mymain");
    $(c3.dom).addClass("myfooter").html('<a target="_blank" href="http://ctmech.ru/">Computer Technologies in Engineering</a>');
    rootLayout.resize();    // Because borders have changed
});
