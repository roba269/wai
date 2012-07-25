// var db = require('mongodb');

// module.exports = new db.Db('wai', new db.Server('localhost', db.Connection.DEFAULT_PORT, {}));

var databaseUrl = "wai";
var collections = ['users', 'submits', 'matches', 'games'];
var db = require('mongojs').connect(databaseUrl, collections);
module.exports = db;
