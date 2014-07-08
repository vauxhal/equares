var auth = require('./auth')

module.exports = function(app) {
    app.get('/profile', function(req, res) {
        if (!req.isAuthenticated()) {
            res.send(401, 'You are not logged in')
            return false
        }
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
