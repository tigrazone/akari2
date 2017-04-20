#ifndef _SAMPLING_H_
#define _SAMPLING_H_

#include "random.h"
#include "constant.h"

namespace hstd {
	

	#if 1	
inline void sincosf(const float x, float* outSin, float* outCos)
  {
float ss,cc;

__asm__ ("fsincos;" : "=t" (cc), "=u" (ss) : "0" (x));

(*outSin)=(float)ss;
(*outCos)=(float)cc;
}
#else
#define FLOAT float
	inline void sincosf(const FLOAT x, FLOAT *ss, FLOAT *cc) {

		FLOAT tsin, tcos;

		__asm
		{
			fninit;
			fld dword ptr x;
			fsincos;
			fstp dword ptr tcos;
			fst dword ptr tsin;
		}

		*ss = tsin;
		*cc = tcos;
	}
#endif

template <class Random, typename T>
class GenericSampling {
public:
	//tigra: use in cam path
	static void uniformCircle(Random &random, T *x, T *y) {
		const T sqrt_r= sqrt(random.next01());
		const T theta = random.next(0.0f, kPI2);
		
		float ss,cc;
		
		sincosf(theta,&ss, &cc);

		*x = sqrt_r * cc;
		*y = sqrt_r * ss;
	}
	
	//tigra: not use
	static Vec3<T> uniformSphereSurface(Random &random) {
		const T tz = random.next(-1.0f, 1.0f);
		const T phi = random.next(0.0f, kPI2);
		const T k = sqrt(1.0f - tz * tz);
		
		
		float ss,cc;
		
		sincosf(phi,&ss, &cc);
		
		const T tx = k * cc;
		const T ty = k * ss;

		return Vec3<T>(tx, ty, tz);
	}
	
	//tigra: not use, commented
	static Vec3<T> uniformHemisphereSurface(Random &random, const Vec3<T> &normal, const Vec3<T> &tangent, const Vec3<T> &binormal) {
		const T tz = random.next(0.0f, 1.0f);
		const T phi = random.next(0.0f, kPI2);
		const T k = sqrt(1.0f - tz * tz);
		
		//tigra: use sincos
		
		float ss,cc;
		
		sincosf(phi,&ss, &cc);
		
		
		const T tx = k * cc;
		const T ty = k * ss;

		return tz * normal + tx * tangent + ty * binormal;
	}

	//tigra: use LambertianBRDF::sample
	static Vec3<T> cosineWeightedHemisphereSurface(Random &random, const Vec3<T> &normal, const Vec3<T> &tangent, const Vec3<T> &binormal) {
		const T phi = random.next(0.0f, kPI2);
		const T r2 = random.next01(), r2s = sqrt(r2);
		//tigra: use sincos
		
		float ss,cc;
		
		sincosf(phi,&ss, &cc);
		
		const T tx = r2s * cc;
		const T ty = r2s * ss;
		const T tz = sqrt(1.0f - r2);

		return tz * normal + tx * tangent + ty * binormal;
	}
};

typedef GenericSampling<Random, float> Sampling;

}

#endif // _SAMPLING_H_