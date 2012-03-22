var request = require('request'),
  cheerio = require('cheerio'),
  jsdom = require('jsdom'),
  fs     = require('fs');
var jquerystring = fs.readFileSync("./jquery-1.6.min.js").toString();
 
function JobScraper(options){
  var defaults = {
    site: 'http://sh.58.com/',
    jobListUrl: 'job.shtml',
	proxy: '',
    pages: 10
  };
    
  this.site = options == null || options.site == null? defaults.site: options.site;
  this.jobListUrl = options == null || options.jobListUrl == null? defaults.jobListUrl: options.jobListUrl;
  this.proxy = options == null || options.proxy == null? defaults.proxy: options.proxy;
  this.pages = options == null || options.page == null? defaults.page: options.page;  
  this.jobCats = new Array();
}

JobScraper.prototype.getJobList = function getJobList(callback) {
  var reqOptions = {uri: this.site + this.jobListUrl};
  if (this.proxy !== '') reqOptions['proxy'] = this.proxy;
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
	    //Use jQuery just as in any regular HTML page
	    var $ = window.jQuery,
        //var $ = cheerio.load(body),
	    $body = $('body');
		scrapeJobList(self, $, $body);
		//dumpJobList(self, './joblist.txt');
		if (callback) callback(self.jobCats);
	  }
	});
  });
}

var scrapeJobList = function scrapeJobList(self, $, pageBody) {
  var $jobtypes = pageBody.find('.types li');
  
  $jobtypes.each(function(i, item) {
    var $cat = $(item).find('h2 a');
    catHref = $cat.attr('href'); 
    catName = $cat.text();
    //console.log(catName + ':' + catHref);
	var catObj = self.jobCats[i] = new Object();
	catObj.href = catHref;
	catObj.name = catName;
	catObj.jobs = new Array();
	
    $jobs = $(item).find('p a');
    $jobs.each(function(j, item) {
	  jobHref = $(item).attr('href'); 
	  jobName = $(item).text();
	  //console.log('  '+ jobName + ':' + jobHref);
	  var jobObj = self.jobCats[i].jobs[j] = new Object();
	  jobObj.href = jobHref;
	  jobObj.name = jobName;
    });
  });
}

JobScraper.prototype.getJobPost = function getJobPost(job, pageNum, callback) {
  var reqOptions = {uri: this.site + job.href + (pageNum>=2? 'pn'+pageNum + '/': '')};
  if (this.proxy !== '') reqOptions['proxy'] = this.proxy;
  var self = this;
  request(reqOptions, function(err, response, body){
    //Just a basic error check
    if(err && response.statusCode !== 200){ 
      console.log('Request error.');
	  return;
    }
	
	var postItems = new Array();
	var start = new Date();
	var $ = cheerio.load(body),
    $body = $('body');
	var $items = $body.find('#maincon tr');
	$items.each(function(i, item) {
	  var post = new Object();
	  $a = $(item).find('td.t a');
	  post.title = $a.text();
	  post.href = $a.attr('href');
	  post.company = $(item).find('td a .fl').text();
	  post.location = $(item).find('td.tc').text();
	  post.postTime = $(item).find('td.pd').text();
	  postItems[i] = post;
	  //console.log(title + '\t'+ company + '\t'+location + '\t' +postTime + '\t' + href);
	});
	var end = new Date();
	console.log(end.getTime() - start.getTime() + ' ms');
	if (callback) callback(job, postItems);
  });
}

var dumpJobList = function dumpJobList(self, fileName) {
  var stream = fs.createWriteStream(fileName, {
    flags: "a",
    encoding: "encoding",
    mode: 0666
  });
  console.log('start dumping');
  self.jobCats.forEach(function(cat) {
    console.log(cat.name + '\t\t' + self.site + cat.href);
	stream.write(cat.name + '\t\t' + self.site + cat.href + '\r\n');
	cat.jobs.forEach(function(job) {
      console.log('\t\t' + job.name + '\t\t' + self.site + job.href);
	  stream.write('\t\t' + job.name + '\t\t' + self.site + job.href + '\r\n');
	});	  
  });
}

exports.JobScraper = JobScraper;


