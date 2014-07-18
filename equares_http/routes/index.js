/*
EquaRes - a tool for numerical analysis of equations.
Copyright (C) 2014 Stepan Orlov

This file is part of EquaRes.

Full text of copyright notice can be found in file copyright_notice.txt in the EquaRes root directory.
License agreement can be found in file LICENSE.md in the EquaRes root directory.
*/

var fs = require('fs')
var simulation = require('../simulation')
var Sim = simulation.Sim
var RecentSim = simulation.RecentSim
var mongoose = require('mongoose')
var dbsearch = require('../dbsearch')

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
        if (req.query.sim)
            Sim.findBySpec(req.query.sim, function(err, s) {
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
            'keywords',
            'public',
            'edit'
        ]))
    },
    simtable: function(req, res) {
        dbsearch(req, {
            model: Sim,
            project: {user: 1, date: 1, name: 1, description: 1, keywords: 1, public: 1},
            pageSize: 25
        }, function(result) {
            if (result.err)
                res.send(result.err.code || 500, result.err.message || result.err.data)
            else
                res.send(JSON.stringify({pages: result.pages, sims: result.records, pagenum: result.pagenum}))
        })
    }
}
