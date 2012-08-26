
/*
 * GET home page.
 */

var fs = require('fs');
var ObjectId = require('mongoose').Types.ObjectId;
var db = require('../models/db');
var submit_ctrl = require('./submit');
var arena_ctrl = require('./arena');
var match_ctrl = require('./match');
var user_ctrl = require('./user');
var admin_ctrl = require('./admin');
var db_util = require('../db_util');

// exports.submit_list = submit_ctrl.submit_list;
exports.submit_list_adv = submit_ctrl.submit_list_adv;
exports.submit_post = submit_ctrl.submit_post;
exports.arena_replay = arena_ctrl.arena_replay;
exports.arena_hvc = arena_ctrl.arena_hvc;
exports.match_list = match_ctrl.match_list;
exports.match_list_by_user = match_ctrl.match_list_by_user;

exports.login = user_ctrl.login;
exports.login_post = user_ctrl.login_post;
exports.logout = user_ctrl.logout;
exports.reg = user_ctrl.reg;
exports.reg_post = user_ctrl.reg_post;
exports.change_passwd = user_ctrl.change_passwd;
exports.change_passwd_post = user_ctrl.change_passwd_post;
exports.show_user = user_ctrl.show_user;
exports.edit = user_ctrl.edit;
exports.edit_post = user_ctrl.edit_post;

exports.rejudge = admin_ctrl.rejudge;

function random_shuffle ( myArray ) {
  var i = myArray.length;
  if ( i == 0 ) return false;
  while ( --i ) {
     var j = Math.floor( Math.random() * ( i + 1 ) );
     var tempi = myArray[i];
     var tempj = myArray[j];
     myArray[i] = tempj;
     myArray[j] = tempi;
   }
}

exports.index = function(req, res) {
  db.games.find({}, function(err, game_list) {
    if (err) {
      req.error('error', 'Failed to get game list.');
      return res.redirect('/');
    }
    random_shuffle(game_list);
    res.render('index', { title: 'WAI : Home', game_list: game_list});
  });
};

exports.game = function(req, res) {
  db.games.findOne({name: req.params.game_name}, function(err, game) {
    if (err) {
      console.log('Failed to get game info');
      return res.redirect('back');
    }
    if (!game) {
      console.log('Failed to get game info');
      return res.redirect('back');
    }
    db_util.get_rank_list(req.params.game_name,
      function(err, rank_list) {
        if (err) {
          req.flash('error', 'Get rank list error.');
          return res.redirect('back');
        }
        var uid;
        if (req.session.user) uid = req.session.user._id;
        else uid = null;
        db.users.findOne({email: "sample@w-ai.org"},
          function(err, sample_user) {
            var sample_uid;
            if (err || !sample_user) sample_uid = 0;
            else sample_uid = sample_user._id;
            res.render('game', {title: 'WAI : ' + req.params.game_name,
                game_name: req.params.game_name,
                game_intro: game.desc,
                hvc: game.hvc,
                show_name: game.show_name,
                uid: uid,
                ranklist: rank_list,
                sample_uid: sample_uid});
          });
      });
  });
}

exports.game_post = function(req, res) {
  // compile the code
  
}

exports.view_code = function(req, res) {
  /*
  if (!req.session.user) {
    req.flash('error', 'You are not login.');
    return res.redirect('back');
  }
  */
  db.submits.findOne({_id: ObjectId(req.params.submit_id)},
    function(err, submit) {
      if (err) {
        console.log('Fetch code failed, err:' + err);
        return;
      }
      if (!submit) {
        console.log('Fetch code failed. No such submit id.');
        return;
      }
      if (submit.allow_view ||
        (req.session.user && req.session.user.is_admin) || 
        (req.session.user && submit.user_id.equals(req.session.user._id))) {
        res.render('code', {title: 'WAI : View Code',
                          code: submit.code, err: submit.compile_output});
      } else {
        res.render('code', {title: 'WAI : View Code',
                  err: '', code: 'You have no permission to view this code.'});
      }
    });
}

exports.ranklist = function(req, res) {
  db_util.get_rank_list(req.params.game_name,
    function(err, rank_list) {
      if (err) {
        req.flash('error', 'Get rank list error.');
        return res.redirect('back');
      }
      res.render('ranklist', {title: 'WAI : Ranklist',
          game_name: req.params.game_name,
          ranklist: rank_list});
    });
}

exports.game_list = function(req, res) {
  db.games.find({}, function(err, game_list) {
    if (err) {
      console.log('Failed to get game_list. err:' + err);
      return;
    }
    res.render('game_list', {title: 'WAI : Gamelist',
      game_list: game_list});
  });
}

exports.faq = function(req, res) {
  db.users.findOne({email: 'sample@w-ai.org'},
    function(err, sample) {
      var sample_uid;
      if (err || !sample) sample_uid = 0;
      else sample_uid = sample._id;
      res.render('faq', {title: 'WAI : FAQ',
        sample_uid: sample_uid});
    });
}
