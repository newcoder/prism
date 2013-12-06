
/**
 * Module dependencies.
 */

var express = require('express')
  , routes = require('./routes')
  , user = require('./routes/user')
  , http = require('http')
  , path = require('path')
  , util = require('util')
  , socketio = require('socket.io')
  , engine = require('./engine').UCIEngine;

// load the elephant eye chinese chess engine
var START_POS = 'rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5C1/9/RNBAKABNR w - - 0 1';
var eleeye = new engine('eleeye.exe', {
  usemillisec: false,
  batch: true,
  debug: true,
  ponder: false,});

var app = express()
, server = http.createServer(app)
, io = socketio.listen(server);

eleeye.evaluate_done(function (result) {
  util.log(result);
  var socket = result.t
  , arr = String(result.m).split(/\s+/)
  , move = arr[1];
  util.log('result:' + result.s + '\t' + result.m);
  util.log(socket);
  if (socket) {
    socket.emit('result', {'move': move});    
  }
});

eleeye.launch();
//eleeye.evaluate(START_POS);

io.sockets.on('connection', function (socket) {
  socket.on('think', function (data) {
    util.log('think:' + data.pos);
    eleeye.evaluate(data.pos, socket);  
  });
});

app.configure(function(){
  app.set('port', process.env.PORT || 3000);
  app.set('views', __dirname + '/views');
  app.set('view engine', 'jade');
  app.use(express.favicon());
  app.use(express.logger('dev'));
  app.use(express.bodyParser());
  app.use(express.methodOverride());
  app.use(app.router);
  app.use(express.static(path.join(__dirname, 'public')));
});

app.configure('development', function(){
  app.use(express.errorHandler());
});

app.get('/', routes.index);
app.get('/users', user.list);

server.listen(app.get('port'), function(){
  util.log("Express server listening on port " + app.get('port'));
});
