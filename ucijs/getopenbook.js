/*
  Get the master game book from the dpxq, about 46996 games
  It will be used to build a opening book database.
*/
var request = require('request'),
  iconv = require('iconv-lite'),
  cheerio = require('cheerio'),
  mongodb = require('mongodb'),
  jsdom = require('jsdom'),
  fs     = require('fs'),
  jquerystring = fs.readFileSync("./jquery-1.6.min.js").toString();
 
  
var proxy = 'http://3.87.248.6:88';
var site_url = 'http://www.dpxq.com/hldcg/search/view_m_';
  
var getDPBook = function getDPBook(url, num, callback) {
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
      bodystr = $('body').html(),
      movelist = bodystr.match(/\[DhtmlXQ_movelist\]\d+\[\/DhtmlXQ_movelist\]/); 
      // movelist encoded by coordinate , c0r0 => c1r1
      if (movelist.length > 0) {
        var moves = movelist[0].match(/\d+/),
          movestr = getMovelistString(moves[0], pstring);  
        if (callback) {
          callback(num, url, movestr);
        }
      }
  });
};

var getDPBook_jsdom = function getDPBook(url, num, callback) {
  var reqOptions = {uri: url, encoding: 'binary'};
  if (proxy !== '') reqOptions['proxy'] = proxy;

  var self = this;
  request(reqOptions, function(err, response, body){
    //Just a basic error check
    if(err && response.statusCode !== 200){ 
      console.log('Request error.');
	  return;
    }
    
    jsdom.env({
      html: body,
      src: [jquerystring],
      done: function (err, window) {
        var $ = window.jQuery,
          $body = $('body'),
          varstr = $body.find('#dhtmlxq_view').html(),
          title = getvar(varstr, 'title');
          console.log(varstr); 
          console.log(title);
          
          var bintitle = iconv.toEncoding(varstr, "binary");
          var newtitle = iconv.fromEncoding(bintitle, 'GBK');
          console.log(newtitle);
          //newtitle = iconv.fromEncoding(newtitle, 'GBK');
          //console.log(newtitle);
        new BookPersister().saveGameInfo(newtitle);
      }
    });
  });
};

var scrapeDPBooks = function (from, callback) {
  var i;
  for (i = from; i <= 46996; i = i + 1) {
    getDPBook(site_url + i + '.html', i, callback);
  }
};

var st_url = 'http://www.stqiyuan.com/game_view.asp?id=0120195F7A30FD';

var getSTBook = function getSTBook(url, callback) {
  var reqOptions = {uri: url, encoding: 'binary'};
  if (proxy !== '') reqOptions['proxy'] = proxy;

  var self = this;
  request(reqOptions, function(err, response, body){
    //Just a basic error check
    if(err && response.statusCode !== 200){ 
      console.log('Request error.');
	  return;
    }
   
    var $ = cheerio.load(body),
      bodystr = $('body').html(),
      movelist = bodystr.match(/\[DHJHtmlXQ_34\]\d+\[\/DHJHtmlXQ_34\]/);
      console.log(bodystr);
      //moves = movelist[0].match(/\d+/);
      // movelist encoded by coordinate , r0c0 => r1c1
    //console.log(moves[1]);
    //console.log(convertMoveList(moves[1]));
  });
};

var testMoves = '23246665102279672234174772421927706209191222897934264757808167558131694726345563242563426042472531712547000177730151394822268685515457676241191171721131345567685434311141224645504179763424271922341907264649395567111672527370405070303426767050517071515071705051303141303136';

// move conversion for ST book
function convertMoveList(moves) {
  var tmps = ""
  for(i = 0;i < moves.length / 2; i = i + 1) {
    tmps += "" + (9 - parseInt(moves.charAt(i*2+1))) + moves.charAt(i*2);
  }
  return tmps;
}

function BookPersister(options) {
  this.host = options == null || options.host == null? "127.0.0.1": options.host;
  this.port = options == null || options.port == null? 27017: options.port;
  this.database = options == null || options.database == null? 'test': options.database;
  this.server = new mongodb.Server(this.host, this.port, {auto_reconnect: true, pool_size: 4, 
    socketOptions: {keepAlive: 1}});
  this.connector = new mongodb.Db(this.database, this.server, {});

  var self = this;

  this.saveGame = function(num, url, moves) {
    if (self.db) {
	    return insertGame(self.db, num, url, moves);
	  }
    self.connector.open(function (error, db) { 
      if (error) throw error;   
      console.log('connected to database:' + self.database + '@' + self.host + ':' + self.port);
      self.db = db;
      insertGame(db, num, url, moves);
    });
  }; 
  
  var insertGame = function(db, num, url, moves) {
    var collGame = new mongodb.Collection(db, 'game');
	  collGame.insert({'num': num, 'url': url, 'moves': moves});
    console.log(url + '   done!');  
  };
  
  this.saveGameInfo = function(title) {
    if (self.db) {
	    return insertGameInfo(self.db, title);
	  }
    self.connector.open(function (error, db) { 
      if (error) throw error;   
      console.log('connected to database:' + self.database + '@' + self.host + ':' + self.port);
      self.db = db;
      insertGameInfo(db, title);
    });
  }; 
  
  var insertGameInfo = function(db, title) {
    var collGame = new mongodb.Collection(db, 'gameinfo');
	  collGame.insert({'title': title});
    console.log(title + '   done!');  
  };
}  

var pstring = '8979695949392919097717866646260600102030405060708012720323436383';

function getMovelistString(m0, p0) {
  var i,t=99,ms='';
  var p=' '.replace(/ /gi,' ').replace(/ /gi,'99').match(/\d\d/gi);
  if (p0=='') p0=B;
  p0=p0.match(/\d{2}/gi);
  for (i=0;i<32;i++) {
    p[p0[i]-0]=((100+i)+'').substr(1);
  }
  var m=m0.match(/\d{4}/gi);
  if (m==null) return '';
  var ml=m.length;
  for (i=0;i<ml;i++) {
    t=p[m[i].substr(0,2)-0];
    if ((t>=0)&&(t<=15)) ms+=c_move(getMoveText(m[i],p,0));//0\u7EA2
    else if ((t>=16)&&(t<=31)) ms+=c_move(getMoveText(m[i],p,1));//1\u9ED1
    else {ms+='\u7740\u6CD5\u9519\u8BEF';continue;}
    p[m[i].substr(2,2)-0]=p[m[i].substr(0,2)-0];
    p[m[i].substr(0,2)-0]='99';
  }
  return ms;
}

function c_move(m) {
  var b='\u8F66\u9A6C\u76F8\u8C61\u4ED5\u58EB\u5E05\u5C06\u70AE\u3000\u3000\u3000\u3000\u3000\u3000\u3000\u5175\u5352\u4E00\uFF11\u4E8C\uFF12\u4E09\uFF13\u56DB\uFF14\u4E94\uFF15\u516D\uFF16\u4E03\uFF17\u516B\uFF18\u4E5D\uFF19\u524D\u4E2D\u540E\u8FDB\u9000\u5E73';
  var s='',n,x;
  for (x=0;x<4;x++) {
    n = m.charCodeAt(x)-49;
    s += b.charAt(n);
  }
  return s;
}

function getMoveText(s,p,b) {
  var tP,m,i,I=' ';
  var t=' UW| UVW| UEGW| UEGIW||| WU| WVU| WGEU| WIGEU'.split('|');
  var f='12357532199AAAAA12468642199BBBBB';
  var r='CDEFGHIJKLMNOPQRST';
  var fC=s.charAt(0)-0;
  var fR=s.charAt(1)-0;
  var tC=s.charAt(2)-0;
  var tR=s.charAt(3)-0;
  var fP=f.charAt(p[s.substr(0,2)-0]);
  if (b==0) {
    m=fP+r.charAt((8-fC)*2);
    if ((fP=='1')||(fP=='2')||(fP=='9')||(fP=='A')) {
      for (i=0;i<=9;i++) {
        tP=p[fC*10+i]; 
        if ((tP<16)&&(f.charAt(tP)==fP)) I+=i;
      }
      if (I.length>=3) {
        m=t[I.length-3].charAt(I.indexOf(fR));
        if (I.length<=4&&fP=='A') {
          p=p.join('_')+'_';
          m+=(('|0'+p.indexOf('11_')/3+'|0'+p.indexOf('12_')/3+'|0'+p.indexOf('13_')/3+'|0'+p.indexOf('14_')/3+'|0'+p.indexOf('15_')/3+'|').match(/\|\d{2,3}/gi).join('').replace(/\|\d?(\d)\d/gi,'$1').replace(eval('/'+fC+'/gi'),'').search(/(\d).*\1/gi)!=-1)?(r.charAt(16-fC*2)):fP;
        }
        else m+=fP;
      }
    }
    if (fR==tR) m+='Z'+r.charAt((8-tC)*2);
    else if (fR>tR) m+='X'+r.charAt(((fP>1)&&(fP<7))?((8-tC)*2):((fR-tR-1)*2+b));
    else m+='Y'+r.charAt(((fP>1)&&(fP<7))?((8-tC)*2):((tR-fR-1)*2+b));
  }
  else {
    m=fP+r.charAt(fC*2+b);
    if ((fP=='1')||(fP=='2')||(fP=='9')||(fP=='B')) {
      for (i=0;i<=9;i++) {
        tP=p[fC*10+i]; 
        if ((tP>15)&&(tP<32)&&(f.charAt(tP)==fP)) I+=i;
      }
      if (I.length>=3) {
        m=t[I.length+3].charAt(I.indexOf(fR));
        if (I.length<=4&&fP=='B') {
          p=p.join('_')+'_';
          m+=(('|0'+p.indexOf('27_')/3+'|0'+p.indexOf('28_')/3+'|0'+p.indexOf('29_')/3+'|0'+p.indexOf('30_')/3+'|0'+p.indexOf('31_')/3+'|').match(/\|\d{2,3}/gi).join('').replace(/\|\d?(\d)\d/gi,'$1').replace(eval('/'+fC+'/gi'),'').search(/(\d).*\1/gi)!=-1)?(r.charAt(fC*2+1)):fP;
        }
        else m+=fP;
      }
    }
    if (fR==tR) m+='Z'+r.charAt(tC*2+b);
    else if (fR>tR) m+='Y'+r.charAt(((fP>1)&&(fP<7))?(tC*2+b):((fR-tR-1)*2+b));
    else m+='X'+r.charAt(((fP>1)&&(fP<7))?(tC*2+b):((tR-fR-1)*2+b));
  }
  return m;
}

function getvar(varstr, v) {
  var r=eval('/\\[(DhtmlXQ_'+v+')\\](.*?)\\[\\/\\1\\]/gi');
  var s=varstr.match(r);
  return (s==null)?'':s[0].replace(r,'$2').replace(/\|\|/gi,'\r\n');
}

//console.log(getMovelistString(movestring, pstring));
//scrapeDPBooks(1, new BookPersister().saveGame);
//scrapeDPBooks(46996, null);
//getDPBook(site_url + 46988 + '.html', 46988, function(num, url, movestr) {
//  console.log(num);
//  console.log(url);
//  console.log(movestr);
//});

getDPBook_jsdom(site_url + 46988 + '.html', 46988, null);

//getSTBook(st_url, null);
//console.log(convertMoveList(testMoves));