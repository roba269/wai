var db = require('./models/db');
var cp = require('child_process');
var waiconst = require('./waiconst');
var queue = require('./queue');
var scheduler = require('./scheduler');

// var SLEEP_IN_MS = 1000;

exports.compile = compile;

function compile(submit, callback) {
  if (submit.lang === 'C++') {
    // console.log('id:' + submit._id);
    var cmd = 'g++ -Wall -static -ftemplate-depth-17 -O3 '
      + waiconst.USER_SRC_PATH + submit._id + '.cpp -o'
      + waiconst.USER_EXE_PATH + submit._id + '.exe';
    // var cmd = 'g++ -Wall -static -ftemplate-depth-17 -O3 ' + __dirname + '/uploads/' + submit._id + '.cpp -o ' + __dirname + '/exe/' + submit._id + '.exe';
    cp.exec(cmd, function(err, stdout, stderr) {
        if (err) {
          console.log('id:' + submit._id + ' error: ' + err);
          db.submits.update({_id: submit._id},
              {$set: {status: 1, 'compile_output': stderr}},
              function(err) {
                if (err) console.log('Update failed: ' + err);
                return callback();
              });
        } else {
          // console.log('id:' + submit._id + ' successful');
          db.submits.update({'game_name': submit.game_name,
              'user_email': submit.user_email, 'last': 1},
            {$set: {'last': 0}},
              function(err) {
                if (err) {
                  console.log('Update last=0 failed.');
                  return callback();
                }
                db.submits.update({_id: submit._id},
                    {$set: {'status': 2, 'last': 1}},
                    function(err) {
                      if (err) {
                        console.log('Update failed: ' + err);
                      } else {
                        scheduler.push_matches(submit);
                      }
                      return callback();
                    });
              });
        }
      });
  } else {
    console.log("I cannot compile languages other than C++ now.");
    return callback();
  }
}

