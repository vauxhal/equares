/*
EquaRes - a tool for numerical analysis of equations.
Copyright (C) 2014 Stepan Orlov

This file is part of EquaRes.

Full text of copyright notice can be found in file copyright_notice.txt in the EquaRes root directory.
License agreement can be found in file LICENSE.md in the EquaRes root directory.
*/

var fs = require('fs')
var mongoose = require('mongoose')
var path = require('path')
var auth = require('./auth')
var simulation = require('./simulation')
var Sim = simulation.Sim

var ObjectId = mongoose.Schema.Types.ObjectId

var TutorialSchema = mongoose.Schema({
    name:           {type: String, index: true},
    title:          String,
    date:           Date,
    keywords:       {type: [String], index: true},
    content:        String,
    user:           {type: ObjectId, index: true}
})

TutorialSchema.index({name: 1, user: 1}, {unique: true})

TutorialSchema.statics.upsert = function(tutorial, done) {
    this.update({user: tutorial.user, name: tutorial.name}, tutorial, {upsert: true}, done)
}
TutorialSchema.statics.have = function(tutorial, done) {
    this.count({user: tutorial.user, name: tutorial.name}, done)
}

var Tutorial = mongoose.model('Tutorial', TutorialSchema, 'tutorials')

function tutorialFromText(text) {
    var lines = text.split('\n')
    var keywords = [], title = ''
    var rxKeywords = /^\s*keywords:\s*(.*)$/, rxTitle = /^#\s*(.*)$/
    var content = []
    for (var i=0; i<lines.length; ++i) {
        var line = lines[i]
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
        if (!title) {
            m = line.match(rxTitle)
            if (m) {
                title = m[1].trim()
                continue
            }
        }
        content.push(line)
    }

    return {
        //name: ???,
        title: title,
        keywords: keywords,
        content: content.join('\n')
    }
}

function tutorialToText(tutorial) {
    var lines = []
    if (tutorial.title)
        lines.push('# ' + tutorial.title)
    if (tutorial.keywords)
        lines.push('keywords: ' + tutorial.keywords.join(', '))
    if (tutorial.content)
        lines.push(tutorial.content)
    return lines.join('\n')
}

function getTutorial(req, res, transform) {
    var a = decodeURI(req.path).split('/')
    if(a.length != 3)
        return res.send(404, 'Tutorial is not found')
    var user, username = a[1], name = a[2]

    function serve() {
        Tutorial.findOne({name: name, user: user}, function(err, tutorial) {
            if (err) {
                console.log(err)
                res.send(500)
            }
            else if (tutorial) {
                tutorial = tutorial.toObject()
                tutorial.username = username
                transform(req, res, tutorial)
            }
            else
                res.send(404, 'Tutorial is not found')
        })
    }

    if (username == '-') {
        user = null
        serve()
    }
    else
        auth.User.findUser(username, function(err, userId) {
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


function getTutorialText(req, res) {
    getTutorial(req, res, function(req, res, tutorial) {
        var relatedSim = []
        var sent
        Sim.find({user: tutorial.user, keywords: 'tutorial ' + tutorial.name}, {name: 1, description: 1, keywords: 1}).stream()
            .on('data', function (doc) {
                var path = '/editor?sim=';
                if (tutorial.user)
                    path += tutorial.username + '/'
                path += doc.name
                var order = 0, kw = doc.keywords
                for (var i=0; i<kw.length; ++i) {
                    var m = kw[i].match(/^tutorial-order\s+(\d+)$/)
                    if (m) {
                        order = +m[1]
                        break
                    }
                }
                relatedSim.push({order: order, text: '- =[' + doc.description + '](' + path + ')'})
            }).on('error', function (err) {
                console.log(err)
                if (!sent)
                    res.send(500)
                sent = true
            }).on('close', function () {
                var text = tutorialToText(tutorial)
                if (relatedSim.length > 0) {
                    relatedSim.sort(function(a, b) { return a.order < b.order? -1: b.order < a.order? 1: 0 })
                    text += '\n\n# Related simulations\n'
                    for (var i=0; i<relatedSim.length; ++i)
                        text += relatedSim[i].text + '\n'
                }
                if (!sent)
                    res.send(text)
                sent = true
            })
    })
}

function getTutorialObj(req, res) {
    getTutorial(req, res, function(req, res, tutorial) {
        res.send({
            name:       tutorial.name,
            title:      tutorial.title,
            date:       tutorial.date,
            keywords:   tutorial.keywords,
            content:    tutorial.content,
            user:       tutorial.username,
            id:         tutorial._id,
            editable:   req.isAuthenticated()?   tutorial.username === req.user.username:   false
        })
    })
}

function checkName(name, res, options) {
    function errmsg() {
        options = options || {}
        return options.errmsg || 'Bad name'
    }
    if (typeof name !== 'string') {
        res.send(400, errmsg() + ' - value is missing or has an invalid type')
        return false
    }
    if (name.indexOf('/') !== -1) {
        res.send(400, errmsg() + ' - must contain no slashes')
        return false
    }
    if (name.length < 1) {
        res.send(400, errmsg() + ' - empty string')
        return false
    }
    return true
}

function tutorialName(title) {
    var result = ''
    if (typeof title != 'string')
        return result
    var badChar = ' /&%\\#?=\t.,:;[](){}<>|+&*@~!^`\'"'
    title = title.toLowerCase()
    for (var i=0, n=title.length; i<n; ++i) {
        var c = title[i]
        if (badChar.indexOf(c) !== -1)
            c = ' '
        result += c
    }
    return result.trim().replace(/\s+/g, '-')
}

function tutorialFromReq(req, res) {
    if (!req.isAuthenticated()) {
        res.send(401, 'You are not logged in')
        return
    }
    var tutorial = tutorialFromText(req.body.text)
    tutorial.name = tutorialName(tutorial.title)
    tutorial.user = req.user.id
    tutorial.date = new Date()

    if (!checkName(tutorial.name, res, {errmsg: 'Bad tutorial name'}))
        return
    return tutorial
}

function uploadTutorial(req, res) {
    var tutorial = tutorialFromReq(req, res)
    if (!tutorial)
        return

    function save() {
        Tutorial.upsert(tutorial, function(err, doc) {
            if (err) {
                console.log(err)
                res.send(500, 'Failed to upload tutorial')
            }
            else
                res.send(['', 'tutorial', tutorial.type, req.user.username, tutorial.name].join('/'))
        })
    }

    if (req.body.overwrite)
        save()
    else
        Tutorial.have(tutorial, function(err, count) {
            if (err)
                res.send(500)
            else if (count > 0)
                res.send(403, 'Tutorial with the specified name already exists')
            else
                save()
        })
}

function copyProps(dst, src) {
    for (var i in src)
        dst[i] = src[i]
    return dst
}

function editTutorial(req, res) {
    var tutorial = tutorialFromReq(req, res)
    if (!tutorial)
        return
    if (!req.body.tutorialId)
        return res.send(400, 'Missing tutorial id')

    Tutorial.findById(req.body.tutorialId, function(err, s) {
        if (err) {
            console.log(err)
            res.send(500, err)
        }
        else if (s) {
            if (!s.user || s.user.toString() !== req.user.id.toString())
                return res.send(403, 'Cannot modify tutorials other than yours')
            function save() {
                copyProps(s, tutorial)
                s.save(function(err) {
                    if (err) {
                        console.log(err)
                        res.send(500, 'Failed to save tutorial: ' + err)
                    }
                    else
                        res.send(['', 'tutorial', req.user.username, tutorial.name].join('/'))
                })
            }
            if (s.name === tutorial.name)
                save()
            else
                Tutorial.have(tutorial, function(err, count) {
                    if (err)
                        res.send(500)
                    else if (count > 0)
                        res.send(403, 'Tutorial with the specified name already exists')
                    else
                        save()
                })
        }
        else
            res.send(404, 'No such tutorial')
    })
}

function removeTutorial(req, res) {
    if (!req.isAuthenticated())
        return res.send(401, 'You are not logged in')
    var name = req.query.name
    if (typeof name != 'string')
        return res.send(400, 'Invalid query')
    name = decodeURI(name)
    Tutorial.findOne({name: name, user: req.user.id}, function(err, s) {
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
            res.send(404, 'No such tutorial')
    })
}

function refreshTutorials() {
    var dir = 'public/meta/tutorials'
    console.log('Loading tutorials ...')
    var date = new Date(2014, 9, 7)
    var tutorials = fs.readdirSync(dir)
    for (var i=0; i<tutorials.length; ++i) {
        var name = tutorials[i],
            text = fs.readFileSync(path.join(dir, name), 'utf8'),
            tutorial = tutorialFromText(text)
        console.log('  ' + name + ': ' + tutorial.title)
        tutorial.name = tutorialName(tutorial.title)
        tutorial.user = null
        tutorial.date = date
        Tutorial.upsert(tutorial, function(err, doc) {
            if (err)
                console.log(err)
        })
    }
    console.log('... Finished loading tutorials')
}

refreshTutorials()

module.exports = function(app) {
    app.use('/tutorial', getTutorialText)
    app.use('/tutorial-obj', getTutorialObj)
    app.post('/upload-tutorial', uploadTutorial)
    app.post('/edit-tutorial', editTutorial)
    app.get('/remove-tutorial', removeTutorial)
}

module.exports.findTutorials = function(cb) {
    var lst = []
    Tutorial.find({}, {name: 1, title: 1, user: 1}).stream()
        .on('data', function (doc) {
            lst.push(doc.toObject())
        }).on('error', function (err) {
            console.log(err)
            cb(err, null)
        }).on('close', function () {
            var n = 0, replied
            function proceed(err) {
                if (replied)
                    return
                if (err) {
                    console.log(err)
                    cb(err)
                    replied = true
                }
                if (n < lst.length)
                    return
                cb(null, lst)
                replied = true
            }
            for (var i=0; i<lst.length; ++i) {
                (function(x) {
                    if (x.user)
                        auth.User.username(x.user, function(username) {
                            if (!username)
                                proceed('Failed to find user by id')
                            x.name = username + '/' + x.name
                            ++n
                            proceed()
                        })
                    else {
                        x.name = '-/' + x.name
                        ++n
                        proceed()
                    }
                 })(lst[i])
            }
        })
}
