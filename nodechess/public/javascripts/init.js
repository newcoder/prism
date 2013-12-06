seajs.config({
  alias: {
    'jquery': 'jquery/1.7.1/jquery',
    'socket.io': 'socket.io/0.9.10/socket.io.min'
  },
  charset: 'utf-8',
  timeout: 20000,
  debug: 0
});

define(function (require) {
  var $ = require('jquery'),
    io = require('socket.io'),
    BoardView = require('./boardview').BoardView,
    boardView = new BoardView($, $("#board")[0], $("#box")[0]);
  
  var socket = io.connect('http://localhost:3000');
  var thinkside = 1;

  boardView.moveDone(function () {
    var fen;
    if (thinkside == boardView.board.sideToMove) {
      fen = boardView.board.toFen();
      socket.emit('think', {'pos': fen}); 
    }
  });
  
  boardView.init();
  boardView.mode = 1;

  socket.on('result', function (data) {
    console.log(data);
    boardView.move(data.move);
  });

  $("#changefen").click(function () {
    var input = $("#feninput").val();
    boardView.board.fromFen(input);
    boardView.mode = 1;
    thinkside = 1;
    boardView.updateView();
  });

  $("#edit").click(function () {
    var input = $("#feninput").val();
    if (input.length > 10) {
      boardView.board.fromFen(input);
    } else {
      boardView.board.startPos();      
    }
    boardView.mode = 0;
    boardView.updateView();
  });

  $("#go").click(function () {
    var fen = boardView.board.toFen();
    boardView.mode = 1;
    thinkside = boardView.board.sideToMove;
    socket.emit('think', {'pos': fen});
  });

});
