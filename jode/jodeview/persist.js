var mongodb = require('mongodb');

function Persist(options) {
  this.host = options == null || options.host == null? "127.0.0.1": options.host;
  this.port = options == null || options.port == null? 27017: options.port;
  this.database = options == null || options.database == null? 'test': options.database;
  this.server = new mongodb.Server(this.host, this.port, {auto_reconnect: true, pool_size: 4, 
    socketOptions: {keepAlive: 1}});
  this.connector = new mongodb.Db(this.database, this.server, {});

  var self = this;
  
  this.getJobPosts = function(query, limit, callback) {
    if (self.db)
	  return findJobPosts(self.db, query, limit, callback);
    self.connector.open(function (error, db) { 
      if (error) throw error;   
	  console.log('connected to database:' + self.database + '@' + self.host + ':' + self.port);
      self.db = db;
	  findJobPosts(self.db, query, limit, callback);
	});
  }; 
  
  var findJobPosts = function(db, query, limit, callback) {
    var collPost = new mongodb.Collection(db, 'post');
    var options = {
      'limit': limit,
      'skip': 10,
      'sort': 'title'
    }
	
	var posts;
	if (query == null){
	  posts = collPost.find(options);
	} else {
	  posts = collPost.find(query, options);
	}
	posts.toArray(function(err, docs) {
	  if (err) throw err;
      if (callback) callback(docs);
	});
  }
  
}

exports.Persist = Persist;


