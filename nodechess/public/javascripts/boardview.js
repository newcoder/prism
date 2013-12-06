define(function (require, exports, module) {
  var Board = require('./board').Board,
    MoveGenerator = require('./board').MoveGenerator,
    consts = require('./consts'),
    BoardView;

  BoardView = (function () {

    // options to the board view
    var boardViewDefaults = {
      mode: 0,                // -1 - no interactive, 0 - placing piece, 1 - playing game, 2 - game replaying, etc.
      player1Side: 0,         // the player facing the screen, 0 - red, 1 - black
      showBox: true,          // show or hide the piece box
      checkPiecePlace: true,  // check whether the place is legal while placing piece
      showMoveShadow: true,   // show shadow for legal moves
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
      this.showMoveShadow = !options || !options.showMoveShadow ? boardViewDefaults.showMoveShadow : options.showMoveShadow;
      this.cellWidth = !options || !options.cellWidth ? boardViewDefaults.cellWidth : options.cellWidth;
      this.cellHeight = !options || !options.cellHeight ? boardViewDefaults.cellHeight : options.cellHeight;
      this.offsetX = !options || !options.offsetX ? boardViewDefaults.offsetX : options.offsetX;
      this.offsetY = !options || !options.offsetY ? boardViewDefaults.offsetY : options.offsetY;
      // internals
      this.lastSelected = null;
      this.boxCells = [];
      this.moveGenerator = null;
      this.moves = [];
      this.onMoveDone = null;
    }

    function onClickBox(self, e) {
      if (self.mode < 0) {
        return;
      }
      self['onClickBox_' + self.mode](e);
    }

    function onClickBoard(self, e) {
      if (self.mode < 0) {
        return;
      }
      self['onClickBoard_' + self.mode](e);
    }

    BoardView.prototype.moveDone = function (callback) {
      this.onMoveDone = callback;
    };

    BoardView.prototype.init = function (board) {
      var r, c, self = this;
      this.board = board || new Board();
      this.board.view = this;
      this.moveGenerator = new MoveGenerator(this.board);
      this.initPieceBox();

      this.$box.on('click', function (e) {
        onClickBox(self, e);
      });

      this.$board.on('click', function (e) {
        onClickBoard(self, e);
      });
      this.updateView();
    };

    BoardView.prototype.initPieceBox = function () {
      var i;
      for (i = 0; i < consts.PIECE_NUM; i = i + 1) {
        this.boxCells[i] = null;
      }
      this.$box.empty();
    };

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
      return;
    };

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
        if (!this.checkPiecePlace || this.board.isLegalPlace(r, c, piece.pt)) {
          // update the board
          this.board.placePiece(r, c, piece.id, piece.r, piece.c);
          // move the piece in board
          this.moveInBoard(piece, e);
          this.lastSelected = null;
        } else {
        // illegal place
          this.playVoice('beep');
        }
      }
      e.stopPropagation();
    };

    // move a piece from box to board
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

    // move a piece in board
    BoardView.prototype.moveInBoard = function (piece, e) {
      var location, x, y, cx, cy,
        self = this;
      location = this.$board.offset();
      x = e.pageX - location.left;
      y = e.pageY - location.top;
      piece.r = this.toRow(y); // destine row
      piece.c = this.toCol(x); // destine col
      // normalize x, y to fit the cell 
      x = (x - this.offsetX - this.cellWidth / 2) / this.cellWidth;
      x = Math.round(x) * this.cellWidth;
      y = (y - this.offsetY - this.cellHeight / 2) / this.cellHeight;
      y = Math.round(y) * this.cellHeight;

      // animate the move
      piece.el.animate({"left": x, "top": y}, 200, function () {
        var $selectedFrame = piece.el.data('selectedFrame');
        if ($selectedFrame) {
          $selectedFrame.remove();
          piece.el.data('selectedFrame', null);
        }
        piece.el.remove();
        piece.el = null;
        self.addPiece(self.$board, piece);
      });
    };

    // click in board in the mode of gaming
    BoardView.prototype.onClickBoard_1 = function (e) {
      var piece;
      if (this.lastSelected) {
        if (this.showMoveShadow) {
          this.clearMoveShadow();
        }
        piece = this.lastSelected.data('piece');
        this.makeMove(piece, e);
      }
      e.stopPropagation();
    };

    BoardView.prototype.makeMove = function (piece, e) {
      var location, x, y, fr = piece.r, fc = piece.c, id = piece.id, tr, tc,
        self = this, eatenPiece;
      location = this.$board.offset();
      x = e.pageX - location.left;
      y = e.pageY - location.top;
      tr = this.toRow(y); // destine row
      tc = this.toCol(x); // destine col
      if (this.legalPlaceToMove(tr, tc)) {
        // update the board
        eatenPiece = this.board.movePiece(tr, tc, fr, fc, this.mode != 1);
        if (eatenPiece) {
        // delete the element of eaten piece
          eatenPiece.el.remove();
          eatenPiece.el = null;
        }
        // normalize x, y to fit the cell 
        x = (x - this.offsetX - this.cellWidth / 2) / this.cellWidth;
        x = Math.round(x) * this.cellWidth;
        y = (y - this.offsetY - this.cellHeight / 2) / this.cellHeight;
        y = Math.round(y) * this.cellHeight;
        // animate the move
        piece.el.animate({"left": x, "top": y}, 200, function () {
          var $selectedFrame = piece.el.data('selectedFrame');
          if ($selectedFrame) {
            $selectedFrame.remove();
            piece.el.data('selectedFrame', null);
          }
          piece.el.remove();
          piece.el = null;
          piece.r = tr;
          piece.c = tc;
          self.addPiece(self.$board, piece);
          if (self.showBox) {
            self.refreshBox();
          }
          self.lastSelected = null;
          self.addMoveTrail(tr, tc, fr, fc);
          // callback
          if (self.onMoveDone) {
            self.onMoveDone();
          }
        });
      } else {
        // illegal place, can not make move
        this.toggleSelectedFrame(this.lastSelected);
        this.lastSelected = null;
      }
    };

    // create move trail to indicate the recent move
    BoardView.prototype.addMoveTrail = function (tr, tc, fr, fc) {
      var self = this,
        createTrail = function (r, c) {
          var $trail_div = self.$('<div class="selected"/>');
          $trail_div.css('width', self.cellWidth);
          $trail_div.css('height', self.cellHeight);
          $trail_div.css('left', self.toCoordX(c));
          $trail_div.css('top', self.toCoordY(r));
          self.$board.append($trail_div);
        };
      createTrail(tr, tc);
      createTrail(fr, fc);
    };

    BoardView.prototype.clearMoveTrail = function () {
      this.$board.find('.selected').remove();
    };

    BoardView.prototype.playVoice = function (voice) {
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
      this.refreshBoard();
      if (this.showBox) {
        this.refreshBox();
      }
    };

    BoardView.prototype.refreshBoard = function () {
      var self = this, moveStr, fr, fc, tr, tc;
      this.$board.empty();
      this.lastSelected = null;
      this.board.pieces.forEach(function (p) {
        if (p.r !== -1 && p.c !== -1) { // piece is on board
          self.addPiece(self.$board, p);
        }
      });
      /*
      if (this.board.lastMove.length > 0) {
      // add the move trail to the last move
        moveStr = this.board.lastMove;
        fc = moveStr[0].charCodeAt(0) - 'a'.charCodeAt(0);
        fr = consts.ROW_NUM - 1 - moveStr[1];
        tc = moveStr[2].charCodeAt(0) - 'a'.charCodeAt(0);
        tr = consts.ROW_NUM - 1 - moveStr[3];
        this.addMoveTrail(tr, tc, fr, fc);
      }
      */
    };

    function onClickPiece(self, $piece, e) {
      if (self.mode < 0) {
        return;
      }
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
      var piece = $piece.data('piece'),
        side = this.board.sideToMove,
        pieceSide = (piece.id > 15) ? 0 : 1;
      if (side !== pieceSide) {
      // can not move piece belongs to other side
        this.playVoice('beep');
      } else {
        this.clearMoveTrail();
        if (this.showMoveShadow) {
          this.clearMoveShadow();
        }
      // focus the piece to make move
        if (this.lastSelected) {
          this.toggleSelectedFrame(this.lastSelected);
        }
        if (this.lastSelected !== $piece) {
          this.toggleSelectedFrame($piece);
          this.lastSelected = $piece;
          // shadow for possible move targets
          if (this.showMoveShadow) {
            this.addMoveShadow(piece);
          }
        } else {
          // click on a piece again, de-select it
          this.lastSelected = null;
        }
      }
      e.stopPropagation();
    };

    BoardView.prototype.legalPlaceToMove = function (r, c) {
      var i, m;
      for (i = 0; i < this.moves.length; i = i + 1) {
        m = this.moves[i];
        if (m.tr === r && m.tc === c) {
          return true;
        }
      }
      return false;
    };

    BoardView.prototype.addMoveShadow = function (piece) {
      var self = this;
      this.moves = this.moveGenerator.generateMoves(piece);
      this.moves.forEach(function (m) {
        var $shadow_div = self.$('<div class="move_shadow"/>');
        $shadow_div.css('width', self.cellWidth);
        $shadow_div.css('height', self.cellHeight);
        $shadow_div.css('left', self.toCoordX(m.tc));
        $shadow_div.css('top', self.toCoordY(m.tr));
        self.$board.append($shadow_div);
      });
    };

    BoardView.prototype.clearMoveShadow = function () {
      this.$('.move_shadow').remove();
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

    // move a piece in the board view, instead of updating the whole view
    BoardView.prototype.movePiece = function (src_r, src_c, dest_r, dest_c) {
    // simulate user click
    // click the piece to be moved
      var piece = this.board.getPieceAt(src_r, src_c);
      if (piece) {
        var e = this.$.Event('click');
        if (piece.el){
          piece.el.trigger(e);
        }
      }
    // click the destination cell on the board
      var x = this.toCoordX(dest_c),
        y = this.toCoordY(dest_r),
        location = this.$board.offset(),
        e = this.$.Event('click');
      e.pageX = x + location.left;
      e.pageY = y + location.top;
      this.$board.trigger(e);
    };

    // move the piece by a move string notation
    BoardView.prototype.move = function (moveStr) {
      var tc, tr, fc, fr;
      fc = moveStr[0].charCodeAt(0) - 'a'.charCodeAt(0);
      fr = consts.ROW_NUM - 1 - moveStr[1];
      tc = moveStr[2].charCodeAt(0) - 'a'.charCodeAt(0);
      tr = consts.ROW_NUM - 1 - moveStr[3];

      if (!this.board.isLegalMove(tr, tc, fr, fc)) {
        return;
      }
      this.movePiece(fr, fc, tr, tc);
    };

    BoardView.prototype.addPiece = function (container, piece) {
      return container.append(this.createPiece(piece));
    };

    BoardView.prototype.refreshBox = function () {
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
      var i, r, c, t,
        boxDir = this.$box.height() > this.$box.width();
      if (piece.r === -1 && piece.c === -1) {
        for (i = 0; i < consts.PIECE_NUM; i = i + 1) {
          if (!this.boxCells[i]) {
            c = i % 4;
            r = Math.floor(i / 4);
            break;
          }
        }
        if (!boxDir) {
          t = r;
          r = c;
          c = t;
        }
      } else {
        r = piece.r;
        c = piece.c;
        piece.r = -1;
        piece.c = -1;
        if (boxDir) {
          i = r * 4 + c;
        } else {
          i = c * 4 + r;
        }
      }

      return {
        'i': i,
        'x': c * this.cellWidth,
        'y': r * this.cellHeight
      };
    };

    BoardView.prototype.toggleSelectedFrame = function ($piece) {
      var $selectedFrame = $piece.data('selectedFrame'),
        piece = $piece.data('piece');
      if ($selectedFrame) {
        $selectedFrame.remove();
        $piece.data('selectedFrame', null);
      } else {
        $selectedFrame = this.$('<div class="selected"/>');
        $piece.data('selectedFrame', $selectedFrame);
        $piece.append($selectedFrame);
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
        c = consts.COL_NUM - 1 - c;
      }
      x = this.offsetX + this.cellWidth * c;
      return x;
    };

    BoardView.prototype.toCoordY = function (r) {
      var y;
      if (this.player1Side !== 0) {
        r = consts.ROW_NUM - 1 - r;
      }
      y = this.offsetY + this.cellHeight * r;
      return y;
    };

    BoardView.prototype.toCol = function (x) {
      var c;
      x = (x - this.offsetX - this.cellWidth / 2) / this.cellWidth;
      c = Math.round(x);
      if (this.player1Side !== 0) {
        c = consts.COL_NUM - 1 - c;
      }
      return c;
    };

    BoardView.prototype.toRow = function (y) {
      var r;
      y = (y - this.offsetY - this.cellHeight / 2) / this.cellHeight;
      r = Math.round(y);
      if (this.player1Side !== 0) {
        r = consts.ROW_NUM - 1 - r;
      }
      return r;
    };

    return BoardView;

  }());

  exports.BoardView = BoardView;
});


