var Jscex = require("jscex");
require("jscex-jit").init(Jscex);
require("jscex-async").init(Jscex);
require("jscex-async-powerpack").init(Jscex);

var test = function() { 
  setTimeout(function(){
	console.log('one done');
  }, 1000);

  setTimeout(function(){
	console.log('two done');
  }, 2000);

  setTimeout(function(){
	console.log('three done');	
  }, 3000);
  
};

var test_async = eval(Jscex.compile("async", function() {
  return $await(test); 
}));

var start = new Date();
test_async().start();
var execTime = new Date().getTime() - start.getTime();
console.log(execTime + " ms");


