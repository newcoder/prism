var ucci = require('./ucci').ucci,
    util = require('util');

function UCIEngine(path, options, uci) {
  var self = this;
  this.engine = new ucci(path, options || {
    usemillisec: false,
    batch: true,
    debug: true,
    ponder: false,}, uci);

  this.serial = 0;
  this.queue = [];
  this.current = null;
  this.evaldone = null;

  // hook up the events, print out on screen
  this.engine.on('engineError', function(err) { 
    util.log(err);
  });

  this.engine.on('engineState', function(newstate) {
    util.log(newstate);
  });

  this.engine.on('commandSent', function(command) {
    util.log(command);
  });

  this.engine.on('ucciok', function(data) {
    util.log(data);
    // print engine id
    util.log('engine id: \n' + util.inspect(self.engine.id, false, null));
    // print engine defaults
    util.log('engine defaults: \n' + util.inspect(self.engine.defaults, false, null));
    // trigger the readyok event.
    self.engine.isready();
  });

  this.engine.on('bye', function(data) {
    util.log(data);
  });

  this.engine.on('id', function(data) {
    util.log(data);
  });

  this.engine.on('option', function(data) {
    util.log(data);
  });

  this.engine.on('readyok', function(data) {
    if (self.queue.length > 0) {
      self.current = self.queue.shift();
      self.engine.position(self.current.p);
      self.engine.go('', 'depth 6');
    }
  });

  this.engine.on('bestmove', function(data) {
    if (self.current && self.evaldone) {
      self.current.m = data;
      self.evaldone(self.current);
    }
    
    if (self.queue.length > 0) {
      self.engine.isready();
    }
  });
};

UCIEngine.prototype.launch = function() {
  this.engine.loadEngine();
  this.engine.boot();
};

UCIEngine.prototype.quit = function() {
  this.engine.quit();
};

UCIEngine.prototype.evaluate = function(pos, token) {
  this.serial = this.serial + 1;
  this.queue.push({'p': pos, 's': this.serial, 't': token });
  this.engine.isready();
  return this.serial;
};

UCIEngine.prototype.evaluate_done = function(callback) {
  this.evaldone = callback;
};

exports.UCIEngine = UCIEngine;
