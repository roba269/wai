
var db = require('./models/db');
var cp = require('child_process');

var SLEEP_IN_MS = 1000;

function compile(submit) {
  if (submit.lang === 'C++') {
    // console.log('id:' + submit._id);
    var cmd = 'g++ -Wall -static -ftemplate-depth-17 -O3 ./uploads/'
      + submit._id + '.cpp -o ./exe/' + submit._id + '.exe';
    cp.exec(cmd, function(err, stdout, stderr) {
        if (err) {
          console.log('id:' + submit._id + ' error: ' + err);
          db.submits.update({_id: submit._id},
              {$set: {status: 1, 'compile_output': stderr}},
              function(err) {
                if (err) console.log('Update failed: ' + err);
              });
        } else {
          // console.log('id:' + submit._id + ' successful');
          db.submits.update({'game_name': submit.game_name,
              'user_email': submit.user_email, 'last': 1},
            {$set: {'last': 0}},
              function(err) {
                if (err) {
                  console.log('Update last=0 failed.');
                  return;
                }
                db.submits.update({_id: submit._id},
                    {$set: {'status': 2, 'last': 1}},
                    function(err) {
                      if (err)
                        console.log('Update failed: ' + err);
                    });
              });
        }
      });
  } else {
    console.log("I cannot compile java now.");
  }
}

function pick_submit() {
  db.submits.findOne({status: 0}, function(err, submit) {
    if (err || !submit) {
      // console.log("Not found uncompiled code.");
      return;
    }
    compile(submit);
  });
}

console.log('Compiler is running.');
setInterval(pick_submit, SLEEP_IN_MS);

