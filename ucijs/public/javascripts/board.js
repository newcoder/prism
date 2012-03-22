var Board, BoardView, COL_NUM, ROW_NUM, START_POS;

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
	this.offsetX = 3;
	this.offsetY = 3;
	this.view = null;
	this.box = null;
	this.isPlacingPiece = true;        // at the stage of placing pieces
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
	  if (p.pt === ch && p.r === r && p.c === c) {
        p.r = -1;
        p.c = -1;
		break;
      }
	}
  };

  Board.prototype.toCoordX = function(c) {
    var x;
    if (this.player1Side !== 0) c = COL_NUM - 1 - c;
    return x = this.offsetX + this.cellWidth * c;
  };

  Board.prototype.toCoordY = function(r) {
    var y;
    if (this.player1Side !== 0) r = ROW_NUM - 1 - r;
    return y = this.offsetY + this.cellHeight * r;
  };
  
  Board.prototype.toCol = function(x) {
    var c;
	x = (x - this.offsetX - this.cellWidth/2)/this.cellWidth;
	c = Math.round(x);
	if (this.player1Side !== 0) c = COL_NUM - 1 - c;
	return c;
  };
  
  Board.prototype.toRow = function(y) {
    var r;
	y = (y - this.offsetY - this.cellHeight/2)/this.cellHeight;
	r = Math.round(y);  
	if (this.player1Side !== 0) r = ROW_NUM - 1 - r;
	return r;
  };

  // move a piece in gaming
  Board.prototype.makeMove = function(move) {
    var pt, dest_c, dest_r, origin_c, origin_r, origin_pt;
    origin_c = move[0] - 'a';
    origin_r = ROW_NUM - 1 - move[1];
    origin_pt = this.board[origin_r][origin_c];
    dest_c = move[2] - 'a';
    dest_r = ROW_NUM - 1 - move[3];
	
	this.moveList.push(move);
    pt = this.board[dest_r][dest_c];
    if (pt !== 0) {
      this.removePiece(dest_r, dest_c);
    }
    this.removePiece(origin_r, origin_c);
    return this.addPiece(dest_r, dest_c, origin_pt);
  };
  
  // check whether a move is legal based on current position
  Board.prototype.isLegalMove = function(move) {
    
  };
  
  // check the place is legal to kind of piece
  Board.prototype.isLegalPlace = function(r, c, pt) {
    var dest_pt = this.board[r][c];
    if (dest_pt !== 0) {
	  //there is a piece at this place
      return false;
    }
	
	return true;
	// more,to be added
  };
  
  // place a piece on the board, origin_c, origin_r is null when move a piece from piece box
  Board.prototype.placePiece = function(dest_r, dest_c, pt, origin_r, origin_c) {
	if (origin_r !== -1 && origin_c !== -1)
	  this.removePiece(origin_r, origin_c);
    this.addPiece(dest_r, dest_c, pt);
  };

  return Board;

})();

BoardView = (function() {

  function BoardView($, el) {
    this.$ = $;
    this.el = el;
	this.lastSelected = null;
  }

  BoardView.prototype.init = function() {
 	var _this = this;
	var container = this.$(this.el[0]);   
	this.board = new Board;
	this.board.view = this;
	container.on('click', function(e) {
	  if (!_this.lastSelected)
	    return;
	
      var location = container.offset();
      var x = e.pageX - location.left;
      var y = e.pageY - location.top;
	  var r = _this.board.toRow(y);
	  var c = _this.board.toCol(x);
	  var pt = _this.lastSelected.data('piece').pt;
	  var origin_r = _this.lastSelected.data('piece').r;
	  var origin_c = _this.lastSelected.data('piece').c;  
	  _this.lastSelected.toggleClass('selected');
	  _this.lastSelected = null;
	  
	  if (_this.board.isPlacingPiece) {
	  // placing piece
	    if (_this.board.isLegalPlace(r, c, pt)) {
		  _this.board.placePiece(r, c, pt, origin_r, origin_c);
		  _this.updateView();
		}
	  } else {
	  // gaming
	  
	  }
	});
	this.updateView();
  };
  
  BoardView.prototype.updateView = function() {
    var _this = this;
	this.$(this.el[0]).empty();
	this.lastSelected = null;
    this.board.pieces.forEach(function(p) {
	  if (p.r !== -1 && p.c !== -1) // piece is on board
	    _this.addPiece(_this.$(_this.el[0]), p);
	});    
  };

  BoardView.prototype.createPiece = function(piece) {
    var $piece_div;
	var _this = this;
    $piece_div = this.$('<div class="piece"/>');
    $piece_div.addClass('piece_' + piece.pt);
    $piece_div.css('width', this.board.cellWidth);
    $piece_div.css('height', this.board.cellHeight);
    $piece_div.css('left', this.board.toCoordX(piece.c));
    $piece_div.css('top', this.board.toCoordY(piece.r));
	$piece_div.data('piece', piece);
    $piece_div.on('click', function(e){
	  if (_this.lastSelected)
	    _this.lastSelected.toggleClass('selected');
	  $piece_div.toggleClass('selected');
	  _this.lastSelected = $piece_div;
	  e.stopPropagation();
	});
    return piece.el = $piece_div;
  };

  BoardView.prototype.addPiece = function(container, piece) {
    return container.append(this.createPiece(piece));
  };

  return BoardView;

})();

PieceBox = (function() {

  function PieceBox($, el) {
    this.$ = $;
    this.el = el;
	this.lastSelected = null;
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
	    _this.addPiece(_this.$(_this.el[0]), p);
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
	return {x: positionTable[pt].r*this.board.cellWidth,
	  y: positionTable[pt].c*this.board.cellHeight
	}
  };

  PieceBox.prototype.createPiece = function(piece) {
    var $piece_div;
	var _this = this;
    $piece_div = this.$('<div class="piece"/>');
    $piece_div.addClass('piece_' + piece.pt);
    $piece_div.css('width', this.board.cellWidth);
    $piece_div.css('height', this.board.cellHeight);
    $piece_div.css('left', this.piecePosition(piece.pt).x);
    $piece_div.css('top', this.piecePosition(piece.pt).y);
    $piece_div.bind('click', function(e){
	  if (_this.lastSelected)
	    _this.lastSelected.toggleClass('selected');
	  $piece_div.toggleClass('selected');
	  _this.lastSelected = $piece_div;
	  e.stopPropagation();
	});
    return piece.el = $piece_div;
  };

  PieceBox.prototype.addPiece = function(container, piece) {
    return container.append(this.createPiece(piece));
  };

  return PieceBox;

})();

