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

    var equaresInfo = equaresBox.info

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
                    text += "<br/>Elements: ";
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

    var HoverPopup = equaresui.HoverPopup = function(popup, enter, timeout) {
        timeout = timeout || 500
        var nin = 0
        var ncheck = 0
        function check() {
            if (ncheck == 0) {
                setTimeout(hide, timeout)
                ++ncheck
            }
        }
        function hide() {
            --ncheck
            if (nin == 0)
                 popup.hide('fast')
             else
                 check()
        }
        this.enter = function() {
            ++nin;
            enter.call(this, popup)
        }
        this.leave = function() {
            --nin;
            check();
        }
        this.target = function(element) {
            $(element).hover(this.enter, this.leave)
        }
    }

    var setBoxes = function(boxes)
    {
        var text = '<h1>Boxes</h1><table>';
        for (var i in boxes)
            text += '<tr></td><td><img src="question.png"></td><td class="scheme-boxlist-box">' + boxes[i] + '</tr>';
        text += '</table>';
        boxDiv.append(text);
        boxDiv.find('tr:odd').addClass('odd');
        boxDiv.find('tr:even').addClass('even');
        var hpBoxHelp = new HoverPopup(boxHelp, function() {
            if (draggingBox)
                return;
            var o = $(this);
            var pos = o.offset();
            pos.left += o.width();
            var box = o.parent().next().text();
            equaresInfo(box, function(info) {
                boxHelp
                    .html(boxHelpText(box, info));
                var wh = $(window).height(),   bh = boxHelp.outerHeight();
                if (pos.top + bh > wh)
                    pos.top = Math.max(0,wh-bh);
                boxHelp
                    .show('fast')
                    .offset(pos);
            })
        })
        boxDiv.find('td').find('img').each(function() { hpBoxHelp.target(this) })

        boxDiv.find(".scheme-boxlist-box")
            .click(function(){
                var box = $(this).text();
                equaresInfo(box, function(info) {
                    schemeEditor.newBox(box, info).select()
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
                schemeEditor.newBox(box, info, ui).select()
            })
        }
    });

    var settingsCell = layout.add( { title: "Settings" } );
    var settingsLayout = settingsCell.setLayout({type: "vertical", fixed: true});
    var propsCell = settingsLayout.add( { title: "Properties" } );
    var extrasCell = settingsLayout.add( { title: "Extras" } );
    var extrasDiv = $('<div id="scheme-box-extras"></div>').appendTo($(extrasCell.dom))

    var portHelp = $('<div id="scheme-porthelp"></div>').appendTo($("body")).hide();
    var hpPortHelp = equaresui.hpPortHelp = new HoverPopup(portHelp, function() {
        var o = $(this);
        var pos = o.offset();
        pos.left += this.getBBox().width + 5; // 2*o.attr('r') + 5;
        var d = this.__data__;
        portHelp.html(d.info.name + ' (' + d.type + ')');
        portHelp
            .show('fast')
            .offset(pos);
    },
    300)

    var statusHelp = $('<div id="scheme-statushelp"></div>').appendTo($("body")).hide();
    var hpStatusHelp = equaresui.hpStatusHelp = new HoverPopup(statusHelp, function() {
        var o = $(this);
        var pos = o.offset();
        pos.left += this.getBBox().width + 5;
        var d = this.parentElement.__data__;
        var msg = d.status.text
            .replace(/ /g, "&nbsp;")
            .replace(/\</g, "&lt;")
            .replace(/\>/g, "&gt;")
            .replace(/\n/g, "<br>")
            .replace(/\r/g, "")
        statusHelp.html(msg);
        if (d.status.level == "error")
            statusHelp.addClass("error")
        else
            statusHelp.removeClass("error")
        statusHelp
            .show('fast')
            .offset(pos);
    },
    300)

    var propsDiv = $('<div id="scheme-box-props"></div>').appendTo($(propsCell.dom))
    propsDiv.html("<h1>Properties</h1>")
    function validateBoxProp(value, userType, setter) {
        var s = value.toString().trim()
        var ok = true
        switch(userType) {
        case 'i':
            ok = /^[+-]?[0-9]+$/.test(s)
            value = +s
            break
        case 'd':
            // Regexp is copied from here: http://regexlib.com/REDetails.aspx?regexp_id=185
            ok = /^[+-]?([0-9]*\.?[0-9]+|[0-9]+\.?[0-9]*)([eE][+-]?[0-9]+)?$/.test(s)
            value = +s
            break
        }
        setter(value, ok)
    }

    equaresui.selectBox = function(box) {
        if (box) {
            extrasDiv.html("")
            function wrap(tag) { return "<" + tag + "></" + tag + ">" }
            function makeEditor(host, prop) {
                var t = prop.userType
                if (t === undefined)
                    host.html("N/A")
                else if (t instanceof Array)
                    host.html("TODO: Array")
                else if (t instanceof Object) {
                    var table = $(wrap("table")).appendTo(host), odd = false
                    for (var name in t) {
                        var row = $(wrap("tr")).appendTo(table)
                        row.addClass(odd ? "odd": "even")
                        row.append($(wrap("td")).html(name))
                        var tdVal = $(wrap("td")).appendTo(row)
                        ;(function(){    // Important: function provides closure for name_
                            var name_ = name
                            makeEditor(tdVal,
                                {
                                    name: name,
                                    userType: t[name],
                                    getter: function() { return prop.getter()[name_] },
                                    setter: function(val) {
                                        var v = prop.getter()
                                        v[name_] = val
                                        prop.setter(v)
                                    }
                                })
                        })()
                        odd = !odd
                    }
                }
                else if (typeof(t) == "string") {
                    switch (t) {
                    case 's': case 'i': case 'd':
                        $('<input type="text">')
                            .attr("value", prop.getter().toString())
                            .change(function() {
                                var e = this
                                validateBoxProp(this.value, t, function(value, ok) {
                                    if (ok)
                                        prop.setter(value)
                                    e.value = prop.getter()
                                })
                            })
                            .appendTo(host)
                        break
                    case 'i:*':
                        $('<input type="text">')
                            .attr("value", prop.getter().toString())
                            .change(function() {
                                var val = this.value.match(/-?[0-9]+/g) || []
                                for (var i=0; i<val.length; ++i)
                                    val[i] = +val[i]
                                prop.setter(val)
                                this.value = prop.getter().toString()
                            })
                            .appendTo(host)
                        break
                    case 'b':
                        $('<input type="checkbox">')
                            .attr("checked", prop.getter())
                            .change(function() {
                                prop.setter(this.checked)
                                this.checked = prop.getter()
                            })
                            .appendTo(host)
                        break
                    case 't':
                        $('<input type="button">')
                            .attr("value", "...")
                            .click(function() {
                                extrasDiv.html("")
                                var textarea
                                $(wrap("div")).attr("id", "scheme-box-extras-hdr")
                                    .append($(wrap("h1")).html(prop.name))
                                    .append(
                                         $('<input type="button" class="scheme-box-extras-ok" value="Ok">')
                                            .click(function() { prop.setter(textarea[0].value) })
                                    )
                                    .appendTo(extrasDiv)
                                $(wrap("div")).attr("id", "scheme-box-extras-text")
                                    .append(textarea = $(wrap("textarea")))
                                    .appendTo(extrasDiv)
                                textarea[0].value = prop.getter()
                                textarea.focus()
                            })
                            .appendTo(host)
                        break
                    case 'BoxType':
                        var editor = $(wrap("select")).appendTo(host)
                        equaresInfo("boxTypes", function(boxes) {
                            function findSelIndex() {
                                var index = -1
                                var v = prop.getter()
                                for(var i=0; i<boxes.length; ++i) {
                                    var b = boxes[i]
                                    editor.append($(wrap("option")).attr("value", b).html(b))
                                    if (b == v)
                                        index = i
                                }
                                return index
                            }
                            var edom = editor[0]
                            edom.selectedIndex = findSelIndex()
                            editor.change(function() {
                                var type = this[edom.selectedIndex].value
                                equaresInfo(type, function(info){
                                    prop.setter(type, info)
                                    var i = findSelIndex()
                                    if (edom.selectedIndex != i)
                                        edom.selectedIndex = i
                                })
                            })
                        })
                        break
                    default:
                        if (t[0] == 'f') {
                            // Edit a combination of flags
                            var tx = {}
                            function hasFlag(flag) {
                                var v = prop.getter()
                                for(var i=0; i<v.length; ++i)
                                    if (v[i] == flag)
                                        return true
                                return false
                            }
                            var flags = t.substr(1).match(/\w+/g)
                            var flagProp = { name: prop.name, userType: {} }
                            for (var i=0; i<flags.length; ++i)
                                flagProp.userType[flags[i]] = 'b'
                            flagProp.getter = function() {
                                var result = {}
                                for (var i=0; i<flags.length; ++i) {
                                    var flag = flags[i]
                                    result[flag] = hasFlag(flag)
                                    }
                                return result
                                }
                            flagProp.setter = function(val) {
                                var result = []
                                for (var i=0; i<flags.length; ++i) {
                                    var flag = flags[i]
                                    if (val[flag])
                                        result.push(flag)
                                }
                                prop.setter(result)
                            }
                        makeEditor(host, flagProp)
                        }
                        else
                            host.html("TODO: " + t)
                        break
                    }
                }
            }

            propsDiv.html("")
            var hdr = $(wrap("h1")).html(box.name).appendTo(propsDiv)
            var table = $(wrap("table")).appendTo(propsDiv)
            var props = {
                name: {
                    userType: 's',
                    getter: function() { return box.name },
                    setter: function(newName) {
                        box.rename(newName)
                        hdr.html(box.name)
                    }
                },
                type: {
                    userType: 'BoxType',
                    getter: function() { return box.type },
                    setter: function(newType, boxInfo) { box.changeType(newType, boxInfo) }
                }
            }
            for (var pname in box.props)
                props[pname] = { userType: box.propType(pname) }
            var odd = true
            for (pname in props) {
                odd = !odd
                var p = props[pname]
                var row = $(wrap("tr")).appendTo(table)
                row.addClass(odd? "odd": "even")
                row.append($(wrap("td")).html(pname))
                var tdVal = $(wrap("td")).appendTo(row)
                ;(function(){    // Important: function provides closure for pname_
                    var pname_ = pname,
                        setter = p.setter instanceof Function ?   p.setter :   function(value) { box.prop(pname_, value) },
                        getter = p.getter instanceof Function ?   p.getter :   function() { return box.prop(pname_) }
                    makeEditor(tdVal, {name: pname_, userType: p.userType, getter: getter, setter: setter})
                })()
            }
            table.find("td:first").next().children().first().focus()
        }
        else {
            propsDiv.html("<h1>No selection</h1>")
            extrasDiv.html("")
        }
    }

    equaresui.openScheme = function(fileToLoad) {
        // var fileToLoad = document.getElementById("fileToLoad").files[0];
        var fileReader = new FileReader();
        fileReader.onload = function(fileLoadedEvent)
        {
            var textFromFileLoaded = fileLoadedEvent.target.result;
            schemeEditor.import(textFromFileLoaded)
        };
        fileReader.readAsText(fileToLoad, "UTF-8");
    }

    equaresui.saveScheme = function(fileName) {
        if (arguments.length < 1)
            fileName = "equares-scheme.json"
        var schemeText = schemeEditor.export()
        var b = new Blob([schemeText], {type: "text/plain"})

        var downloadLink = document.createElement("a");
        downloadLink.download = fileName;
        downloadLink.innerHTML = "Download File";
        if (window.webkitURL != null) {
            // Chrome allows the link to be clicked
            // without actually adding it to the DOM.
            downloadLink.href = window.webkitURL.createObjectURL(b);
        }
        else {
            // Firefox requires the link to be added to the DOM
            // before it can be clicked.
            downloadLink.href = window.URL.createObjectURL(b);
            downloadLink.onclick = function(event) { document.body.removeChild(event.target); }
            downloadLink.style.display = "none";
            document.body.appendChild(downloadLink);
        }
        downloadLink.click();

    }
    equaresui.runScheme = function() {
        alert("TODO: equaresui.runScheme()")
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
