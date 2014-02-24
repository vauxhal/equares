
/**
 * Module dependencies.
 */

var express = require('express');
var routes = require('./routes/');
var equares = require('./routes/equares');
var http = require('http');
var path = require('path');
var url = require('url');
var captcha = require('./mycaptcha');

var mongoose = require('mongoose');
var flash = require('connect-flash');


var env = process.env.NODE_ENV || 'development',
  config = require('./config/config')[env];


mongoose.connect(config.db);

var app = express();

// all environments
app.set('port', process.env.PORT || 3000);
app.set('views', path.join(__dirname, 'views'));
app.set('view engine', 'jade');

app.use(express.favicon(path.join(__dirname, 'public/images/favicon.ico')));
app.use(express.logger('dev'));
app.use(express.json());
app.use(express.urlencoded());
app.use(express.methodOverride());
app.use(express.cookieParser('your secret here'));
app.use(express.session({secret: "dde9796b-28ae-4b55-9d03-2cecc8d9ead3"}));

app.use(captcha({ url: '/captcha.png' })); // captcha params
app.use(flash());

require('./auth')(app)

app.use(app.router);
app.use(express.static(path.join(__dirname, 'public')));

// development only
if ('development' == env) {
  app.use(express.errorHandler());
}

app.get('/', routes.index);
app.get('/editor', routes.editor);

equares.bind(app)

app.use(function(err, req, res, next){
  res.status(err.status || 500);
  res.render('500', { error: err });
});

app.use(function(req, res, next){
  res.status(404);
  if (req.accepts('html')) {
    res.render('404', { url: req.url });
    return;
  }
  if (req.accepts('json')) {
    res.send({ error: 'Not found' });
    return;
  }
  res.type('txt').send('Not found');
});

http.createServer(app).listen(app.get('port'), function(){
  console.log('Express server listening on port ' + app.get('port'));
});
