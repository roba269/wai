var db = require('../models/db');
var db_util = require('../db_util');
var fs = require('fs');
var ObjectId = require('mongoose').Types.ObjectId;
var waiconst = require('../waiconst');
var queue = require('../queue');
var waiconst = require('../waiconst');

exports.submit_list = function(req, res) {
  if (!req.session.user) {
    req.flash('error', 'You are not login.');
    return res.redirect('back');
  }
  var user_email = req.session.user.email;
  var game_name = req.params.game_name;
  var submit_list = [];
  var query;
  if (game_name === 'all') {
    query = {user_email: user_email};
  } else {
    query = {user_email: user_email, game_name: game_name};
  }
  db.submits.find(query, function(err, submits) {
    if (err) {
      req.flash('error', 'Failed to get submition list.');
      return res.redirect('back');
    }
    submits.forEach(function(submit) {
      submit_list.push(submit);
    });
    res.render('submit_list', { title: 'WAI : Submitions',
      submits: submit_list});
  });
};

exports.submit_list_adv = function(req, res) {
  var game_name = req.params.game_name;
  var user_id = req.params.user_id;
  var query = {};
  var page_num = 0;
  if (req.params.page_num) {
    page_num = parseInt(req.params.page_num);
  }
  if (game_name !== 'all') {
    query['game_name'] = game_name;
  }
  if (user_id !== 'all') {
    user_id = ObjectId(req.params.user_id);
    query['user_id'] = user_id;
  }
  db.submits.find(query, null, {limit: waiconst.ITEM_PER_PAGE,
    skip: page_num * waiconst.ITEM_PER_PAGE})
    .sort({'date': -1}, function(err, submits) {
    if (err) {
      req.flash('error', 'Failed to get submition list.');
      return res.redirect('back');
    }
    res.render('submit_list', { title: 'WAI : Submitions',
      submits: submits, game_name: game_name,
      get_user_id: user_id, page_num: page_num});
  });
};
/*
exports.submit_list_by_user = function(req, res) {
  var game_name = req.params.game_name;
  var user_id = ObjectId(req.params.user_id);
  var query;
  if (game_name === 'all') {
    query = {'user_id': user_id};
  } else {
    query = {'user_id': user_id, 'game_name': game_name};
  }
  db.submits.find(query)
    .sort({'date': -1}, function(err, submits) {
    if (err) {
      req.flash('error', 'Failed to get submition list.');
      return res.redirect('back');
    }
    res.render('submit_list', { title: 'WAI : Submitions',
      submits: submits});
  });
};
*/
exports.submit_post = function(req, res) {
  fs.readFile(req.files.code.path, function(err, data) {
    if (err) {
      req.flash('error', 'Submit failed.');
      return res.redirect('back');
    }
    db_util.get_latest_submit(req.session.user.email, req.params.game_name, function(err, latest_submit) {
      if (err) {
        req.flash('error', 'Get Latest Submit failed.');
        return res.redirect('back');
      }
      /* TODO: Theorectically, the *version* should be an atomic
          auto_inc key. But is it neccessary? */
      var next_version;
      if (!latest_submit) next_version = 1;
      else next_version = latest_submit.version + 1;
      var submit = {
        user_id: ObjectId(req.session.user._id),
        user_nick: req.session.user.nick,
        user_email: req.session.user.email,
        game_name: req.params.game_name,
        lang: req.body['lang'],
        code: data,
        size: req.files.code.size / 1024.0,
        date: new Date(),
        status: 0,
        compile_output: '',
        ori_name: req.files.code.name,
        version: next_version,
        allow_view: req.body['allow_view'],
      };
      db.submits.insert(submit, {safe: true},
        function(err, result) {
        if (err) {
          req.flash('error', 'Submit failed.');
          return res.redirect('back');
        }
        var ext = '';
        if (req.body['lang'] === 'C++') {
          ext = '.cpp';
        } else {
          ext = '.txt';
        }
        fs.rename(req.files.code.path,
            waiconst.USER_SRC_PATH + result[0]._id + ext,
            function(err) {
              if (err) {
                console.log('rename failed: ' + err);
                req.flash('error', 'Submit failed.');
                return res.redirect('back');
              }
              // push to the compile queue
              queue.compile_queue.push(submit);
              req.flash('success', 'Submit successfully.');
              return res.redirect('back');
            });
      });
    });
  });  
}
