var fs = require('fs')
var simulation = require('../simulation')
var Sim = simulation.Sim
var RecentSim = simulation.RecentSim
var auth = require('../auth')
var mongoose = require('mongoose')

function readExamples(cb) {
    var examples = []
    Sim.find({$query: {user: null}, $orderby: {description: 1}}, {name: 1, description: 1}).stream()
        .on('data', function (doc) {
            examples.push(doc)
        }).on('error', function (err) {
            console.log(err)
            cb(err, null)
        }).on('close', function () {
            cb(null, examples)
        })
}

module.exports = {
    index: function(req, res){
        res.render('index', {req: req})
    },
    editor: function(req, res) {
        if (req.query.sim) {
            Sim.findOne({_id: req.query.sim}, function(err, s) {
                if (err) {
                    console.log(err)
                    res.send(500)
                }
                else if (s) {
                    var sim = s.toObject()
                    for (var i in {__v: 1, _id: 1, user: 1, date: 1})
                        delete sim[i]
                    sim = JSON.stringify(sim)
                    req.session.simulation = sim
                    req.body.simulation = sim
                    simulation.RecentSim.set(req, function() {
                        res.redirect('/editor')
                    })
                }
                else
                    res.send(404)
            })
        }
        else
            res.render('editor', {req: req})
    },
    example: function(req, res) {
        var name = req.url.match('^/examples/(.*)$')[1]
        Sim.findOne({name: name, user: null}, function(err, s) {
            if (err) {
                console.log(err)
                res.send(500)
            }
            else if (s) {
                var result = s.toObject()
                res.send(result)
            }
            else
                res.send(404)
        })
    },
    simulations: function(req, res) {
        res.render('simulations', {req: req})
    },
    menu: function(req, res, next) {
        var match = req.path.match('^/menu-(.*)$'), name
        if (!match || typeof (name = match[1]) !== 'string')
            return next()

        var param = {req: req}
        function renderMenu() {
            var menuName = name.length > 0? 'menu-' + name: 'menu'
            fs.stat('views/' + menuName + '.jade', function(err, stats) {
                if (err)
                    menuName = 'menu'
                res.render(menuName, param)
            })
        }

        switch(name)
        {
        case 'editor':
            readExamples(function(err, examples) {
                if (err)
                    res.send(err)
                else {
                    param.examples = examples
                    renderMenu()
                }
            })
            break
        default:
            renderMenu()
        }
    },
    simheaders: function(req, res) {
        res.send(JSON.stringify([
            'user',
            'date',
            'name',
            'description',
            'public'
        ]))
    },
    simtable: function(req, res) {
        var sims = []
        var sent = false
        var count = 0
        var finished = false
        function proceed() {
            if (!finished || count < sims.length || sent)
                return
            res.send(JSON.stringify(sims))
            sent = true
        }

        var user = req.user? new mongoose.Types.ObjectId(req.user.id.toString()): null
        Sim.find({$query: { $or: [{public: true}, {user: user}]}, $orderby: {date: 1}},
            {user: 1, date: 1, name: 1, description: 1, public: 1}).stream()
            .on('data', function (doc) {
                var obj = doc.toObject()
                sims.push(obj)
                auth.User.username(doc.user, function(username) {
                    ++count
                    obj.user = username
                    proceed()
                })
            }).on('error', function (err) {
                console.log(err)
                if (!sent) {
                    res.send(500, 'Unable to read simulation table')
                    sent = true
                }
            }).on('close', function () {
                finished = true
                proceed()
            })
    }
}
