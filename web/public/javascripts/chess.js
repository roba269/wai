var steps = [];
var is_over = false;
var canvas;
var step_idx = 0;
var socket = io.connect('http://localhost:3000');

var dir8 = [[-1,-1],[-1,0],[-1,1],[0,-1],[0,1],[1,-1],[1,0],[1,1]];

const LEFT_MARGIN = 25;
const TOP_MARGIN = 25;
const GRID_SIZE = 40;
const NUM_ROW = 8;
const NUM_COL = 8;

var loaded_img = [];
var inter_id;
var who_to_move;
var play_status = 0;
var move_status = 0;
var to_move_x = -1, to_move_y = -1;

ChessType = {
  B_KING : 1,
  B_QUEEN : 2,
  B_ROOK : 3,
  B_BISHOP: 4,
  B_KNIGHT: 5,
  B_PAWN: 6,
  W_KING : 11,
  W_QUEEN : 12,
  W_ROOK : 13,
  W_BISHOP: 14,
  W_KNIGHT: 15,
  W_PAWN: 16,
};

var pro_type_2_chess_type = [[],
  [ChessType.W_QUEEN, ChessType.W_ROOK,
    ChessType.W_BISHOP, ChessType.W_KNIGHT],
  [ChessType.B_QUEEN, ChessType.B_ROOK,
    ChessType.B_BISHOP, ChessType.B_KNIGHT],
];

const chess_text = ['', 'K', 'Q', 'R', 'B', 'Kn', 'P', '', '', '', '', 'k', 'q', 'r', 'b', 'kn', 'p', '', '', '', ''];

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
  this.bd[0][0] = this.bd[0][7] = ChessType.B_ROOK;
  this.bd[0][1] = this.bd[0][6] = ChessType.B_KNIGHT;
  this.bd[0][2] = this.bd[0][5] = ChessType.B_BISHOP;
  this.bd[0][3] = ChessType.B_QUEEN;
  this.bd[0][4] = ChessType.B_KING;
  for (var i = 0 ; i < NUM_COL ; ++i)
    this.bd[1][i] = ChessType.B_PAWN;
  for (var i = 6 ; i < 8 ; ++i)
    for (var j = 0 ; j < NUM_COL ; ++j)
      this.bd[i][j] = this.bd[7-i][j] + 10;
}

Board.prototype.setBoard = function(board_a) {
  for (var i = 0 ; i < NUM_ROW ; ++i)
    for (var j = 0 ; j < NUM_COL ; ++j)
      this.bd[i][j] = board_a.bd[i][j];
}

Board.prototype.makeMove = function(move) {
  // Note: we do not check validation
  var x1 = move.x1;
  var y1 = move.y1;
  var x2 = move.x2;
  var y2 = move.y2;
  var cur = move.color;
  if ((this.bd[x1][y1] == ChessType.W_KING ||
      this.bd[x1][y1] == ChessType.B_KING) && Math.abs(y1-y2) == 2)
  {
    // castling
    if (y2 < y1) {
      this.bd[x1][3] = (cur == 1 ? ChessType.W_ROOK : ChessType.B_ROOK);
      this.bd[x1][0] = 0;
    } else {
      this.bd[x1][5] = (cur == 1 ? ChessType.W_ROOK : ChessType.B_ROOK);
      this.bd[x1][7] = 0;
    }
  }
  if ((this.bd[x1][y1] == ChessType.W_PAWN ||
    this.bd[x1][y1] == ChessType.B_PAWN) && y1 != y2) {
    // capture in pass pawn
    if (this.bd[x2][y2] == 0) this.bd[x1][y2] = 0;
  }
  this.bd[move.x2][move.y2] = this.bd[move.x1][move.y1];
  this.bd[move.x1][move.y1] = 0;
  if (move.pro_type !== undefined) {
    this.bd[move.x2][move.y2] = pro_type_2_chess_type[move.color][move.pro_type];
  }
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
    for (var i = 0 ; i < NUM_ROW ; ++i)
      for (var j = 0 ; j < NUM_COL ; ++j) {
        if ((i + j) % 2 === 0) {
          ctx.fillStyle = 'rgb(255,206,158)';
        } else {
          ctx.fillStyle = 'rgb(209,139,71)';
        }
        ctx.fillRect(LEFT_MARGIN + j * GRID_SIZE,
          TOP_MARGIN + i * GRID_SIZE,
          GRID_SIZE, GRID_SIZE);
      }
}

function load(id, is_hvc) {
  g_is_hvc = is_hvc;
  canvas = document.getElementById('arena');
  var counter = 6 * 2;
  for (var flg = 0 ; flg < 2 ; ++flg)
    for (var j = 1 ; j <= 6 ; ++j) {
      var idx = flg * 10 + j;
      loaded_img[idx] = new Image();
      loaded_img[idx].src = "/images/chess/chess" + idx + ".svg";
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
        if (tmp.length > 5) item.pro_type = parseInt(tmp[5]);
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
    var ctx = canvas.getContext('2d');
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
  ctx.lineWidth = 3;
  ctx.strokeRect(LEFT_MARGIN + y * GRID_SIZE,
    TOP_MARGIN + x * GRID_SIZE,
    GRID_SIZE, GRID_SIZE);
  ctx.lineWidth = 1;
}

function drawChess(x, y, type) {
    var ctx = canvas.getContext('2d');
    ctx.drawImage(loaded_img[type],
        LEFT_MARGIN + y * GRID_SIZE,
        TOP_MARGIN + x * GRID_SIZE,
        GRID_SIZE, GRID_SIZE);
}

function onMouseMove(evt) {
/*
    tmp = getMousePos(canvas, evt);
    document.getElementById("x_pos").innerHTML = tmp.x;
    document.getElementById("y_pos").innerHTML = tmp.y;
*/
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
    var tmp_x = Math.floor((tmp.y - LEFT_MARGIN) / GRID_SIZE);
    var tmp_y = Math.floor((tmp.x - TOP_MARGIN) / GRID_SIZE);
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
    var tmp_move = {x1: to_move_x, y1: to_move_y, x2: tmp_x, y2: tmp_y, color: 1};
    var tmp_board = new Board();
    tmp_board.setBoard(show_board[step_idx]);
    if (show_board[step_idx].bd[tmp_move.x1][tmp_move.y1] === ChessType.W_PAWN) {
      if (tmp_move.x2 === 0) {
        var pro_type = parseInt(prompt("Which Type do you want to promote to? (0: Queen, 1: Rook, 2: Bishop, 3:Knight", "0"));
        if (pro_type < 0 || pro_type > 3) pro_type = 0;
        tmp_move.pro_type = pro_type;
      }
    }
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

