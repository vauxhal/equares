var fs = require('fs')
var mongoose = require('mongoose')

var SimSchema = mongoose.Schema({
    name:           String,
    description:    String,
    definition:     String
})

var Sim = mongoose.model('Sim', SimSchema, 'simulations')

/*
function importExampleFiles(info) {
    Sim.remove({}, function() {
        for (var n=info.length, i=0; i<n; ++i) {
            (function() {
                var x = info[i]
                var definition = ""
                fs.createReadStream('public/examples/' + x.name + '.json')
                    .on('data', function(data) { definition += data })
                    .on('end', function() {
                        console.log(x.name)
                        Sim.create({
                            name: x.name,
                            description: x.description,
                            definition: definition
                        }, function(err, sim) {
                            if (err)
                                throw err
                        })
                    })
            })()
        }
    })
}

importExampleFiles([
    { name: 'simple-pendulum-1', description: 'Simple pendulum, phase trajectory' },
    { name: 'simple-pendulum-2', description: 'Simple pendulum, several plots' },
    { name: 'double-pendulum-t', description: 'Double pendulum, trajectory projection' },
    { name: 'double-pendulum-psec', description: 'Double pendulum, Poincare map' },
    { name: 'double-pendulum-b-psec', description: 'Double pendulum b, Poincare map' }
])
//*/

module.exports = {
    Sim: Sim
}
