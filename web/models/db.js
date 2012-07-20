var db = require('mongodb');

module.exports = new db.Db('wai', new db.Server('localhost', db.Connection.DEFAULT_PORT, {}));
