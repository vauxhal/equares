var equaresui = {};

(function() {

function wrap(tag) { return $("<" + tag + "></" + tag + ">") }

formatInfo.init('simInfo')

equaresui.setSceneSource = function() {
    this.clear();
    var layoutOptions = {
        type: "horizontal",
        fixed: true,
        toolhandle: false
    };
    this.setTitle( "Simulation editor" );
    var layout = this.setLayout( layoutOptions );
    
    var docCell = layout.add( { title: "Info pane", width: {min: 50, max: 500} } );
    var boxCell = layout.add( { title: "Boxes", width: {min: 50, max: 300} } );
    docCell.dom.id = 'simInfoContainer'
    var docDiv = $(docCell.dom)
    var simInfo = wrap('div').attr('id', 'simInfo').appendTo(docDiv)
    function showBoxes() {
        docCell.header().minimize(docCell);
        boxCell.header().restore(boxCell);
    }
    function showSimInfo() {
        boxCell.header().minimize(boxCell);
        docCell.header().restore(docCell);
    }
    $('<input type="button" value="Show box list"/>').appendTo(
        wrap('div').addClass('leftPaneTools').append('<hr/>').appendTo(docDiv)
    ).click(showBoxes)

    boxCell.header().minimize(boxCell);
    var boxDiv = $(boxCell.dom);
    var boxHelp = $("body").append('<div id="scheme-boxhelp"></div>').children("#scheme-boxhelp").hide();
    boxDiv.addClass('scheme-boxlist');
    var draggingBox = false;

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
                    var f = new equaresBox.PortFormat(p)
                    text += "<br/>Format: " + f.toHtml()
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

    // Fill box div with box types
    ;(function() {
        var boxes = equaresBox.boxTypes
        var text = '<h1>Boxes</h1><table>';
        for (var i in boxes)
            text += '<tr></td><td><img src="images/question.png"></td><td class="scheme-boxlist-box">' + boxes[i] + '</tr>';
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
            var info = equaresBox.boxInfo[box]
            boxHelp
                .html(boxHelpText(box, info));
            var wh = $(window).height(),   bh = boxHelp.outerHeight();
            if (pos.top + bh > wh)
                pos.top = Math.max(0,wh-bh);
            boxHelp
                .show('fast')
                .offset(pos);
        })
        boxDiv.find('td').find('img')
            .each(function() { hpBoxHelp.target(this) })
            .click(function(){
                var box = $(this).parent().next().text()
                var a = wrap('a').attr('href', '/doc#box/'+box).attr('target', '_blank').attr('style', 'display: none;').appendTo('body')
                a[0].click()
                a.remove()
            })

        boxDiv.find(".scheme-boxlist-box")
            .click(function(){
                var box = $(this).text();
                schemeEditor.newBox(box).select()
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

        $('<input type="button" value="Show info pane"/>').appendTo(
            wrap('div').attr('class', 'leftPaneTools').append('<hr/>').appendTo(boxDiv)
        ).click(showSimInfo)


    })()

    var schemeCell = layout.add( { title: "Scheme" } );
    var schemeEditor = ctmEquaresSchemeEditor.newEditor(schemeCell.dom)

    var schemeDiv = $(schemeCell.dom);
    schemeDiv.droppable({
        scope: "newBox",
        drop: function(event, ui) {
            var box = ui.helper.text();
            schemeEditor.newBox(box, ui).select()
        }
    });

    var settingsCell = layout.add( { title: "Settings" } );
    var settingsLayout = settingsCell.setLayout({type: "vertical", fixed: true});
    var propsCell = settingsLayout.add( { title: "Properties" } );
    var extrasCell = settingsLayout.add( { title: "Extras" } );
    ;(function() {
        var resizing = 0
        extrasCell.addResizeHandler(function() {
            if (++resizing == 1)
                $(this.dom).find("textarea.linenum-text").resize()
            --resizing
        })
    })()
    var extrasDiv = $('<div id="scheme-box-extras"></div>').appendTo($(extrasCell.dom))

    var portHelp = $('<div id="scheme-porthelp"></div>').appendTo($("body")).hide();
    var hpPortHelp = equaresui.hpPortHelp = new HoverPopup(portHelp, function() {
        var o = $(this)
        var pos = o.offset()
        pos.left += this.getBBox().width + 5
        var d = this.__data__
        var f = d.getFormat()
        portHelp.html(d.info.name + ' (' + d.type + ')<br/>Format: ' + f.toHtml())
        portHelp
            .show('fast')
            .offset(pos)
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
    function validateBoxProp(value, userType, setter) {
        var s = value.toString().trim()
        var ok = true
        switch(userType) {
        case 'i':
            ok = /^[+-]?[0-9]+$/.test(s)
            value = +s
            break
        case 'd':
            // TODO: remove validation for doubles since we are going to allow expressions
            // Regexp is copied from here: http://regexlib.com/REDetails.aspx?regexp_id=185
            // ok = /^[+-]?([0-9]*\.?[0-9]+|[0-9]+\.?[0-9]*)([eE][+-]?[0-9]+)?$/.test(s)
            // value = +s
            break
        }
        setter(value, ok)
    }

    function replaceSel(textarea, replacement) {
        var selStart = textarea.prop('selectionStart'),
            selEnd = textarea.prop('selectionEnd'),
            v = textarea.val(),
            textBefore = v.substring(0, selStart),
            textAfter  = v.substring(selEnd)
        textarea
            .val(textBefore + replacement + textAfter)
            .prop('selectionStart', selStart+=replacement.length)
            .prop('selectionEnd', selStart)
            .trigger('input')
    }

    var lastEditedTextPropName = ''
    function loadTextProp(prop) {
        extrasDiv.html('')
        if (lastEditedTextPropName == 'info')
            loadSimInfo() // In the case we haven't saved edited info text
        if (arguments.length == 0)
            return lastEditedTextPropName = ''
        lastEditedTextPropName = prop.name
        var textarea, tools, okbtn
        wrap("div").attr("id", "scheme-box-extras-hdr")
            .append(wrap("h1").html(prop.name))
            .append(tools = wrap('div').addClass('scheme-box-extras-tools'))
            .appendTo(extrasDiv)
        tools
            .append(
                 (okbtn = $('<input type="button" value="Ok">'))
                    .click(function() {
                        prop.setter(textarea[0].value)
                        textarea.removeClass('modified')
                    })
            )
        wrap("div").attr("id", "scheme-box-extras-text")
            .append(textarea = wrap("textarea"))
            .appendTo(extrasDiv)
        if (prop.userType === 't')
            textarea.linenum()
        else if (prop.userType === 'M') {
            // Add 'image' button
            $('.scheme-box-extras-tools').prepend(
                $('<input type="button" title="Image" value=" ">').addClass('icon-button ui-icon-image')
                    .click(function() {
                        imageView.pick(function(url) {
                            replaceSel(textarea, '![](' + url + ')')
                            textarea.focus()
                        })
                    })
            )
        }

        textarea[0].value = prop.getter()
        var firstInput = true
        textarea.on('input', function() {
            $(this).addClass('modified')
            if (prop.name == 'info') {
                loadSimInfo(this.value)
            }
            if (firstInput) {
                firstInput = false

                // Warn user that it's necessary to press Ok
                warningMessage('press Ok to save!')
                var c = okbtn.css('backgroundColor')
                okbtn.css({backgroundColor: '#c00'})
                okbtn.animate({backgroundColor: c})
            }
        })

        return textarea
    }

    function makeParameterValue(t)
    {
        if (t === undefined)
            return undefined
        if (t instanceof Array)
            return []
        if (t instanceof Object) {
            var result = {}
            for (var name in t)
                result[name] = makeParameterValue(t[name])
            return result
        }
        switch(t) {
        case 's': case 't': case 'T': case 'M': return ''
        case 'BoxType': return 'Param'
        case 'i': case 'd': return 0
        case 'b': return false
        case 'i:*': case 's:*': return []
        default: return undefined
        }
    }

    function makeEditor(host, prop) {
        var t = prop.userType
        if (t === undefined)
            host.html("N/A")
        else if (t instanceof Array) {
            var table = wrap("table").appendTo(host), odd = false
            var length = prop.getter().length
            for (var idx=0; idx<length; ++idx) {
                var row = wrap("tr").appendTo(table)
                row.addClass(odd ? "odd": "even")
                row.append(wrap("td").html(idx))
                var tdVal = wrap("td").appendTo(row)
                ;(function(){    // Important: function provides closure for idx_
                    var idx_ = idx
                    makeEditor(tdVal,
                        {
                            name: idx,
                            userType: t[0],
                            getter: function() { return prop.getter()[idx_] },
                            setter: function(val) {
                                var v = prop.getter()
                                v[idx_] = val
                                prop.setter(v)
                            }
                        })
                    wrap('td').appendTo(row)
                        .append(
                            $('<input type="button">')
                                .attr("value", "-").click(function(){
                                    var v = prop.getter()
                                    v.splice(idx_, 1)
                                    prop.setter(v)
                                    equaresui.selectBox()
                                })
                        )
                        .append(
                            $('<input type="button">')
                                .attr("value", "+").click(function(){
                                    var v = prop.getter()
                                    v.splice(idx_, 0, makeParameterValue(t[0]))
                                    prop.setter(v)
                                    equaresui.selectBox()
                                })
                        )
                    odd = !odd
                })()
            }
            table.append(wrap('tr').append(wrap('td')
                .html($('<input type="button">')
                    .attr("value", "+")
                    .click(function() {
                        var v = prop.getter()
                        v.push(makeParameterValue(t[0]))
                        prop.setter(v)
                        equaresui.selectBox()
                    })
                )
            ))
        }
        else if (t instanceof Object) {
            var table = wrap("table").appendTo(host), odd = false
            for (var name in t) {
                var row = wrap("tr").appendTo(table)
                row.addClass(odd ? "odd": "even")
                row.append(wrap("td").html(name))
                var tdVal = wrap("td").appendTo(row)
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
                            $(e).removeClass('modified')
                        })
                    })
                    .keypress(function() {
                        $(this).addClass('modified')
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
                        $(this).removeClass('modified')
                    })
                    .keypress(function() {
                        $(this).addClass('modified')
                    })
                    .appendTo(host)
                break
            case 's:*':
                $('<input type="text">')
                    .attr("value", prop.getter().join(', '))
                    .change(function() {
                        var val = this.value.toLowerCase().split(',')
                        for (var i=0; i<val.length; ++i) {
                            var s = val[i].trim()
                            if (s.length > 0)
                                val[i] = s
                            else
                                val.splice(i, 1)
                        }
                        prop.setter(val)
                        this.value = prop.getter().join(', ')
                        $(this).removeClass('modified')
                    })
                    .keypress(function() {
                        $(this).addClass('modified')
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
            case 'T':
            case 'M':
                $('<input type="button">')
                    .attr("value", "...")
                    .click(function() {
                        loadTextProp(prop).focus()
                    })
                    .appendTo(host)
                break
            case 'BoxType':
                var editor = wrap("select").appendTo(host)
                function findSelIndex() {
                    var index = -1
                    var v = prop.getter()
                    var boxes = equaresBox.boxTypes
                    for(var i=0; i<boxes.length; ++i) {
                        var b = boxes[i]
                        editor.append(wrap("option").attr("value", b).html(b))
                        if (b == v)
                            index = i
                    }
                    return index
                }
                var edom = editor[0]
                edom.selectedIndex = findSelIndex()
                editor.change(function() {
                    var type = this[edom.selectedIndex].value
                    prop.setter(type)
                    var i = findSelIndex()
                    if (edom.selectedIndex != i)
                        edom.selectedIndex = i
                })
                host.append(
                    wrap('a')
                        .attr('href', '/doc#box/'+prop.getter()).attr('target', '_blank')
                        .attr('style', 'margin-left: 10px;')
                        .html('help')
                )
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
                else if (t[0] == 'e') {
                    // Edit an enumerated value
                    var values = t.substr(1).match(/\w+/g)
                    var editor = wrap("select").appendTo(host)
                    function setEnumValues() {
                        var index = -1
                        var v = prop.getter()
                        for(var i=0; i<values.length; ++i) {
                            var vi = values[i]
                            editor.append(wrap("option").attr("value", vi).html(vi))
                            if (v == vi)
                                index = i
                        }
                        return index
                    }
                    var edom = editor[0]
                    edom.selectedIndex = setEnumValues()
                    editor.change(function() {
                        var v = this[edom.selectedIndex].value
                        prop.setter(v)
                    })
                }
                else
                    host.html("TODO: " + t)
                break
            }
        }
    }

    function loadProps(title, props, options) {
        options = options || {}
        var makeGetter = options.makeGetter || function(pname, props) { return function() { return props[pname] } },
            makeSetter = options.makeSetter || function(pname, props) { return function(value) {
                props[pname] = value
                schemeEditor.modify()
            } }
        loadTextProp() // Clears extrasDiv when called without args
        propsDiv.html("")
        var hdr = wrap("h1").html(title).appendTo(propsDiv)
        var table = wrap("table").appendTo(propsDiv)
        var odd = true
        for (pname in props) {
            odd = !odd
            var p = props[pname]
            var row = wrap("tr").appendTo(table)
            row.addClass(odd? "odd": "even")
            row.append(wrap("td").html(pname))
            var userType = p.userType
            var prop = {
                name: pname,
                userType: userType,
                getter: p.getter instanceof Function ?   p.getter :   makeGetter(pname, props),
                setter: p.setter instanceof Function ?   p.setter :   makeSetter(pname, props)
            }
            makeEditor(wrap("td").appendTo(row), prop)
        }
        table.find("td:first").next().children().first().focus()
    }

    equaresui.selectBox = function(box) {
        var pname
        var props
        if (arguments.length == 0)
            box = equaresui.selectedBox
        else
            equaresui.selectedBox = box
        if (box) {
            props = {
                name: {
                    userType: 's',
                    getter: function() { return box.name },
                    setter: function(newName) {
                        box.rename(newName)
                        propsDiv.children('h1').first().html(box.name)
                    }
                },
                type: {
                    userType: 'BoxType',
                    getter: function() { return box.type },
                    setter: function(newType) {
                        box.changeType(newType)
                    }
                }
            }
            for (pname in box.props)
                props[pname] = { userType: box.propType(pname) }
            loadProps(box.name, props, {
                makeGetter: function(pname, props) { return function() { return box.prop(pname) } },
                makeSetter: function(pname, props) { return function(value) { box.prop(pname, value) } }
            })
        }
        else {
            props = {}
            for (pname in simPropFields)
                props[pname] = {userType: simPropFields[pname]}
            loadProps("Simulation properties", props, {
                makeGetter: function(pname, props) { return function() { return simProps[pname] } },
                makeSetter: function(pname, props) { return function(value) {
                    if (pname == 'name' && simProps[pname] != value)
                        checkOverwrite = true
                    var simInfoChanged = pname == 'info' && simProps[pname] != value
                    simProps[pname] = value
                    schemeEditor.modify()
                    if (simInfoChanged)
                        loadSimInfo()
                } }
            })
        }
    }
    equaresui.selectedBox = null
    
    // Load simulation properties
    var simPropFields = {name: 's', description: 's', info: 'M', keywords: 's:*', script: 't', public: 'b'}
    function defaultSimProps() {
        return {
            name: '',
            description: '',
            info: '',
            keywords: [],
            script: '',
            public: false
        }
    }
    var simProps = defaultSimProps()
    var checkOverwrite = true
    equaresui.selectBox(null)

    function loadSimInfo(info) {
        if (arguments.length < 1)
            info = simProps.info
        formatInfo.update(info)
    }

    function beforeLoadSimulation()
    {
        var loadingProgress = $("#loading-progress")
        loadingProgress.progressbar("value", 0)
        $("#loading-progress-overlay").show()
    }
    function loadSimulation(obj, modified)
    {
        var loadingProgress = $("#loading-progress")
        schemeEditor.import(obj.definition,
            function() {
                loadingProgress.progressbar("value", 100)
                for (var pname in simPropFields) {
                    var v = obj[pname]
                    if (v === undefined) switch(simPropFields[pname]) {
                    case 's': case 't': case 'T': case 'M':
                        v = ''
                        break
                    case 'b':
                        v = false
                        break
                    case 'd': case 'i':
                        v = 0
                        break
                    case 'i:*':   case 's:*':
                        v = []
                        break
                    }
                    simProps[pname] = v
                }
                equaresui.selectBox(null)
                schemeEditor.modified = modified
                checkOverwrite = true
                loadSimInfo()
                showSimInfo()
            },
            function(percent) {
                loadingProgress.progressbar("value", percent)
            }
        )
    }

    equaresui.loadExample = function(exampleName) {
        beforeLoadSimulation()
        $.get(exampleName)
            .done(function(obj) {
                loadSimulation(obj, true)
            })
            .fail(function() {
                errorMessage('Failed to load example')
                $("#loading-progress-overlay").hide()
            })
    }

    ;(function() {
        var fileUpload = $('#simulation-uploader')
        fileUpload.change(function () {
            if (fileUpload[0].files.length == 0)
                return;
            var fileReader = new FileReader();
            fileReader.onload = function(fileLoadedEvent) {
                var simulation = fileLoadedEvent.target.result
                loadSimulation(JSON.parse(simulation), true)
            }
            beforeLoadSimulation()
            fileReader.readAsText(fileUpload[0].files[0], "UTF-8")
        })
        equaresui.uploadSimulation = function() {
            fileUpload.click();
        }
    })()

    function simulationText() {
        return JSON.stringify($.extend({}, simProps, {definition: schemeEditor.export()}))
    }

    equaresui.downloadSimulation = function() {
        var fileName = simProps.name
        if ((typeof fileName == 'string') && fileName.length > 0)
            fileName += ".json"
        else
            fileName = "equares-scheme.json"
        var b = new Blob([simulationText()], {type: "text/plain"})
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
    equaresui.saveSimulation = function() {
        $.post('cmd/savesim', {simulation: simulationText(), overwrite: !checkOverwrite})
            .done(function() {
                checkOverwrite = false
                infoMessage('saved')
            })
            .fail(function(err) {
                if (checkOverwrite && err.status == 403) {
                    $("#confirm-overwrite").dialog({
                        resizable: false,
                        width: 600,
                        modal: true,
                        buttons: {
                            Overwrite: function() {
                                $(this).dialog("close")
                                checkOverwrite = false
                                equaresui.saveSimulation()
                            },
                            Cancel: function() {
                                $(this).dialog("close")
                            }
                        }
                    })
                }
                else {
                    var msg = 'Save failed'
                    if (err.responseText.length > 0)
                        msg += ': ' + err.responseText
                    errorMessage(msg)
                }
            })
    }
    equaresui.clearSimulation = function() {
        schemeEditor.clearSimulation()
        simProps = defaultSimProps()
        loadSimInfo()
        checkOverwrite = true
        equaresui.selectBox(null)
        showBoxes()
    }

    equaresui.runScheme = function() {
        var simulation = schemeEditor.exportSimulation()
        function dummyStopSim() {}
        equaresui.stopSimulation = dummyStopSim
        $.ajax("cmd/runSimulation", {data: {script: simProps.script, simulation: simulation}, type: "POST", cache: false})
            .done(function() {
                var dlg = $("#running-simulation")
                var stopButton = $(".ui-dialog-buttonpane button:contains('Stop')")
                stopButton.button("enable")
                dlg.dialog("open")
                var status = dlg.find(".status").html("running")
                var running = true
                equaresui.stopSimulation = function() {
                    $.ajax("cmd/toggle", {type: "GET", cache: false})
                }

                var interactiveInput = $('#simulation-interactive-input').html("").css("text-align", "center")
                var outfiles = $("#simulation-output-files").html("").css("text-align", "center")

                var equaresStatEvent = new EventSource("cmd/statEvent");
                equaresStatEvent.onmessage = function(event) {
                    if (+event.data === 0) {
                        // Simulation has finished
                        status.html("finished");
                        running = false
                        stopButton.button("disable")
                        equaresui.stopSimulation = dummyStopSim
                        equaresStatEvent.close()
                        equaresOutputEvent.close()
                    }
                }
                var equaresOutputEvent = new EventSource("cmd/outputEvent");
                var rxFile = /^==([0-9])+==\> file: (.*)/,
                    rxSync = /^==([0-9])+==\> sync/,
                    rxNum = /^[+-]?(\d+(\.\d*)?|\.\d+)([eE][+-]?\d+)?$/
                var inputInfo = {},   inputRefs = {},   outFileInfo = {}
                var started = false
                var prefix, syncToken, inputPrefix
                var iaStarted = false   // Input announcement started
                var iaFinished = false  // Input announcement finished
                var faStarted = false   // File announcement started
                var faFinished = false  // File announcement finished
                equaresOutputEvent.onmessage=function(event) {
                    data = JSON.parse(event.data);
                    var str = data.text, i, j, fi, ii, irefs, m
                    switch (data.stream) {
                    case 0: return  // Ignore stdin
                    case 1:         // Parse stdout
                        if (!started) {
                            // Wait for start token
                            m = str.match(/^==([0-9])+==\> started/)
                            if (m && m.length > 1) {
                                prefix = "==" + m[1] + "==> "
                                syncToken = "==" + m[1] + "==<"
                                inputPrefix = "==" + m[1] + "==:"
                                started = true
                            }
                            return
                        }
                        if (!str.match(prefix))
                            // Ignore everything not starting with the right prefix
                            return
                        // Cut prefix
                        str = str.substr(prefix.length).trimRight()
                        if (!iaStarted) {
                            // Wait for input annouuncement
                            if (str === "begin input announcement")
                                iaStarted = true
                            return
                        }
                        if (!iaFinished) {
                            // Process input annouuncement
                            if (str === "end input announcement") {
                                iaFinished = true
                            }
                            else {
                                ii = JSON.parse(str)
                                inputInfo[ii.consumer] = ii
                                switch (ii.type) {
                                case 'image':
                                    irefs = inputRefs[ii.refImage];
                                    if (!irefs)
                                        irefs = inputRefs[ii.refImage] = [];
                                    irefs.push(ii)
                                    break
                                case 'signal':
                                    (function(ii) { // Closure for ii
                                        interactiveInput.append(
                                            $('<input type="button" value="' + ii.name + '"/>').click(function() {
                                                $.ajax("cmd/input", {data: {cmd: inputPrefix + ii.consumer + ' 1'}, type: "GET", cache: false})
                                                    .fail(function(error) {
                                                        errorMessage(error.responseText || error.statusText || ("Ajax error: " + error.status))
                                                    })
                                                })
                                        )
                                    })(ii)
                                    break
                                case 'simple':
                                    (function(ii) { // Closure for ii
                                        var data = [], editors = [], i
                                        ii.acquirePortInput = function(data) {
                                            var n = editors.length
                                            if (n > data.length)
                                                n = data.length
                                            for (i=0; i<n; ++i)
                                                editors[i].val(data[i])
                                        }
                                        function send() {
                                            for (i=0; i<ii.items.length; ++i) {
                                                var v = editors[i].val()
                                                if (!v.match(rxNum))
                                                    return errorMessage('Please enter a real number in field \'' + ii.items[i])
                                                data[i] = +v
                                            }
                                            $.ajax("cmd/input", {data: {cmd: inputPrefix + ii.consumer + ' ' + data.join(' ')}, type: "GET", cache: false})
                                                .fail(function(error) {
                                                    errorMessage(error.responseText || error.statusText || ("Ajax error: " + error.status))
                                                })
                                        }
                                        for (i=0; i<ii.items.length; ++i) {
                                            interactiveInput.append(wrap('div')
                                                .append(wrap('span').text(ii.items[i]))
                                                .append(editors[i] = $('<input type="text"/>').change(send))
                                            )
                                        }
                                    })(ii)
                                    break
                                case 'range':
                                    (function(ii) { // Closure for ii
                                        var data = [], editors = [], i, dataLabel = [], dontsend = true
                                        function updateLabels() {
                                            for (i=0; i<ii.items.length; ++i)
                                                dataLabel[i].text(data[i] = +editors[i].slider('value'))
                                        }
                                        ii.acquirePortInput = function(data) {
                                            dontsend = true
                                            var n = editors.length
                                            if (n > data.length)
                                                n = data.length
                                            for (i=0; i<n; ++i)
                                                editors[i].slider('value', +data[i])
                                            updateLabels()
                                            dontsend = false
                                        }
                                        function send() {
                                            if (dontsend)
                                                return
                                            $.ajax("cmd/input", {data: {cmd: inputPrefix + ii.consumer + ' ' + data.join(' ')}, type: "GET", cache: false})
                                                .fail(function(error) {
                                                    errorMessage(error.responseText || error.statusText || ("Ajax error: " + error.status))
                                                })
                                        }
                                        for (i=0; i<ii.items.length; ++i) {
                                            var r = ii.items[i]
                                            interactiveInput.append(wrap('div')
                                                .append(wrap('span').text(r.name + ' = '))
                                                .append(dataLabel[i] = wrap('span').text(r.vmin))
                                                .append(wrap('span').text(' in range [' + r.vmin + ', ' + r.vmax + ']'))
                                                .append(editors[i] = wrap('div').slider({
                                                    value: +r.vmin,
                                                    min: +r.vmin,
                                                    max: +r.vmax,
                                                    step: (+r.vmax-r.vmin)/(r.resolution||1),
                                                    slide: updateLabels,
                                                    change: send
                                                }))
                                            )
                                        }
                                        dontsend = false
                                    })(ii)
                                    break
                                }
                            }
                            return
                        }
                        if (!faStarted) {
                            // Wait for file annouuncement
                            if (str === "begin file announcement")
                                faStarted = true
                            return
                        }
                        if (!faFinished) {
                            // Process file annouuncement
                            if (str === "end file announcement") {
                                // Create elements for displaying announced files
                                for(i in outFileInfo) {
                                    irefs = inputRefs[i]
                                    function inputMethods(irefs) {
                                        var methods = {}
                                        for (var j in irefs)
                                            methods[irefs[j].method] = 1
                                        var m = []
                                        for (j in methods)
                                            m.push(j)
                                        return m.join(', ')
                                    }
                                    outfiles.append('<span>'+i+(irefs? ' (interactive: '+inputMethods(irefs)+')': '')+'</span><br/>' )
                                    fi = outFileInfo[i]
                                    switch(fi.type) {
                                    case "image":
                                        fi.jq = $('<img src="' + "user/" + i + '" alt="' + i + '"/>')
                                            .css("width", fi.size.width)
                                            .css("height", fi.size.height)
                                            .addClass("outputFile")
                                            .appendTo(outfiles)
                                        outfiles.append("<br/>")
                                        for (j in irefs) {
                                            (function(iref) {  // Provide closure for irefs
                                                fi.jq.on(iref.method, function(e) {
                                                    $.ajax("cmd/input", {data: {cmd: inputPrefix + iref.consumer + ' ' + e.offsetX + ' ' + e.offsetY}, type: "GET", cache: false})
                                                    .fail(function(error) {
                                                        errorMessage(error.responseText || error.statusText || ("Ajax error: " + error.status))
                                                    })
                                                return
                                                })
                                            })(irefs[j])
                                        }
                                        break
                                    case "text":
                                        fi.jq = wrap('div').addClass("outputFile").appendTo(outfiles).html("Waiting...")
                                        break
                                    default:
                                        fi.jq = wrap('div').addClass("outputFile").appendTo(outfiles).html("UNKNOWN FILE TYPE")
                                    }
                                }
                                faFinished = true
                            }
                            else {
                                fi = JSON.parse(str)
                                outFileInfo[fi.name] = fi
                            }
                            return
                        }
                        m = str.match("file: (.*)")
                        if (m && m.length > 1) {
                            var name = m[1], svrname = "user/" + name
                            fi = outFileInfo[name]
                            switch(fi.type) {
                            case "image":
                                fi.jq.attr("src", svrname + "#" + new Date().getTime())
                                break
                            case "text":
                                $.get('cmd/simtextfile', {file:name}).done(function(data) {
                                    fi.jq.html(data)
                                })
                                break
                            }
                            return
                        }
                        if (str === "sync") {
                            $.ajax("cmd/sync", {data: {cmd: syncToken}, type: "GET", cache: false})
                            .fail(function(error) {
                                errorMessage(error.responseText || error.statusText || ("Ajax error: " + error.status))
                            })
                            return
                        }
                        m = str.match(/^input:\s+(\w+)\s+(.*$)/)
                        if (m && m.length > 2) {
                            var consumer = inputInfo[m[1]]
                            if (consumer && (consumer.acquirePortInput instanceof Function))
                                consumer.acquirePortInput(m[2].split(/\s+/))
                        }
                        if (str === "finished") {
                            equaresui.stopSimulation()
                            return
                        }
                        break
                    case 2:         // Report error
                        m = str.match(/^==\d+==> ERROR: line \d+: (.*)/)
                        if (m && m.length > 1) {
                            str = m[1].trimRight()
                            equaresui.stopSimulation()
                            // If possible, select box in problem
                            m = str.match(/\[box='(.*)'\]/)
                            if (m && m.length > 1)
                                schemeEditor.findBox(m[1]).select(true)
                        }
                        else {
                            m = str.match(/^==\d+==> (.*)/)
                            if (m && m.length > 1)
                                str = m[1].trimRight()
                        }
                        outfiles.css("text-align", "left").append('<span style="color: red">' + str + '</span><br/>')
                        break
                    }
                }
            })
            .fail(function(error){
                errorMessage(error.responseText || error.statusText || ("Ajax error: " + error.status));
            });
    }

    function quickload() {
        beforeLoadSimulation()
        $.get('cmd/quickload')
            .done(function(simulation) {
                loadSimulation(JSON.parse(simulation), false)
            })
            .fail(function() {
                errorMessage('quickload failed')
                $("#loading-progress-overlay").hide()
            })
    }
    var saving = false
    function quicksave() {
        if (saving)
            return
        if (schemeEditor.modified) {
            saving = true
            $.post('cmd/quicksave', {simulation: simulationText()})
                .done(function() {
                    schemeEditor.modified = false
                    infoMessage('quicksaved', 1000)
                })
                .fail(function(xhr) {
                    // Note: xhr.readyState==0 means we're doing post on page unload
                    if (xhr.readyState !== 0)
                        errorMessage('quicksave failed')
                })
                .always(function() {saving = false})
        }
    }

    quickload()

    setInterval(quicksave, 10000)

    $(window).unload(quicksave)

    wrap('div').attr('id', 'before_login_action').hide().appendTo(document.body).click(quicksave)
    wrap('div').attr('id', 'after_login_action').hide().appendTo(document.body).click(quickload)
    wrap('div').attr('id', 'before_logout_action').hide().appendTo(document.body).click(quicksave)
    wrap('div').attr('id', 'after_logout_action').hide().appendTo(document.body).click(quickload)
}

})();
