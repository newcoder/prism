// Copyright 2013, QT Inc.
// All rights reserved.
//
// Author: wndproc@google.com (Ray Ni)
//
// routines for Fast Fourier Transform

#ifndef FFT_H
#define FFT_H

#include <complex>
#include <cmath>
#include <iterator>

namespace prism {

	const double PI = 3.1415926536;

	inline unsigned int bitReverse(unsigned int x, int log2n)
	{
		int n = 0;
		int mask = 0x1;
		for (int i=0; i < log2n; i++)
		{
			n <<= 1;
			n |= (x & 1);
			x >>= 1;
		}
		return n;
	}

	template<class Iter_T>
	void FFT(Iter_T a, Iter_T b, int log2n)
	{
		typedef std::iterator_traits<Iter_T>::value_type complex;
		const complex J(0, 1);
		int n = 1 << log2n;
		for (int i=0; i < n; ++i) 
		{
			b[bitReverse(i, log2n)] = a[i];
		}

		for (int s = 1; s <= log2n; ++s)
		{
			int m = 1 << s;
			int m2 = m >> 1;
			complex w(1, 0);
			complex wm = exp(-J * (PI / m2));
			for (int j=0; j < m2; ++j)
			{
				for (int k=j; k < n; k += m)
				{
					complex t = w * b[k + m2];
					complex u = b[k];
					b[k] = u + t;
					b[k + m2] = u - t;
				}
				w *= wm;
			}
		}
	}

}

#endif