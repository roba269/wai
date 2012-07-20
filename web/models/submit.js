var mongodb = require('./db');

function Submit(submit) {
  this.user_email = submit.user_email;
  this.game_name = submit.game_name;
  this.lang = submit.lang;
  this.code = submit.code;
  this.status = 0;
  this.compile_output = '';
};

module.exports = Submit;

Submit.prototype.save = function submit_save(callback) {
  var submit = new Submit(this);
  mongodb.open(function(err, db) {
    if (err) return callback(err);
    db.collection('submits', function(err, collection) {
      if (err) {
        mongodb.close();
        return callback(err);
      }
      collection.insert(submit, {safe:true}, function(err, submit) {
        mongodb.close();
        callback(err, submit);
      }); 
    });
  });
};

Submit.getById = function(obj_id, callback) {
  mongodb.open(function(err, db) {
    if (err) return callback(err);
    db.collection('submits', function(err, collection) {
      if (err) {
        mongodb.close();
        return callback(err);
      }
      collection.findOne({_id: obj_id}, function(err, doc) {
        mongodb.close();
        if (doc) {
          var submit = new Submit(doc);
          callback(err, submit);
        } else {
          callback(err, null);
        }
      });
    });
  });
};
