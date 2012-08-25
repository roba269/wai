var db = require('./models/db');
var async = require('async');
var db_util = require('./db_util');
var scheduler = require('./scheduler');
var compiler = require('./compiler');

exports.match_queue =
async.queue(function(match, queue_callback) {
  async.series([
    function (callback) {
      db_util.get_latest_usable_submit_uid(match.uid1,
        match.game, callback);
    },
    function (callback) {
      db_util.get_latest_usable_submit_uid(match.uid2,
        match.game, callback);
    },
  ], function (err, submits) {
    if (err || submits.length < 2 || !submits[0] || !submits[1]) {
      console.log('get latest usable sumbit failed. err:' + err);
      return queue_callback();
    }
    if (submits[0].version > match.version1 ||
        submits[1].version > match.version2) {
          // the queued match is out-dated, just drop it
          match.status = 4;
          db.matches.save(match);
          return queue_callback();
        }
    // run the match
    match.date = new Date();
    match.status = 1;
    db.matches.save(match);
    scheduler.start_match(match.uid1, match.sid1,
      match.uid2, match.sid2, match.game, queue_callback);
  });
}, 2);

exports.compile_queue =
async.queue(function(submit, queue_callback) {
  console.log('I recv a sumbit');
  compiler.compile(submit, queue_callback);
}, 2);

