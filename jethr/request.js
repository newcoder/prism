var http = require('http');

var options = {
  host: "3.87.248.6",
  port: 88,
  path: "http://www.google.com",
  headers: {
    Host: "www.google.com"
  }
};
http.get(options, function(res) {
  console.log(res);
  res.pipe(process.stdout);
});

//add comments 