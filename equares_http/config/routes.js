var User = require('./user');
var Auth = require('./authorization.js');

module.exports = function(app, passport){
	app.get("/", function(req, res){ 
        res.render("home");
	});

             /*
    app.post("/login",
        passport.authenticate('local', {
            failureFlash: true
        }), function(req, res) {

            // deBUG
            var auth = req.isAuthenticated()

            res.end();
        }
    )
        */
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
                    req.session.needsLoginCaptcha = true
                    req.flash('error', 'Incorrect user name or password')
                    return res.send(401)
                }
                req.logIn(user, function(err) {
                    if (err)
                        return next(err)
                    req.session.needsLoginCaptcha = false
                    return res.end()
                })
            }
        )(req, res, next)
    })

	app.get("/signup", function (req, res) {
        res.render("signup", {message: req.flash('message')});
	});

	app.post("/signup", Auth.userExist, function (req, res, next) {
        if (req.body.captcha != req.session.captcha) {
            req.flash('message', 'Human test failed. Please try again')
            res.redirect("/signup")
            return
        }

		User.signup(req.body.email, req.body.password, function(err, user){
			if(err) throw err;
			req.login(user, function(err){
				if(err) return next(err);
				return res.redirect("profile");
			});
		});
	});

	app.get("/profile", Auth.isAuthenticated , function(req, res){ 
        res.render("profile", { user : req.user});
	});

    app.post('/logout', function(req, res){
		req.logout();
        res.end();
	});
    app.get('/loginform', function(req, res) {
        if (req.isAuthenticated())
            res.render('userinfo', {user: req.user.email})
        else
            res.render('loginform', {message: req.flash('error'), captcha: req.session.needsLoginCaptcha})
    })
}
