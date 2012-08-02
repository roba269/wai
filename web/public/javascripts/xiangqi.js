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
  // this.bd[3][3] = this.bd[4][4] = 1;
  // this.bd[3][4] = this.bd[4][3] = 2;
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
  /*
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
  */
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

function load(match_id) {
  canvas = document.getElementById('arena');
  draw();
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
  socket.emit('req_steps', {match_id: match_id});
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

function drawChess(x, y, type) {
    var ctx = canvas.getContext('2d');
    if (type < 10) ctx.fillStyle = "rgb(0,0,0)";
    else ctx.fillStyle = "rgb(255,0,0)";
    ctx.beginPath();
    ctx.arc(LEFT_MARGIN + y * GRID_SIZE + GRID_SIZE / 2,
            TOP_MARGIN + x * GRID_SIZE + GRID_SIZE / 2,
            GRID_SIZE/2-2, 0, Math.PI*2, true);
    ctx.fill();
    // ctx.textBaseline = 'middle';
    ctx.fillStyle = "rgb(255,255,255)";
    ctx.fillText(chess_text[type],
        LEFT_MARGIN + y * GRID_SIZE + GRID_SIZE / 2,
        TOP_MARGIN + x * GRID_SIZE + GRID_SIZE / 2);
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


