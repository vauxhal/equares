    var fs = require('fs')
var mongoose = require('mongoose')
var ObjectId = mongoose.Schema.Types.ObjectId

var SimSchema = mongoose.Schema({
    name:           String,
    description:    String,
    definition:     String,
    date:           Date,
    info:           String,
    keywords:       [String],
    script:         String,
    user:           {type: ObjectId, index: true},
    public:         Boolean
})

SimSchema.index({name: 1, user: 1}, {unique: true})

SimSchema.statics.upsert = function(sim, done) {
    this.update({user: sim.user, name: sim.name}, sim, {upsert: true}, done)
}
SimSchema.statics.have = function(sim, done) {
    this.count({user: sim.user, name: sim.name}, done)
}

var Sim = mongoose.model('Sim', SimSchema, 'simulations')

// Refresh example simulations in database
function refreshExamples() {
    var dir = 'public/examples/'
    var nFilesLoaded = 0
    var nDirs
    
    function readFile(fileName) {
        var text = ""
        fs.createReadStream(dir + fileName)
            .on('data', function(data) { text += data })
            .on('end', function() {
                var obj = JSON.parse(text)
                if (!(obj.date instanceof Date))
                    obj.date = new Date(2014, 2, 6)
                if (!obj.keywords)
                    obj.keywords = []
                if (!(obj.script))
                    obj.script = ''
                obj.user = null
                obj.public = true
                console.log("  " + fileName + ": " + obj.name + " (" + obj.description + ")")
                Sim.create(obj, function(err, sim) {
                    if (err)
                        console.log(err)
                    if (++nFilesLoaded == nDirs)
                        console.log('... Finished loading example simulations')
                })
            })
    }

    fs.readdir(dir, function(err, files) {
        if (err)
            console.log(err)
        else Sim.remove({user: null}, function() {
            console.log('Loading example simulations to the database ...')
            nDirs = files.length
            for (var i=0; i<nDirs; ++i) {
                var fileName = files[i]
                readFile(fileName)
            }
        })
    })
}

refreshExamples()



var RecentSimSchema = mongoose.Schema({
    simulation: String,
    user:       {type: ObjectId, index: true}
})

RecentSimSchema.statics.get = function(req, done) {
    var RecentSim = this
    if (!(done instanceof Function))
        done = function() {}
    if (!req.isAuthenticated())
        return done()
    RecentSim.findOne({user: req.user.id}, function(err, s) {
        if (err) {
            console.log(err)
            done()
        }
        else if (s)
            done(s.simulation)
        else
            done()
    })
}

RecentSimSchema.statics.set = function(req, done) {
    var RecentSim = this
    if (!(done instanceof Function))
        done = function() {}
    if (!req.isAuthenticated())
        return done()
    RecentSim.update({user: req.user.id}, {simulation: req.body.simulation}, {upsert: true}, function(err) {
        if(err)
            console.log(err)
        done()
    })
}

var RecentSim = mongoose.model('RecentSim', RecentSimSchema, 'recentSimulations')

module.exports = {
    Sim: Sim,
    RecentSim: RecentSim
}
