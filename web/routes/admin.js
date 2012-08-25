var queue = require('../queue');
var db = require('../models/db');
var ObjectId = require('mongoose').Types.ObjectId;

exports.rejudge = function(req, res) {
  if (!req.session.user || req.session.user.is_admin !== true) {
    req.flash('error', 'You have no such permission.');
    return res.redirect('back');
  }
  var match_id = req.params.match_id;
  db.matches.findOne({_id: ObjectId(match_id)},
    function(err, match) {
      if (err || !match) {
        req.flash('error', 'Failed to rejudge');
        return res.redirect('back');
      }
      queue.match_queue.push(match);
      req.flash('success', 'Rejudge successfully.');
      return res.redirect('back');
  });
}

