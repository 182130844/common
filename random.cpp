#include <time.h>
#include "random.h"


namespace shadow {

	namespace random
	{
		//
		//  214013 and 2531011 come from vc++ rand() function
		//  the range of pseudorandom number is [0,32767]
		//

#define DEFAULT_BIT       16
#define DEFAULT_MAX       0x7fff
#define RESTRICT(s, b, m) ((s >> b) & m)
#define UPDATE_SEED(s)    s = (s * 214013 + 2531011)

		static unsigned long _seed = 0;

		static void shift() {
			int n = 1 + (time(0) % 10);
			for (int i = 0; i < n; i++) {
				UPDATE_SEED(_seed);
			}
		}

		void seed(int s) {
			_seed = s;
		}

		int rand() {
			shift();
			return RESTRICT(_seed, DEFAULT_BIT, DEFAULT_MAX);
		}

		int rand(int nMin, int nMax) {
			shift();
			int ret = RESTRICT(_seed, DEFAULT_BIT, DEFAULT_MAX);
			return nMin + (ret % (nMax - nMin + 1));
		}
	}
}
