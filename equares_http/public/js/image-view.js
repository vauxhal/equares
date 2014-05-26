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

function makeKeywords(str) {
    var val = str.toLowerCase().split(',')
    for (var i=0; i<val.length; ++i) {
        var s = val[i].trim()
        if (s.length > 0)
            val[i] = s
        else
            val.splice(i, 1)
    }
    return val
}

function searchQuery() {
    var f = $('#img-filter > form')[0].elements
    var result = {}
    for (var i=0, n=f.length; i<n; ++i) {
        var e = $(f[i])
        if (e.is(':visible'))
            result[e.attr('name')] = e.val()
    }
    return result
}

var page

function loadImageThumbnails(cbks) {
    $('#img-apply-filter').attr('disabled', true)
    var rq = searchQuery()
    if (typeof page == 'number')
        rq.page = page
    $.get('/image-thumbnails', rq)
        .done(function(data) {
            var tn = $('#image-thumbnails')
            tn.html(data).find('a').click(function(e) {
                e.preventDefault()
                callback.call(this, 'pick', cbks)
            })
            function imginfo() {
                var d = $(this).parent(), info = {}, a = d.prev(), img = a.children('img')
                info.name = a.attr('href').split('/')[3]
                info.title = img.attr('title')
                info.keywords = d.find('.img-kw').text()
                return info
            }
            tn.find('.img-edit').click(function() {
                var info = imginfo.call(this)
                var dlg = $('#img-edit')
                dlg.find('#img-edit-title').val(info.title)
                dlg.find('#img-edit-keywords').val(info.keywords)
                var imgUploader = dlg.find('#img-edit-uploader')
                // Reset file uploader - see http://stackoverflow.com/questions/1043957/clearing-input-type-file-using-jquery
                imgUploader.replaceWith(imgUploader=imgUploader.clone(true))
                dlg.dialog({
                    resizable: true,
                    modal: true,
                    width: 500,
                    buttons: {
                        Commit: function() {
                            var file = imgUploader[0].files[0]
                            function editImage(data, type) {
                                var args = {
                                    name: info.name,
                                    title: $('#img-edit-title').val(),
                                    keywords: makeKeywords($('#img-edit-keywords').val())
                                }
                                if (data)
                                    $.extend(args, {
                                        data: data,
                                        contentType: type
                                    })

                                $.post('/edit-image', {img: JSON.stringify(args)})
                                    .done(function(url) {
                                        infoMessage('File modified')
                                        loadImageThumbnails(cbks)
                                    })
                                    .fail(function(xhr) {
                                        errorMessage(xhr.responseText || xhr.statusText || ("Failed to modify image: " + xhr.status));
                                    })
                            }
                            if (file) {
                                var fileReader = new FileReader
                                fileReader.onload = function(fileLoadedEvent) {
                                    editImage(fileLoadedEvent.target.result, file.type)
                                }
                                fileReader.readAsBinaryString(file)
                            }
                            else
                                editImage()
                            $(this).dialog("close")
                        },
                        Cancel: function() {
                            $(this).dialog("close")
                        }
                    }
                })
            })
            tn.find('.img-remove').click(function() {
                var name = imginfo.call(this).name
                var dlg = $('#confirm-img-remove')
                dlg.find('span.dlgmsg').html('This will permanently remove image <b>' + name + '</b> from database. Continue?')
                dlg.dialog({
                    resizable: false,
                    width: 450,
                    modal: true,
                    buttons: {
                        "Remove": function() {
                            $.get('remove-image', {name: name})
                                .done(function() {loadImageThumbnails(cbks)})
                                .fail(function(xhr) {
                                    errorMessage(xhr.responseText || xhr.statusText || ("Failed to remove image: " + xhr.status));
                                })
                            $(this).dialog("close")
                        },
                        Cancel: function() {
                            $(this).dialog("close")
                        }
                    }
                })
            })
        })
        .fail(function(xhr) {
            errorMessage(xhr.responseText || xhr.statusText || ("Failed to load image thumbnails: " + xhr.status));
        })
        .always(function() {
            $('#img-apply-filter').attr('disabled', false)
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
                keywords: makeKeywords($('#image-upload-keywords').val())
            })})
                .done(function(url) {
                    infoMessage('File uploaded')
                    callback.call({href: url}, 'pick', cbks)
                })
                .fail(function(xhr) {
                    errorMessage(xhr.responseText || xhr.statusText || ("Failed to upload image: " + xhr.status));
                })
                .always(function() {
                    // Reset file uploader - see http://stackoverflow.com/questions/1043957/clearing-input-type-file-using-jquery
                    imgUploader.replaceWith(imgUploader=imgUploader.clone(true))
                    uploadBtn.attr('disabled', true)
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
                var findImg = loadImageThumbnails.bind(null, cbks)
                $('#img-toggle-advanced').click(function(e) {
                    e.preventDefault()
                    var adv = $('#img-advanced-search'), wasadv = adv.is(':visible')
                    $(this).html(wasadv? 'Advanced': 'Basic')
                    $('#img-advanced-search').fadeToggle('fast', findImg)
                })
                $('#img-apply-filter').click(function(e) {
                    e.preventDefault()
                    findImg()
                })
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
