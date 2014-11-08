function unsupportedBrowser(msg) {
    var dlg = $('#unsupported-browser-error')
    function showDlg() {
        dlg.dialog({
            modal: true,
            resizable: false,
            buttons: {
                Ok: function() {
                    $(this).dialog("close")
                }
            }
        })
    }
    if (dlg.length) {
        $('#unsupported-browser-error-message').text(msg)
        showDlg()
    }
    else {
        $.get('try-different-browser', {message:msg})
            .done(function(data) {
                dlg = $(data).appendTo($('body'))
                showDlg()
            })
            .fail(function(error) {
                errorMessage(msg + '<br/>' + (error.responseText || error.statusText || ("Ajax error: " + error.status)))
            })
    }
}
