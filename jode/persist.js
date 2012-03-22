var mongodb = require('mongodb');

// dynamic schemas

// collection for job category
// collection name: category
// field: name string        --the name of job category
//        href string        --href to the job post page of the category
//        jobs object        --array of jobs under the category

// collection for job name list
// collection name: job
// field: name string        --the name of job
//        href string        --href to the job post page of the job
//        cat  string        --the category name

// collection for job post
// collection name: post
// field: title string       --the title of a job post
//        company string     --the organization 
//        location  string   --working location
//        postTime string    --post time
//        scrapeTime timeStamp    --time the post being scraped
//        href string        --url of the job post details
//        job string         --job name
//        cat string         --job category

function Persist(options) {
  this.host = options == null || options.host == null? "127.0.0.1": options.host;
  this.port = options == null || options.port == null? 27017: options.port;
  this.database = options == null || options.database == null? 'test': options.database;
  this.server = new mongodb.Server(this.host, this.port, {auto_reconnect: true, pool_size: 4, 
    socketOptions: {keepAlive: 1}});
  this.connector = new mongodb.Db(this.database, this.server, {});

  var self = this;

  this.saveJobCats = function(jobCats) {
    if (self.db) 
	  return insertJobCats(self.db, jobCats);
	self.connector.open(function (error, db) { 
      if (error) throw error;   
	  console.log('connected to database:' + self.database + '@' + self.host + ':' + self.port);
	  self.db = db;
	  insertJobCats(db, jobCats);
	});
  }; 
  
  var insertJobCats = function(db, jobCats) {
    var collCat = new mongodb.Collection(db, 'category');
    var collJob = new mongodb.Collection(db, 'job');
  
    collCat.remove();
    collJob.remove();
  
    var start = new Date();
    //insert all job categories
    jobCats.forEach(function(cat) {
	  //insert job name list to jobs collection
	  cat.jobs.forEach(function(job) {
	    collJob.insert({name: job.name, href: job.href, cat: cat.name});
	  });
	  //insert job category to job category collection
	  collJob.find({cat:cat.name}).toArray(function(err, docs) {
	    collCat.insert({name: cat.name, href: cat.href, jobs: docs});
	  });	  
    });  
    var end = new Date();
    console.log(end.getTime() - start.getTime() + ' ms');
  };
  
  this.saveJobPosts = function(job, jobPosts) {
    if (self.db)
	  return insertJobPosts(self.db, job, jobPosts);
    self.connector.open(function (error, db) { 
      if (error) throw error;   
	  console.log('connected to database:' + self.database + '@' + self.host + ':' + self.port);
      self.db = db;
	  insertJobPosts(self.db, job, jobPosts);
	});
  }; 
  
  var insertJobPosts = function(db, job, jobPosts) {
    var collPost = new mongodb.Collection(db, 'post');
	var start = new Date();
	jobPosts.forEach(function(post) {
	  var pt = post.postTime;
	  if (post.postTime.search(/今天|小时/) >= 0){
		var ptmon = new Date().getMonth()++,
		ptday = new Date().getDate();
		pt = ptmon + '-' + ptday;
	  }
      //insert job posts to the post collection
	  collPost.insert({title: post.title, 
		company: post.company, 
		location: post.location,
		postTime: pt,
		scrapeTime: new Date().getTime(),
		href: post.href,
		job: job.name,
		cat: job.cat});	  
	});  
	var end = new Date();
	console.log(end.getTime() - start.getTime() + ' ms');  
  };
  
  this.printJobCats = function() {
    if (self.db)
	  return outputJobCats(self.db);
    self.connector.open(function (error, db) { 
      if (error) throw error;   
	  console.log('connected to database:' + self.database + '@' + self.host + ':' + self.port);
      self.db = db;
	  outputJobCats(self.db);
	});
  }; 
  
  var outputJobCats = function(db) {
	var collCat = new mongodb.Collection(db, 'category');
    var collJob = new mongodb.Collection(db, 'job');
	  
	//print out  
    collJob.find().toArray(function(err, docs) {
      console.dir(docs);
    });	  
	collCat.find().toArray(function(err, docs) {
      console.dir(docs);
    });	    
  }

  this.getJobByCat = function(catName, callback) {
    if (self.db)
	  return findJobByCat(self.db, catName, callback);
    self.connector.open(function (error, db) { 
      if (error) throw error;   
	  console.log('connected to database:' + self.database + '@' + self.host + ':' + self.port);
      self.db = db;
	  findJobByCat(self.db, catName, callback);
	});
  };
  
  var findJobByCat = function(db, catName, callback) {
    var collJob = new mongodb.Collection(db, 'job');
    collJob.find({cat:catName}).toArray(function(err, docs) {
	  if (err) throw err;
      if (callback) callback(docs);
	});  
  }

  this.getJobByName = function(query, callback) {
    if (self.db)
	  return findJobByName(self.db, query, callback);
    self.connector.open(function (error, db) { 
      if (error) throw error;   
	  console.log('connected to database:' + self.database + '@' + self.host + ':' + self.port);
      self.db = db;
	  findJobByName(self.db, query, callback);
	});
  };
  
  var findJobByName = function(db, query, callback) {
    var collJob = new mongodb.Collection(db, 'job');
      
	var jobs;
	if (query == null){
	  jobs = collJob.find();
	} else {
	  jobs = collJob.find(query);
	}
	jobs.toArray(function(err, docs) {
	  if (err) throw err;
      if (callback) callback(docs);
	});
  }
  
}

exports.Persist = Persist;


