var db = require('./models/db');
var async = require('async');

exports.get_latest_result = get_latest_result;
exports.get_latest_submit = get_latest_submit;
exports.get_rank_list = get_rank_list;
exports.update_leader = update_leader;

function submit_id_to_user_email(submit_id, callback) {
  db.sumits.findOne({_id: ObjectId(submit_id)},
    function(err, submit) {
      if (err) {
        console.log('Cannot find submit id:' + err);
        callback(err, null);
      } else {
        callback(err, submit.user_email);
      }
    });
}

function user_id_to_user(user_id, callback) {
  db.users.findOne({_id: user_id},
    function(err, user) {
      if (err) {
        console.log('Cannot find user id %j', user_id);
        callback(err, null);
      } else {
        callback(err, user);
      }
    });
}

function get_real_str(str) {
  if (!str) return ""
  return str.replace(/_/g, " ");
}

// Get the match result between specified user_id
// and every other users' latest version
// Note that because the new version may be submitted
// at any time, the match result is still unknown. At
// that time, the result will be the previous versions'.
function get_latest_result(inp_user_id, game_name, inp_callback) {
  async.waterfall([
    // get user list
    function(callback) {
      db.users.find({}, function(err, users) {
        if (err) return callback(err);
        callback(null, users);
      });
    },
    // for each user, get his latest usable submit on this game
    function(users, callback) {
      async.map(users, function _get_last_submit(user, cbk) {
        db.submits.findOne({'user_email': user.email, 'game_name': game_name, 'last': 1},
          function(err, submit) {
            if (err) return cbk(err);
            if (!submit) return cbk(null, null);
            cbk(null, submit._id);
          });
      }, function(err, result) {
        if (err) callback(err);
        // console.log('result: %j', result);
        var last_submit = {};
        for (var idx = 0 ; idx < users.length && idx < result.length ; ++idx) {
          last_submit[users[idx]._id] = result[idx];
        }
        callback(null, users, last_submit);
      });
    },
    // for each user, find the match results
    function(users, last_submition, callback) {
      async.map(users, function _get_result(user, cbk) {
          if (user._id.equals(inp_user_id)) {
            return cbk(null, null);
          }
          db.matches.find({$or: [
              {'game': game_name, 'uid1': inp_user_id, 'uid2': user._id, 'last': 1},
              {'game': game_name, 'uid1': user._id, 'uid2': inp_user_id, 'last': 1}]
            }, function(err, match_list) {
              if (err) return cbk(err, null);
              var matches_view = []
              for (var idx = 0 ; idx < match_list.length ; ++idx) {
                var flg1 = 1;
                var flg2 = 1;
                if (!match_list[idx].sid1.equals(last_submition[match_list[idx].uid1])) flg1 = 0;
                if (!match_list[idx].sid2.equals(last_submition[match_list[idx].uid2])) flg2 = 0;
                matches_view.push({
                  'game_name': game_name,
                  'match_id': match_list[idx]._id,
                  'uid1': match_list[idx].uid1,
                  'uid2': match_list[idx].uid2,
                  'sid1': match_list[idx].sid1,
                  'sid2': match_list[idx].sid2,
                  'nick1': match_list[idx].nick1,
                  'nick2': match_list[idx].nick2,
                  'flg1': flg1,
                  'flg2': flg2,
                  'status': match_list[idx].status,
                  'result': match_list[idx].result,
                  'result_str': get_real_str(match_list[idx].result_str),
                  'reason': get_real_str(match_list[idx].reason),
                  'version1': match_list[idx].version1,
                  'version2': match_list[idx].version2,
                  'date': match_list[idx].date,
                });
              }
              cbk(null, matches_view);
            }
          );
        }, function(err, matches_view_array) {
            if (err) return callback(err);
            var matches_view_list = [];
            for (var idx = 0 ; idx < matches_view_array.length ; ++idx) {
              if (!matches_view_array[idx]) continue;
              for (var idx2 = 0 ; idx2 < matches_view_array[idx].length ; ++idx2) {
                matches_view_list.push(matches_view_array[idx][idx2]);
              }
            }
            callback(null, matches_view_list);
        });
    },
  ], function(err, result) {
    if (err) return callback(err);
    inp_callback(null, result);
  });
}

// function get_latest_result(user_id, callback) {
//   db.users.find({}, function(err, users) {
//     if (err) {
//       console.log('users.find failed.');
//       callback(err, null);
//     }
//     var match_to_show = [];
//     for (var idx = 0 ; idx < users.length ; ++idx) {
//       if (users[idx]._id === user_id) continue;
//       db.matches.find({$or: [
//           {'uid1': user_id, 'uid2': users[idx]._id},
//           {'uid1': user[idx]._id, 'uid2': user_id}]
//         }).sort({date: -1}, function(err, match_list) {
//           if (err) {
//             console.log('matches.find failed, err:' + err);
//             return callback(err, null);
//           }
//           if (match_list.length === 0) {
//             return callback(err, null);
//           }
//           for (var i = 0 ; i < match_list.length ; ++i) {
//             user_id_to_user(match_list[i].uid1,
//               function(err, user1) {
//                 user_id_to_user(match_list[i].uid2,
//                   function(err, user2) {
//                     match_to_show.push({
//                       'black_nick': user1.nick,
//                       'black_version': 0, /* FIXME */
//                       'white_nick': user2.nick,
//                       'white_version': 0, /* FIXME */
//                       'status': match_list[i].status,
//                       'result': match_list[i].result,
//                     });
// 
//                   });
//               });
//           }
//         });
// 
//     }
//   });
// 
//   db.matches.find({$or: [{uid1: ObjectId(user_id)},
//     {uid2: ObjectId(user_id)}]}, function(err, match_list) {
//       if (err) {
//         console.log('get_latest_result failed.');
//         callback(err, null);
//       }
//       match_list.forEach(
//     });
// }
// 

function get_user_list(callback) {
  db.users.find({}, function(err, users) {
    if (err) return callback(err);
    callback(null, users);
  });
}
/*
function get_user_score(inp_user_id, game_name, inp_callback) {
  async.waterfall([
    // get user list
    get_user_list,
    function _get_match_score(users, callback) {
      for (var idx = 0 ; idx < users.length ; ++idx) {
        var user = users[idx];
        if (user._id.equals(inp_user_id)) continue;
        db.matches.find({$or: [
            {'game': game_name, 'uid1': inp_user_id, 'uid2': user._id, 'last': 1},
            {'game': game_name, 'uid1': user._id, 'uid2': inp_user_id, 'last': 1}]
          }, function(err, match_list) {
            if (err) return callback(err, null);
            var score = 0
            for (var idx = 0 ; idx < match_list.length ; ++idx) {
              if (match_list[idx].result < 0) continue;  // error
              if (match_list[idx].result == 0) { // draw
                score += 1; 
                continue;
              }
              var me;
              if (match_list[idx].uid1.equals(inp_user_id)) me = 0;
              else me = 1;
              if ((me === 0 && match_list[idx].result == 1) || 
                  (me === 1 && match_list[idx].result == 2)) score += 3;
            }
            callback(null, score);
          });
      }
    },
  ], function(err, score) {
    if (err) return callback(err);
    inp_callback(null, score);
  });
}

function get_rank_list(game_name, callback) {
  async.waterfall([
    get_user_list,
    function _get_user_score(user_list, callback) {
      async.map(user_list, function(user, callback) {

      }, function (err, score_list) {

      }
    }
  ], function(err, rank_list) {
    if (err) return callback(err);
    callback(null, rank_list);
  }
}
*/

function get_rank_list(game_name, callback) {
  db.submits.find({'game_name': game_name}, function(err, submits) {
    if (err || !submits) {
      console.log('db.submits.find failed, err:' + err);
      return callback(err, null);
    }
    var users = [];
    var users_set = {};
    for (var sub_idx = 0 ; sub_idx < submits.length ; ++sub_idx) {
      if (submits[sub_idx].user_id in users_set) continue;
      users_set[submits[sub_idx].user_id] = true;
      users.push({'_id': submits[sub_idx].user_id,
        'nick': submits[sub_idx].user_nick,
        'email': submits[sub_idx].user_email});
    }
    var score_list = [];
    for (var idx = 0 ; idx < users.length ; ++idx) {
      score_list.push({user: users[idx], score: 0});
    }
    if (users.length <= 1) {
      return callback(null, score_list);
    }
    // console.log('init score_list: %j', score_list);
    var counter = users.length * (users.length - 1);
    for (var idx = 0 ; idx < users.length ; ++idx) {
      for (var idx2 = 0 ; idx2 < users.length ; ++idx2) {
        if (idx === idx2) continue;
        (function(idx, idx2) {
          // console.log('idx: %j, idx2: %j', idx, idx2);
          db.matches.find({'uid1': users[idx]._id,
            'uid2': users[idx2]._id,
            'game': game_name, 'last': 1}).sort({date: -1},
            function(err, match_list) {
              --counter;
              if (err) {
                console.log('get last match failed. err:' + err);
                return callback(err, null);
              }
              if (match_list.length !== 0) {
                var match = match_list[0];
                if (match.result === 0) {
                  // draw
                  score_list[idx].score += 1;
                  score_list[idx2].score += 1;
                } else if (match.result === 1) {
                  score_list[idx].score += 3;
                } else if (match.result === 2) {
                  // console.log('idx2:%j users.length:%j score_list: %j', idx2, users.length, score_list);
                  score_list[idx2].score += 3;
                } else console.log('Unexpected result for match bewteen user %s and %s', users[idx].nick, users[idx2].nick);
              }
              if (counter === 0) {
                // sort score_list
                score_list = score_list.sort(function(ita, itb) {
                  if (ita.score < itb.score) return 1;
                  if (ita.score > itb.score) return -1;
                  return 0;
                });
                // console.log('socre_list: %j', score_list);
                callback(null, score_list);
              }
            });
        })(idx, idx2);
      }
    }
  });
}

// Given user_id and game_name, return the latest submition
// (sorting by submit time, for any status)

function get_latest_submit(email, game_name, callback) {
  db.submits.find({'user_email': email, 'game_name': game_name})
      .sort({'date': -1}, function(err, submit_list) {
        if (err) return callback(err, null);
        if (!submit_list || submit_list.length === 0)
          return callback(null, null);
        callback(null, submit_list[0]);
    });
}

// Given user_id and game_name, return the latest usable submition
// (status == 2)

function get_latest_usable_submit(email, game_name, callback) {
  db.submits.findOne({'user_email': email, 'game_name': game_name,
    'last': 1}, function(err, submit) {
      if (err) return callback(err, null);
      if (!sumbit) return callback(null, null);
      callback(null, submit);
    });
}

function update_leader(game_name, callback) {
  get_rank_list(game_name, function(err, rank_list) {
    if (err) return callback(err);
    if (rank_list.length === 0) {
      return callback(null);
    } else {
      db.games.findOne({name: game_name}, function(err, game) {
        if (err || !game) {
          console.log('Find game failed. err:' + err);
          return callback(err);
        }
        if (game.leader !== rank_list[0].user) {
          game.leader = rank_list[0].user;
          game.update_date = new Date();
          db.games.save(game);
        }
        callback(null);
      });
    }
  });
}

