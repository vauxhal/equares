var auth = require('./auth')
var sim = require('./simulation')
var activator = require('activator')

module.exports = function(app) {
    app.get('/profile', function(req, res) {
        if (!req.isAuthenticated())
            return res.render('profile', {message: req.flash('accountactivationrequired').join('\n')})
        auth.User.findUser(req.user.username, function(err, user) {
            if (err) {
                console.log(err)
                return res.send(500, 'Unable to resolve user')
            }
            else if (!user)
                return res.send(404, 'No such user')
            res.render('profile', {user: user, message: req.flash('accountactivationrequired').join('\n')})
        })
    })

    function changeUser(pass, req, res, cb) {
        if (!req.isAuthenticated())
            return res.send(401, 'You are not logged in')
        auth.User.findUser(req.user.username, function(err, user) {
            if (err) {
                console.log(err)
                return res.send(500, 'Unable to resolve user')
            }
            else if (!user)
                return res.send(404, 'No such user')
            auth.hash(pass, user.salt, function(err, hash) {
                if (err) {
                    console.log(err)
                    return res.send(500)
                }
                if (user.hash !== hash.toString())
                    return res.send(401, 'Current password is incorrect')
                cb(user)
            })
        })
    }

    app.post('/profile/changepassword', function(req, res) {
        if (!(typeof req.body.current_pass == 'string'   &&   req.body.current_pass.length > 0))
            return res.send(400, 'Current password is missing')
        if (!(typeof req.body.new_pass == 'string'   &&   req.body.new_pass.length > 0))
            return res.send(400, 'New password is missing')
        if(req.body.new_pass !== req.body.confirm_new_pass)
            return res.send(400, 'Two copies of the new password differ')
        changeUser(req.body.current_pass, req, res, function(user) {
            auth.hash(req.body.new_pass, user.salt, function(err, hash) {
                user.hash = hash.toString()
                user.save(function(err) {
                    if (err) {
                        console.log(err)
                        return res.send(500, 'Failed to change password: ' + err.toString())
                    }
                    res.send('Password has been changed')
                })
            })
        })
    })

    app.post('/profile/changeemail', function(req, res) {
        if (!(typeof req.body.pass == 'string'   &&   req.body.pass.length > 0))
            return res.send(400, 'Password is missing')
        if (!(typeof req.body.new_email == 'string'   &&   req.body.new_email.match(auth.rxEmail)))
            return res.send(400, 'New email is missing or malformed')
        changeUser(req.body.pass, req, res, function(user) {
            user.email = req.body.new_email
            user.save(function(err) {
                if (err && err.code === 11001)
                    return res.send(403, 'New email has already been specified for another account')
                if (err) {
                    console.log(err)
                    return res.send(500, 'Failed to change email: ' + err.toString())
                }
                req.flash('accountactivationrequired', 'Email address has been changed. A message has been sent to the new email address. Please activate your account by following the link in the message.')
                req.activator = {
                    id: req.user.id,
                    body: 'Email address has been changed'
                }
                activator.createActivate(req, res)
            })
        })
    })

    app.post('/profile/deleteaccount', function(req, res) {
        if (!(typeof req.body.pass == 'string'   &&   req.body.pass.length > 0))
            return res.send(400, 'Password is missing')
        if (req.body.sure !== 'on')
            return res.send(400, 'You\'re not sure to delete your account')
        changeUser(req.body.pass, req, res, function(user) {
            sim.Sim.remove({user: user._id}, function(err) {
                if (err)
                    console.log(err)
                user.remove(function(err) {
                    if (err) {
                        console.log(err)
                        return res.send(500, 'Failed to remove account')
                    }
                    res.send('Account has been removed')
                })
            })
        })
    })
}
