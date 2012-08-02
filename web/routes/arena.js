var db = require('../models/db');

exports.arena_replay = function(req, res) {
  /* if (!req.session.user) {
    req.flash('error', 'You are not login.');
    return res.redirect('back');  
  } */
  res.render('arena_' + req.params.game_name, {title: 'WAI : Arena',
    match_id: req.params.match_id,
    game_name: req.params.game_name,
    layout: 'arena_layout'});
};

