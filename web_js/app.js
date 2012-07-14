var app = require('http').createServer(handler)
  , io = require('socket.io').listen(app)
  , fs = require('fs')

app.listen(8080);

var child_process = require('child_process')

function handler (req, res) {
  fs.readFile(__dirname + '/index.html',
  function (err, data) {
    if (err) {
      res.writeHead(500);
      return res.end('Error loading index.html');
    }

    res.writeHead(200);
    res.end(data);
  });
}


io.sockets.on('connection', function (socket) {
    var match = child_process.spawn('../hvc_match.exe',
         ['../test_judge/renju_judge.exe',
          '../test_ai/renju_bf.exe']);
    socket.on('put_chess', function(data) {
        console.log('recv put chess: x:' + data.x + " y:" + data.y);
        match.stdin.write(data.x + " " + data.y + "\n");
        match.stdout.once('data', function(data) {
            console.log("computer data: {" + data + "}");
            if (data.toString().charAt(0) === ':') {
                console.log("winner: " + data);
                resp = {'is_over': true, 
                    'winner': data.toString().charAt(1)};
            } else {
                resp = {'is_over': false,
                    'x': parseInt(data.toString().split(' ')[0]),
                    'y': parseInt(data.toString().split(' ')[1])};
            }
            socket.emit('put_response', resp);
        });
    });
});

// disconnection

