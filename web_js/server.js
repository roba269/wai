var http = require('http');
var io = require('socket.io');

app = http.createServer(function(req, res) {
    res.writeHead(200);
    res.write('heelo');
    res.end();
}).on(
    'connection', function() {
        console.log('recv a connection');
    }
).on(
    'close', function() {
        console.log('close a conn');
    }    
).listen(8081);

io.listen(app).sockets.on('connection', function(socket) {
    socket.emit('news', { hello: 'world'});
    socket.on('other', function(data) {
        console.log(data);
    });
});

console.log('the server starts');

