/*
EquaRes - a tool for numerical analysis of equations.
Copyright (C) 2014 Stepan Orlov

This file is part of EquaRes.

Full text of copyright notice can be found in file copyright_notice.txt in the EquaRes root directory.
License agreement can be found in file LICENSE.md in the EquaRes root directory.
*/

var mongoose = require('mongoose')
var auth = require('./auth')

function pageNumbers(page, pages) {
    function wrap(tag, options) {
        options = options || {}
        var result = '<' + tag
        if (options.attr)
            for (var i in options.attr) {
                var a = options.attr[i]
                if (typeof a == 'string')
                    a = JSON.stringify(a)
                else
                    a = '"' + a.toString() + '"'
                result += ' ' + i + '=' + a
            }
        result += '>'
        if (options.body)
            result += options.body
        result += '</' + tag + '>'
        return result
    }
    function formatPage(p) {
        return p === page ?   wrap('span', {body: p+1}) :   wrap('a', {attr: {href: '#'}, body: p+1})
    }
    function formatPages(center, cache) {
        var borderPages = 4,
            n1 = Math.max(0, center-borderPages),
            n2 = Math.min(pages-1, center+borderPages),
            result = ''
        for (var i=n1; i<=n2; ++i) {
            if (!cache[i]) {
                if (i>0 && !cache[i-1])
                    result += wrap('span', {body: '...'})
                result += formatPage(i)
                cache[i] = 1
            }
        }
        return result
    }

    var cache = {}
    return pages === 1 ?   '' :   formatPages(0, cache) + formatPages(page, cache) + formatPages(pages-1, cache)
}

function dbsearch(req, options, cb) {
    options = options || {}
    var records = []
    var processed = false, count = 0, finished = false
    var total
    var pageSize = options.pageSize || 25
    var page = req.query.page === undefined?  0:   +req.query.page

    function proceed() {
        if (!finished || count < records.length || processed)
            return
        var pages = Math.ceil(total/pageSize)
        cb({pages: pages, records: records, pagenum: pageNumbers(page, pages)})
        processed = true
    }

    var user = req.user? new mongoose.Types.ObjectId(req.user.id.toString()): null
    var query = options.query, andClause
    function addAndCondition(cond) {
        if (andClause)
            andClause.push(cond)
        else if (query)
            query = {$and: andClause=[query, cond]}
        else
            query = cond
    }
    if (options.model.schema.paths.public) {
        if (req.query.public == 'true')
            addAndCondition({public: true})
        else
            addAndCondition({$or: [{public: true}, {user: user}]})
    }
    if (req.query.keywords) {
        var kw = req.query.keywords.toLowerCase().split(',')
        for (var i=0; i<kw.length; ++i) {
            var s = kw[i].trim()
            if (s.length > 0)
                addAndCondition({keywords: s})
        }
    }

    function doSearch() {
        if (req.query.text) {
            options.model.textSearch(req.query.text,
                {
                    filter: query,
                    project: options.project
                },
                function(err, output) {
                    if (err) {
                        console.log(err)
                        return cb({err: {data: err, code: 500}})
                    }
                    total = output.results.length
                    var n1 = page*pageSize, n2 = (page+1)*pageSize
                    if (n2 > total)
                        n2 = total
                    for (var i=n1; i<n2; ++i) {
                        (function() {
                            var doc = output.results[i].obj, obj = doc.toObject()
                            records.push(obj)
                            auth.User.username(doc.user, function(username) {
                                ++count
                                obj.user = username
                                obj.edit = req.user && username == req.user.username
                                proceed()
                            })
                        })()
                    }
                    finished = true
                    proceed()
                })
        }
        else {
            options.model.count(query, function(err, n) {
                if (err) {
                    console.log(err)
                    return cb({err: {data: err, code: 500}})
                }
                total = n
                var n1 = page*pageSize, n2 = (page+1)*pageSize
                if (n2 > total)
                    n2 = total
                if (n1 > n2)
                    n1 = n2
                options.model.find({$query: query || {}, $orderby: {date: -1}},
                    options.project,
                    {skip: n1, limit: n2-n1}).stream()
                    .on('data', function (doc) {
                        var obj = doc.toObject()
                        records.push(obj)
                        auth.User.username(doc.user, function(username) {
                            ++count
                            obj.user = username
                            obj.edit = req.user && username == req.user.username
                            proceed()
                        })
                    }).on('error', function (err) {
                        console.log(err)
                        if (!processed) {
                            cb({err: {data: err, message: 'Unable to read query results from database', code: 500}})
                            processed = true
                        }
                    }).on('close', function () {
                        finished = true
                        proceed()
                    })
            })
        }
    }

    if (req.query.user === '-') {
        addAndCondition({user: null})
        doSearch()
    }
    else if (req.query.user) {
        auth.User.findUser(req.query.user, function(err, user) {
            if (err) {
                console.log(err)
                return cb({err: {data: err, message: 'Unable to resolve user', code: 500}})
            }
            else if (!user)
                return cb({err: {message: 'No such user', code: 404}})
            addAndCondition({user: user._id})
            doSearch()
        })
    }
    else
        doSearch()
}

module.exports = dbsearch
