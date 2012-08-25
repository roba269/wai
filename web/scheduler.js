var db = require('./models/db');
var cp = require('child_process');
var util = require('util');
var async = require('async');
var waiconst = require('./waiconst');
var db_util = require('./db_util');
var queue = require('./queue');

exports.start_match = start_match;
exports.push_matches = push_matches;

// var SLEEP_IN_MS = 300000; // 5 minutes

function start_match(uid1, sid1, uid2, sid2, game, callback) {
  console.log('game_name:' + game + ' uid1:' + uid1
      + ' sid1:' + sid1 + ' uid2:' + uid2 + ' sid2:' + sid2);
  var match = cp.spawn(waiconst.MATCH_PATH + 'match.exe', [
    util.format(waiconst.JUDGE_PATH + '%s_judge.exe', game),
    util.format(waiconst.USER_EXE_PATH + '%s.exe', sid1),
    util.format(waiconst.USER_EXE_PATH + '%s.exe', sid2)]);
  var trans = [];
  var result;
  var result_str;
  var reason;
  match.stdout.on('data', function(data) {
    // console.log('stdout from judge:' + data);
    var str_list = data.toString().split('\n');
    for (var i = 0 ; i < str_list.length ; ++i) {
      data_str = str_list[i];
      if (data_str.length === 0) continue;
      if (data_str[0] < '0' || data_str[0] > '9') {
        trans.push(data_str.substr(1));
      } else {
        var tmp = data_str.split(' ');
        result = parseInt(tmp[0]);
        result_str = tmp[1];
        if (tmp.length > 2) reason = tmp[2];
      }
    }
  });
  match.on('exit', function(code) {
    // console.log('Judge quit, error code: ' + code + ' result:' + result);
    db.matches.update(
      {'uid1': uid1, 'uid2': uid2, 'game': game, 'last': 1},
      {$set: {'last': 0}}, function(err) {
        if (err) console.log('update last=0 failed');
        db.matches.update({'sid1': sid1, 'sid2': sid2},
          {$set: {'last': 1, 'status': 2, 'result': result, 'result_str': result_str,
            'reason': reason, 'trans': trans}}, function (err) {
            if (err) console.log('update matches failed:' + err);
            else {
              db_util.update_leader(game, function(err) {
                if (err) console.log('update_leader failed.');
                callback();
              });
            }
          });
      });
  });
}

function push_matches(submit1) {
  async.waterfall([
    function(callback) {
      db.games.find({}, function(err, game_list) {
        if (err) return callback(err);
        return callback(null, game_list);
      });
    },
    function(game_list, callback) {
      db.users.find({}, function(err, user_list) {
        if (err) return callback(err);
        return callback(null, game_list, user_list);
      });
    },
  ], function(err, game_list, user_list) {
    if (err) {
      console.log('push matches failed. err:' + err);
      return;
    }
    game_list.forEach(function(game) {
      user_list.forEach(function(user2) {
        db_util.get_latest_usable_submit_uid(user2._id, game.name,
          function(err, submit2) {
            if (err || !submit2) return;
            if (submit1.user_id.equals(user2._id)) return;
            var match_item = {
              'game': game.name,
              'uid1': submit1.user_id,
              'uid2': user2._id,
              'nick1': submit1.user_nick,
              'nick2': user2.nick,
              'sid1': submit1._id,
              'sid2': submit2._id,
              'status': 0,
              'result': -1,
              'last': 0,  // TODO: is it right?
              'version1': submit1.version,
              'version2': submit2.version,
              // Note: no date field
            };
            db.matches.save(match_item);
            queue.match_queue.push(match_item);
            var match_item2 = {
              'game': game.name,
              'uid2': submit1.user_id,
              'uid1': user2._id,
              'nick2': submit1.user_nick,
              'nick1': user2.nick,
              'sid2': submit1._id,
              'sid1': submit2._id,
              'status': 0,
              'result': -1,
              'last': 0,  // TODO: is it right?
              'version2': submit1.version,
              'version1': submit2.version,
            };
            db.matches.save(match_item2);
            queue.match_queue.push(match_item2);
            // console.log('push match into queue: %j', match_item);
            // console.log('push match into queue: %j', match_item2);
          });
      });
    });
  });
}

