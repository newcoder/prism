var jobscraper = require('./jobscraper')
,persist = require('./persist.js');
var jobpersist = new persist.Persist();
var scraper = new jobscraper.JobScraper({proxy: 'http://3.87.248.6:88'});
//var scraper = new jobscraper.JobScraper();
//scraper.getJobPost('/xiaoshouyuan/', jobpersist.saveJobPosts);
//scraper.getJobList(jobpersist.saveJobCats);

//jobpersist.getJobByName(null,null);


//scrape ten pages for all jobs from the site.
jobpersist.getJobByName(null,function(jobList) {
  jobList.forEach(function(job) {
    [4,5,6,7,8,9,10].forEach(function(i) {
	  scraper.getJobPost(job, i, jobpersist.saveJobPosts);
	});
  });
})


//jobpersist.terminate();

/*
var GB2312UTF8 = require('./GB2312UTF8').GB2312UTF8;
//test:
//  GBK => UTF8:

//  UTF8 => GBK:
  var gbk = GB2312UTF8.UTF8ToGB2312('Ê±¼ä');
  console.log(gbk);
  
  var utf8 = GB2312UTF8.GB2312ToUTF8(gbk);
  console.log(utf8);
*/