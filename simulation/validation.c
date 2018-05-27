#include <stdio.h>
#include <math.h>

int main() {
	double a;
	printf("t,a");
	for(unsigned int t = 120000; t < 180000; t++) {
		if(t >= 0 && t <= 16000) {
			a = 4.24130141489228 * pow(10, -5) * t + 1.40090814584814;
		} else if (t > 16000 && t <= 37200) {
			a = 3.4932435120246 * pow(10, -22) * pow(t, 6) - 6.0137525133258 * pow(10, -17) * pow(t, 5) + 4.27325583289267 * pow(10, -12) * pow(t, 4) - 1.60300051943013 * pow(10, -7) * pow(t, 3) + 0.00333952376594387 * pow(t, 2) - 36.3492749029025 * t + 160449.656477025;
		} else if (t > 37200 && t <= 40500) {
			a = -1.49350189856461 * pow(10, -14) * pow(t, 4) + 2.19414872603138 * pow(10, -9) * pow(t, 3) - 0.000125097528935285 * pow(t, 2) + 3.29683601045748*t - 31354.7202506453;
		} else if (t > 40500 && t <= 125100) {
			a = 2.172653597427 * pow(10, -8) * pow(t, 2) - 0.0282022850738493 * t + 3697.35684913407;
		} else if (t> 125100 && t <= 180000) {
			a = -9.91894988952945 * pow(10, -13) * pow(t, 3) + 4.97807830666201 * pow(10, -6) * pow(t, 2) - 0.0906045209884312 * t + 5961.90970204993;
			a = (a >= 0) ? a : 0;
		} else {
			a = 0;
		}
		if(t % 100 == 0) {
			printf("%lu - %f\n", t, a);
		}

	}
}
