ctm.initMenu.push(function() {
    function setMenuItemHandler(j, func) {
        j.click(function(e) {
            e.preventDefault()
            func.call(this)
        })
    }
    setMenuItemHandler($('.example-simulation'), function() { equaresui.loadExample(this.href) })
    setMenuItemHandler($('#run-simulation'), function() { equaresui.runScheme() })
    setMenuItemHandler($('#upload-simulation'), function() { equaresui.uploadSimulation() })
    setMenuItemHandler($('#download-simulation'), function() { equaresui.downloadSimulation() })
    setMenuItemHandler($('#save-simulation'), function() { equaresui.saveSimulation() })
    setMenuItemHandler($('#clear-simulation'), function() { equaresui.clearSimulation() })
})

ctm.initPage.push(function(c1, c2, c3) {
    // Create running simulation dialog
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
