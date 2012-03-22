// Print all of the news items on hackernews
var jsdom = require('jsdom');

jsdom.env({
  html: 'http://nj.58.com/yewu/',
  scripts: [
    'http://code.jquery.com/jquery-1.5.min.js'
  ],
  done: function(errors, window) {
    var $ = window.$;
    console.log('ְλ');
    $('#zhiweilist a').each(function() {
      console.log(' -', $(this).text());
    });
  }
});