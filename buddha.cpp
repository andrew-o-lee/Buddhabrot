#include <stdio.h>
#include <string>
#include <math.h>
#include <limits.h>

/*
	clang++ buddha.cpp -std=c++11 -o buddha
	./buddha 2000 2000 10000000 500000 5000 500
	./buddha 2000 2000
*/

typedef struct {
	short red, green, blue; 
} Color;

typedef struct {
	double real, imag;
} Complex;

typedef struct {
	int red, green, blue, max;
} Threshold;

typedef struct {
	int width, height;
	Color** data;
} Canvas;

class Buddha {

private:
	Canvas canvas;
	Threshold threshold;
	int samples;

public:
	Buddha(Canvas canvas, Threshold threshold, int samples) {
		this->canvas = canvas;
		this->threshold = threshold;
		this->samples = samples;
		this->threshold.max = threshold.red;
		if (threshold.green > this->threshold.max) {
			this->threshold.max = threshold.green;
		}
		if (threshold.blue > this->threshold.max) {
			this->threshold.max = threshold.blue;
		}
	}

	void print_progress(int count) {
		printf("%i%%\r", count * 100 / samples);
	}

	// return a random complex number that is within a circle of
	// 	radius 2 and is probably not in the mandelbrot set.
	Complex mandel_rand() {
		bool cardiod, bulb_p2;
		Complex c;
		do {
			double theta = 2 * M_PI * ((double) rand() / RAND_MAX);
			double radius = 2 * ((double) rand() / RAND_MAX);
			c.real = cos(theta) * radius;
			c.imag = sin(theta) * radius;
			// https://en.wikipedia.org/wiki/Mandelbrot_set#Optimizations
			double q = pow(c.real - 0.25, 2) + pow(c.imag, 2);
			cardiod = q * (q + c.real - 0.25) <= 0.25 * pow(c.imag, 2);
			bulb_p2 = pow(c.real + 1, 2) + pow(c.imag, 2) <= (1 / 16);
		} while(cardiod || bulb_p2);
		return c;
	}

	void retrace(Complex* orbit, int length) {
		for (int i = 0; i < length; i++) {
			Complex c = orbit[i];
			int x = (int) (canvas.width * (c.real + 2) / 4);
			int y = (int) (canvas.height * (c.imag + 2) / 4);
			if (i < threshold.red) {
				canvas.data[x][y].red++;
			}
			if (i < threshold.green) {
				canvas.data[x][y].green++;
			}
			if (i < threshold.blue) {
				canvas.data[x][y].blue++;
			}
		}
	}

	void iterate() {
		Complex orbit[threshold.max];
		Complex z, c;
		int count = 0;
		while (count < samples) {
			z.real = 0;
			z.imag = 0;
			c = mandel_rand();
			for (int i = 0; i < threshold.max; i++) {
				double z_real_sq = z.real * z.real;
				double z_imag_sq = z.imag * z.imag;
				z.imag = 2 * z.real * z.imag + c.imag;
				z.real = z_real_sq - z_imag_sq + c.real;
				orbit[i] = z;
				if (z_real_sq + z_imag_sq > 4) {
					retrace(orbit, i - 1);
					count++;
					if (count % 10 == 0) {
						print_progress(count);
					}
					break;
				}
			}
		}
	}
};

// adjust histogram values
short smoothstep(short x) {
	return (short) (120 / (1 + pow(2, -2 * (cbrt(x) - 5))));
}

Color prettify(Color color) {
	color.red = smoothstep(color.red);
	color.green = smoothstep(color.green);
	color.blue = smoothstep(color.blue);
	return color;
}

int main (int argc, char* argv[]) {
	if (argc != 7 && argc != 3) {
		fprintf(stderr, "\nwidth height samples red green blue\n");
		return -1;
	}

	Canvas canvas;
	canvas.width = std::stoi(argv[1], nullptr, 0);
	canvas.height = std::stoi(argv[2], nullptr, 0);

	if (argc == 7) {
		// buddhabrot generation mode
		canvas.data = (Color**) malloc(sizeof(Color*) * canvas.width);
		for (int i = 0; i < canvas.width; i++) {
			canvas.data[i] = (Color*) calloc(canvas.height, sizeof(Color));
		}

		Threshold threshold;
		threshold.red = std::stoi(argv[4], nullptr, 0);
		threshold.green = std::stoi(argv[5], nullptr, 0);
		threshold.blue = std::stoi(argv[6], nullptr, 0);

		Buddha buddha (canvas, threshold, std::stoi(argv[3], nullptr, 0));
		buddha.iterate();

		FILE* f = fopen(".buddhadata", "wb");
		for (int x = 0; x < canvas.width; x++) {
			for (int y = 0; y < canvas.height; y++) {
				fwrite(&canvas.data[x][y], sizeof(short), sizeof(Color) / sizeof(short), f);
			}
			free(canvas.data[x]);
		}
		free(canvas.data);
		fclose(f);

		printf("\ndone!\n");
		return 0;
	} else {
		// buddhabrot render mode
		FILE* in_f = fopen(".buddhadata", "rb");
		if (!in_f) {
			fprintf(stderr, "\ngenerate buddhadata first\n");
			fprintf(stderr, "width height samples red green blue\n");
			return -1;
		}
		FILE* out_f = fopen("buddha.ppm", "wb");
		fprintf(out_f, "P6\t%i\t%i\t%i\n", canvas.width, canvas.height, SHRT_MAX);
		for (int x = 0; x < canvas.width; x++) {
			for (int y = 0; y < canvas.height; y++) {
				Color buffer;
				fread(&buffer, sizeof(short), sizeof(Color) / sizeof(short), in_f);
				Color color = prettify(buffer);
				fwrite(&color, sizeof(short), sizeof(Color) / sizeof(short), out_f);
			}
		}
		fclose(in_f);
		fclose(out_f);

		printf("done!\n");
		return 0;
	}
}









