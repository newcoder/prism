var Board, BoardView, COL_NUM, ROW_NUM, START_POS,
  __bind = function(fn, me){ return function(){ return fn.apply(me, arguments); }; };

START_POS = 'rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5C1/9/RNBAKABNR w - - 0 1';
ALL_PIECES = 'rnbakabnrccpppppPPPPPCCRNBAKABNR';

ROW_NUM = 10;

COL_NUM = 9;

Board = (function() {

  function Board(option, initFen) {
    this.option = option;
    this.initFen = initFen;
    this.board = [[], [], [], [], [], [], [], [], [], []];
    this.pieces = [];
    this.moveList = [];
    this.sideToMove = 0;
    this.player1Side = 0;
    this.cellWidth = 55;
    this.cellHeight = 55;
	this.view = null;
	this.box = null;
	
    this.init();
  }
  
  Board.prototype.initPieces = function() {
    var _this = this;
    var pieceArr = ALL_PIECES.split('');
	this.pieces = [];
    pieceArr.forEach(function(p){
	  _this.pieces.push({pt: p, r: -1, c: -1});
	});	
  };
  
  Board.prototype.init = function() {
    if (!this.initFen) this.initFen = START_POS;
    return this.fen(this.initFen);
  };

  Board.prototype.fen = function(fen) {
    var arr, fenstr, i, j, row, rows, _len;
    // init board and the piece array
	this.initPieces();    
    for (i = 0; i <= 9; i++) {
      for (j = 0; j <= 8; j++) {
        this.board[i][j] = 0;
      }
    }
	
	// parse the fen string
    arr = fen.split(' ');
    fenstr = arr[0];
    this.side = arr[1] !== 'b';
    rows = fenstr.split('/');
    for (i = 0, _len = rows.length; i < _len; i++) {
      row = rows[i];
      this.row2Board(i, row);
    }
	
	// update views
	if (this.view)
	  this.view.updateView();
	if (this.box)
	  this.box.updateView();
  };

  Board.prototype.row2Board = function(r, row) {
    var c, ch, i, _len;
    c = 0;
    for (i = 0, _len = row.length; i < _len; i++) {
      ch = row[i];
      if (('0' <= ch && ch <= '9')) {
        c = c + (ch - '0');
      } else {
        this.addPiece(r, c, ch);
        c = c + 1;
      }
    }
  };

  // add a piece to chess board
  Board.prototype.addPiece = function(r, c, ch) {
    this.board[r][c] = ch;
	var i, p;
	for (i = 0; i < this.pieces.length; i++) {
      p = this.pieces[i];
	  if (p.pt === ch && p.r === -1 && p.c === -1) {
        p.r = r;
        p.c = c;
		break;
      }
	}
  };

  // remove a piece from chess board
  Board.prototype.removePiece = function(r, c) {
    var ch;
    ch = this.board[r][c];
    this.board[r][c] = 0;
	var i, p;
	for (i = 0; i < this.pieces.length; i++) {
      p = this.pieces[i];
	  if (p.pt === ch && p.r !== -1 && p.c !== -1) {
        p.r = -1;
        p.c = -1;
		break;
      }
	}
  };

  Board.prototype.toCoordX = function(c) {
    var offsetX, x;
    offsetX = 3;
    if (this.player1Side !== 0) c = COL_NUM - 1 - c;
    return x = offsetX + this.cellWidth * c;
  };

  Board.prototype.toCoordY = function(r) {
    var offsetY, y;
    offsetY = 3;
    if (this.player1Side !== 0) r = ROW_NUM - 1 - r;
    return y = offsetY + this.cellWidth * r;
  };

  Board.prototype.movePiece = function(move) {
    var ch, dest_c, dest_r, origin_c, origin_r;
    origin_c = move[0] - 'a';
    origin_r = ROW_NUM - 1 - move[1];
    dest_c = move[2] - 'a';
    dest_r = ROW_NUM - 1 - move[3];
    this.moveList.push(move);
    ch = this.board[dest_r][dest_c];
    if (ch !== 0) {
      this.removePiece(dest_r, dest_c);
    }
    this.removePiece(origin_r, origin_c);
    ch = this.board[origin_r][origin_c];
    return this.addPiece(dest_r, dest_c, ch);
  };
  
  // check whether a move is legal based on current position
  Board.prototype.isLegal = function(move) {
    
  }

  return Board;

})();

BoardView = (function() {

  function BoardView($, el) {
    this.$ = $;
    this.el = el;
  }

  BoardView.prototype.init = function() {
    this.board = new Board;
	this.board.view = this;
	this.updateView();
  };
  
  BoardView.prototype.updateView = function() {
    var _this = this;
	this.$(this.el[0]).empty();
    this.board.pieces.forEach(function(p) {
	  if (p.r !== -1 && p.c !== -1) // piece is on board
	    _this.placePiece(_this.$(_this.el[0]), p);
	});    
  };

  BoardView.prototype.createPiece = function(piece) {
    var $piece_div;
    $piece_div = this.$('<div class="piece"/>').draggable();
    $piece_div.addClass('piece_' + piece.pt);
    $piece_div.css('width', this.board.cellWidth);
    $piece_div.css('height', this.board.cellHeight);
    $piece_div.css('left', this.board.toCoordX(piece.c));
    $piece_div.css('top', this.board.toCoordY(piece.r));
    $piece_div.bind('dragstart', __bind(onDragStart, this));
    $piece_div.bind('drag', __bind(onDrag, this));
    $piece_div.bind('dragstop', __bind(onDragStop, this));
    return piece.el = $piece_div;
  };

  BoardView.prototype.placePiece = function(container, piece) {
    return container.append(this.createPiece(piece));
  };

  BoardView.prototype.disableDrag = function() {
    var pieces = this.board.pieces;
    pieces.forEach(function(p) {  
	  if (p.el)
	    p.el.draggable("option", "disabled", true);
	}); 
  };

  var onDragStart = function(event, ui) {};

  var onDrag = function(event, ui) {};

  var onDragStop = function(event, ui) {};

  return BoardView;

})();

PieceBox = (function() {

  function PieceBox($, el) {
    this.$ = $;
    this.el = el;
  }

  PieceBox.prototype.init = function(board) {
	this.board = board;
	this.board.box = this;
	this.updateView();
  };
  
  PieceBox.prototype.updateView = function() {
    var _this = this;
	this.$(this.el[0]).empty();
    this.board.pieces.forEach(function(p) {
	  if (p.r === -1 && p.c === -1) // piece is not on board, should be appear in the box
	    _this.placePiece(_this.$(_this.el[0]), p);
	});    
  };
  
  PieceBox.prototype.piecePosition = function(pt) {
    var positionTable = {
	'a': {r: 0, c: 0},
    'c': {r: 0, c: 1},
    'r': {r: 0, c: 2},
    'b': {r: 0, c: 3},
    'n': {r: 0, c: 4},
    'k': {r: 0, c: 5},
    'p': {r: 0, c: 6},
 	'A': {r: 1, c: 0},
    'C': {r: 1, c: 1},
    'R': {r: 1, c: 2},
    'B': {r: 1, c: 3},
    'N': {r: 1, c: 4},
    'K': {r: 1, c: 5},
    'P': {r: 1, c: 6}
	};
	return {x: positionTable[pt].c*this.board.cellWidth,
	  y: positionTable[pt].r*this.board.cellHeight
	}
  };

  PieceBox.prototype.createPiece = function(piece) {
    var $piece_div;
    $piece_div = this.$('<div class="piece"/>').draggable();
    $piece_div.addClass('piece_' + piece.pt);
    $piece_div.css('width', this.board.cellWidth);
    $piece_div.css('height', this.board.cellHeight);
    $piece_div.css('left', this.piecePosition(piece.pt).x);
    $piece_div.css('top', this.piecePosition(piece.pt).y);
    $piece_div.bind('dragstart', __bind(onDragStart, this));
    $piece_div.bind('drag', __bind(onDrag, this));
    $piece_div.bind('dragstop', __bind(onDragStop, this));
    return piece.el = $piece_div;
  };

  PieceBox.prototype.placePiece = function(container, piece) {
    return container.append(this.createPiece(piece));
  };

  PieceBox.prototype.disableDrag = function() {
    var pieces = this.board.pieces;
    pieces.forEach(function(p) {  
	  if (p.el)
	    p.el.draggable("option", "disabled", true);
	}); 
  };

  var onDragStart = function(event, ui) {};

  var onDrag = function(event, ui) {};

  var onDragStop = function(event, ui) {};

  return PieceBox;

})();

