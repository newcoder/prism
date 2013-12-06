var util   = require('util'),
  exec  = require('child_process').spawn,
  EventEmitter = require('events').EventEmitter;
var EngineState = {uninitialized: 'uninitialized', loaded: 'loaded', idle: 'idle', thinking: 'thinking' };

// commands are acceptable under certain engine status
var CommandStateMap = {
    uninitialized: [],
    loaded:   ['ucci', 'uci'],
    idle:     ['isready', 'setoption', 'position', 'banmoves', 'go', 'probe', 'quit'],
    thinking: ['isready', 'ponderhit', 'stop', 'probe']
  };

var ucciOptions = {
  usemillisec: true,
  batch: false,
  debug: false,
  ponder: false,
  usebook: true,
  useegtb: true,
  bookfiles: '',
  egtbpaths: '',
  evalapi: 'evaluate.dll',
  hashsize: 16,
  threads: 0,
  promotion: false,
  idle: 'none', //none, small, medium, large
  pruning: 'large',
  knowledge: 'small',
  randomness: 'none',
  style: 'solid', //solid, normal, risky
  newgame: ''
};

var extend = function (origin, add) {
  // Don't do anything if add isn't an object
  if (!add) {
    return origin;
  }
  var keys = Object.keys(add),
    i = keys.length;
  while (i) {
    i = i - 1;
    origin[keys[i]] = add[keys[i]];
  }
  return origin;
};

function ucci(path, options, uci) {
  this.path = path || 'eleeye.exe';
  this.options = extend(ucciOptions, options);
  this.defaults = {};
  this.id = { name: '', copyright: '', author: '', user: ''};
  this.child = null;
  this.state = EngineState.uninitialized;
  this.pondering = false;
  this.uci = uci;
}

// when ucci received feedback from the engine, it will emit event:
// 'uciok'
// 'ucciok'
// 'readyok'
// 'bye'
// 'option'
// 'id'
// 'bestmove'
// 'nobestmove'
// 'info'
// 'pophash'
// when engine state changed, it will emit event:
// 'engineState'
// when engine error occurs, it will emit event:
// 'engineError'
// when command send to engine, it will emit event:
// 'commandSent'
// when command reject due to incorrect state, it will emit event:
// 'commandReject'
util.inherits(ucci, EventEmitter);

ucci.prototype.changeEngineState = function (newState) {
  var oldState = this.state;
  if (oldState !== newState) {
    this.state = newState;
    this.emit('engineState', newState);
  }
};

ucci.prototype.applyOptions = function () {
  var opt;
  for (opt in this.options) {
    if (this.options.hasOwnProperty(opt) && this.defaults[opt] && (this.defaults[opt].value !== this.options[opt])) {
      this.setoption(opt, this.options[opt]);
    }
  }
};

// listen to the engine's feedback 
var onFeedback = function (self, data) {
  self.processFeedback(data);
};

var onError = function (self, data) {
  self.changeEngineState(EngineState.uninitialized);
};

var onExit = function (self, code) {
  self.changeEngineState(EngineState.uninitialized);
};

ucci.prototype.loadEngine = function (path) {
  this.path = path || this.path;
  var self = this;
  this.child = exec(this.path);

  this.child.stdout.on('data', function (data) {
    onFeedback(self, data);
  });
  this.child.stderr.on('data', function (data) {
    onError(self, data);
  });
  this.child.on('exit', function (code) {
    onExit(self, code);
  });

  this.changeEngineState(EngineState.loaded);
};

ucci.prototype.isCommandAccept = function (cmdName) {
  return CommandStateMap[this.state].indexOf(String(cmdName)) >= 0;
};

ucci.prototype.sendCommand = function (command) {
  var reg = /^\w*/,
    cmdName = command.match(reg);
  if (!this.isCommandAccept(cmdName)) {
    util.log('command <<' + cmdName + '>> not accept in current status: ' + this.status);
    this.emit('commandReject', command);
    return false;
  }
  this.emit('commandSent', command);
  this.child.stdin.write(command + '\r\n');
  return true;
};

// send 'ucci' to the engine, this command will boot the engine
// engine will reply a 'ucciok' after it startup
ucci.prototype.boot = function () {
  if (this.uci) {
    this.sendCommand('uci');
  } else {
    this.sendCommand('ucci');
  }  
};

// send 'quit' to the engine, engine will response this command 
// with a 'bye', then it will exit
ucci.prototype.quit = function () {
  this.sendCommand('quit');
};

// check the readiness of the engine
ucci.prototype.isready = function () {
  this.sendCommand('isready');
};

// set options
ucci.prototype.setoption = function (name, value) {
  var cmd = this.uci? 'setoption name ' : 'setoption ';
  this.sendCommand(cmd + name + ' ' + value);
};

ucci.prototype.position = function (fen, moves) {
  fen = !fen ? 'startpos' : 'fen ' + fen;
  var cmd = 'position ' + fen;
  if (!!moves) {
    cmd = cmd + ' moves ' + moves;
  }
  this.sendCommand(cmd);
};

ucci.prototype.banmoves = function (moves) {
  if (!!moves) {
    this.sendCommand('banmoves ' + moves);
  }
};

// option could be: 'ponder' or 'draw'
// modes could be: 'depth num|infinite' or 'nodes num' or 'time num'
ucci.prototype.go = function (option, modes) {
  var opt = option || '';
  if (opt === '') {
    sent = this.sendCommand('go ' + modes);
  } else {
    sent = this.sendCommand('go ' + opt + ' ' + modes);
  }

  if (sent) {
    this.pondering = opt === 'ponder';
    this.changeEngineState(EngineState.thinking);
  }
};

ucci.prototype.ponderhit = function (draw) {
  var cmd = 'ponderhit';
  if (!!draw) {
    cmd = cmd + ' draw';
  }
  if (!this.pondering) {
    this.emit('commandReject', cmd);
    return;
  }
  this.sendCommand(cmd);
};

ucci.prototype.stop = function () {
  this.sendCommand('stop');
};

ucci.prototype.probe = function (fen, moves) {
  fen = !fen ? 'startpos' : 'fen ' + fen;
  var cmd = 'probe ' + fen;
  if (!!moves) {
    cmd = cmd + ' moves ' + moves;
  }
  this.sendCommand(cmd);
};

// in most cases, engine process will exit after receive the 'quit' command.
// so, in those cases, no need to kill the process.
ucci.prototype.unloadEngine = function () {
  if (this.child && (this.status === EngineState.quit)) {
    this.child.kill();
  }
};

ucci.prototype.getDefaults = function (option) {
  return this.defaults[option];
};

ucci.prototype.getOption = function (option) {
  return this.options[option];
};

// dispatch feedback from the engine
ucci.prototype.processFeedback = function (data) {
  var lines = String(data).split(/\r\n|\n/),
    self = this;
  lines.forEach(function (line) {
    if (!line) {
      return;
    }
    var name = String(line).split(/\s+/, 1);
    self['on_' + name[0]](line);
    self.emit(name[0], line);
  });
};

// id has following property: { name, copyright, author, user} 
// a sample id information from the eleeye engine:
// id version 3.15
// id copyright 2004-2008 www.elephantbase.net
// id author Morning Yellow
// id user ElephantEye Test Team
ucci.prototype.on_id = function (line) {
  var arr = String(line).split(/\s+/),
    name = arr[1],
	  value = arr.slice(2).join(' ');
  this.id[name] = value;
};

// ucci sample options:
// option usemillisec type check default true
// option promotion type check default false
// option batch type check default false
// option debug type check default false
// option ponder type check default false
// uci sample option
// option name batch type check
//
ucci.prototype.on_option = function (line) {
  var arr = String(line).split(/\s+/),
    name = arr[1],
    type = arr[3],
    value = arr[arr.length - 1],
    i,
    min,
    max,
    choices = [];

  // uci option 
  if (name === 'name') {
    name = arr[2];
    type = arr[4];
  }  
    
  if (type === 'check') {
    value = (value === 'true') || (value === 'on');
    this.defaults[name] = {'type': type, 'value': value};
  } else if (type === 'button') {
    this.defaults[name] = {'type': type};
  } else if (type === 'spin') {
    min = arr[arr.indexOf('min') + 1];
    max = arr[arr.indexOf('max') + 1];
    this.defaults[name] = {'type': type, 'value': value, 'min': min, 'max': max };
  } else if (type === 'combo') {
    for (i = 0; i < arr.length - 1; i = i + 1) {
      if (arr[i] === 'var') {
        choices.push(arr[i + 1]);
        i = i + 1;
      }
    }
    this.defaults[name] = {'type': type, 'value': value, 'choices': choices };
  } else {
    this.defaults[name] = {'type': type, 'value': value};
  }
};

ucci.prototype.on_uciok = function (line) {
  this.uci = true;
  this.changeEngineState(EngineState.idle);
  this.applyOptions();
};

ucci.prototype.on_ucciok = function (line) {
  this.uci = false;
  this.changeEngineState(EngineState.idle);
  this.applyOptions();
};

ucci.prototype.on_readyok = function (line) {
};

ucci.prototype.on_bye = function (line) {
  this.changeEngineState(EngineState.uninitialized);
};

ucci.prototype.on_info = function (line) {

};

ucci.prototype.on_bestmove = function (line) {
  this.changeEngineState(EngineState.idle);
};

ucci.prototype.on_nobestmove = function (line) {
  this.changeEngineState(EngineState.idle);
};

ucci.prototype.on_pophash = function (line) {

};

exports.ucci = ucci;
