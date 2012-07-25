var db = require('./models/db');

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

