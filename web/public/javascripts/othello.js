var steps = [];
var is_over = false;
var canvas;
var step_idx = 0;
var socket = io.connect('http://localhost:3000');

var dir8 = [[-1,-1],[-1,0],[-1,1],[0,-1],[0,1],[1,-1],[1,0],[1,1]];

const LEFT_MARGIN = 25;
const TOP_MARGIN = 25;
const GRID_SIZE = 60;
const NUM_ROW = 8;
const NUM_COL = 8;

var inter_id;
var play_status = 0;
var g_is_hvc;

function in_board(x, y) {
  return x >= 0 && x < NUM_ROW && y >= 0 && y < NUM_COL;
}

function Board() {
  this.bd = new Array();
  for (var i = 0 ; i < NUM_ROW ; ++i) {
    this.bd[i] = new Array();
    for (var j = 0 ; j < NUM_COL ; ++j) {
      this.bd[i][j] = 0;
    }
  }
  this.bd[3][3] = this.bd[4][4] = 1;
  this.bd[3][4] = this.bd[4][3] = 2;
}

Board.prototype.setBoard = function(board_a) {
  for (var i = 0 ; i < NUM_ROW ; ++i)
    for (var j = 0 ; j < NUM_COL ; ++j)
      this.bd[i][j] = board_a.bd[i][j];
}

Board.prototype.makeMove = function(move) {
  // Note: we donnot check validation
  this.bd[move.x][move.y] = move.color;
  for (var d = 0 ; d < 8 ; ++d) {
    tx = move.x + dir8[d][0];
    ty = move.y + dir8[d][1];
    while (in_board(tx, ty) && this.bd[tx][ty] == 3 - move.color) {
      tx += dir8[d][0];
      ty += dir8[d][1];
    }
    if (in_board(tx, ty) && this.bd[tx][ty] == move.color) {
      tx -= dir8[d][0];
      ty -= dir8[d][1];
      while (tx != move.x || ty != move.y) {
        this.bd[tx][ty] = move.color;
        tx -= dir8[d][0];
        ty -= dir8[d][1];
      }
    }
  }
}

Board.prototype.outputForDebug = function() {
  for (var i = 0 ; i < NUM_ROW ; ++i) {
    var str = '';
    for (var j = 0 ; j < NUM_COL ; ++j)
      str += this.bd[i][j];
    console.log(str);
 }
}

var show_board = [new Board()];

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
    for (i = 0 ; i <= NUM_COL ; ++i) {
        ctx.moveTo(i * GRID_SIZE + LEFT_MARGIN, TOP_MARGIN);
        ctx.lineTo(i * GRID_SIZE + LEFT_MARGIN,
              (NUM_ROW) * GRID_SIZE + TOP_MARGIN);
    }
    for (i = 0 ; i <= NUM_ROW ; ++i) {
        ctx.moveTo(LEFT_MARGIN, i * GRID_SIZE + TOP_MARGIN);
        ctx.lineTo(LEFT_MARGIN + (NUM_COL) * GRID_SIZE,
                    i * GRID_SIZE + TOP_MARGIN);
    }
    ctx.stroke();
}

function load(id, is_hvc) {
  g_is_hvc = is_hvc;
  canvas = document.getElementById('arena');
  draw();
  if (is_hvc === 0) {
    socket.once('res_steps', function(data) {
      for (var i = 0 ; i < data.steps.length ; ++i) {
        var tmp = data.steps[i].split(' ');
        var item = {'color': parseInt(tmp[0]),
          'x': parseInt(tmp[1]), 'y': parseInt(tmp[2])};
        if (item.x !== -1)
          steps.push(item);
      }
      // steps = data.steps;
    });
    socket.emit('req_steps', {match_id: id});
  } else {
    document.getElementById("info").innerHTML =
      "If you cannot move in your turn, just click anywhere of the chessboard to continue.";
    canvas.addEventListener('mousemove', onMouseMove);
    canvas.addEventListener('mousedown', onMouseDown);
    socket.emit('req_hvc', {submit_id: id});
    socket.once('game_over', function(data) {
      is_over = true;
      alert("Game over." + data.res_str.replace('_', ' ')
        + " " + data.reason.replace('_', ' '));
    });
  }
}

function draw() {
    drawBoard();
    var idx = step_idx;
    for (var i = 0 ; i < NUM_ROW ; ++i)
      for (var j = 0 ; j < NUM_COL ; ++j) {
        if (show_board[idx].bd[i][j] !== 0)
          drawChess(i, j, show_board[idx].bd[i][j]);
      }
}

function drawChess(x, y, color) {
    var ctx = canvas.getContext('2d');
    ctx.fillStyle = "rgb(0,0,0)";
    ctx.beginPath();
    ctx.arc(LEFT_MARGIN + y * GRID_SIZE + GRID_SIZE / 2,
            TOP_MARGIN + x * GRID_SIZE + GRID_SIZE / 2,
            GRID_SIZE/2-2, 0, Math.PI*2, true);
    if (color === 1) ctx.fill();
    else ctx.stroke();
}

function onMouseMove(evt) {
/*
    tmp = getMousePos(canvas, evt);
    document.getElementById("x_pos").innerHTML = tmp.x;
    document.getElementById("y_pos").innerHTML = tmp.y;
*/
}

function onMouseDown(evt) {
    if (g_is_hvc === 0) {
      return;
    }
    if (is_over) return;
    tmp = getMousePos(canvas, evt);
    chess_pos = {x: 0, y: 0};
    chess_pos.y = Math.floor((tmp.x - LEFT_MARGIN) / GRID_SIZE);
    chess_pos.x = Math.floor((tmp.y - TOP_MARGIN) / GRID_SIZE);
    if (chess_pos.x < 0 || chess_pos.x >= NUM_ROW ||
            chess_pos.y < 0 || chess_pos.y >= NUM_COL) {
        return;
    }
    // drawChess(chess_pos.x, chess_pos.y, 1);
    var tmp_move = {x: chess_pos.x, y: chess_pos.y, color: 1};
    var tmp_board = new Board();
    tmp_board.setBoard(show_board[step_idx]);
    show_board[step_idx].makeMove(tmp_move);
    draw();
    steps.push(tmp_move);
    socket.emit('put_chess', {x: chess_pos.x, y: chess_pos.y});
    socket.once('put_response', function(data) {
        if (data.is_over) {
            is_over = true;
            alert("over " + data.res_str + " " + data.reason);
        } else if (data.invalid_step) {
            steps.pop();
            show_board[step_idx].setBoard(tmp_board);
            draw();
        } else {
          if (data.x === -1) {
            alert("Computer cannot move. You continue.");
            return;
          }
          var tmp_move = {x: data.x, y: data.y, color: 2};
          show_board[step_idx].makeMove(tmp_move);
          draw();
          steps.push(tmp_move);
        }
    });
}

function nextStep() {
  if (steps.length === 0) {
    console.log('Steps are not loaded.');
    return;
  }
  ++step_idx;
  if (step_idx > steps.length) {
    step_idx = steps.length;
    return;
  }
  if (step_idx == show_board.length) {
    show_board.push(new Board());
    show_board[step_idx].setBoard(show_board[step_idx-1]);
    show_board[step_idx].makeMove(steps[step_idx-1]);
  }
  draw();
}

function prevStep() {
  if (steps.length === 0) {
    console.log('Steps are not loaded.');
    return;
  }
  if (--step_idx < 0) {
    step_idx = 0;
    return;
  }
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

