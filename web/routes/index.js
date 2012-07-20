
/*
 * GET home page.
 */

var crypto = require('crypto');
var fs = require('fs');
var User = require('../models/user');

exports.index = function(req, res) {
  res.render('index', { title: 'WAI : Home'});
};

exports.login = function(req, res) {
    res.render('login', { title: 'WAI : Login'});
};

exports.login_post = function(req, res) {
  var md5 = crypto.createHash('md5');
  var passwd = md5.update(req.body['passwd']).digest('base64');
  User.get(req.body['email'], function(err, user) {
    if (!user || user.passwd != passwd) {
      req.flash('error', 'Email dose not exist, or password error.');
      return res.redirect('/login');
    }
    req.session.user = user;
    req.flash('success', 'Login successfully.');
    res.redirect('/');
  });
};

exports.logout = function(req, res) {
  req.session.user = null;
  res.redirect('/');
};

exports.reg = function(req, res) {
    res.render('reg', { title: 'WAI : Sign Up'});
};

exports.reg_post = function(req, res) {
    if (req.body['passwd'] != req.body['passwd2']) {
        req.flash('error', 'The two passwords does not fit.');
        return res.redirect('/reg');
    }
    var md5 = crypto.createHash('md5');
    var passwd = md5.update(req.body['passwd']).digest('base64');
    var newUser = new User({
        'email': req.body['email'],
        'passwd': passwd,
    });
    User.get(newUser.email, function(err, user) {
        if (user)
            err = 'Email already exists.'
        if (err) {
            req.flash('error', err);
            return res.redirect('/reg');
        }
        newUser.save(function(err) {
            if (err) {
                req.flash('error', err);
                return res.redirect('/reg');
            }
            req.session.user = newUser;
            req.flash('success', 'Sign up successfully.');
            res.redirect('/');
        });
    });
};

exports.game = function(req, res) {
    res.render('game', {title: 'WAI : ' + req.params.game_name,
            game_name: req.params.game_name,
            game_intro: 'This is introduction for ' + req.params.game_name});
}

exports.submit_post = function(req, res) {
  fs.readFile(req.files.code.path, function(err, data) {
    if (err) {
      req.flash('error', 'Submit failed.');
      res.redirect('back');
    }
    var submit = {
      user_email: req.session.user,
      game_name: req.params.game_name,
      lang: req.body['lang'],
      code: data,
      status: 0,
      compile_output: '',
    };
    submit.save(function(err) {
      if (err) {
        req.flash('error', 'Submit failed.');
        res.redirect('back');
      }
    });
    res.redirect('back');
  });  
}

exports.game_post = function(req, res) {
  // compile the code
  
}

