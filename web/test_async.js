var async = require('async');
/*
async.series([
  function(callback) {
    setTimeout(function() {
      console.log('run func 1');
      callback();
    }, 2000);
  },
  function(callback) {
    setTimeout(function() {
      console.log('run func 2');
      callback();
    }, 1000);
  },
], function(err, results) {
  console.log('err: %j', err);
  console.log('results: %j', results);
});
*/
async.waterfall([
  function(callback) {
    callback(null, 3, 4);
  },
  function(a, b, callback) {
    console.log(a, b);
    callback(null, 5);
  },
], function(err, results) {
  console.log('err: %j', err);
  console.log('results: %j', results);
});
