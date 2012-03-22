// Print all of the news items on hackernews
var jsdom = require('jsdom');

jsdom.env({
  html: 'http://news.ycombinator.com/',
  scripts: [
    'http://code.jquery.com/jquery-1.5.min.js'
  ],
  done: function(errors, window) {
    var $ = window.$;
    console.log('HN Links');
    $('td.title:not(:last) a').each(function() {
      console.log(' -', $(this).text());
    });
  }
});