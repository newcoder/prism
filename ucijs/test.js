var ucci = require('./ucci').ucci,
    util = require('util');

var eleeye = new ucci('eleeye.exe', {
  usemillisec: false,
  batch: true,
  debug: true,
  ponder: false,});

eleeye.on('engineError', function(err) { 
  util.log(err);
});

eleeye.on('engineState', function(newstate) {
  util.log(newstate);
});

eleeye.on('commandSent', function(command) {
  util.log(command);
});

eleeye.on('ucciok', function(data) {
  util.log(data);
  // print engine id
  util.log('engine id: \n' + util.inspect(eleeye.id, false, null));
  // print engine defaults
  util.log('engine defaults: \n' + util.inspect(eleeye.defaults, false, null));
  // let the engine go away
  eleeye.quit();  
});

eleeye.on('bye', function(data) {
  util.log(data);
});

eleeye.on('id', function(data) {
  //util.log(data);
});

eleeye.on('option', function(data) {
  //util.log(data);
});

eleeye.loadEngine();
eleeye.ucci();

