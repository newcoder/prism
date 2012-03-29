define(function (require, exports, module) {
  require('./utils');
  var consts = require('./consts');

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

      // '炮，马，车' could be at any place in the board
      anyPlace = function (r, c) {
        return r >= 0 && r < consts.ROW_NUM && c >= 0 && c < consts.COL_NUM;
      },

      // '卒' could be at any place across the river, only ten places on own side
      p_placeChecker = function (r, c) {
        return ((r > 4) || (r > 2 && c % 2 === 0)) && anyPlace(r, c);
      },

      makeRedChecker = function (blackChecker) {
        var redChecker = function (r, c) {
          return blackChecker(consts.ROW_NUM - r - 1, consts.COL_NUM - c - 1);
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

  var MoveGenerator = (function () {

    var k_offset = [[1, 0], [-1, 0], [0, 1], [0, -1]],
      a_offset = [[-1, -1], [1, 1], [-1, 1], [1, -1]],
      elephant_eye = a_offset,
      b_offset = [[-2, -2], [2, 2], [-2, 2], [2, -2]],
      n_offset = [[-2, -1], [-2, 1], [-1, 2], [1, 2], [2, 1], [2, -1], [1, -2], [-1, -2]],
      horse_leg = [[-1, 0], [-1, 0], [0, 1], [0, 1], [1, 0], [1, 0], [0, -1], [0, -1]],
      p_offset = [[1, 0], [0, 1], [0, -1]],
      P_offset = [[-1, 0], [0, 1], [0, -1]],
      piecesCaptured = [],

      sameSide = function (pt1, pt2) {
        return ((pt1.toUpperCase() === pt1) && (pt2.toUpperCase() === pt2)) ||
          ((pt1.toUpperCase() !== pt1) && (pt2.toUpperCase() !== pt2));
      },

      generic_moves = function (piece, offset, barrier) {
        var i, p, b,
          fr = piece.r,
          fc = piece.c,
          pt = piece.pt,
          moves = [];
        for (i = 0; i < offset.length; i = i + 1) {
          p = offset[i];
          r = fr + p[0];
          c = fc + p[1];
          if (barrier) {
            b = barrier[i];
          }
          if (this.placeChecker.check(pt, r, c)
              && ((this.board[r][c] === 0) || !sameSide(this.board[r][c], pt))) {
            if (!barrier || this.board[fr + b[0]][fc + b[1]] === 0) {
              moves.push({'pt': pt, 'fr': fr, 'fc': fc, 'tr': r, 'tc': c});
              if (this.board[r][c] !== 0) {
                piecesCaptured.push([this.board[r][c], r, c]);
              }
            }
          }
        }
        return moves;
      },

      k_moves = function (piece) {
        return this.generic_moves(piece, k_offset);
      },

      K_moves = k_moves,

      a_moves = function (piece) {
        return this.generic_moves(piece, a_offset);
      },

      A_moves = a_moves,

      b_moves = function (piece) {
        return this.generic_moves(piece, b_offset, elephant_eye);
      },

      B_moves = b_moves,

      n_moves = function (piece) {
        return this.generic_moves(piece, n_offset, horse_leg);
      },

      N_moves = n_moves,

      p_moves = function (piece) {
        return this.generic_moves(piece, p_offset);
      },

      P_moves = function (piece) {
        return this.generic_moves(piece, P_offset);
      },

      r_moves = function (piece) {
        var i,
          r = piece.r,
          c = piece.c,
          pt = piece.pt,
          moves = [];
        // generate legal moves in 4 directions, duplicated code?
        for (i = r + 1; i < consts.ROW_NUM; i = i + 1) {
          if (this.board[i][c] === 0) {
            moves.push({'pt': pt, 'fr': piece.r, 'fc': piece.c, 'tr': i, 'tc': c});
          } else if (!sameSide(this.board[i][c], pt)) {
            moves.push({'pt': pt, 'fr': piece.r, 'fc': piece.c, 'tr': i, 'tc': c});
            piecesCaptured.push([this.board[i][c], i, c]);
            break;
          } else {
            break;
          }
        }
        for (i = r - 1; i >= 0; i = i - 1) {
          if (this.board[i][c] === 0) {
            moves.push({'pt': pt, 'fr': piece.r, 'fc': piece.c, 'tr': i, 'tc': c});
          } else if (!sameSide(this.board[i][c], pt)) {
            moves.push({'pt': pt, 'fr': piece.r, 'fc': piece.c, 'tr': i, 'tc': c});
            piecesCaptured.push([this.board[i][c], i, c]);
            break;
          } else {
            break;
          }
        }
        for (i = c + 1; i < consts.COL_NUM; i = i + 1) {
          if (this.board[r][i] === 0) {
            moves.push({'pt': pt, 'fr': piece.r, 'fc': piece.c, 'tr': r, 'tc': i});
          } else if (!sameSide(this.board[r][i], pt)) {
            moves.push({'pt': pt, 'fr': piece.r, 'fc': piece.c, 'tr': r, 'tc': i});
            piecesCaptured.push([this.board[r][i], r, i]);
            break;
          } else {
            break;
          }
        }
        for (i = c - 1; i >= 0; i = i - 1) {
          if (this.board[r][i] === 0) {
            moves.push({'pt': pt, 'fr': piece.r, 'fc': piece.c, 'tr': r, 'tc': i});
          } else if (!sameSide(this.board[r][i], pt)) {
            moves.push({'pt': pt, 'fr': piece.r, 'fc': piece.c, 'tr': r, 'tc': i});
            piecesCaptured.push([this.board[r][i], r, i]);
            break;
          } else {
            break;
          }
        }
        return moves;
      },

      R_moves = r_moves,

      c_moves = function (piece) {
        var i, cannon_stand = false,
          r = piece.r,
          c = piece.c,
          pt = piece.pt,
          moves = [];
        for (i = r + 1; i < consts.ROW_NUM; i = i + 1) {
          if (!cannon_stand) {
            if (this.board[i][c] === 0) {
              moves.push({'pt': pt, 'fr': piece.r, 'fc': piece.c, 'tr': i, 'tc': c});
            } else {
              cannon_stand = true;
            }
          } else {
            if (this.board[i][c] !== 0) {
              if (!sameSide(this.board[i][c], pt)) {
                moves.push({'pt': pt, 'fr': piece.r, 'fc': piece.c, 'tr': i, 'tc': c});
                piecesCaptured.push([this.board[i][c], i, c]);
              }
              break;
            }
          }
        }
        cannon_stand = false;
        for (i = r - 1; i >= 0; i = i - 1) {
          if (!cannon_stand) {
            if (this.board[i][c] === 0) {
              moves.push({'pt': pt, 'fr': piece.r, 'fc': piece.c, 'tr': i, 'tc': c});
            } else {
              cannon_stand = true;
            }
          } else {
            if (this.board[i][c] !== 0) {
              if (!sameSide(this.board[i][c], pt)) {
                moves.push({'pt': pt, 'fr': piece.r, 'fc': piece.c, 'tr': i, 'tc': c});
                piecesCaptured.push([this.board[i][c], i, c]);
              }
              break;
            }
          }
        }
        cannon_stand = false;
        for (i = c + 1; i < consts.COL_NUM; i = i + 1) {
          if (!cannon_stand) {
            if (this.board[r][i] === 0) {
              moves.push({'pt': pt, 'fr': piece.r, 'fc': piece.c, 'tr': r, 'tc': i});
            } else {
              cannon_stand = true;
            }
          } else {
            if (this.board[r][i] !== 0) {
              if (!sameSide(this.board[r][i], pt)) {
                moves.push({'pt': pt, 'fr': piece.r, 'fc': piece.c, 'tr': r, 'tc': i});
                piecesCaptured.push([this.board[r][i], r, i]);
              }
              break;
            }
          }
        }
        cannon_stand = false;
        for (i = c - 1; i >= 0; i = i - 1) {
          if (!cannon_stand) {
            if (this.board[r][i] === 0) {
              moves.push({'pt': pt, 'fr': piece.r, 'fc': piece.c, 'tr': r, 'tc': i});
            } else {
              cannon_stand = true;
            }
          } else {
            if (this.board[r][i] !== 0) {
              if (!sameSide(this.board[r][i], pt)) {
                moves.push({'pt': pt, 'fr': piece.r, 'fc': piece.c, 'tr': r, 'tc': i});
                piecesCaptured.push([this.board[r][i], r, i]);
              }
              break;
            }
          }
        }
        return moves;
      },

      C_moves = c_moves;

    function MoveGenerator(board) {
      this.generator = {
        'generic_moves': generic_moves,
        'a': a_moves,
        'b': b_moves,
        'k': k_moves,
        'p': p_moves,
        'c': c_moves,
        'n': n_moves,
        'r': r_moves,
        'A': A_moves,
        'B': B_moves,
        'K': K_moves,
        'P': P_moves,
        'C': C_moves,
        'N': N_moves,
        'R': R_moves,
        'board': board.board,
        'placeChecker': board.placeChecker      
      };
    }

    // generate legal moves for a piece on the board
    // return a moves array, each item is an object { pt, fr, fc, tr, tc }
    MoveGenerator.prototype.generateMoves = function (piece) {
      return this.generator[piece.pt](piece);
    };

    // generate all moves for the side 
    MoveGenerator.prototype.generateAllMoves = function () {

    };

    return MoveGenerator;
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
        pieceArr = consts.ALL_PIECES.split('');
      this.pieces = [];
      pieceArr.forEach(function (p, i) {
        self.pieces.push({id: i, pt: p, r: -1, c: -1});
      });
    };

    Board.prototype.clear = function () {
      var i, j;
      // clear board and initialize the piece array
      this.initPieces();
      for (i = 0; i <= consts.ROW_NUM - 1; i = i + 1) {
        for (j = 0; j <= consts.COL_NUM - 1; j = j + 1) {
          this.board[i][j] = 0;
        }
      }
    };

    Board.prototype.init = function () {
      if (!this.initFen) {
        this.initFen = consts.START_POS;
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
      for (i = 0; i <= consts.ROW_NUM - 1; i = i + 1) {
        for (j = 0; j <= consts.COL_NUM - 1; j = j + 1) {
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
        if (i < consts.ROW_NUM - 1) {
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

    Board.prototype.getPieceAt = function (r, c) {
      var i;
      for (i = 0; i < this.pieces.length; i = i + 1) {
        p = this.pieces[i];
        if (p.r === r && p.c === c) {
          return p;
        }
      }
      return null;
    };

    // move a piece in gaming
    Board.prototype.makeMove = function (move) {
      var pt, dest_c, dest_r, origin_c, origin_r, origin_pt;
      origin_c = move[0].charCodeAt(0) - 'a'.charCodeAt(0);
      origin_r = consts.ROW_NUM - 1 - move[1];
      origin_pt = this.board[origin_r][origin_c];
      dest_c = move[2].charCodeAt(0) - 'a'.charCodeAt(0);
      dest_r = consts.ROW_NUM - 1 - move[3];

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
  
  exports.Board = Board;
  exports.PlaceChecker = PlaceChecker;
  exports.MoveGenerator = MoveGenerator;
  
});