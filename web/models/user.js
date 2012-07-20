var mongodb = require('./db');

function User(user) {
    this.email = user.email;
    this.passwd = user.passwd;
};

module.exports = User;

User.prototype.save = function save(callback) {
    var user = {email: this.email, passwd: this.passwd};
    mongodb.open(function(err,db) {
        if (err) return callback(err);
        db.collection('users', function(err, collection) {
            if (err) {
                mongodb.close();
                return callback(err);
            }
            collection.ensureIndex('email', {unique: true});
            collection.insert(user, {safe:true}, function(err, user) {
                mongodb.close();
                callback(err, user);
            });
        });
    });
};

User.get = function get(user_email, callback) {
    mongodb.open(function(err, db) {
        if (err) return callback(err);
        db.collection('users', function(err, collection) {
            if (err) {
                mongodb.close();
                return callback(err);
            }
            collection.findOne({email: user_email}, function(err, doc) {
                mongodb.close();
                if (doc) {
                    var user = new User(doc);
                    callback(err, user);
                } else {
                    callback(err, null);
                }
            });
        });
    });
};
