var mongoose = require('mongoose')
//var textSearch = require('mongoose-text-search');
var auth = require('./auth')
var cp = require('child_process')

var ObjectId = mongoose.Schema.Types.ObjectId

var ImgSchema = mongoose.Schema({
    name:           {type: String, index: true},
    title:          String,
    date:           Date,
    keywords:       {type: [String], index: true},
    contentType:    String,
    data:           String,
    preview:        String,
    user:           {type: ObjectId, index: true}
})

ImgSchema.index({name: 1, user: 1}, {unique: true})

ImgSchema.statics.upsert = function(img, done) {
    this.update({user: img.user, name: img.name}, img, {upsert: true}, done)
}
ImgSchema.statics.have = function(img, done) {
    this.count({user: img.user, name: img.name}, done)
}

var Img = mongoose.model('Img', ImgSchema, 'images')

function pickImage(req, res) {
    res.render('pick-image', {req: req})
}

function imageThumbnails (req, res) {
    // res.send('<a href="/images/logo.png"><img alt="logo" title="logo" src="/images/logo.png"/></a>')

    var pageSize = 100
    var page = req.query.page === undefined?  0:   +req.query.page
    Img.count({}, function(err, n) {
        if (err) {
            console.log(err)
            return res.send(500)
        }
        var total = n, n1 = page*pageSize, n2 = (page+1)*pageSize
        var sent = false, count = 0, finished = false
        if (n2 > total)
            n2 = total
        if (n1 > n2)
            n1 = n2
        var images = []

        function proceed() {
            if (!finished || count < images.length || sent)
                return
            // res.send(JSON.stringify({pages: Math.ceil(total/pageSize), images: images}))

            for (var i=0; i<count; ++i) {
                var img = images[i],
                    a = ['', 'img', img.user? img.user: '-', 'preview', img.name],
                    previewName  = a.join('/')
                a.splice(3, 1)
                var name = a.join('/')
                res.write('<a href="' + name + '"><img alt="' + img.name + '" title="' + img.title + '" src="' + previewName + '"/></a>')
            }
            res.end()
            sent = true
        }

        Img.find({$query: {}, $orderby: {date: 1}},
            {name: 1, user: 1, keywords: 1, title: 1},
            {skip: n1, limit: n2-n1}).stream()
            .on('data', function (doc) {
                var obj = doc.toObject()
                images.push(obj)
                auth.User.username(doc.user, function(username) {
                    ++count
                    obj.user = username
                    obj.edit = req.user && username == req.user.username
                    proceed()
                })
            }).on('error', function (err) {
                console.log(err)
                if (!sent) {
                    res.send(500, 'Unable to read image list')
                    sent = true
                }
            }).on('close', function () {
                finished = true
                proceed()
            })
    })
}

function getImage(req, res) {
    var a = req.path.split('/'), preview
    if(a.length == 3)
        preview = false
    else if (a.length == 4 && a[2] == 'preview')
        preview = true
    else
        return res.send(404, 'Image is not found')
    var user = a[1], name = a[preview? 3: 2]

    function serve() {
        Img.findOne({name: name, user: user}, function(err, img) {
            if (err) {
                console.log(err)
                res.send(500)
            }
            else if (img) {
                if (preview) {
                    res.contentType('image/png')
                    res.end(img.preview, 'binary')
                }
                else {
                    res.contentType(img.contentType)
                    res.end(img.data, 'binary')
                }
            }
            else
                res.send(404, 'Image is not found')
        })
    }

    if (user == '-') {
        user = null
        serve()
    }
    else
        auth.User.findUser(user, function(err, userId) {
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

function uploadImage(req, res) {
    if (!req.isAuthenticated())
        return res.send(401, 'You are not logged in')
    var img = JSON.parse(req.body.img)
    img.date = new Date()
    img.user = req.user.id

    function imgresize(param, cb)
    {
        var dir = '/home/stepan/build-equares-release/bin'
        var imgresize = cp.spawn(dir + '/imgresize', param, {cwd: dir})
        var resized = '', stderr = ''
        imgresize.stdin.end(img.data, 'binary')
        imgresize.stdout.setEncoding('binary')
        imgresize.stdout.on('data', function(data) {
            resized += new Buffer(data, 'base64').toString('binary')
        })
        imgresize.on('close', function(code) {
            // console.log('Image has been resized, code=' + code)
            if (code || stderr) {
                var s = 'Image resize error (code ' + code + ')'
                if (stderr)
                    s += ': ' + stderr
                console.log(s)
                cb(s)
            }
            else
                cb(null, resized)

        })
        imgresize.stderr.on('data', function(data) {
            stderr += data
        })
    }

    function save() {
        delete img.overwrite
        imgresize([100, 100], function(err, resized) {
            if (err)
                res.send(500, 'Failed to generate image preview')
            else {
                img.preview = resized
                Img.upsert(img, function(err, doc) {
                    if (err) {
                        console.log(err)
                        res.send(500, 'Failed to upload image')
                    }
                    else
                        res.send(['', 'img', req.user.username, img.name].join('/'))
                })
            }
        })
    }

    if (img.overwrite)
        save()
    else
        Img.have(img, function(err, count) {
            if (err)
                res.send(500)
            else if (count > 0)
                res.send(403, 'Image with the specified name already exists')
            else
                save()
        })
}

module.exports = function(app) {
    app.get('/pick-image', pickImage)
    app.get('/image-thumbnails', imageThumbnails)
    app.use('/img', getImage)
    app.post('/upload-image', uploadImage)
}
