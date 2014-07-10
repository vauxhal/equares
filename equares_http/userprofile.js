var auth = require('./auth')

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
}
