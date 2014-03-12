var Sim = require('../simulation').Sim
var fs = require('fs')

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
        res.render('editor', {req: req})
    },
    example: function(req, res) {
        var name = req.url.match('^/examples/(.*)$')[1]
        Sim.findOne({name: name, user: null}, function(err, s) {
            if (err) {
                console.log(err)
                res.send(404)
            }
            if (s) {
                var result = s.toObject()
                res.send(result)
            }
            else
                res.send(404)
        })
    },
    simulations: function(req, res){
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
    }
}
