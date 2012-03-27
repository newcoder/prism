var START_POS = 'rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5C1/9/RNBAKABNR w - - 0 1';
var ALL_PIECES = 'rnbakabnrccpppppPPPPPCCRNBAKABNR';
var ALL_PIECETYPES = 'rnbakcpRNBAKCP';
var PIECE_NUM = 32;
var ROW_NUM = 10;
var COL_NUM = 9;

// Production steps of ECMA-262, Edition 5, 15.4.4.18
// Reference: http://es5.github.com/#x15.4.4.18
if (!Array.prototype.forEach) {
  Array.prototype.forEach = function (callback, thisArg) {
    var T, k, O, len, kValue;
    if (this === null) {
      throw new TypeError(" this is null or not defined");
    }
    O = this;
    len = O.length >>> 0; // Hack to convert O.length to a UInt32
    if ({}.toString.call(callback) !== "[object Function]") {
      throw new TypeError(callback + " is not a function");
    }
    if (thisArg) {
      T = thisArg;
    }
    k = 0;
    while (k < len) {
      if (O.hasOwnProperty(k)) {
        kValue = O[k];
        callback.call(T, kValue, k, O);
      }
      k = k + 1;
    }
  };
}

var PlaceChecker = (function () {
  // legal places of '仕'    
  var a_legalPlaces = [[0, 3], [0, 5], [1, 4], [2, 3], [2, 5]],
  // legal places of '象'  
    b_legalPlaces = [[0, 2], [0, 6], [2, 0], [2, 4], [2, 8], [4, 2], [4, 6]],

    placeChecker = function (places, r, c) {
      var i, p;
      for (i = 0; i < places.length; i = i + 1) {
        p = places[i];
        if (r === p[0] && c === p[1]) {
          return true;
        }
      }
      return false;
    },

    a_placeChecker = function (r, c) {
      return placeChecker(a_legalPlaces, r, c);
    },

    b_placeChecker = function (r, c) {
      return placeChecker(b_legalPlaces, r, c);
    },

    // '将' only could be in the palace
    k_placeChecker = function (r, c) {
      return r >= 0 && r <= 2 && c >= 3 && c <= 5;
    },

    // '炮，马，车' could be at any place
    anyPlace = function (r, c) {
      return true;
    },

    // '卒' could be at any place across the river, only ten places on own side
    p_placeChecker = function (r, c) {
      return (r > 4) || (r > 2 && c % 2 === 0);
    },

    makeRedChecker = function (blackChecker) {
      var redChecker = function (r, c) {
        return blackChecker(ROW_NUM - r - 1, COL_NUM - c - 1);
      };
      return redChecker;
    },

    A_placeChecker = makeRedChecker(a_placeChecker),
    B_placeChecker = makeRedChecker(b_placeChecker),
    K_placeChecker = makeRedChecker(k_placeChecker),
    P_placeChecker = makeRedChecker(p_placeChecker);


  function PlaceChecker() {
    this.checker = {
      'a': a_placeChecker,
      'b': b_placeChecker,
      'k': k_placeChecker,
      'p': p_placeChecker,
      'c': anyPlace,
      'n': anyPlace,
      'r': anyPlace,
      'A': A_placeChecker,
      'B': B_placeChecker,
      'K': K_placeChecker,
      'P': P_placeChecker,
      'C': anyPlace,
      'N': anyPlace,
      'R': anyPlace
    };
  }

  PlaceChecker.prototype.check = function (pt, r, c) {
    return this.checker[pt](r, c);
  };

  return PlaceChecker;
}());


var Board = (function () {

  function Board(option, initFen) {
    this.option = option;
    this.initFen = initFen;
    this.board = [[], [], [], [], [], [], [], [], [], []];
    this.pieces = [];
    this.moveList = [];
    this.sideToMove = 0;
    this.view = null;
    this.placeChecker = new PlaceChecker();
    this.init();
  }

  Board.prototype.initPieces = function () {
    var self = this,
      pieceArr = ALL_PIECES.split('');
    this.pieces = [];
    pieceArr.forEach(function (p, i) {
      self.pieces.push({id: i, pt: p, r: -1, c: -1});
    });
  };

  Board.prototype.clear = function () {
    var i, j;
    // clear board and initialize the piece array
    this.initPieces();
    for (i = 0; i <= ROW_NUM - 1; i = i + 1) {
      for (j = 0; j <= COL_NUM - 1; j = j + 1) {
        this.board[i][j] = 0;
      }
    }
  };

  Board.prototype.init = function () {
    if (!this.initFen) {
      this.initFen = START_POS;
    }
    return this.fromFen(this.initFen);
  };

  Board.prototype.fromFen = function (fen) {
    var arr, fenstr, i, row, rows, len;

    if (!fen) {
      return;
    }
	// parse the fen string
    arr = fen.split(' ');
    if (arr.length === 6) { // valid fen string
      this.clear();
      fenstr = arr[0];
      this.sideToMove = arr[1] !== 'b' ? 0 : 1;
      rows = fenstr.split('/');
      for (i = 0, len = rows.length; i < len; i = i + 1) {
        row = rows[i];
        this.parseFenRow(i, row);
      }
    }

	// update views
    if (this.view) {
      this.view.updateView();
    }
    if (this.box) {
      this.box.updateView();
    }
  };

  Board.prototype.parseFenRow = function (r, row) {
    var c, ch, i, len;
    c = 0;
    for (i = 0, len = row.length; i < len; i = i + 1) {
      ch = row[i];
      if (('0' <= ch && ch <= '9')) {
        c = c + (ch.charCodeAt(0) - '0'.charCodeAt(0));
      } else {
        this.addPiece(r, c, ch);
        c = c + 1;
      }
    }
  };

  Board.prototype.toFen = function () {
    var i, j, spaces, fen;
    fen = [];
    spaces = 0;
    for (i = 0; i <= ROW_NUM - 1; i = i + 1) {
      for (j = 0; j <= COL_NUM - 1; j = j + 1) {
        if (this.board[i][j] === 0) {
          spaces = spaces + 1;
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
      if (i < ROW_NUM - 1) {
        fen.push('/');
      }
    }

    if (this.sideToMove === 0) {
      fen.push(' w');
    } else {
      fen.push(' b');
    }
    fen.push(' -');
    fen.push(' -');
    fen.push(' 0');
    fen.push(' 1');

    return fen.join('');
  };

  // add a piece to chess board by the piece type
  Board.prototype.addPiece = function (r, c, pt) {
    this.board[r][c] = pt;
    var i, p;
    for (i = 0; i < this.pieces.length; i = i + 1) {
      p = this.pieces[i];
      if (p.pt === pt && p.r === -1 && p.c === -1) {
        p.r = r;
        p.c = c;
        break;
      }
    }
  };

  // add a piece to chess board by the piece index
  Board.prototype.addPieceById = function (r, c, id) {
    this.board[r][c] = this.pieces[id].pt;
    this.pieces[id].r = r;
    this.pieces[id].c = c;
  };

  // remove a piece from chess board
  Board.prototype.removePiece = function (r, c) {
    var ch, i, p;
    ch = this.board[r][c];
    this.board[r][c] = 0;
    for (i = 0; i < this.pieces.length; i = i + 1) {
      p = this.pieces[i];
      if (p.pt === ch && p.r === r && p.c === c) {
        p.r = -1;
        p.c = -1;
        break;
      }
    }
  };

  // move a piece in gaming
  Board.prototype.makeMove = function (move) {
    var pt, dest_c, dest_r, origin_c, origin_r, origin_pt;
    origin_c = move[0].charCodeAt(0) - 'a'.charCodeAt(0);
    origin_r = ROW_NUM - 1 - move[1];
    origin_pt = this.board[origin_r][origin_c];
    dest_c = move[2].charCodeAt(0) - 'a'.charCodeAt(0);
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
  Board.prototype.isLegalMove = function (move) {

  };

  // check the place is legal on the board
  Board.prototype.isLegalPlace = function (r, c, pt) {
    var dest_pt = this.board[r][c];
    if (dest_pt !== 0) {
	  //there is a piece at this place
      return false;
    }
    return this.placeChecker.check(pt, r, c);
  };

  // place a piece on the board, origin_c, origin_r is null when move a piece from piece box
  Board.prototype.placePiece = function (dest_r, dest_c, id, origin_r, origin_c) {
    if (origin_r !== -1 && origin_c !== -1) {
      this.removePiece(origin_r, origin_c);
    }
    this.addPieceById(dest_r, dest_c, id);
  };

  return Board;

}());

var BoardView = (function () {

  // options to the board view
  var boardViewDefaults = {
    mode: 0,                // 0 - placing piece, 1 - playing game, 2 - game replaying, etc.
    player1Side: 0,         // the player facing the screen, 0 - red, 1 - black
    showBox: true,          // show or hide the piece box
    checkPiecePlace: true,  // check whether the place is legal while placing piece
    cellWidth: 55,
    cellHeight: 55,
    offsetX: 3,
    offsetY: 3
  };

  function BoardView($, elBoard, elBox, options) {
    this.$ = $;
    this.$board = this.$(elBoard);
    this.$box = this.$(elBox);
    this.board = null;
    // options    
    this.mode = !options || !options.mode ? boardViewDefaults.mode : options.mode;
    this.player1Side = !options || !options.player1Side ? boardViewDefaults.player1Side : options.player1Side;
    this.showBox = !options || !options.showBox ? this.mode === 0 : options.showBox;
    this.checkPiecePlace = !options || !options.checkPiecePlace ? boardViewDefaults.checkPiecePlace : options.checkPiecePlace;
    this.cellWidth = !options || !options.cellWidth ? boardViewDefaults.cellWidth : options.cellWidth;
    this.cellHeight = !options || !options.cellHeight ? boardViewDefaults.cellHeight : options.cellHeight;
    this.offsetX = !options || !options.offsetX ? boardViewDefaults.offsetX : options.offsetX;
    this.offsetY = !options || !options.offsetY ? boardViewDefaults.offsetY : options.offsetY;
    // internals
    this.lastSelected = null;
    this.boxCells = [];
  }

  BoardView.prototype.initPieceBox = function () {
    var i;
    for (i = 0; i < PIECE_NUM; i = i + 1) {
      this.boxCells[i] = null;
    }
    this.$box.empty();
  };

  function onClickBox(self, e) {
    self['onClickBox_' + self.mode](e);
  }

  // click in box in the mode of placing piece
  BoardView.prototype.onClickBox_0 = function (e) {
    var piece;
    if (!this.lastSelected) {
      e.stopPropagation();
      return;
    }

    if (this.isFreePiece(this.lastSelected)) {
      // a piece in the box is selected currently
      this.toggleSelectedFrame(this.lastSelected);
      this.lastSelected = null;
    } else {
      // a piece on the board is selected, remove the selected piece from board
      piece = this.lastSelected.data('piece');
      this.board.removePiece(piece.r, piece.c);
      // move the piece from board to box
      this.moveToBox(piece, e);
      this.lastSelected = null;
    }
    e.stopPropagation();
  };

  // move a piece from board to box
  BoardView.prototype.moveToBox = function (piece, e) {
    var location, x, y, zindex, cx, cy,
      self = this;
    location = this.$board.offset();
    x = e.pageX - location.left;
    y = e.pageY - location.top;
    // normalize x, y to fit the cell 
    x = (x - this.offsetX - this.cellWidth / 2) / this.cellWidth;
    x = Math.round(x) * this.cellWidth;
    y = (y - this.offsetY - this.cellHeight / 2) / this.cellHeight;
    y = Math.round(y) * this.cellHeight;
    // calculate dest r, c in piece box
    location = this.$box.offset();
    cx = e.pageX - location.left;
    cy = e.pageY - location.top;
    cx = (cx - this.cellWidth / 2) / this.cellWidth;
    piece.c = Math.round(cx);
    cy = (cy - this.cellHeight / 2) / this.cellHeight;
    piece.r = Math.round(cy);
    // animate the move
    zindex = this.$board.css('z-index');
    this.$board.css('z-index', 10);
    piece.el.animate({"left": x, "top": y}, 200, function () {
      var $selectedFrame = piece.el.data('selectedFrame');
      if ($selectedFrame) {
        $selectedFrame.remove();
        piece.el.data('selectedFrame', null);
      }
      piece.el.remove();
      piece.el = null;
      self.addFreePiece(self.$box, piece);
      self.$board.css('z-index', zindex);
    });
  };

  // click box in the mode of gaming
  BoardView.prototype.onClickBox_1 = function (e) {
  };

  function onClickBoard(self, e) {
    self['onClickBoard_' + self.mode](e);
  }

  /*
     var 
        origin_r = self.lastSelected.data('piece').r,
        origin_c = self.lastSelected.data('piece').c;
      self.toggleSelectedFrame(self.lastSelected);
      self.lastSelected = null;  
  */

  // click in board in the mode of placing piece
  BoardView.prototype.onClickBoard_0 = function (e) {
    var piece, location, x, y, r, c;
    if (!this.lastSelected) {
      e.stopPropagation();
      return;
    }

    piece = this.lastSelected.data('piece');
    location = this.$board.offset();
    x = e.pageX - location.left;
    y = e.pageY - location.top;
    r = this.toRow(y);
    c = this.toCol(x);
    if (this.isFreePiece(this.lastSelected)) {
    // a piece in the box is selected currently
      if (!this.checkPiecePlace || this.board.isLegalPlace(r, c, piece.pt)) {
        // update the board
        this.board.placePiece(r, c, piece.id, piece.r, piece.c);
        // move the piece from box to board
        this.moveToBoard(piece, e);
        this.lastSelected = null;
      } else {
      // the place (r,c) is illegal for piece.pt, beep
        this.playVoice('beep');
      }
    } else {
    // a piece in the board is selected currently
    }
    e.stopPropagation();
  };

  BoardView.prototype.moveToBoard = function (piece, e) {
    var location, x, y, zindex, cx, cy,
      self = this;
    location = this.$box.offset();
    x = e.pageX - location.left;
    y = e.pageY - location.top;
    // normalize x, y to fit the cell 
    x = (x - this.cellWidth / 2) / this.cellWidth;
    x = Math.round(x) * this.cellWidth;
    y = (y - this.cellHeight / 2) / this.cellHeight;
    y = Math.round(y) * this.cellHeight;
    // calculate dest r, c in board    
    location = this.$board.offset();
    cx = e.pageX - location.left;
    cy = e.pageY - location.top;
    piece.r = this.toRow(cy);
    piece.c = this.toCol(cx);
    // animate the move
    zindex = this.$box.css('z-index');
    this.$box.css('z-index', 10);
    piece.el.animate({"left": x, "top": y}, 200, function () {
      var $selectedFrame = piece.el.data('selectedFrame');
      if ($selectedFrame) {
        $selectedFrame.remove();
        piece.el.data('selectedFrame', null);
      }
      piece.el.remove();
      piece.el = null;
      self.addPiece(self.$board, piece);
      self.$box.css('z-index', zindex);
    });
  };

  BoardView.prototype.playVoice = function (voice) {
  };

  BoardView.prototype.init = function (board) {
    var r, c, self = this;
    this.board = board || new Board();
    this.board.view = this;
    this.initPieceBox();

    this.$box.on('click', function (e) {
      onClickBox(self, e);
    });

    this.$board.on('click', function (e) {
      onClickBoard(self, e);
    });
    this.updateView();
  };

  BoardView.prototype.isFreePiece = function ($piece) {
    var r = $piece.data('piece').r,
      c = $piece.data('piece').c;
    return r === -1 && c === -1;
  };

  BoardView.prototype.showPieceBox = function () {
    this.showBox = true;
    this.$box.show();
  };

  BoardView.prototype.hidePieceBox = function () {
    this.showBox = false;
    this.$box.hide();
  };

  BoardView.prototype.updateView = function () {
    this.updateBoard();
    if (this.showBox) {
      this.updateBox();
    }
  };

  BoardView.prototype.updateBoard = function () {
    var self = this;
    this.$board.empty();
    this.lastSelected = null;
    this.board.pieces.forEach(function (p) {
      if (p.r !== -1 && p.c !== -1) { // piece is on board
        self.addPiece(self.$board, p);
      }
    });
  };

  function onClickPiece(self, $piece, e) {
    self['onClickPiece_' + self.mode]($piece, e);
  }

  // piece clicked in placing mode
  BoardView.prototype.onClickPiece_0 = function ($piece, e) {
    if (this.lastSelected) {
      this.toggleSelectedFrame(this.lastSelected);
    }
    if (this.lastSelected !== $piece) {
      this.toggleSelectedFrame($piece);
      this.lastSelected = $piece;
    } else {
      // click on a piece again, de-select it
      this.lastSelected = null;
    }
    e.stopPropagation();
  };

  // piece clicked in gaming mode
  BoardView.prototype.onClickPiece_1 = function ($piece, e) {

  };

  BoardView.prototype.createPiece = function (piece) {
    var $piece_div,
      self = this;
    $piece_div = this.$('<div class="piece"/>');
    $piece_div.addClass('piece_' + piece.pt);
    $piece_div.css('width', this.cellWidth);
    $piece_div.css('height', this.cellHeight);
    $piece_div.css('left', this.toCoordX(piece.c));
    $piece_div.css('top', this.toCoordY(piece.r));
    $piece_div.data('piece', piece);
    $piece_div.on('click', function (e) {
      onClickPiece(self, $piece_div, e);
    });
    piece.el = $piece_div;
    return $piece_div;
  };

  // TODO...move a piece in the board view, instead of updating the whole view
  BoardView.prototype.movePiece = function (src_r, src_c, dest_r, dest_c) {

  };

  BoardView.prototype.addPiece = function (container, piece) {
    return container.append(this.createPiece(piece));
  };

  BoardView.prototype.updateBox = function () {
    var self = this;
    this.initPieceBox();
    this.board.pieces.forEach(function (p) {
      if (p.r === -1 && p.c === -1) {// piece is not on board, should be appear in the box
        self.addFreePiece(self.$box, p);
      }
    });
  };

  // get a cell from the box to place the piece, use a 8*4 piece box instead of 7*2 
  BoardView.prototype.getFreeCell = function (piece) {
    var i, r, c;
    if (piece.r === -1 && piece.c === -1) {
      for (i = 0; i < PIECE_NUM; i = i + 1) {
        if (!this.boxCells[i]) {
          c = i % 4;
          r = Math.floor(i / 4);
          break;
        }
      }
    } else {
      r = piece.r;
      c = piece.c;
      piece.r = -1;
      piece.c = -1;
      i = r * 4 + c;
    }

    return {
      'i': i,
      'x': c * this.cellWidth,
      'y': r * this.cellHeight
    };
  };

  BoardView.prototype.toggleSelectedFrame = function ($piece) {
    var $selectedFrame = $piece.data('selectedFrame'),
      piece = $piece.data('piece'),
      $container = piece.r === -1 && piece.c === -1 ? this.$box : this.$board;
    if ($selectedFrame) {
      $selectedFrame.remove();
      $piece.data('selectedFrame', null);
    } else {
      $selectedFrame = this.$('<div class="selected"/>');
      $selectedFrame.css('left', $piece.css('left'));
      $selectedFrame.css('top', $piece.css('top'));
      $piece.data('selectedFrame', $selectedFrame);
      $container.append($selectedFrame);
    }
  };

  function onClickFreePiece(self, $piece, e) {
    self['onClickFreePiece_' + self.mode]($piece, e);
  }

  // free piece clicked in placing mode
  BoardView.prototype.onClickFreePiece_0 = function ($piece, e) {
    if (this.lastSelected) {
      this.toggleSelectedFrame(this.lastSelected);
    }

    if (this.lastSelected !== $piece) {
      this.toggleSelectedFrame($piece);
      this.lastSelected = $piece;
    } else {
    // click on a free piece again
      this.lastSelected = null;
    }
    e.stopPropagation();
  };

  // free piece clicked in gaming mode
  BoardView.prototype.onClickFreePiece_1 = function ($piece, e) {

  };

  BoardView.prototype.createFreePiece = function (piece) {
    var $piece_div,
      self = this,
      freeCell = this.getFreeCell(piece);
    $piece_div = this.$('<div class="piece"/>');
    $piece_div.addClass('piece_' + piece.pt);
    $piece_div.css('width', this.cellWidth);
    $piece_div.css('height', this.cellHeight);
    $piece_div.css('left', freeCell.x);
    $piece_div.css('top', freeCell.y);
    $piece_div.data('piece', piece);
    this.boxCells[freeCell.i] = piece;
    $piece_div.on('click', function (e) {
      onClickFreePiece(self, $piece_div, e);
    });
    piece.el = $piece_div;
    return $piece_div;
  };

  BoardView.prototype.addFreePiece = function (container, piece) {
    return container.append(this.createFreePiece(piece));
  };

  BoardView.prototype.toCoordX = function (c) {
    var x;
    if (this.player1Side !== 0) {
      c = COL_NUM - 1 - c;
    }
    x = this.offsetX + this.cellWidth * c;
    return x;
  };

  BoardView.prototype.toCoordY = function (r) {
    var y;
    if (this.player1Side !== 0) {
      r = ROW_NUM - 1 - r;
    }
    y = this.offsetY + this.cellHeight * r;
    return y;
  };

  BoardView.prototype.toCol = function (x) {
    var c;
    x = (x - this.offsetX - this.cellWidth / 2) / this.cellWidth;
    c = Math.round(x);
    if (this.player1Side !== 0) {
      c = COL_NUM - 1 - c;
    }
    return c;
  };

  BoardView.prototype.toRow = function (y) {
    var r;
    y = (y - this.offsetY - this.cellHeight / 2) / this.cellHeight;
    r = Math.round(y);
    if (this.player1Side !== 0) {
      r = ROW_NUM - 1 - r;
    }
    return r;
  };

  return BoardView;

}());

