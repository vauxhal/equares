var equaresui = {};

(function() {

equaresui.setConsoleSource = function() {
    var layoutOptions = {
        type: "vertical",
        fixed: true
    };
    this.setTitle( "equares console" );
    var layout = this.setLayout( layoutOptions );
    var c1 = layout.add( { title: "Output" } );
    var equaresOutput = $(c1.dom)
        .append('<div class="equaresio"></div>')
        .children("div.equaresio");
    var c2 = layout.add( { title: "History" } );
    var equaresInput = $(c2.dom)
        .append('<div class="equaresio"></div>')
        .children("div.equaresio");
    var c3 = layout.add( { title: "Command line", height: 25 } );
    $(c3.dom).append(
        '<form class="equaresCmdForm"><span>&nbsp;Command:</span>&nbsp;<input type="text" class="equaresCmd" />&nbsp;<input type="submit" class="equaresSendCmd" value="Execute" /></form>' +
        '&nbsp;<span class="equaresDebug"></span>' +
        '<div class="equaresStat">Status: <span class="equaresStat"></span>&nbsp;<input type="button" value="Start/stop" class="equaresToggle" /><div class="buttonphantom"></div></div>');
    var equaresCmdForm = $(c3.dom).find("form.equaresCmdForm");
    var equaresCmd = $(c3.dom).find("input.equaresCmd");
    var equaresSendCmd = $(c3.dom).find("input.equaresSendCmd");
    var equaresDebug = $(c3.dom).find("span.equaresDebug");
    var equaresStatElement = $(c3.dom).find("span.equaresStat");
    var equaresToggle = $(c3.dom).find("input.equaresToggle");

    var equaresRunning = false;

    function equaresStat() {
        $.ajax("equaresStat.cmd")
            .done(function(msg){
                var running = parseInt(msg) != 0;
                equaresRunning = running;
                equaresStatElement.html(running? "Running": "Not running");
            })
            .fail(function(){
                equaresStatElement.html("Ajax error");
            });
    }

    function setupEvents()
    {
        if(typeof(EventSource)!=="undefined")
        {
            var equaresStatEvent = new EventSource("equaresStatEvent.cmd");
            var wasRunning;
            equaresStatEvent.onmessage = function(event) {
                var running = parseInt(event.data) != 0;
                equaresRunning = running;
                equaresStatElement.html(running? "Running": "Not running");
                if( !running )
                    equaresOutput.html("");
                if( running && wasRunning === false )
                    equaresInput.html("");
                wasRunning = running;
            };

            var equaresOutputEvent =new EventSource("equaresOutputEvent.cmd");
            equaresOutputEvent.onmessage=function(event) {
                var output = equaresOutput;
                var data = JSON.parse(event.data);
                var str = data.text
                    .replace(/ /g, "&nbsp;")
                    .replace(/\</g, "&lt;")
                    .replace(/\>/g, "&gt;");
                if (data.stream == 0)
                    str = '<span style="color: green;">' + str + '</span>';
                else if (data.stream == 2)
                    str = '<span style="color: red;">' + str + '</span>';
                output.append(str + "<br/>");
                output.scrollTop(output[0].scrollHeight);
                if( data.stream == 0 ) {
                    equaresInput.append(str + "<br/>");
                    equaresInput.scrollTop(output[0].scrollHeight);
                }
            }
        }
        else
            equaresStatElement.html("Event sources are not supported");
    }


    equaresStat();
    setupEvents();
    equaresToggle.click(function() {
        $.ajax("equaresToggle.cmd")
            //.done(function(){})
            .fail(function(){
                equaresDebug.html("equaresToggle.cmd: Ajax error");
            });
    });
    equaresSendCmd.click(function() {
        var cmd = equaresCmd[0].value;
        if (cmd.length == 0) {
            alert("Please enter command");
            equaresCmd.focus();
            return;
        }
        if (!equaresRunning) {
            alert("Please start equares first");
            return;
        }
        equaresCmd[0].value = "";
        $.ajax("equaresExec.cmd", {data: {cmd: cmd}, type: "GET"})
            //.done(function(){})
            .fail(function(){
                equaresDebug.html("equaresExec.cmd: Ajax error");
        });
    });
    equaresCmdForm.submit(function(event) {
        return false;
    });
}

equaresui.setWorkbenchSource = function() {
    this.clear();
    this.setTitle( "equares workbench" );
    var layoutOptions = {
        type: "vertical",
        fixed: true
    };
    var masterLayout = this.setLayout( layoutOptions );
    var cControls = masterLayout.add( { title: "Controls", height: 25 } );
    var cIO = masterLayout.add( { title: "Input and output" } );

    layoutOptions = {
        type: "horizontal",
        fixed: true
    };
    $(cControls.dom).append(
        '<form class="equaresCmdForm"><input type="submit" value="Start/Stop" id="toggle" />&nbsp;<input type="button" value="pause" id="pause" /></form>' +
        '&nbsp;<span class="equaresDebug"></span>');
    var layout = cIO.setLayout( layoutOptions );
    var c1 = layout.add( { title: "Input" } );
    var equaresInput = $(c1.dom)
        .append('<div class="equaresio"></div>')
        .children("div.equaresio");
    equaresInput[0].contentEditable = true;
    var c2 = layout.add( { title: "Output" } );
    var equaresOutput = $(c2.dom)
        .append('<div class="equareso1"></div>')
        .children("div.equareso1");
    var equaresOutput2 = $(c2.dom)
        .append('<div class="equareso2">Output data will be placed here</div>')
        .children("div.equareso2");

    var equaresForm = $(cControls.dom).find("form.equaresCmdForm");
    var equaresToggle = $(cControls.dom).find("input#toggle");
    var equaresPause = $(cControls.dom).find("input#pause");
    var equaresDebug = $(cControls.dom).find("span.equaresDebug");

    equaresForm.submit(function(event) {
        return false;
    });

    var equaresRunning = false;

    function equaresStat() {
        $.ajax("equaresStat.cmd")
            .done(function(msg){
                var running = parseInt(msg) != 0;
                equaresRunning = running;
                equaresToggle[0].value = (running? "Stop": "Start");
            })
            .fail(function(){
                equaresToggle[0].value = "Run/stop";
                equaresDebug.html("equaresStat(): Ajax error");
            });
    }

    function setupEvents()
    {
        if(typeof(EventSource)!=="undefined")
        {
            var equaresStatEvent = new EventSource("equaresStatEvent.cmd");
            equaresStatEvent.onmessage = function(event) {
                var running = parseInt(event.data) != 0;
                equaresRunning = running;
                equaresToggle[0].value = (running? "Stop": "Start");
            };

            var equaresOutputEvent =new EventSource("equaresOutputEvent.cmd");
            var rxFile = /^==([0-9])+==\> file: (.*)/,
                rxSync = /^==([0-9])+==\> sync/;
            equaresOutputEvent.onmessage=function(event) {
                var output = equaresOutput;
                var data = JSON.parse(event.data);
                var rxResult = rxFile.exec(data.text);
                if (rxResult && rxResult.length > 2) {
                    var fileName = rxResult[2] + "#" + new Date().getTime();
                    equaresOutput2.html('<img src="' + fileName + '" />')
                }
                else {
                    rxResult = rxSync.exec(data.text);
                    if (rxResult && rxResult.length > 1) {
                        var threadId = rxResult[1];
                        $.ajax("equaresExec.cmd", {data: {cmd: "==" + threadId + "==<"}, type: "GET"})
                    }
                }
                var str = data.text
                    .replace(/ /g, "&nbsp;")
                    .replace(/\</g, "&lt;")
                    .replace(/\>/g, "&gt;");
                if (data.stream == 0)
                    str = '<span style="color: green;">' + str + '</span>';
                else if (data.stream == 2)
                    str = '<span style="color: red;">' + str + '</span>';
                output.append(str + "<br/>");
                output.scrollTop(output[0].scrollHeight);
            }
        }
        else
            equaresDebug.html("Event sources are not supported");
    }

    equaresStat();
    setupEvents();
    equaresToggle.click(function() {
        var cmd = equaresInput.html();
        cmd = cmd
            .replace(/<\/div>/g, "\n")
            .replace(/<br *\/?>/g, "\n")
            .replace(/<div>/g, "")
            .replace(/&nbsp;/g, " ")
            .replace(/&lt;/g, "<")
            .replace(/&gt;/g, ">")
            ;
        if (!equaresRunning   &&   cmd.length === 0) {
            alert("Please provide some input");
            equaresInput.focus();
            return;
        }
        var wasRunning = equaresRunning;
        $.ajax("equaresToggle.cmd")
            .done(function(){
                if (!wasRunning) {
                    equaresOutput.html("");
                    $.ajax("equaresExec.cmd", {data: {cmd: "===={\n" + cmd + "\n====}"}, type: "GET"})
                        //.done(function(){})
                        .fail(function(){
                            equaresDebug.html("equaresExec.cmd: Ajax error");
                        });
                }
            })
            .fail(function(){
                equaresDebug.html("equaresToggle.cmd: Ajax error");
            });
    });
}

equaresui.setSceneSource = function() {
    this.clear();
    var layoutOptions = {
        type: "horizontal",
        fixed: true
    };
    this.setTitle( "equares scheme editor" );
    var layout = this.setLayout( layoutOptions );
    
    var boxCell = layout.add( { title: "Boxes", width: {min: 50, max: 300} } );
    var boxDiv = $(boxCell.dom);
    var boxHelp = $("body").append('<div id="scheme-boxhelp"></div>').children("#scheme-boxhelp").hide();
    boxDiv.addClass('scheme-boxlist');
    var draggingBox = false;

    var equaresInfo = function(request, callback) {
        $.ajax("equaresRequestInfo.cmd", {data: {cmd: request}, type: "GET"})
            .done(function(data) {
                var reply = JSON.parse(data);
                if (reply.error)
                    alert("equaresRequestInfo.cmd error: \n" + reply.stderr);
                else
                    callback(eval("(function(){return " + reply.stdout + "})()"));
            })
            .fail(function() {
                // equaresDebug.html("equaresExec.cmd: Ajax error");
                alert("equaresRequestInfo.cmd: Ajax error");
            });
    }

    var boxItemHelpText = function(item, caption) {
        if (item === undefined)
            return "";
        var text = "<h2>" + caption + "</h2>";
        if (item.length == 0)
            text += "None."
        else {
            text += "<ul>";
            for (var index in item) {
                var p = item[index];
                text += "<li>"
                text += "<b>" + p.name + "</b>"
                if (p.help)
                    text += "<br/>" + p.help;
                if (p.format) {
                    text += "<br/>Format: ";
                    var count = 1;
                    for (var i=0; i<p.format.length; ++i) {
                        if (i > 0)
                            text += " x ";
                        text += p.format[i];
                        count *= p.format[i];
                    }
                    text += count == 1? " element": " elements";
                }
                if (p.hints) {
                    text += "<br/>items: ";
                    for (var i=0; i<p.hints.length; ++i) {
                        if (i > 0)
                            text += ", ";
                        text += "<i>" + p.hints[i] + "</i>";
                    }
                }
                text += "</li>"
            }
            text += "</ul>"
        }
        return text;
    }

    var boxHelpText = function(box, info)
    {
        var text = "<h1>" + box + "</h1>";
        text += (info.help? info.help: "No help available") + "<br>";
        text += boxItemHelpText(info.inputs, "Input ports");
        text += boxItemHelpText(info.outputs, "Output ports");
        text += boxItemHelpText(info.properties, "Fixed parameters");
        return text;
    }

    var setBoxes = function(boxes)
    {
        var text = '<h1>Boxes</h1><table>';
        for (var i in boxes)
            text += '<tr><td class="scheme-boxlist-box">' + boxes[i] + '</td><td><img src="question.png"></td></tr>';
        text += '</table>';
        boxDiv.append(text);
        boxDiv.find('tr:odd').addClass('odd');
        boxDiv.find('tr:even').addClass('even');
        var hideBoxHelp
        boxDiv.find('td').find('img')
            .hover(function() {
                if (draggingBox)
                    return;
                var o = $(this);
                var pos = o.offset();
                pos.left += o.width();
                var box = o.parent().prev().text();
                equaresInfo(box, function(info) {
                    boxHelp
                        .html(boxHelpText(box, info));
                    var wh = $(window).height(),   bh = boxHelp.outerHeight();
                    if (hideBoxHelp)
                        clearTimeout(hideBoxHelp);
                    if (pos.top + bh > wh)
                        pos.top = Math.max(0,wh-bh);
                    boxHelp
                        .show('fast')
                        .offset(pos);
                })
            }, function() {
                hideBoxHelp = setTimeout(function() { hideBoxHelp = undefined; boxHelp.hide('fast'); }, 500)
            });
        boxDiv.find(".scheme-boxlist-box")
            .click(function(){
                var box = $(this).text();
                equaresInfo(box, function(info) {
                    schemeEditor.newBox(box, info);
                })
            })
            .draggable({
                helper: function() {
                    return $("body")
                        .append('<div id="scheme-boxdrag">' + $(this).text() + '</div>')
                        .children("#scheme-boxdrag");
                },
                start: function() { draggingBox = true; },
                stop: function() { draggingBox = false; },
                scope: "newBox"
            });
        //boxDiv.find('li').wrapInner('<a href="kaka">');
        /*
        boxDiv.find('li').each(function(index) {
            var e = this;
            equaresInfo($(e).text(), function(info) {
                if (info.help)
                    $(e).append('<br/>' + info.help);
            })
        })
        */
    }

    // Fill box div with box types
    equaresInfo("boxTypes", setBoxes)

    var schemeCell = layout.add( { title: "Scheme" } );
    var schemeEditor = ctmEquaresSchemeEditor.newEditor(schemeCell.dom)

    var schemeDiv = $(schemeCell.dom);
    schemeDiv.droppable({
        scope: "newBox",
        drop: function(event, ui) {
            var box = ui.helper.text();
            equaresInfo(box, function(info) {
                schemeEditor.newBox(box, info, ui);
                })
        }
    });

    var settingsCell = layout.add( { title: "Settings" } );
    var settingsLayout = settingsCell.setLayout({type: "vertical", fixed: true});
    var itemsCell = settingsLayout.add( { title: "Items" } );
    var propsCell = settingsLayout.add( { title: "Properties" } );

    var itemsDiv = $(itemsCell.dom);
    var propsDiv = $(itemsCell.dom);

    var portHelp = $("body").append('<div id="scheme-porthelp"></div>').children("#scheme-porthelp").hide();
    var hidePortHelp;

    equaresui.enterPort = function() {
        var o = $(this);
        var pos = o.offset();
        pos.left += this.getBBox().width + 5; // 2*o.attr('r') + 5;
        var d = this.__data__;
        portHelp.html(d.info.name + ' (' + d.type + ')');
        if (hidePortHelp)
            clearTimeout(hidePortHelp);
        portHelp
            .show('fast')
            .offset(pos);
    }
    equaresui.leavePort = function() {
        hidePortHelp = setTimeout(function() { hidePortHelp = undefined; portHelp.hide('fast'); }, 300)
    }
}

$(document).ready(function() {
    // Add source menu tools for equares docks
    ctmDock.toolSets.sourceMenuTools.addTools( [
        ctmDock.newTool( {
            title: "Console",
            src: "console.png",
            alt: "console",
            handler: equaresui.setConsoleSource
            } ),
        ctmDock.newTool( {
            title: "Plot",
            src: "plot.png",
            alt: "plot",
            handler: equaresui.setWorkbenchSource
            } ),
        ctmDock.newTool( {
            title: "Scheme",
            src: "scheme.png",
            alt: "scheme",
            handler: equaresui.setSceneSource
            } )
    ] );
});
})();
