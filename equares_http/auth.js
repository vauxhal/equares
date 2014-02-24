var mongoose = require('mongoose')
var crypto = require('crypto')
var passport = require('passport')
var LocalStrategy = require('passport-local').Strategy

// Based on this: https://github.com/visionmedia/node-pwd

/**
 * Hashes a password with optional `salt`, otherwise
 * generate a salt for `pass` and invoke `fn(err, salt, hash)`.
 *
 * @param {String} password to hash
 * @param {String} optional salt
 * @param {Function} callback
 * @api private
 */
var hash = (function() {
     // Bytesize.
     var len = 128

     // Iterations. ~300ms
     var iterations = 12000

     return function (pwd, salt, fn) {
        if (3 == arguments.length)
            crypto.pbkdf2(pwd, salt, iterations, len, fn)
        else {
            fn = salt
            crypto.randomBytes(len, function(err, salt) {
                if (err)
                    return fn(err)
                salt = salt.toString('base64')
                crypto.pbkdf2(pwd, salt, iterations, len, function(err, hash) {
                    if (err)
                        return fn(err)
                    fn(null, salt, hash)
                })
            })
        }
    }
})()



// Describe user schema

UserSchema = mongoose.Schema({
    username:   String,
    email:      String,
    salt:       String,
    hash:       String
});

UserSchema.statics.findUser = function(usernameOrEmail, done) {
    var condition = {}
    condition[usernameOrEmail.indexOf('@') == -1? "username": "email"] = usernameOrEmail
    this.findOne(condition, done)
}

UserSchema.statics.signup = function(username, email, password, done) {
    var User = this;
    this.findOne({$or: [{username: username}, {email: email}]}, function(err, user) {
        if (user)
            return done(null, false, {message: 'User with specified name or email has already been registered'})
        hash(password, function(err, salt, hash) {
            if(err)
                throw err
            User.create({
                username: username,
                email : email,
                salt : salt,
                hash : hash
            }, function(err, user) {
                if(err)
                    throw err
                done(null, user)
            })
        })
    })
}

UserSchema.statics.isValidUserPassword = function(usernameOrEmail, password, done) {
    this.findUser(usernameOrEmail, function(err, user) {
        if(err)
            return done(err)
        if(!user)
            return done(null, false, { message : 'Incorrect username or email' })
        hash(password, user.salt, function(err, hash) {
            if(err)
                return done(err)
            if(hash == user.hash)
                return done(null, user)
            done(null, false, { message : 'Incorrect password' })
        })
    })
}

var User = mongoose.model("User", UserSchema);



// Configure passport

// Specify local authorization strategy
passport.use(new LocalStrategy(
    {
        usernameField: 'username',
        passwordField: 'password'
    },
    User.isValidUserPassword.bind(User))
)

// Enable login sessions
passport.serializeUser(function(user, done) {
    done(null, user.id)
})
passport.deserializeUser(function(id, done) {
    User.findById(id, function(err, user) {
        done(err, user)
    })
})

function auth(app) {

    // Use passport

    app.use(passport.initialize());
    app.use(passport.session());



    // Serve authorization-specific routes

    app.get('/loginform', function(req, res) {
        if (req.isAuthenticated())
            res.render('userinfo', {user: req.user.username})
        else
            res.render('loginform', {message: req.flash('error'), captcha: req.session.needsLoginCaptcha})
    })

    app.post("/login", function(req, res, next) {
        if (req.session.needsLoginCaptcha && req.body.captcha !== req.session.captcha) {
            req.flash('error', 'Human test failed')
            return res.send(401)
        }
        passport.authenticate(
            'local',
            function(err, user, info) {
                if (err)
                    return next(err)
                if (!user) {
                    // Authorization failed
                    if (typeof(req.session.failedAuthAttempts) == "number")
                        ++req.session.failedAuthAttempts
                    else
                        req.session.failedAuthAttempts = 1
                    if (req.session.failedAuthAttempts > 5)
                        req.session.needsLoginCaptcha = true
                    req.flash('error', 'Incorrect user name or password')
                    return res.send(401)
                }
                req.logIn(user, function(err) {
                    if (err)
                        return next(err)
                    req.session.failedAuthAttempts = 0
                    req.session.needsLoginCaptcha = false
                    return res.end()
                })
            }
        )(req, res, next)
    })

    app.get("/signup", function (req, res) {
        res.render("signup", {message: req.flash('message')})
    })

    app.get("/logindlg", function (req, res) {
        res.render("logindlg", {message: req.flash('error')})
    })

    app.post("/signup", function (req, res, next) {
        function fail() {
            req.session.captcha = undefined
            res.send(401)
        }

        function validateField(field, uiName, rx) {
            if (req.body[field].length == 0) {
                req.flash('message', "The field '" + uiName + "' is empty")
                return false
            }
            if (!req.body[field].match(rx)) {
                req.flash('message', "The field '" + uiName + "' contains an invalid value")
                return false
            }
            return true
        }

        function validate() {
            if (!(validateField('username', 'Name', /^\w+$/) &&
                  validateField('email', 'Email', /^([0-9a-zA-Z]([-\.\w]*[0-9a-zA-Z])*@([0-9a-zA-Z][-\w]*[0-9a-zA-Z]\.)+[a-zA-Z]{2,9})$/) &&
                  validateField('password', 'Password', /.+/)   // TODO
                  ))
                return false
            if (req.body.password !== req.body.password2) {
                req.flash('message', "The fields 'Password' and 'Confirm password' contain different data")
                return fail()
            }
            if (req.body.captcha !== req.session.captcha) {
                req.flash('message', 'Human test failed. Please try again')
                return false
            }
            return true
        }

        if (!validate())
            return fail()

        User.signup(req.body.username, req.body.email, req.body.password, function(err, user, messages) {
            if (err)
                throw err
            if (user)
                req.login(user, function(err) {
                    if (err)
                        return next(err)
                    // res.redirect("/")
                    res.send(200)
                })
            else {
                for (var n in messages)
                    req.flash(n, messages[n])
                fail()
            }
        })
    })

    app.post('/logout', function(req, res) {
        req.logout();
        res.end();
    });

    app.get('/errormsg', function(req, res) {
        // TODO better
        var msg = req.flash('message')
        if(!msg || (msg instanceof Array) && msg.length == 0)
            msg = req.flash('error')
        res.render('errormsg', {message: msg})
    })
}

auth.User = User

module.exports = auth
