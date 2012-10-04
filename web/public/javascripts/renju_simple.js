var steps = [];
var is_over = false;
var canvas;
var step_idx = 0;
var socket = io.connect('http://localhost:3000');

const LEFT_MARGIN = 15;
const TOP_MARGIN = 15;
const GRID_SIZE = 30;
const NUM_ROW = 15;
const NUM_COL = 15;

var inter_id;
var play_status = 0;
var who_to_move;
var g_is_hvc;

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

function load(id, is_hvc) {
  g_is_hvc = is_hvc;
  canvas = document.getElementById('arena');
  canvas.height = 800;
  canvas.width = 800;
  draw();
  if (is_hvc === 0) {
    socket.once('res_steps', function(data) {
      for (var i = 0 ; i < data.steps.length ; ++i) {
        var tmp = data.steps[i].split(' ');
        var item = {'color': parseInt(tmp[0]),
          'x': parseInt(tmp[1]), 'y': parseInt(tmp[2])};
        steps.push(item);
      }
      // steps = data.steps;
    });
    socket.emit('req_steps', {match_id: id});
  } else {
    canvas.addEventListener('mousemove', onMouseMove);
    canvas.addEventListener('mousedown', onMouseDown);
    who_to_move = 1;
    socket.emit('req_hvc', {submit_id: id});
    socket.once('game_over', function(data) {
      is_over = true;
      alert("Game over. " + data.res_str + " " + data.reason);
    });
  }
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
    ctx.arc(LEFT_MARGIN + y * GRID_SIZE,
            TOP_MARGIN + x * GRID_SIZE,
            GRID_SIZE/2, 0, Math.PI*2, true);
    if (color === 0) ctx.fill();
    else ctx.stroke();
}
function onMouseMove(evt) {
}
function onMouseDown(evt) {
    if (g_is_hvc === 0) return;
    if (is_over) return;
    if (who_to_move !== 1) return;
    tmp = getMousePos(canvas, evt);
    chess_pos = {x: 0, y: 0};
    chess_pos.y = Math.floor((tmp.x - LEFT_MARGIN) / GRID_SIZE + 0.5);
    chess_pos.x = Math.floor((tmp.y - TOP_MARGIN) / GRID_SIZE + 0.5);
    if (chess_pos.x < 0 || chess_pos.x >= NUM_ROW ||
            chess_pos.y < 0 || chess_pos.y >= NUM_COL) {
        return;
    }
    steps.push(chess_pos);
    ++step_idx;
    draw();
    socket.emit('put_chess', {x: chess_pos.x, y: chess_pos.y});
    who_to_move = 2;
    socket.once('put_response', function(data) {
        who_to_move = 1;
        if (data.is_over) {
            is_over = true;
            alert("over " + data.res_str + " " + data.reason);
        } else if (data.invalid_step) {
            steps.pop();
            --step_idx;
            draw();
        } else {
            steps.push({x: data.x, y: data.y});
            ++step_idx;
            draw();
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

function autoPlay() {
  if (play_status === 0) {
    play_status = 1;
    document.getElementById("autoplay").innerHTML = "Pause";
    inter_id = setInterval(nextStep, 1000);
  } else if (play_status === 1) {
    clearInterval(inter_id);
    play_status = 0;
    document.getElementById("autoplay").innerHTML = "Auto Play";
  }
}

