var steps = [];
var is_over = false;
var canvas;
var step_idx = 0;
var socket = io.connect('http://localhost:3000');

var dir8 = [[-1,-1],[-1,0],[-1,1],[0,-1],[0,1],[1,-1],[1,0],[1,1]];

const LEFT_MARGIN = 25;
const TOP_MARGIN = 25;
const GRID_SIZE = 24;
const NUM_ROW = 23;
const NUM_COL = 23;

var total_hp = [3, 9, 2];
var char_list = ['S', 'B', 'b', 'P', 'G', 'H', 'L'];
var pic_name = ['stone', 'brick', 'broken_brick', 'empty', 'mine', 'mine_red', 'mine_blue'];
var loaded_img = {};
var tank_img;
var inter_id;
var play_status = 0;

function in_board(x, y) {
  return x >= 0 && x < NUM_ROW && y >= 0 && y < NUM_COL;
}

function Board() {
/*
  for (var i = 0 ; i < NUM_ROW ; ++i) {
    this.bd[i] = new Array();
    for (var j = 0 ; j < NUM_COL ; ++j) {
      this.bd[i][j] = 0;
    }
  }
*/
  this.tank = new Array();
  this.cmd = new Array();
  for (var i = 0 ; i < 2 ; ++i) {
    this.tank[i] = new Array();
    this.cmd[i] = new Array();
    for (var j = 0 ; j < 5 ; j++) {
      this.tank[i][j] = {
        x: 0, y: 0, type: 0,
        hp: 0, revive: 0, noharm: 0,
        dir: 0,
      }
      this.cmd[i][j] = {
        type: 0, x: 0, y: 0,
      }
    }
  }
}

Board.prototype.setBoard = function(board_a) {
  for (var i = 0 ; i < NUM_ROW ; ++i)
    for (var j = 0 ; j < NUM_COL ; ++j)
      this.bd[i][j] = board_a.bd[i][j];
}

Board.prototype.setMap = function(map_buf) {
  var info = map_buf.split(' ')
  this.rnd = parseInt(info[0])
  this.mp = info[1];
  this.red_score = parseInt(info[2]);
  this.blue_score = parseInt(info[3]);
  this.red_kill = parseInt(info[4]);
  this.blue_kill = parseInt(info[5]);
  var idx = 6;
  for (var i = 0 ; i < 2 ; ++i)
    for (var j = 0 ; j < 5 ; ++j) {
      this.tank[i][j].x = parseInt(info[idx++]);
      this.tank[i][j].y = parseInt(info[idx++]);
      this.tank[i][j].type = parseInt(info[idx++]);
      this.tank[i][j].hp = parseInt(info[idx++]);
      this.tank[i][j].revive = parseInt(info[idx++]);
      this.tank[i][j].noharm = parseInt(info[idx++]);
      this.cmd[i][j].type = parseInt(info[idx++]);
      this.cmd[i][j].x = parseInt(info[idx++]);
      this.cmd[i][j].y = parseInt(info[idx++]);
    }
}
/*
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
*/
var show_board = [];

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
  canvas.addEventListener('mousemove', onMouseMove);
  var counter = char_list.length + 2 * 3 * 5;
  var flg = 0;
  for (var idx = 0 ; idx < char_list.length ; ++idx) {
    var ch = char_list[idx];
    loaded_img[ch] = new Image();
    loaded_img[ch].src = "/images/tankcraft/"  + pic_name[idx] + ".png";
    loaded_img[ch].onload = function() {
      if (--counter === 0) {
        canvas = document.getElementById('arena');
        draw();
      }
    }
  }
  tank_img = new Array();
  for (var tank_col = 0 ; tank_col < 2 ; ++tank_col) {
    tank_img[tank_col] = new Array();
    for (var tank_type = 0 ; tank_type < 3 ; ++tank_type) {
      tank_img[tank_col][tank_type] = new Array();
      for (var tank_dir = 0 ; tank_dir < 4 ; ++tank_dir) {
        tank_img[tank_col][tank_type][tank_dir] = new Image();
        tank_img[tank_col][tank_type][tank_dir].src =
          "/images/tankcraft/tank" + tank_col + tank_type + tank_dir + ".png";
        tank_img[tank_col][tank_type][tank_dir].onload = function() {
          if (--counter === 0) {
            canvas = document.getElementById('arena');
            draw();
          }
        }
      }
    }
  }
  socket.once('res_steps', function(data) {
    for (var i = 0 ; i < data.steps.length ; ++i) {
      // var tmp = data.steps[i].split(' ');
      // var item = {'color': parseInt(tmp[0]),
      //   'x1': parseInt(tmp[1]), 'y1': parseInt(tmp[2]),
      //   'x2': parseInt(tmp[3]), 'y2': parseInt(tmp[4])};
      steps.push(data.steps[i]);
    }
    // steps = data.steps;
  });
  socket.emit('req_steps', {match_id: match_id});
}

var drawCanvasImage = function(ctx,grid,i,j,x,y) {
  return function() {
    ctx.drawImage(grid[i][j],
      TOP_MARGIN + y * GRID_SIZE,
      LEFT_MARGIN + x * GRID_SIZE);
  };
}

function drawBa(ctx, x, y) {
  var oldWidth = ctx.lineWidth;
  ctx.lineWidth = 3;
  ctx.strokeStyle = "rgb(200,0,200)"
  cx = TOP_MARGIN + y * GRID_SIZE + GRID_SIZE / 2;
  cy = LEFT_MARGIN + x * GRID_SIZE + GRID_SIZE / 2;
  ctx.beginPath();
  ctx.arc(cx, cy, GRID_SIZE / 3, 0, 2*Math.PI, false);
  ctx.moveTo(cx - GRID_SIZE / 3, cy);
  ctx.lineTo(cx + GRID_SIZE / 3, cy);
  ctx.moveTo(cx, cy - GRID_SIZE / 3);
  ctx.lineTo(cx, cy + GRID_SIZE / 3);
  ctx.stroke();
  ctx.lineWidth = oldWidth;
}

function draw() {
  drawBoard();
  if (steps.length === 0 || step_idx === 0) return;
  var i, j;
  var mine = [];
  var taken = new Array();
  for (i = 0 ; i < NUM_ROW ; ++i)
    taken[i] = new Array();
  var idx = step_idx - 1;
  var ctx = canvas.getContext('2d');
  // var grid = new Array();
  for (i = 0 ; i < NUM_ROW ; ++i) {
    // grid[i] = new Array();
    for (j = 0 ; j < NUM_COL ; ++j) {
      if (taken[i][j] === 1) continue;
      // grid[i][j] = new Image();
      var ch = show_board[idx].mp[i*NUM_COL+j];
      if (ch >= '0' && ch <= '9') ch = 'P';
      /*
      grid[i][j].onload = drawCanvasImage(ctx,grid,i,j,i,j);
      grid[i][j].src = "/images/tankcraft/"  + CharToName[ch] + ".png";
      */
      var x = TOP_MARGIN + j * GRID_SIZE;
      var y = LEFT_MARGIN + i * GRID_SIZE;
      ctx.drawImage(loaded_img[ch], x, y);
    }
  }
  /*
  var tank_show = new Array();
  for (i = 0 ; i < 2 ; ++i) {
    tank_show[i] = new Array();
    for (j = 0 ; j < 5 ; ++j) {
      tank_show[i][j] = new Image();
      var x = show_board[idx].tank[i][j].x;
      var y = show_board[idx].tank[i][j].y;
      if (x < 0) continue;
      tank_show[i][j].onload = drawCanvasImage(ctx,tank_show,i,j,x,y);
      tank_show[i][j].src = "/images/tankcraft/tank" +
        i + show_board[idx].tank[i][j].type + ".png";
      taken[x][y] = 1;
    }
  }
  */
  for (i = 0 ; i < 2 ; ++i) {
    for (j = 0 ; j < 5 ; ++j) {
      var x = show_board[idx].tank[i][j].x;
      var y = show_board[idx].tank[i][j].y;
      var type = show_board[idx].tank[i][j].type;
      if (x < 0) continue;
      var dx = TOP_MARGIN + y * GRID_SIZE;
      var dy = LEFT_MARGIN + x * GRID_SIZE;
      var dir = show_board[idx].cmd[i][j].type;
      if (dir > 3) dir = 3;
      ctx.drawImage(tank_img[i][type][dir], dx, dy);
    }
  }
  for (i = 0 ; i < 2 ; ++i) {
    for (j = 0 ; j < 5 ; ++j) {
      var x = show_board[idx].tank[i][j].x;
      var y = show_board[idx].tank[i][j].y;
      if (x < 0) continue;
      if (show_board[idx].cmd[i][j].type === 5 /* FIRE */) {
        drawBa(ctx, show_board[idx].cmd[i][j].x,
          show_board[idx].cmd[i][j].y);
      }
    }
  }

  ctx.fillStyle = "rgb(0,255,0)";
  document.getElementById("red_score").innerHTML =
      " Red Score: " + show_board[idx].red_score;
  document.getElementById("blue_score").innerHTML =
      "Blue Score: " + show_board[idx].blue_score;
  document.getElementById("red_kill").innerHTML =
      " Red Kill: " + show_board[idx].red_kill;
  document.getElementById("blue_kill").innerHTML =
      "Blue Kill: " + show_board[idx].blue_kill;
  /*
  ctx.fillText("Red Score: " + this.red_score, 570, 10);
  ctx.fillText("Blue Score: " + this.blue_score, 570, 30);
  ctx.fillText("Red Kill: " + this.red_kill, 570, 50);
  ctx.fillText("Blue Kill: " + this.blue_kill, 570, 70);
  */
}

function nextStep() {
  if (steps.length === 0) {
    console.log('Steps are not loaded.');
    return;
  }
  if (step_idx >= steps.length) {
    step_idx = steps.length;
    return;
  }
  if (step_idx === show_board.length) {
    show_board.push(new Board());
  }
  show_board[step_idx].setMap(steps[step_idx]);
  ++step_idx;
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

function onMouseMove(evt) {
  draw();
  if (step_idx === 0) return;
  var idx = step_idx - 1;
  var ctx = canvas.getContext('2d');
  var mouse_pos = getMousePos(canvas, evt);
  var pos_x = Math.floor((mouse_pos.y - LEFT_MARGIN) / GRID_SIZE);
  var pos_y = Math.floor((mouse_pos.x - TOP_MARGIN) / GRID_SIZE);
  var p = 0;
  var mine = [];
  for (var x = 0 ; x < NUM_ROW ; ++x)
    for (var y = 0 ; y < NUM_COL ; ++y) {
      var ch = show_board[idx].mp[p];
      if (ch === 'G' || ch === 'H' || ch === 'L') {
        mine.push({'x': x, 'y': y});
      }
      ++p;
    }
  for (var i = 0 ; i < 2 ; ++i)
    for (var j = 0 ; j < 5 ; ++j)
      if (show_board[idx].tank[i][j].x === pos_x &&
          show_board[idx].tank[i][j].y === pos_y)
      {
        // draw tank id
        ctx.fillStyle = "rgb(255,255,255)";
        ctx.textBaseline = "top";
        ctx.textAlign = "start";
        ctx.fillText(''+(i*5+j), 
          TOP_MARGIN + pos_y * GRID_SIZE + GRID_SIZE / 2,
          LEFT_MARGIN + pos_x * GRID_SIZE + GRID_SIZE / 2);

        ctx.fillStyle = "rgb(0,255,0)";
        ctx.textBaseline = "bottom";
        ctx.textAlign = "start";
        ctx.fillText(show_board[idx].tank[i][j].hp+'/'+
          total_hp[show_board[idx].tank[i][j].type],
          TOP_MARGIN + pos_y * GRID_SIZE + GRID_SIZE / 2,
          LEFT_MARGIN + pos_x * GRID_SIZE + GRID_SIZE / 2);
        var vis = 0;
        for (var mine_idx = 0 ; mine_idx < mine.length ; ++mine_idx) {
          if (Math.abs(mine[mine_idx].x - pos_x) +
              Math.abs(mine[mine_idx].y - pos_y) <= 2) {
                vis = 1;
                break;
              }
        }
        if (vis === 0) {
          for (var tj = 0 ; tj < 5 ; ++tj) {
            if (Math.abs(show_board[idx].tank[1-i][tj].x - pos_x) +
              Math.abs(show_board[idx].tank[1-i][tj].y - pos_y) <= 4) {
                vis = 1;
                break;
              }
          }
        }
        if (vis === 0) {
          ctx.fillStyle = "rgb(0,255,0)";
          ctx.textBaseline = "bottom";
          ctx.textAlign = "end";
          ctx.fillText("@", TOP_MARGIN + pos_y * GRID_SIZE + GRID_SIZE/2,
            LEFT_MARGIN + pos_x * GRID_SIZE + GRID_SIZE/2);
        }
        // show cmd
        // console.log('cmd type', show_board[idx].cmd[i][j].type);
      }
}
