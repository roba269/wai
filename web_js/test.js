var match = require('child_process').spawn('./test.exe');

match.stdin.write("4 4\n");

match.stdout.on('data', function(data) {
    console.log('test.exe output:' + data);
    var x = parseInt(data.toString().split(" ")[0]);
    var y = parseInt(data.toString().split(" ")[0]);
    match.stdin.write(x + " " + y + "\n");
});

match.on('exit', function(code) {
    console.log('test.exe exit with code:' + code)
});

