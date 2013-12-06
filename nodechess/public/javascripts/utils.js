define(function (require, exports) {
  // Production steps of ECMA-262, Edition 5, 15.4.4.18
  // Reference: http://es5.github.com/#x15.4.4.18
  if (!Array.prototype.forEach) {
    Array.prototype.forEach = function (callback, thisArg) {
      var T, k, O, len, kValue;
      if (this === null) {
        throw new TypeError(" this is null or not defined");
      }
      O = this;
      len = O.length >>> 0; // Hack to convert O.length to a UInt32
      if ({}.toString.call(callback) !== "[object Function]") {
        throw new TypeError(callback + " is not a function");
      }
      if (thisArg) {
        T = thisArg;
      }
      k = 0;
      while (k < len) {
        if (O.hasOwnProperty(k)) {
          kValue = O[k];
          callback.call(T, kValue, k, O);
        }
        k = k + 1;
      }
    };
  }

});
