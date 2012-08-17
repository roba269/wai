var db = require('../models/db');

exports.arena_replay = function(req, res) {
  /* if (!req.session.user) {
    req.flash('error', 'You are not login.');
    return res.redirect('back');  
  } */
  res.render('arena',
    {title: 'WAI : Arena', hvc: 0,
      id: req.params.match_id,
      game_name: req.params.game_name,
      layout: 'arena_layout'});
};

exports.arena_hvc = function(req, res) {
  /* TODO: limit concurrent user number? */
  db.games.find({'name': req.params.game_name},
    function(err, games) {
      if (err || games.length === 0 || games[0].hvc === 0) {
        req.flash('error', 'This game doesn\'t support HvC feature.');
        return res.redirect('back');
      }
      res.render('arena',
        {title: 'WAI : Arena HvC', hvc: 1,
          id: req.params.submit_id,
          game_name: req.params.game_name,
          layout: 'arena_layout'});
    });
};
