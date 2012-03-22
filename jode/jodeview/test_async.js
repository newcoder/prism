var async = require('./async');
var EventProxy = require('./eventproxy').EventProxy;


async.parallel([
    function(callback){
        setTimeout(function(){
			console.log('one done');
            callback(null, 'one');
        }, 200);
    },
	function(callback){
        setTimeout(function(){
 			console.log('two done');
			callback(null, 'two');
        }, 300);
    },
    function(callback){
        setTimeout(function(){
			console.log('three done');			
            callback(null, 'three');
        }, 100);
    },
],
// optional callback
function(err, results){
    // in this case, the results array will equal ['one','two', 'three']
    // because the functions were run in parallel and the second
    // function had a shorter timeout before calling the callback.
	console.log(err);
	console.log(results);
});


// an example using an object instead of an array
async.parallel({
    one: function(callback){
        setTimeout(function(){
		    console.log('one done');
            callback(null, 1);
        }, 400);
    },
    two: function(callback){
        setTimeout(function(){
		    console.log('two done');
            callback(1, 2);
        }, 100);
    },
	three: function(callback){
        setTimeout(function(){
		    console.log('three done');
            callback(null, 3);
        }, 30);
    },
},
function(err, results) {
    // results is now equals to: {one: 1, two: 2}
	console.log(err);
	console.log(results);
});

var ep = new EventProxy();

var one, two, three;
var done = function done(one, two, three) {
  console.log(one);
  console.log(two);
  console.log(three);
}

// you have to take care of the arguments order
ep.assign('two', 'one', 'three', done);

setTimeout(function(){
	console.log('one done');
	one = 'one';
	return;  //due to some error
	ep.trigger('one', one);
}, 2000);

setTimeout(function(){
	console.log('two done');
	two = 'two';
	ep.trigger('two', two);
}, 3000);

setTimeout(function(){
	console.log('three done');	
    three = 'three';	
	ep.trigger('three', three);
}, 10000);

 