var imageView = {}

;(function() {

var pickImage

function callback(name, callbacks) {
    var cb
    if (arguments.length > 1) {
        if (callbacks)
            cb = callbacks[name]
    }
    else
        cb = name
    if (cb instanceof Function)
        cb.call(this)
}

function loadImageThumbnails(cbks) {
    $.get('/image-thumbnails')
        .done(function(data) {
            var tn = $('#image-thumbnails')
            tn.html(data).find('a').click(function(e) {
                e.preventDefault()
                callback.call(this, 'pick', cbks)
            })
        })
        .fail(function(xhr) {
            errorMessage(xhr.responseText || xhr.statusText || ("Failed to load image thumbnails: " + xhr.status));
        })
}

function show(cbks) {
    if (pickImage) {
        pickImage.dialog({
            // resizable: false,
            width: 600,
            modal: true,
            open: function() {
                loadImageThumbnails(cbks)
                callback('open', cbks)
            },
            buttons: {
                Close: function() {
                    $(this).dialog("close")
                    callback('close', cbks)
                }
            }
        })
    }
    else {
        $.ajax('/pick-image', {cache: false})
            .done(function(data) {
                pickImage = $(data).appendTo($('body')).filter('#pick-image')
                show(cbks)
            })
            .fail(function(xhr) {
                errorMessage(xhr.responseText || xhr.statusText || ("Failed to load image view: " + xhr.status));
            })
    }

}

imageView.pick = function(callback) {
    show({
        pick: function() {
            callback(this.href)
            pickImage.dialog('close')
        }
    })
}

})()
