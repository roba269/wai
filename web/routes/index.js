
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
var db_util = require('../db_util');

// exports.submit_list = submit_ctrl.submit_list;
exports.submit_list_by_user = submit_ctrl.submit_list_by_user;
exports.submit_post = submit_ctrl.submit_post;
exports.arena_replay = arena_ctrl.arena_replay;
exports.match_list = match_ctrl.match_list;
exports.match_list_by_user = match_ctrl.match_list_by_user;

exports.login = user_ctrl.login;
exports.login_post = user_ctrl.login_post;
exports.logout = user_ctrl.logout;
exports.reg = user_ctrl.reg;
exports.reg_post = user_ctrl.reg_post;

exports.index = function(req, res) {
  res.render('index', { title: 'WAI : Home'});
};

exports.game = function(req, res) {
  /*
  if (!req.session.user) {
    req.flash('error', 'You are not logged in.');
    return res.redirect('back');
  }
  */
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
        res.render('game', {title: 'WAI : ' + req.params.game_name,
            game_name: req.params.game_name,
            game_intro: game.desc,
            uid: uid,
            ranklist: rank_list});
      });
  });
}

exports.game_post = function(req, res) {
  // compile the code
  
}

exports.view_code = function(req, res) {
  if (!req.session.user) {
    req.flash('error', 'You are not login.');
    return res.redirect('back');
  }
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
      res.render('code', {title: 'WAI : View Code',
                          code: submit.code});
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

