var request = require('request'),
  cheerio = require('cheerio');

var options = {
  site: 'http://sh.58.com/job.shtml',
  proxy: 'http://3.87.248.6:88',
  pages: 10
};

var JobScraper = new Object();

JobScraper.get58JobList = function(options) {
  //Tell the request that we want to fetch youtube.com, send the results to a callback function
  request({uri: options.site,proxy: options.proxy }, function(err, response, body){

    //Just a basic error check
    if(err && response.statusCode !== 200){ 
      console.log('Request error.');
	  return;
    }
  
    var $ = cheerio.load(body);
	s = $('#types');
	console.log(s.toString());
    //$body = $('body');
    $jobtypes = $('#types').find('.types li');
    $jobtypes.each(function(i, item) {
	  var $cat_a = $(item).find('h2 a');
	  cat_href = $cat_a.attr('href'); 
	  cat_title = $cat_a.text();
	  console.log(cat_title + ':' + cat_href);
	
	  $jobs = $(item).find('p a');
	  $jobs.each(function(i, item) {
	    job_href = $(item).attr('href'); 
	    job_title = $(item).text();
	    console.log('  '+ job_title + ':' + job_href);
      });
	});
  });
}

JobScraper.get58JobList(options);

