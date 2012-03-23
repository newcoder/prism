var Board, BoardView, COL_NUM, ROW_NUM, START_POS;

START_POS = 'rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5C1/9/RNBAKABNR w - - 0 1';
ALL_PIECES = 'rnbakabnrccpppppPPPPPCCRNBAKABNR';
ROW_NUM = 10;
COL_NUM = 9;

// Production steps of ECMA-262, Edition 5, 15.4.4.18
// Reference: http://es5.github.com/#x15.4.4.18
if ( !Array.prototype.forEach ) {
  Array.prototype.forEach = function( callback, thisArg ) {
    var T, k;
    if ( this == null ) {
      throw new TypeError( " this is null or not defined" );
    }
    var O = Object(this);
    var len = O.length >>> 0; // Hack to convert O.length to a UInt32
    if ( {}.toString.call(callback) != "[object Function]" ) {
      throw new TypeError( callback + " is not a function" );
    }
    if ( thisArg ) {
      T = thisArg;
    }
    k = 0;
    while( k < len ) {
      var kValue;
      if ( k in O ) {
        kValue = O[ k ];
        callback.call( T, kValue, k, O );
      }
      k++;
    }
  };
}

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
  
  Board.prototype.clear = function() {
    var i,j;
    // clear board and initialize the piece array
	this.initPieces();    
    for (i = 0; i <= ROW_NUM - 1; i++) {
      for (j = 0; j <= COL_NUM - 1; j++) {
        this.board[i][j] = 0;
      }
    }    
  }
  
  Board.prototype.init = function() {
    if (!this.initFen) this.initFen = START_POS;
    return this.fromFen(this.initFen);
  };

  Board.prototype.fromFen = function(fen) {
    var arr, fenstr, i, row, rows, _len;
    
	if (!fen) 
	  return;
	  
	// parse the fen string
    arr = fen.split(' ');
	if (arr.length === 6) { // valid fen string
	  this.clear();      
	  fenstr = arr[0];
      this.sideToMove = arr[1] !== 'b'? 0: 1;
      rows = fenstr.split('/');
      for (i = 0, _len = rows.length; i < _len; i++) {
        row = rows[i];
        this.parseFenRow(i, row);
      }
	}
	
	// update views
	if (this.view)
	  this.view.updateView();
	if (this.box)
	  this.box.updateView();
  };

  Board.prototype.parseFenRow = function(r, row) {
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
  
  Board.prototype.toFen = function() {
    var i, j, spaces, fen;
	fen = [];
	spaces = 0;
    for (i = 0; i <= ROW_NUM - 1; i++) { 
	  for (j = 0; j <= COL_NUM - 1; j++) {
	    if (this.board[i][j] === 0) {
		  spaces++;
		} else {
		  if (spaces > 0) {
		    fen.push(spaces);
			spaces = 0;
			fen.push(this.board[i][j]);
		  } else {
		    fen.push(this.board[i][j]);
		  }
        }

      }
	  if (spaces > 0) {
		fen.push(spaces);
		spaces = 0;
	  }      
	  if (i < ROW_NUM - 1) fen.push('/');	  
	}
	
	if (this.sideToMove === 0)
	  fen.push(' w');
	else
	  fen.push(' b');
	
	fen.push(' -');
	fen.push(' -');
	fen.push(' 0');
	fen.push(' 1');
	
	return fen.join('');
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

  function BoardView($, elBoard, elBox) {
    this.$ = $;
    this.$board = this.$(elBoard);
	this.$box = this.$(elBox);
	this.board = null;
	this.lastSelected = null;
	this.showBox = true;
	this.pieceToSelect = null;
  }

  BoardView.prototype.init = function(board) {
  	var _this = this;
	this.board = board || new Board;
	this.showBox = this.board.isPlacingPiece;
	this.board.view = this;
	
	this.$box.on('click', function(e) {
	  if (!_this.lastSelected)
	    return;
		
	  if (_this.isFreePiece(_this.lastSelected)) {
	    _this.lastSelected.toggleClass('selected');
	    _this.lastSelected = null;
	  } else {
	    // remove the selected piece from board, put back to box
 	    var r = _this.lastSelected.data('piece').r;
	    var c = _this.lastSelected.data('piece').c;
        _this.board.removePiece(r, c);
        _this.updateView();		
	  }
	  e.stopPropagation();
	});

	this.$board.on('click', function(e) {
	  if (!_this.lastSelected)
	    return;
	
      var location = _this.$board.offset();
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
	      // after the view update, select the placed piece if it was on the board
		  if (origin_r !== -1 && origin_c !== -1) {
		    _this.pieceToSelect = {'pt': pt, 'r': r, 'c': c};
          } else {
		    _this.pieceToSelect = null;
          } 		  
		  _this.updateView();
		}
	  } else {
	  // gaming	  
	  }
	  e.stopPropagation();
	});
	this.updateView();
  };
  
  BoardView.prototype.isFreePiece = function($piece) {
	var r = $piece.data('piece').r;
	var c = $piece.data('piece').c;
	return r === -1 && c === -1;
  };
  
  BoardView.prototype.showPieceBox = function(show) {
    if (this.showBox !== show) { 
	  this.showBox = show;
	  this.updateView();
	}
  };
  
  BoardView.prototype.updateView = function() {
    this.updateBoard();
	if (this.showBox) 
	  this.updateBox();
  };
  
  BoardView.prototype.updateBoard = function() {
    var _this = this;
	this.$board.empty();
	this.lastSelected = null;
    this.board.pieces.forEach(function(p) {
	  if (p.r !== -1 && p.c !== -1) // piece is on board
	    _this.addPiece(_this.$board, p);
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
	  if (_this.lastSelected !== $piece_div) {
	    $piece_div.toggleClass('selected');
	    _this.lastSelected = $piece_div;
	  } else {
	    // click on a piece again, de-select it
	    _this.lastSelected = null;
	  }
	  e.stopPropagation();
	});
	if (_this.pieceToSelect 
	&& _this.pieceToSelect.pt === piece.pt
	&& _this.pieceToSelect.r === piece.r
	&& _this.pieceToSelect.c === piece.c) {
	  $piece_div.click();
	}
    return piece.el = $piece_div;
  };

  BoardView.prototype.addPiece = function(container, piece) {
    return container.append(this.createPiece(piece));
  };

  BoardView.prototype.updateBox = function() {
    var _this = this;
	this.$box.empty();
    this.board.pieces.forEach(function(p) {
	  if (p.r === -1 && p.c === -1) // piece is not on board, should be appear in the box
	    _this.addFreePiece(_this.$box, p);
	});    
  };
  
  BoardView.prototype.piecePosition = function(pt) {
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

  BoardView.prototype.createFreePiece = function(piece) {
    var $piece_div;
	var _this = this;
    $piece_div = this.$('<div class="piece"/>');
    $piece_div.addClass('piece_' + piece.pt);
    $piece_div.css('width', this.board.cellWidth);
    $piece_div.css('height', this.board.cellHeight);
    $piece_div.css('left', this.piecePosition(piece.pt).x);
    $piece_div.css('top', this.piecePosition(piece.pt).y);
	$piece_div.data('piece', piece);
    $piece_div.bind('click', function(e){
	  if (_this.lastSelected) {	   
	    if (_this.isFreePiece(_this.lastSelected)) {
		  _this.lastSelected.toggleClass('selected');
		  if (_this.lastSelected !== $piece_div) {
		    $piece_div.toggleClass('selected');
			_this.lastSelected = $piece_div;
		  } else {
		    // click on a free piece again
			_this.lastSelected = null;
		  }
		  s.stopPropagation();
		} else {
		  // a piece on board was selected at the time, wanna move the piece to box?
		  // just bubble up the event, let $box.onclick handle it
		}
      } else {	  
	    $piece_div.toggleClass('selected');
	    _this.lastSelected = $piece_div;
	    e.stopPropagation();
	  }
	});
    return piece.el = $piece_div;
  };

  BoardView.prototype.addFreePiece = function(container, piece) {
    return container.append(this.createFreePiece(piece));
  };
  
  return BoardView;

})();

