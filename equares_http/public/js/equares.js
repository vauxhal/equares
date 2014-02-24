ctm.initPage.push(function(c1, c2, c3) {
    function setMenuItemHandler(j, func) {
        j.click(function(e) {
            e.preventDefault()
            func.call(this)
        })
    }
    setMenuItemHandler($('.example-simulation'), function() { equaresui.loadExample(this.href) })
    setMenuItemHandler($('#run-simulation'), function() { equaresui.runScheme() })
    setMenuItemHandler($('#upload-simulation'), function() { $("#open-scheme-file-dialog").dialog("open") })
    setMenuItemHandler($('#download-simulation'), function() { $("#save-scheme-file-dialog").dialog("open") })

    // Create dialogs
    function updateOpenState() {
        var f = $("#open-scheme-file-dialog :file")[0]
        var o = $(".ui-dialog-buttonpane button:contains('Open')")
        o.button(f.files.length>0? "enable": "disable")
    }
    $("#open-scheme-file-dialog :file").change(updateOpenState)

    $("#open-scheme-file-dialog").dialog({
        autoOpen: false,
        modal: true,
        width: 400,
        buttons: {
            Open: function() {
                equaresui.openScheme($("#open-scheme-file")[0].files[0])
                $(this).dialog("close")
            },
            Cancel: function() { $(this).dialog("close") }
        },
        open: function() {
                //$(this).children("form")[0].reset()
                updateOpenState()
            }
    })
    $("#save-scheme-file-dialog").dialog({
        autoOpen: false,
        modal: true,
        buttons: {
            Save: function() {
                equaresui.saveScheme($("#save-scheme-file-name").val())
                $(this).dialog("close")
            },
            Cancel: function() { $(this).dialog("close") }
        }
    })

    $("#running-simulation").dialog({
        autoOpen: false,
        modal: true,
        width: 800,
        close: function() { equaresui.stopSimulation() },
        buttons: {
            Stop: function() { equaresui.stopSimulation() },
            Close: function() { $(this).dialog("close") }
        }
    })

    var loadingProgressLabel = $("#loading-progress-label"),
        loadingProgress = $("#loading-progress").progressbar({
        value: false,
        change: function() {
            loadingProgressLabel.text( loadingProgress.progressbar( "value" ) + "%" );
        },
        complete: function() {
            $("#loading-progress-overlay").hide()
        }
    })

    $(c2.dom).addClass("mymain");
    $(c3.dom).addClass("myfooter").html('<a target="_blank" href="http://ctmech.ru/">Computer Technologies in Engineering</a>');

    // Init equaresBox engine; Set scene source by default when the initialization is done
    equaresBox.init(function() {
        equaresui.setSceneSource.call(c2)
    }, function(percent) {
        loadingProgress.progressbar("value", percent)
    })
})
