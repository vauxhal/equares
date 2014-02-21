var User = require('./user');
var Auth = require('./authorization.js');

module.exports = function(app, passport){
	app.get("/", function(req, res){ 
		if(req.isAuthenticated()){
		  res.render("home", { user : req.user}); 
		}else{
			res.render("home", { user : null});
		}
	});

	app.get("/login", function(req, res){ 
        res.render("login", {message: req.flash('error')});
	});

    /*
	app.post("/login" 
		,passport.authenticate('local',{
            successRedirect : "/",
			failureRedirect : "/login",
            failureFlash: true
		})
	);
    */
    app.post("/login",
        passport.authenticate('local', {
            successRedirect : "/",
            failureRedirect : "/login",
            failureFlash: true
        })
    )

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
            res.render('loginform')
    })
}
