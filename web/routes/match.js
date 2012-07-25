var db = require('../models/db');
var db_util = require('../db_util');

exports.match_list = function(req, res) {
  if (!req.session.user) {
    req.flash('error', 'You are not login.');
    return res.redirect('back');
  }
  db.matches.find({game: req.params.game_name}, function(err, matches) {
    if (err) {
      req.flash('error', 'Failed to get match list.');
      return res.redirect('back');
    }
    var match_list = [];
    matches.forEach(function (match) {
      var flg = 0;
      console.log('match.uid1:' + match.uid1 + ' match.uid2:' + match.uid2 + ' user_sesson:' + req.session.user._id);
      if (match.uid1 == req.session.user._id) {
        flg = 1;
      } else if (match.uid2 == req.session.user._id) {
        flg = 2;
      }
      if (flg !== 0) {
        var match_item = {
          user_id1: match.uid1,
          submit_id1: match.sid1,
          user_id2: match.uid2,
          submit_id2: match.sid2,
          flg: flg,
          result: match.result,
          match_id: match._id,
        };
        match_list.push(match_item);
      }
    });
    res.render('match_list', {title: 'WAI : My Matches',
      game_name: req.params.game_name, matches: match_list});
  });
};
