/*
EquaRes - a tool for numerical analysis of equations.
Copyright (C) 2014 Stepan Orlov

This file is part of EquaRes.

Full text of copyright notice can be found in file copyright_notice.txt in the EquaRes root directory.
License agreement can be found in file LICENSE.md in the EquaRes root directory.
*/

var snippetView = {}

;(function() {

var pickSnippet

function currentSnippetLink() {
    var links = $('#snippet-list a.current-snippet')
    return links.length === 1 ?   links[0] :   null
}
function currentSnippetRef() {
    var link = currentSnippetLink()
    return link ?   link.href :   undefined
}

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

function searchQuery(type) {
    var f = $('#snippet-filter > form')[0].elements
    var result = {type: type}
    for (var i=0, n=f.length; i<n; ++i) {
        var e = $(f[i])
        if (e.is(':visible'))
            result[e.attr('name')] = e.val()
    }
    return result
}

function clearCurrentSnippet() {
    $('#current-snippet-doc').html('')
    $('#current-snippet-tools').hide()
}

function loadSnippetList(type, cbks, page) {
    $('#snippet-apply-filter').attr('disabled', true)
    var rq = searchQuery(type)
    if (typeof page == 'number')
        rq.page = page
    $.get('/snippet-selection', rq)
        .done(function(data) {
            var tn = $('#snippet-list').html(data)
            var pagenum = tn.children('.snippet-pagenum'), previews = tn.children('.snippet-previews')
            pagenum.find('a').click(function(e) {
                loadSnippetList(type, cbks, $(this).text()-1)
            })
            var previewLinks = previews.find('a')
            previewLinks.click(function(e) {
                e.preventDefault()
                // Load snippet code
                var a = this
                $.get(this.href.replace('/snippet/', '/snippet-obj/'), function(snippet) {
                    formatInfo.update(snippet.doc, $('#current-snippet-doc')[0])
                    $('#current-snippet-tools').show()
                    previewLinks.removeClass('current-snippet')
                    $(a).addClass('current-snippet')
                })
                .fail(function(xhr) {
                    clearCurrentSnippet()
                    errorMessage(xhr.responseText || xhr.statusText || ("Failed to modify snippet: " + xhr.status));
                })
            })
            function snippetRef() {
                var d = $(this).parent(), info = {}, a = d.prev()
                return a.attr('href')
            }
            tn.find('.snippet-edit').click(function() {
                var ref = snippetRef.call(this)
                var dlg = $('#snippet-edit')
                $.get(ref)
                    .done(function(text) {
                        // dlg.find('#snippet-edit-code').val(text)
                        infoMessage('TODO: Edit snippet')
                    })
                    .fail(function(xhr) {
                        errorMessage(xhr.responseText || xhr.statusText || ("Failed to modify snippet: " + xhr.status));
                    })
            })
            tn.find('.snippet-remove').click(function() {
                var ref = snippetRef.call(this)
                var dlg = $('#confirm-snippet-remove')
                dlg.find('span.dlgmsg').html('This will permanently remove snippet <b>' + name + '</b> from database. Continue?')
                dlg.dialog({
                    resizable: false,
                    width: 450,
                    modal: true,
                    buttons: {
                        "Remove": function() {
                            debugger
                            $.get('remove-snippet', {type: 'TODO', name: 'TODO'})
                                .done(function() {loadSnippetList(type, cbks, page)})
                                .fail(function(xhr) {
                                    errorMessage(xhr.responseText || xhr.statusText || ("Failed to remove snippet: " + xhr.status));
                                })
                            $(this).dialog("close")
                        },
                        Cancel: function() {
                            $(this).dialog("close")
                        }
                    }
                })
            })
            clearCurrentSnippet()
        })
        .fail(function(xhr) {
            $('#snippet-list').html('ERROR')
            errorMessage(xhr.responseText || xhr.statusText || ("Failed to load snippet list: " + xhr.status));
        })
        .always(function() {
            $('#snippet-apply-filter').attr('disabled', false)
        })
}

function show(type, cbks) {
    if (pickSnippet) {
        pickSnippet.dialog({
            // resizable: false,
            width: 800,
            height: 600,
            modal: true,
            open: function() {
                loadSnippetList(type, cbks)
                callback('open', cbks)
            },
            buttons: {
                Pick: function() {
                    var snippetLink = currentSnippetLink()
                    if (!snippetLink)
                        return errorMessage('Please select a snippet')
                    callback.call(snippetLink, 'pick', cbks)
                    $(this).dialog("close")
                },
                Close: function() {
                    $(this).dialog("close")
                    callback('close', cbks)
                }
            }
        })
    }
    else {
        $.ajax('/pick-snippet', {cache: false})
            .done(function(data) {
                pickSnippet = $(data).appendTo($('body')).filter('#pick-snippet')
                var findSnippets = loadSnippetList.bind(null, type, cbks)
                $('#snippet-toggle-advanced').click(function(e) {
                    e.preventDefault()
                    var adv = $('#snippet-advanced-search'), wasadv = adv.is(':visible')
                    $(this).html(wasadv? 'Advanced': 'Basic')
                    $('#snippet-advanced-search').fadeToggle('fast', findSnippets)
                })
                $('#snippet-apply-filter').click(function(e) {
                    e.preventDefault()
                    findSnippets()
                })
                // $('#snippet-tools').children().click(function(e) { warningMessage('TODO: ' + this.title) })
                function rm() {
                    if (pickSnippet) {
                        pickSnippet.remove()
                        pickSnippet = undefined
                    }
                }

                var snippetCode = $('#edit-snippet-code')
                snippetCode.linenum()

                function reloadSnippetDoc() {
                    var snippetDoc = snippetCode.val() // TODO
                    formatInfo.update(snippetDoc, $('#current-snippet-doc')[0])
                }

                var firstInput = true
                snippetCode.on('input', function() {
                    $(this).addClass('modified')
                    reloadSnippetDoc()
                    if (firstInput) {
                        firstInput = false

                        // Warn user that it's necessary to press Ok
                        warningMessage('press Save button to save!')
                        var savebtn = $('#snippet-tool-save')
                        var c = savebtn.css('backgroundColor')
                        savebtn.css({backgroundColor: '#c00'})
                        savebtn.animate({backgroundColor: c})
                    }
                })

                var openEditor, closeEditor
                ;(function() {
                    var hiddenByEditor = $('#snippet-list,#current-snippet-tools'),
                        editor = $('#edit-snippet')
                    openEditor = function() {
                        hiddenByEditor.hide('fast')
                        snippetCode.removeClass('modified')
                        editor.show('fast')
                        firstInput = true
                        reloadSnippetDoc()
                    }
                    closeEditor = function() {
                        editor.hide('fast')
                        hiddenByEditor.show('fast')
                    }
                })()

                $('#snippet-tool-new').click(function(e) {
                    snippetCode.val('/*#\ntitle: Snippet title\nkeywords: keyword1, key word 2\n\n# Snippet title\n\nSnippet description\n*/\n\n// Add snippet code here\n')
                    openEditor()
                })

                $('#snippet-tool-edit').click(function(e) {
                    var snippetRef = currentSnippetRef()
                    if (!snippetRef)
                        return
                    $.get(snippetRef)
                        .done(function(text) {
                            snippetCode.val(text)
                            openEditor()
                        })
                        .fail(function(xhr) {
                            errorMessage(xhr.responseText || xhr.statusText || ("Failed to load snippet code: " + xhr.status));
                        })
                })
                $('#snippet-tool-cancel').click(function(e) {
                    closeEditor()
                })

                ;(function(ids){
                    for (var i=0; i<ids.length; ++i) {
                        var id = ids[i]
                        var e = $('#' + id)
                        if (e.length == 0)
                            e = $('<div id="'+id+'"></div>').hide().appendTo(pickSnippet)
                        e.click(rm)
                    }
                })(['after_login_action', 'after_logout_action'])
                show(type, cbks)
                })
            .fail(function(xhr) {
                errorMessage(xhr.responseText || xhr.statusText || ("Failed to load snippet view: " + xhr.status));
            })
    }

}

snippetView.pick = function(type, callback) {
    show(type, {
        pick: function() {
            $.get(this.href).done(function(text) {
                pickSnippet.dialog('close')
                callback(text)
            })
            .fail(function(xhr) {
                errorMessage(xhr.responseText || xhr.statusText || ("Failed to load snippet: " + xhr.status));
            })
        }
    })
}

})()
