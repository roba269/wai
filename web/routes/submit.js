var db = require('../models/db');
var fs = require('fs');

exports.submit_list = function(req, res) {
  if (!req.session.user) {
    req.flash('error', 'You are not login.');
    return res.redirect('back');
  }
  var user_email = req.session.user.email;
  var game_name = req.params.game_name;
  var submit_list = [];
  db.submits.find({user_email: user_email, game_name: game_name}, function(err, submits) {
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

exports.submit_post = function(req, res) {
  console.log("req.body:" + req.body);
  console.log("req.files:" + req.files);
  fs.readFile(req.files.code.path, function(err, data) {
    if (err) {
      req.flash('error', 'Submit failed.');
      return res.redirect('back');
    }
    var submit = {
      user_email: req.session.user.email,
      game_name: req.params.game_name,
      lang: req.body['lang'],
      code: data,
      size: req.files.code.size / 1024.0,
      date: new Date(),
      status: 0,
      compile_output: '',
      ori_name: req.files.code.name,
    };
    db.submits.insert(submit, {safe: true}, function(err, result) {
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
      /* FIXME: the path is bad */
      fs.rename('./' + req.files.code.path,
          './uploads/' + result[0]._id + ext,
          function(err) {
            if (err) console.log('rename failed: ' + err);
          });
      req.flash('success', 'Submit successfully.');
      return res.redirect('back');
    });
  });  
}
