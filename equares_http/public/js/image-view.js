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

function bindUpload(cbks) {
    var imgUploader = $('#image-uploader'),
        uploadBtn = $('#image-upload')

    imgUploader.change(function () {
        uploadBtn.attr('disabled', imgUploader[0].files.length == 0)
    })

    uploadBtn.click(function() {
        var fileReader = new FileReader
        var file = imgUploader[0].files[0]
        fileReader.onload = function(fileLoadedEvent) {
            var data = fileLoadedEvent.target.result
            $.post('/upload-image', {img: JSON.stringify({
                name: file.name,
                data: data,
                contentType: file.type,
                title: $('#image-upload-title').val(),
                keywords: (function(val) {
                    val = val.toLowerCase().split(',')
                    for (var i=0; i<val.length; ++i) {
                        var s = val[i].trim()
                        if (s.length > 0)
                            val[i] = s
                        else
                            val.splice(i, 1)
                    }
                    return val
                })($('#image-upload-keywords').val())
            })})
                .done(function(url) {
                    infoMessage('File uploaded')
                    callback.call({href: url}, 'pick', cbks)
                })
                .fail(function(xhr) {
                    errorMessage(xhr.responseText || xhr.statusText || ("Failed to upload image: " + xhr.status));
                })
        }
        fileReader.readAsBinaryString(file)
    })
}

function show(cbks) {
    if (pickImage) {
        pickImage.dialog({
            // resizable: false,
            width: 800,
            height: 600,
            modal: true,
            open: function() {
                $('.accordion').accordion({ heightStyle: "fill" })
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
                function rm() {
                    if (pickImage) {
                        pickImage.remove()
                        pickImage = undefined
                    }
                }
                ;(function(ids){
                    for (var i=0; i<ids.length; ++i) {
                        var id = ids[i]
                        var e = $('#' + id)
                        if (e.length == 0)
                            e = $('<div id="'+id+'"></div>').hide().appendTo(pickImage)
                        e.click(rm)
                    }
                })(['after_login_action', 'after_logout_action'])
                bindUpload(cbks)
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
            var url = this.href.replace(window.location.origin, '')
            pickImage.dialog('close')
            callback(url)
        }
    })
}

})()
