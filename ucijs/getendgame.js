/*
  Get the end game database from the forthfreak
  It will be used to build a end game table.
*/
var request = require('request'),
  cheerio = require('cheerio'),
  mongodb = require('mongodb'),
  jsdom = require('jsdom'),
  fs     = require('fs'),
  jquerystring = fs.readFileSync("./jquery-1.6.min.js").toString();
 
  
var proxy = 'http://3.87.248.6:88';
var site_url = 'http://lpforth.forthfreak.net/';
  
var getEndGame = function getEndGame(url, callback) {
  var reqOptions = {uri: url};
  if (proxy !== '') reqOptions['proxy'] = proxy;

  var self = this;
  request(reqOptions, function(err, response, body){
    //Just a basic error check
    if(err && response.statusCode !== 200){ 
      console.log('Request error.');
	  return;
    }

    var $ = cheerio.load(body),
      $body = $('body'),
      $items = $body.find('tr');
    $items.each(function(i, item) {
      var eg = {}, 
        tds = $(item).find('td');
      if (i === 0) {
      // skip the header
        return;
      }
      eg.originname = $(tds[0]).find('a').text();
      eg.name = convertName(eg.originname);
      eg.order = $(tds[2]).text();
      eg.redwin = $(tds[3]).text();
      eg.redlongestwin = $(tds[4]).text();
      eg.blackdraw = $(tds[5]).text();
      eg.blacklongestwin = $(tds[6]).text();
      eg.date = $(tds[8]).text();
      
      if (callback) {
        callback(eg);
      }
    });
  });
};

/*
notation used in naming the end game of the web site
k > r > c > n > p > b > e
K, k -- (k)ing
B, b -- (b)ishop, assistant, guard
E, e -- (e)lephant, minister
R, r -- (r)ook, chariot, car
N, n -- k(n)ight, horse
C, c -- (c)annon, gun
P, p -- (p)awn

we use following notation:
Red  Black
K,    k -- (k)ing
A,    a -- (a)ssistant, guard
B,    b -- elephant, minister
R,    r -- (r)ook, chariot, car
N,    n -- k(n)ight, horse
C,    c -- (c)annon, gun
P,    p -- (p)awn

we use uppercase for red pieces
name 'kreekpb' will transfer to 'KRBBkpa', we always have K/k, so we will drop them in the name
so the name become 'RBBpa'
*/
function convertName(name) {
  var i, newname = [], red = true;
  if (name.length <= 1) return;
  for (i = 1; i < name.length; i = i + 1) {
    if (red) {
      if (name[i] === 'b') {
        newname.push('A');
      } else if (name[i] === 'e') {
        newname.push('B');
      } else if (name[i] !== 'k') {
        newname.push(name[i].toUpperCase());
      } else {
        red = false;
      }
    } else {
      if (name[i] === 'b') {
        newname.push('a');
      } else if (name[i] === 'e') {
        newname.push('b');
      } else if (name[i] !== 'k') {
        newname.push(name[i]);
      } else {
      }
    } 
  }
  return newname.join('');
}

function EndGamePersister(options) {
  this.host = options == null || options.host == null? "127.0.0.1": options.host;
  this.port = options == null || options.port == null? 27017: options.port;
  this.database = options == null || options.database == null? 'test': options.database;
  this.server = new mongodb.Server(this.host, this.port, {auto_reconnect: true, pool_size: 4, 
    socketOptions: {keepAlive: 1}});
  this.connector = new mongodb.Db(this.database, this.server, {});

  var self = this;

  this.saveEndGame = function(eg) {
    if (self.db) {
	    return insertEndGame(self.db, eg);
	  }
    self.connector.open(function (error, db) { 
      if (error) throw error;   
      console.log('connected to database:' + self.database + '@' + self.host + ':' + self.port);
      self.db = db;
      insertEndGame(db, eg);
    });
  }; 
  
  var insertEndGame = function(db, eg) {
    var collGame = new mongodb.Collection(db, 'endgame');
	  collGame.insert({'name': eg.name, 'originname': eg.originname, 'order': eg.order, 'redwin': eg.redwin, 
    'redlongestwin': eg.redlongestwin, 'blackdraw': eg.blackdraw, 'blacklongestwin': eg.blacklongestwin, 'date': eg.date});
    console.log(eg.name + '   saved!');  
  };

}

var scrapeEndGames = function (callback) {
  getEndGame(site_url + 'available-rook-js.html', callback);
  getEndGame(site_url + 'available-cannon-js.html', callback);
  getEndGame(site_url + 'available-knight-js.html', callback);
  getEndGame(site_url + 'available-pawn-js.html', callback);
  getEndGame(site_url + 'available-misc-js.html', callback);
 
};

scrapeEndGames(new EndGamePersister().saveEndGame);




