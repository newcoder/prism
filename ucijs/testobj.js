var util   = require('util');

function A()                        // Define super class
{
    this.x = 1;
}
 
A.prototype.DoIt = function()        // Define Method
{
    this.x += 1;
}
 
B.prototype = new A;                // Define sub-class
B.prototype.constructor = B;
function B()
{
    A.call(this);                    // Call super-class constructor (if desired)
    this.y = 2;
}
 
B.prototype.DoIt = function()        // Define Method
{
    A.prototype.DoIt.call(this);    // Call super-class method (if desired)
    this.y += 1;
}
 
b = new B;
a = new A;
a.DoIt();
b.DoIt();

util.log('A: \n' + util.inspect(a, false, null));
util.log('B: \n' + util.inspect(b, false, null));
 
