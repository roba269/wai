
/**
 * Module dependencies.
 */

var express = require('express')
  , routes = require('./routes');
var app = module.exports = express.createServer();
var io = require('socket.io').listen(app);
var db = require('./models/db');
var ObjectId = require('mongoose').Types.ObjectId;
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
      return req.session.user.nick;
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
app.get('/arena/replay/:game_name/:match_id', routes.arena_replay);
app.post('/submit/:game_name', routes.submit_post);
app.get('/submit_list/:game_name/:user_id', routes.submit_list_by_user);
app.get('/match_list/:game_name', routes.match_list);
app.get('/view_code/:submit_id', routes.view_code);
app.get('/match/:game_name/:user_id', routes.match_list_by_user);
app.get('/ranklist/:game_name', routes.ranklist);

app.listen(3000, function(){
  console.log("Express server listening on port %d in %s mode", app.address().port, app.settings.env);
});

// require('child_process').fork('compiler.js',[],{env: process.env});

io.sockets.on('connection', function(socket) {
  socket.on('req_steps', function(data) {
    db.matches.findOne({'_id': ObjectId(data.match_id)}, function(err, match) {
      if (err) {
        console.log('Fetch Steps failed, err:' + err);
        return;
      }
      if (!match) {
        console.log('Fetch Steps. Match is null.');
        return;
      }
      var resp = {steps : match.trans};
      console.log('%j', resp);
      socket.emit('res_steps', resp);
    });
  });
});
