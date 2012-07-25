var steps = [];
var is_over = false;
var canvas;
var step_idx = 0;
var socket = io.connect('http://localhost:3000');

const LEFT_MARGIN = 25;
const TOP_MARGIN = 25;
const GRID_SIZE = 50;
const NUM_ROW = 15;
const NUM_COL = 15;

function getMousePos(canvas, evt) {
    // get canvas position
    var obj = canvas;
    var top = 0;
    var left = 0;
    
    while(obj && obj.tagName != 'BODY') {
        top += obj.offsetTop;
        left += obj.offsetLeft;
        obj = obj.offsetParent;
    }

    // return relative mouse position
    var mouseX = evt.clientX - left + window.pageXOffset;
    var mouseY = evt.clientY - top + window.pageYOffset;
    return {
        x: mouseX,
        y: mouseY
    };
  }

function drawBoard() {
    var ctx = canvas.getContext('2d');
    ctx.clearRect(0, 0, 900, 900);
    ctx.fillStyle = "rgb(0,0,0)";
    var i;
    ctx.beginPath();
    for (i = 0 ; i < NUM_COL ; ++i) {
        ctx.moveTo(i * GRID_SIZE + LEFT_MARGIN, TOP_MARGIN);
        ctx.lineTo(i * GRID_SIZE + LEFT_MARGIN,
              (NUM_ROW-1) * GRID_SIZE + TOP_MARGIN);
    }
    for (i = 0 ; i < NUM_ROW ; ++i) {
        ctx.moveTo(LEFT_MARGIN, i * GRID_SIZE + TOP_MARGIN);
        ctx.lineTo(LEFT_MARGIN + (NUM_COL-1) * GRID_SIZE,
                    i * GRID_SIZE + TOP_MARGIN);
    }
    ctx.stroke();
}

function load(match_id) {
  canvas = document.getElementById('arena');
  // canvas.addEventListener('mousemove', onMouseMove);
  // canvas.addEventListener('mousedown', onMouseDown);
  draw();
  socket.once('res_steps', function(data) {
    for (var i = 0 ; i < data.steps.length ; ++i) {
      var tmp = data.steps[i].split(' ');
      var item = {'color': parseInt(tmp[0]),
        'x': parseInt(tmp[1]), 'y': parseInt(tmp[2])};
      steps.push(item);
    }
    // steps = data.steps;
  });
  socket.emit('req_steps', {match_id: match_id});
}

function draw() {
    drawBoard();
    var color = 0;
    for (var i = 0 ; i < step_idx ; ++i) {
      drawChess(steps[i].x, steps[i].y, color);
      color = 1 - color;
    }
}

function drawChess(x, y, color) {
    var ctx = canvas.getContext('2d');
    ctx.fillStyle = "rgb(0,0,0)";
    ctx.beginPath();
    ctx.arc(LEFT_MARGIN + x * GRID_SIZE,
            TOP_MARGIN + y * GRID_SIZE,
            GRID_SIZE/2, 0, Math.PI*2, true);
    if (color === 0) ctx.fill();
    else ctx.stroke();
}
function onMouseMove(evt) {
    tmp = getMousePos(canvas, evt);
    document.getElementById("x_pos").innerHTML = tmp.x;
    document.getElementById("y_pos").innerHTML = tmp.y;
}
function onMouseDown(evt) {
    if (is_over) return;
    tmp = getMousePos(canvas, evt);
    chess_pos = {x: 0, y: 0};
    chess_pos.x = Math.floor((tmp.x - LEFT_MARGIN) / GRID_SIZE + 0.5);
    chess_pos.y = Math.floor((tmp.y - TOP_MARGIN) / GRID_SIZE + 0.5);
    if (chess_pos.x < 0 || chess_pos.x >= NUM_COL ||
            chess_pos.y < 0 || chess_pos.y >= NUM_ROW) {
        return;
    }
    drawChess(chess_pos.x, chess_pos.y, 0);
    steps.push(chess_pos);
    socket.emit('put_chess', {x: chess_pos.x, y: chess_pos.y});
    socket.once('put_response', function(data) {
        if (data.is_over) {
            is_over = true;
            alert("over winner = " + data.winner);
        } else {
            drawChess(data.x, data.y, 1);
        }
    });
}

function nextStep() {
  if (steps.length === 0) {
    console.log('Steps are not loaded.');
    return;
  }
  if (++step_idx >= steps.length)
    step_idx = steps.length;
  draw();
}

function prevStep() {
  if (steps.length === 0) {
    console.log('Steps are not loaded.');
    return;
  }
  if (--step_idx < 0)
    step_idx = 0;
  draw();
}

