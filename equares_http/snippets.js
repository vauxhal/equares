/*
EquaRes - a tool for numerical analysis of equations.
Copyright (C) 2014 Stepan Orlov

This file is part of EquaRes.

Full text of copyright notice can be found in file copyright_notice.txt in the EquaRes root directory.
License agreement can be found in file LICENSE.md in the EquaRes root directory.
*/

var fs = require('fs')
var mongoose = require('mongoose')
var textSearch = require('mongoose-text-search');
var auth = require('./auth')
var cp = require('child_process')
var dbsearch = require('./dbsearch')
var path = require('path')

var ObjectId = mongoose.Schema.Types.ObjectId

var SnippetSchema = mongoose.Schema({
    name:           {type: String, index: true},
    title:          String,
    date:           Date,
    keywords:       {type: [String], index: true},
    type:           {type: String, index: true},
    data:           String,
    doc:            String,
    user:           {type: ObjectId, index: true}
})

// give our schema text search capabilities
SnippetSchema.plugin(textSearch);

SnippetSchema.index({name: 1, user: 1, type: 1, doc: 1}, {unique: true})
SnippetSchema.index({name: 'text', title: 'text', keywords: 'text'})

SnippetSchema.statics.upsert = function(snippet, done) {
    this.update({user: snippet.user, name: snippet.name, type: snippet.type}, snippet, {upsert: true}, done)
}
SnippetSchema.statics.have = function(snippet, done) {
    this.count({user: snippet.user, name: snippet.name, type: snippet.type}, done)
}

var Snippet = mongoose.model('Snippet', SnippetSchema, 'snippets')

function snippetFromText(text) {
    var lines = text.split('\n')
    var rxDocStart = /^\/\*#$/
    var docLines = [], dataLines = [], out = dataLines
    var keywords = [], title = ''
    var rxKeywords = /^\s*keywords:\s*(.*)$/, rxTitle = /^\s*title:\s*(.*)$/
    for (var i=0; i<lines.length; ++i) {
        var line = lines[i]
        if (out === dataLines && line === '/*#') {
            out = docLines
            continue
        }
        else if (out === docLines) {
            if (line === '*/') {
                out = dataLines
                continue
            }
            var m = line.match(rxKeywords)
            if (m) {
                m = m[1].toLowerCase().split(',')
                for (var ik=0; ik<m.length; ++ik) {
                    var kw = m[ik].trim()
                    if (kw)
                        keywords.push(kw)
                }
                continue
            }
            m = line.match(rxTitle)
            if (m) {
                if (!title)
                    title = m[1].trim()
                continue
            }
        }
        out.push(line)
    }

    return {
        data: dataLines.join('\n'),
        doc: docLines.join('\n'),
        title: title,
        keywords: keywords
    }
}

function snippetToText(snippet) {
    var lines = []
    if (snippet.title)
        lines.push('title: ' + snippet.title)
    if (snippet.keywords)
        lines.push('keywords: ' + snippet.keywords.join(', '))
    if (snippet.doc)
        lines.push(snippet.doc)
    if (lines.length > 0) {
        lines.splice(0, 0, '/*#')
        lines.push('*/')
    }
    if (snippet.data)
        lines.push(snippet.data)
    return lines.join('\n')
}

function pickSnippet(req, res) {
    res.render('pick-snippet', {req: req})
}

function snippetSelection (req, res) {
    if (typeof req.query.type != 'string')
        return res.send(400)
    dbsearch(req, {
        model: Snippet,
        query: {type: req.query.type},
        project: {name: 1, user: 1, keywords: 1, title: 1},
        pageSize: 100
    }, function(result) {
        if (result.err)
            return res.send(result.err.code || 500, result.err.message || result.err.data)
        var count = result.records.length
        if (result.pagenum)
            res.write('<div class="snippet-pagenum">' + result.pagenum + '</div>')
        res.write('<div class="snippet-previews">')
        for (var i=0; i<count; ++i) {
            var snippet = result.records[i],
                a = ['', 'snippet', req.query.type, snippet.user? snippet.user: '-', snippet.name],
                name = a.join('/')
            res.write('<div class="snippet-container">')
            res.write('<a href="' + name + '">' + snippet.name + ' - ' + snippet.title + '</a>')
            if (snippet.edit)
                res.write(
                    '<div class="snippet-control">' +
                        '<div class="snippet-tool snippet-edit"></div><div class="snippet-tool snippet-remove"></div>' +
                    '</div>'
                )
            res.write('</div>')
        }
        res.end('</div>')
    })
}

function getSnippet(req, res, transform) {
    var a = req.path.split('/')
    if(a.length != 4)
        return res.send(404, 'Snippet is not found')
    var type = a[1], user = a[2], name = a[3]

    function serve() {
        Snippet.findOne({name: name, user: user, type: type}, function(err, snippet) {
            if (err) {
                console.log(err)
                res.send(500)
            }
            else if (snippet) {
                snippet = snippet.toObject()
                snippet.username = user
                res.send(transform(snippet))
            }
            else
                res.send(404, 'Snippet is not found')
        })
    }

    if (user == '-') {
        user = null
        serve()
    }
    else
        auth.User.findUser(user, function(err, userId) {
            if (err) {
                console.log(err)
                return res.send(500, 'Unable to resolve user')
            }
            else if (!userId)
                return res.send(404, 'No such user')
            user = userId._id
            serve()
        })
}


function getSnippetText(req, res) {
    return getSnippet(req, res, function(snippet) {
        return snippetToText(snippet)
    })
}

function getSnippetObj(req, res) {
    return getSnippet(req, res, function(snippet) {
        return {
            name:       snippet.name,
            title:      snippet.title,
            date:       snippet.date,
            keywords:   snippet.keywords,
            type:       snippet.type,
            data:       snippet.data,
            user:       snippet.username
        }
    })
}

function checkName(name, res, options) {
    function errmsg() {
        options = options || {}
        return options.errmsg || 'Bad name'
    }
    if (!(typeof name == 'string')) {
        res.send(400, errmsg() + ' - value is missing or has an invalid type')
        return false
    }
    if (name.indexOf('/') === -1) {
        res.send(400, errmsg() + ' - must contain no slashes')
        return false
    }
    return true
}

function uploadSnippet(req, res) {
    if (!req.isAuthenticated())
        return res.send(401, 'You are not logged in')
    var snippet = snippetFromText(req.body.text)
    snippet.name = req.body.name
    snippet.type = req.body.type
    snippet.user = req.user.id
    snippet.date = new Date()

    if (!checkName(snippet.name, res, {errmsg: 'Bad snippet name'}))
        return
    if (!checkName(snippet.type, res, {errmsg: 'Bad snippet type'}))
        return

    function save() {
        Snippet.upsert(snippet, function(err, doc) {
            if (err) {
                console.log(err)
                res.send(500, 'Failed to upload snippet')
            }
            else
                res.send(['', 'snippet', snippet.type, req.user.username, snippet.name].join('/'))
        })
    }

    if (req.body.overwrite)
        save()
    else
        Snippet.have(snippet, function(err, count) {
            if (err)
                res.send(500)
            else if (count > 0)
                res.send(403, 'Snippet with the specified name and type already exists')
            else
                save()
        })
}

function copyProps(dst, src) {
    for (var i in src)
        dst[i] = src[i]
    return dst
}

function editSnippet(req, res) {
    if (!req.isAuthenticated())
        return res.send(401, 'You are not logged in')
    var snippet = JSON.parse(req.body.snippet)
    if (typeof snippet.name != 'string')
        return res.send(400, 'Invalid query')
    Snippet.findOne({name: snippet.name, user: req.user.id, type: snippet.type}, function(err, s) {
        if (err) {
            console.log(err)
            res.send(500, err)
        }
        else if (s) {
            delete snippet.name
            delete snippet.type
            copyProps(s, snippet)
            s.save(function(err) {
                if (err) {
                    console.log(err)
                    res.send(500, 'Failed to modify snippet')
                }
                else
                    res.send(['', 'snippet', snippet.type, req.user.username, snippet.name].join('/'))
            })
        }
        else
            res.send(404, 'No such snippet')
    })
}

function removeSnippet(req, res) {
    if (!req.isAuthenticated())
        return res.send(401, 'You are not logged in')
    var name = req.query.name, type = req.query.type
    if (typeof name != 'string')
        return res.send(400, 'Invalid query')
    Snippet.findOne({name: name, user: req.user.id, type: type}, function(err, s) {
        if (err) {
            console.log(err)
            res.send(500, err)
        }
        else if (s)
            s.remove(function(err) {
                if (err) {
                    console.log(err)
                    res.send(500, err)
                }
                else
                    res.end()
            })
        else
            res.send(404, 'No such snippet')
    })
}

function refreshSnippets() {
    var dir = 'public/meta/snippets'
    var types = fs.readdirSync(dir)
    console.log('Loading snippets ...')
    var date = new Date(2014, 7, 21)
    for (var it=0; it<types.length; ++it) {
        var type = types[it], subdir = path.join(dir, type)
        var snippets = fs.readdirSync(subdir)
        for (var is=0; is<snippets.length; ++is) {
            var name = snippets[is],
                text = fs.readFileSync(path.join(subdir, name), 'utf8'),
                snippet = snippetFromText(text)
            console.log('  ' + type + '/' + name + ': ' + snippet.title)
            snippet.type = type
            snippet.name = name
            snippet.user = null
            snippet.date = date
            Snippet.upsert(snippet, function(err, doc) {
                if (err)
                    console.log(err)
            })
        }
    }
    console.log('... Finished loading snippets')
}

refreshSnippets()

module.exports = function(app) {
    app.get('/pick-snippet', pickSnippet)
    app.get('/snippet-selection', snippetSelection)
    app.use('/snippet', getSnippetText)
    app.use('/snippet-obj', getSnippetObj)
    app.post('/upload-snippet', uploadSnippet)
    app.post('/edit-snippet', editSnippet)
    app.get('/remove-snippet', removeSnippet)
}
