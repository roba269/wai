var steps = [];
var is_over = false;
var canvas;
var step_idx = 0;
var socket = io.connect('http://localhost:3000');

var dir8 = [[-1,-1],[-1,0],[-1,1],[0,-1],[0,1],[1,-1],[1,0],[1,1]];

const LEFT_MARGIN = 25;
const TOP_MARGIN = 25;
const GRID_SIZE = 40;
const NUM_ROW = 10;
const NUM_COL = 9;

var loaded_img = [];
var inter_id;
var play_status = 0;
var move_status = 0;
var who_to_move;
var to_move_x = -1, to_move_y = -1;

ChessType = {
  B_SHUAI : 1,
  B_SHI : 2,
  B_XIANG : 3,
  B_JU : 4,
  B_MA : 5,
  B_PAO : 6,
  B_ZU : 7,
  R_SHUAI : 11,
  R_SHI : 12,
  R_XIANG : 13,
  R_JU : 14,
  R_MA : 15,
  R_PAO : 16,
  R_ZU : 17,
};

const chess_text = ['', 'Shuai', 'Shi', 'Xiang', 'Ju', 'Ma', 'Pao', 'Zu', '', '', '', 'Shuai', 'Shi', 'Xiang', 'Ju', 'Ma', 'Pao', 'Zu', '', '', ''];

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
  this.bd[0][0] = this.bd[0][8] = ChessType.B_JU;
  this.bd[0][1] = this.bd[0][7] = ChessType.B_MA;
  this.bd[0][2] = this.bd[0][6] = ChessType.B_XIANG;
  this.bd[0][3] = this.bd[0][5] = ChessType.B_SHI;
  this.bd[0][4] = ChessType.B_SHUAI;
  this.bd[2][1] = this.bd[2][7]  = ChessType.B_PAO;
  this.bd[3][0] = this.bd[3][2] = this.bd[3][4] =
    this.bd[3][6] = this.bd[3][8] = ChessType.B_ZU;
  for (var i = 0 ; i < 5 ; i++)
    for (var j = 0 ; j < NUM_COL ; j++)
      if (this.bd[i][j] !== 0)
        this.bd[NUM_ROW-1-i][j] = this.bd[i][j] + 10;
}

Board.prototype.setBoard = function(board_a) {
  for (var i = 0 ; i < NUM_ROW ; ++i)
    for (var j = 0 ; j < NUM_COL ; ++j)
      this.bd[i][j] = board_a.bd[i][j];
}

Board.prototype.makeMove = function(move) {
  // Note: we donnot check validation
  this.bd[move.x2][move.y2] = this.bd[move.x1][move.y1];
  this.bd[move.x1][move.y1] = 0;
}

Board.prototype.outputForDebug = function() {
  console.log('begin output chessboard');
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
    ctx.strokeStyle = "rgb(0,0,0)";
    var i;
    ctx.beginPath();
    for (i = 0 ; i < NUM_COL ; ++i) {
        ctx.moveTo(i * GRID_SIZE + LEFT_MARGIN, TOP_MARGIN);
        ctx.lineTo(i * GRID_SIZE + LEFT_MARGIN,
              4 * GRID_SIZE + TOP_MARGIN);
        if (i === 0 || i === NUM_COL - 1) {
          ctx.moveTo(i * GRID_SIZE + LEFT_MARGIN,
              4 * GRID_SIZE + TOP_MARGIN);
          ctx.lineTo(i * GRID_SIZE + LEFT_MARGIN,
              5 * GRID_SIZE + TOP_MARGIN);
        }
        ctx.moveTo(i * GRID_SIZE + LEFT_MARGIN,
              5 * GRID_SIZE + TOP_MARGIN);
        ctx.lineTo(i * GRID_SIZE + LEFT_MARGIN,
              9 * GRID_SIZE + TOP_MARGIN);
    }
    for (i = 0 ; i < NUM_ROW ; ++i) {
        ctx.moveTo(LEFT_MARGIN, i * GRID_SIZE + TOP_MARGIN);
        ctx.lineTo(LEFT_MARGIN + (NUM_COL-1) * GRID_SIZE,
                    i * GRID_SIZE + TOP_MARGIN);
    }
    ctx.moveTo(LEFT_MARGIN + 3 * GRID_SIZE, TOP_MARGIN);
    ctx.lineTo(LEFT_MARGIN + 5 * GRID_SIZE,
      TOP_MARGIN + 2 * GRID_SIZE);
    ctx.moveTo(LEFT_MARGIN + 5 * GRID_SIZE, TOP_MARGIN);
    ctx.lineTo(LEFT_MARGIN + 3 * GRID_SIZE,
      TOP_MARGIN + 2 * GRID_SIZE);
    ctx.moveTo(LEFT_MARGIN + 3 * GRID_SIZE,
      TOP_MARGIN + 7 * GRID_SIZE);
    ctx.lineTo(LEFT_MARGIN + 5 * GRID_SIZE,
      TOP_MARGIN + 9 * GRID_SIZE);
    ctx.moveTo(LEFT_MARGIN + 5 * GRID_SIZE,
      TOP_MARGIN + 7 * GRID_SIZE);
    ctx.lineTo(LEFT_MARGIN + 3 * GRID_SIZE,
      TOP_MARGIN + 9 * GRID_SIZE);
    ctx.stroke();
}

function load(id, is_hvc) {
  g_is_hvc = is_hvc;
  canvas = document.getElementById('arena');
  var counter = 7 * 2;
  for (var flg = 0 ; flg < 2 ; ++flg)
    for (var j = 1 ; j <= 7 ; ++j) {
      var idx = flg * 10 + j;
      loaded_img[idx] = new Image();
      loaded_img[idx].src = "/images/xiangqi/xiangqi" + idx + ".png";
      loaded_img[idx].onload = function() {
        if (--counter === 0) {
          draw();
        }
      }
    }
  if (is_hvc === 0) {
    socket.once('res_steps', function(data) {
      for (var i = 0 ; i < data.steps.length ; ++i) {
        var tmp = data.steps[i].split(' ');
        var item = {'color': parseInt(tmp[0]),
          'x1': parseInt(tmp[1]), 'y1': parseInt(tmp[2]),
          'x2': parseInt(tmp[3]), 'y2': parseInt(tmp[4])};
        steps.push(item);
      }
      // steps = data.steps;
    });
    socket.emit('req_steps', {match_id: id});
  } else {
    document.getElementById("info").innerHTML = "To move a chess, click it, and then click its target position."
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
    var idx = step_idx;
    for (var i = 0 ; i < NUM_ROW ; ++i)
      for (var j = 0 ; j < NUM_COL ; ++j) {
        if (show_board[idx].bd[i][j] !== 0)
          drawChess(i, j, show_board[idx].bd[i][j]);
      }
    if (move_status === 1) {
      drawRect(to_move_x, to_move_y, 'rgb(0,0,200)');
    }
}

function drawRect(x, y, col) {
  var ctx = canvas.getContext('2d');
  ctx.strokeStyle = 'rgb(0,0,200)';
  ctx.strokeRect(LEFT_MARGIN + y * GRID_SIZE - GRID_SIZE / 2,
    TOP_MARGIN + x * GRID_SIZE - GRID_SIZE / 2,
    GRID_SIZE, GRID_SIZE);
}

function drawChess(x, y, type) {
    var ctx = canvas.getContext('2d');
    ctx.drawImage(loaded_img[type],
        LEFT_MARGIN + y * GRID_SIZE - GRID_SIZE / 2,
        TOP_MARGIN + x * GRID_SIZE - GRID_SIZE / 2,
        GRID_SIZE, GRID_SIZE);
}

function onMouseMove(evt) {
}
function onMouseDown(evt) {
    if (g_is_hvc === 0) return;
    if (is_over) return;
    if (who_to_move !== 1) return;
    if (evt.button == 2 || evt.button == 3) {
      move_status = 0;
      return;
    }
    var tmp = getMousePos(canvas, evt);
    var tmp_x = Math.floor((tmp.y - LEFT_MARGIN) / GRID_SIZE + 0.5);
    var tmp_y = Math.floor((tmp.x - TOP_MARGIN) / GRID_SIZE + 0.5);
    if (tmp_x < 0 || tmp_x >= NUM_ROW || tmp_y < 0 || tmp_y >= NUM_COL) {
        return;
    }
    if (move_status === 0) {
      to_move_x = tmp_x;
      to_move_y = tmp_y;
      move_status = 1;
      draw();
      return;
    }
    move_status = 0;
    if (to_move_x === tmp_x && to_move_y == tmp_y) {
      draw();
      return;
    }
    var tmp_move = {x1: to_move_x, y1: to_move_y, x2: tmp_x, y2: tmp_y};
    var tmp_board = new Board();
    tmp_board.setBoard(show_board[step_idx]);
    show_board[step_idx].makeMove(tmp_move);
    draw();
    steps.push(tmp_move);
    socket.emit('put_chess', tmp_move);
    who_to_move = 2;
    socket.once('put_response', function(data) {
        who_to_move = 1;
        if (data.is_over) {
            is_over = true;
            alert("over winner = " + data.winner);
        } else if (data.invalid_step) {
            steps.pop();
            show_board[step_idx].setBoard(tmp_board);
            draw();
        } else {
            show_board[step_idx].makeMove(data);
            draw();
            steps.push(data);
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

