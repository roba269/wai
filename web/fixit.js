var db = require('./models/db');

function fixit() {
  var user_list;
  db.games.find({}, function(err, game_list) {
    if (err || !game_list) return;
    game_list.forEach(function(game) {
      console.log('Fixing game:' + game.name);
      db.users.find({}, function(err, user_list) {
        if (err || !user_list) return;
        user_list.forEach(function(user1) {
          user_list.forEach(function(user2) {
            if (user1 === user2) return;
            db.matches.update({
              'game': game.name,
              'uid1': user1._id,
              'uid2': user2._id,
            }, {$set: {last: 0}}, {multi: true},
            function(err) {
              if (err) return;
              db.matches.find({
                'game': game.name,
                'uid1': user1._id,
                'uid2': user2._id,
                'status': 2,
              }).sort({version1: -1, version2: -1})
              .limit(1,
                function(err, match_list) {
                  if (err || !match_list) return;
                  if (match_list.length === 0) return;
                  console.log('match: %j', match_list[0]);
                  match_list[0].last = 1;
                  db.matches.save(match_list[0]);
                });
            });
          });
        });
      });
    });
  });
}

fixit();
