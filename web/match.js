var db = require('../models/db');
var db_util = require('../db_util');

exports.match_list = function(req, res) {
  if (!req.session.user) {
    req.flash('error', 'You are not login.');
    return res.redirect('back');
  }
  db.matches.find({}, function(err, matches) {
    if (err) {
      req.flash('error', 'Failed to get match list.');
      return res.redirect('back');
    }
    matches.forEach(function(match) {
      db_util.submit_id_to_user_email(match.sid1,
        function(err, user_email1) {
          db_util.submit_id_to_user_email(match.sid2,
          function(err, user_email2) {
            
            if (user_email1 === req.session.user.email) {

            } else if (user_email2 == req.session.user.email) {

            }
          });
        });
    });
  });
};

