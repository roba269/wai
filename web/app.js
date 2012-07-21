
/**
 * Module dependencies.
 */

var express = require('express')
  , routes = require('./routes');

var app = module.exports = express.createServer();

// Configuration

app.configure(function(){
  app.set('views', __dirname + '/views');
  app.set('view engine', 'ejs');
  app.use(express.bodyParser({uploadDir:'./uploads'}));
  app.use(express.methodOverride());
  app.use(express.cookieParser());
  app.use(express.session({ secret: "robaroba" }));
  app.use(app.router);
  app.use(express.static(__dirname + '/public'));
});

app.configure('development', function(){
  app.use(express.errorHandler({ dumpExceptions: true, showStack: true }));
});

app.configure('production', function(){
  app.use(express.errorHandler());
});

app.dynamicHelpers({
  error: function(req, res) {
    var err = req.flash('error');
    if (err.length) return err;
    return null;
  },
  success: function(req, res) {
    var suc = req.flash('success');
    if (suc.length) return suc;
    return null;
  },
  user: function(req, res) {
    if (req.session.user != null)
      return req.session.user.email;
    return null;
  },
});

// Routes

app.get('/', routes.index);
app.get('/login', routes.login);
app.post('/login', routes.login_post);
app.get('/logout', routes.logout);
app.get('/reg', routes.reg);
app.post('/reg', routes.reg_post);
app.get('/game/:game_name', routes.game);
app.post('/game/:game_name', routes.game_post);
app.get('/arena/:game_name', routes.arena);
app.post('/submit/:game_name', routes.submit_post);
app.get('/submit_list/:game_name', routes.submit_list);

app.listen(3000, function(){
  console.log("Express server listening on port %d in %s mode", app.address().port, app.settings.env);
});

require('child_process').fork('compiler.js',[],{env: process.env});

