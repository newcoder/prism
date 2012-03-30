seajs.config({      
    alias: {
      'jquery': 'jquery/1.7.1/jquery'
    },  
    charset: 'utf-8',  
    timeout: 20000,  
    debug: 0  
}); 

define(function(require) {
  var $ = require ('jquery');
  var BoardView = require('./boardview').BoardView;

  var boardView = new BoardView($, $("#board")[0], $("#box")[0]);
  boardView.init();	  
  boardView.mode = 1;  
      
  $("#changefen").click(function(){ 
    var input = $("#feninput").val();
    boardView.board.fromFen(input);
  }); 
  
  $("#getfen").click(function(){ 
    boardView.board.makeMove('b0c2');
  }); 
});
