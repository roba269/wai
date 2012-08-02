var db = require('./models/db');
var cp = require('child_process');
var util = require('util');

var SLEEP_IN_MS = 1000;

function start_match(uid1, submit1, uid2, submit2, game) {
  var cmd = util.format('../match.exe ../test_judge/%s_judge.exe ./exe/%s.exe ./exe/%s.exe', game, submit1._id, submit2._id);
  console.log('cmd:' + cmd);
  var match = cp.spawn('../match.exe', [
    util.format('../test_judge/%s_judge.exe', game),
    util.format('./exe/%s.exe', submit1._id),
    util.format('./exe/%s.exe', submit2._id)]);
  var trans = [];
  var result;
  var reason;
  match.stdout.on('data', function(data) {
    console.log('stdout from judge:' + data);
    var str_list = data.toString().split('\n');
    for (var i = 0 ; i < str_list.length ; ++i) {
      data_str = str_list[i];
      if (data_str.length === 0) continue;
      if (data_str[0] < '0' || data_str[0] > '9') {
        trans.push(data_str.substr(1));
        // var tmp = data_str.substr(1).split(' ');
        // trans.push({color: parseInt(tmp[0]), x: parseInt(tmp[1]), y: parseInt(tmp[2])});
      } else {
        var tmp = data_str.split(' ');
        result = parseInt(tmp[0]);
        if (tmp.length > 1) reason = tmp[1];
        // console.log('result:' + result + " reason:" + reason);
      }
    }
  });
  match.on('exit', function(code) {
    // console.log('Judge quit, error code: ' + code + ' result:' + result);
    db.matches.update(
      {'uid1': uid1, 'uid2': uid2, 'game': game, 'last': 1},
      {$set: {'last': 0}}, function(err) {
        if (err) console.log('update last=0 failed');
        db.matches.update({'sid1': submit1._id, 'sid2': submit2._id},
          {$set: {'last': 1, 'status': 2, 'result': result,
            'reason': reason, 'trans': trans}}, function (err) {
            if (err) console.log('update matches failed:' + err);
          });
      });
  });
}

function get_latest_submit(user1, user2, game) {
  console.log('user1: %j', user1);
  console.log('user2: %j', user2);
  db.submits.find({'user_email': user1.email, 'game_name': game, 'status': 2}).sort({date: -1}, function(err, submit_list1) {
      if (err) {
        console.log('get_latest_submit1 error:' + err);
        return;
      }
      if (submit_list1.length === 0) {
        console.log('No submit for user %s game %s', user1.email, game);
        return;
      }
      var submit1 = submit_list1[0];
      db.submits.find({'user_email': user2.email, 'game_name': game, 'status': 2}).sort({date: -1}, function(err, submit_list2) {
          if (err) {
            console.log('get_latest_submit2 error:' + err);
            return;
          } 
          if (submit_list2.length === 0) {
            console.log('No submit for user %s game %s',
              user2.email, game);
            return;
          }
          var submit2 = submit_list2[0];
          console.log('submit1: ' + submit1._id + ' submit2: ' + submit2._id);
          db.matches.findOne(
            {'sid1': submit1._id, 'sid2': submit2._id}, 
            function(err, match) {
              if (err) {
                console.log('find match error: ' + err);
                return;
              }
              if (!match) {
                var match_item = {
                  'game': game,
                  'uid1': user1._id,
                  'uid2': user2._id,
                  'nick1': user1.nick,
                  'nick2': user2.nick,
                  'sid1': submit1._id,
                  'sid2': submit2._id,
                  'status': 1,
                  'result': -1,
                  'last': 0,  // TODO: is it right?
                  'date': new Date(),
                }
                db.matches.save(match_item);
                start_match(user1._id, submit1, user2._id, submit2, game);
              }
            }
          );
        });
    });
}

function schedule_match() {
  db.games.find(function(err, games) {
    if (err) {
      console.log('db.games.find failed, err:' + err);
      return;
    }
    games.forEach(function(game) {
      db.users.find(function(err, users) {
        if (err) {
          console.log('db.users.find failed, err:' + err);
          return;
        }
        console.log('users.length:' + users.length);
        for (var idx = 0 ; idx < users.length ; ++idx) {
          for (var idx2 = 0 ; idx2 < users.length ; ++idx2) {
             if (idx === idx2) continue;
             console.log('idx1:' + idx + ' idx2:' + idx2);
             get_latest_submit(users[idx], users[idx2], game.name);
          }
        }
        console.log('done');
      });
    });
  });
}

// setTimeout(schedule_match, SLEEP_IN_MS);
schedule_match();
