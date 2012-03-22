START_POS= 'rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5C1/9/RNBAKABNR w - - 0 1'
ROW_NUM = 10
COL_NUM = 9

class Board
  constructor: (@option, @initFen) ->
    @board = [[],[],[],[],[],[],[],[],[],[]]
    @pieces = []
    @unusedPieces = 'rnbakabnrccpppppPPPPPCCRNBAKABNR'.split('');
    @lastMove = null
    @sideToMove = 0
    @player1Side = 0
    @cellWidth = 55
    @cellHeight = 55
    @init()

  init: ->
    @initFen = START_POS if !@initFen
    @fen2Board @initFen

  fen2Board: (fen) ->
    for i in [0..9]
      for j in [0..8]
        @board[i][j] = 0
    arr = fen.split(' ')
    fenstr = arr[0]
    @side = arr[1] != 'b'
    rows = fenstr.split('/')
    @row2Board i, row for row, i in rows
    return
  
  row2Board: (r, row) ->
    c = 0
    for ch, i in row
      if '0' <= ch <= '9'
        c = c + (ch - '0')
      else
        @board[r][c] = ch
        @pieces.push {pt: ch, r: r, c: c}
        index = @unusedPieces.indexOf ch
        @unusedPieces.splice index, 1 if index >=0
        c = c + 1
    return
   
  toCoordX: (r) ->
    offsetX = 3    
    if @player1Side != 0 
      r = ROW_NUM - r
    x = offsetX + @cellWidth*r 
      
  toCoordY: (c) ->
    offsetY = 3    
    if @player1Side != 0 
      c = COL_NUM - c
    y = offsetY + @cellWidth*c 
      
class BoardView 
  constructor: (@$, @el) ->

  init: -> 
    @board = new Board
    @placePiece @$(@el[0]), piece for piece in @board.pieces
    return
  
  createPiece: (piece) ->
    $piece_div = @$('<div class="piece"/>').draggable() 
    $piece_div.addClass 'piece_' + piece.pt
    $piece_div.css 'width', @board.cellWidth
    $piece_div.css 'height', @board.cellHeight
    $piece_div.css 'left', @board.toCoordX piece.c
    $piece_div.css 'top', @board.toCoordY piece.r
    $piece_div.bind 'dragstart', @onDragStart
    $piece_div.bind 'drag', @onDrag
    $piece_div.bind 'dragstop', @onDragStop
    piece.el = $piece_div
  
  placePiece: (container, piece) ->   
    container.append @createPiece piece

  disableDrag: ->
    @disablePieceDrag piece.el for piece in @board.pieces
      
  disablePieceDrag: (el) ->
    el.draggable( "option", "disabled", true ); 

  onDragStart: (event, ui) =>
  onDrag: (event, ui) =>
  onDragStop: (event, ui) =>