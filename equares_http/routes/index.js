var Sim = require('../simulation').Sim

module.exports = {
    index: function(req, res){
        res.render('index')
    },
    editor: function(req, res) {
        var examples = []
        Sim.find({$query: {user: null}, $orderby: {description: 1}}, {name: 1, description: 1}).stream()
            .on('data', function (doc) {
                examples.push(doc)
            }).on('error', function (err) {
                console.log(err)
            }).on('close', function () {
                res.render('editor', {examples: examples})
            })
    },
    example: function(req, res) {
        var name = req.url.match('^/examples/(.*)$')[1]
        Sim.findOne({name: name, user: null}, function(err, s) {
            if (err) {
                console.log(err)
                res.send(404)
            }
            if (s) {
                var result = JSON.parse(s.definition)
                result.name = s.name
                result.description = s.description
                res.send(result)
            }
            else
                res.send(404)
        })
    }
}
