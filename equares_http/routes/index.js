var Sim = require('../simulation').Sim

module.exports = {
    index: function(req, res){
        res.render('index')
    },
    editor: function(req, res) {
        var examples = []
        Sim.find().stream()
            .on('data', function (doc) {
                examples.push(doc)
            }).on('error', function (err) {
                console.log(err)
            }).on('close', function () {
                res.render('editor', {examples: examples})
            })
    }
}
