var mongoose = require('mongoose')
var auth = require('./auth')

function dbsearch(req, options, cb) {
    var records = []
    var processed = false, count = 0, finished = false
    var total
    var pageSize = options.pageSize || 25
    function proceed() {
        if (!finished || count < records.length || processed)
            return
        cb({pages: Math.ceil(total/pageSize), records: records})
        processed = true
    }

    var page = req.query.page === undefined?  0:   +req.query.page

    var user = req.user? new mongoose.Types.ObjectId(req.user.id.toString()): null
    var query, andClause
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
            query = {public: true}
        else
            query = {$or: [{public: true}, {user: user}]}
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
                options.model.find({$query: query || {}, $orderby: {date: 1}},
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

    if (req.query.user) {
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
