
**A summary for what have been done in the prism project:** 
* Basic time series routines: normalization, distance, linear fit, local max/min, data reduction by turning points, piecewise, etc..
* A very versatile pattern searching algorithm.
* Basic OHLC data processing routines, split restore, shrink by week, by month, etc.  
* Centralized OHLC  data storage. OHLC data are stored in key-value database kyoto cabinet.
* A extensible indicator calculation framework, and implemented following indicator:  SMA, EMA, MACD, RSI, KDJ, CR(new), etc.
* A rule-based strategy and the back test framework for it.
