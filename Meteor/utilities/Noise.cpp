#include "Noise.h"

#include <math.h>

namespace
{
	static const int permutations[] =
	{
		151,160,137,91,90,15, 131,13,201,95,96,53,194,233,7,225,140,36,
		103,30,69,142,8,99,37,240, 21,10,23,190,6,148,247,120,234,75,0,
		26,197,62,94,252,219,203,117, 35,11,32,57,177,33,88,237,149,56,
		87,174,20,125,136,171,168,68,175,74,165,71,134,139,48,27,166,77,
		146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,
		40,244,102,143,54,65,25,63,161,1,216,80,73,209,76,132,187,208,89,
		18,169,200,196,135,130,116,188,159,86,164,100,109,198,173,186,3,
		64,52,217,226,250,124,123,5,202,38,147,118,126,255,82,85,212,
		207,206,59,227,47,16,58,17,182,189,28,42,223,183,170,213,119,248,
		152,2,44,154,163,70,221,153,101,155,167,43,172,9,129,22,39,253,
		19,98,108,110,79,113,224,232,178,185,112,104,218,246,97,228,251,
		34,242,193,238,210,144,12,191,179,162,241,81,51,145,235,249,14,
		239,107,49,192,214,31,181,199,106,157,184,84,204,176,115,121,50,
		45,127,4,150,254,138,236,205,93,222,114,67,29,24,72,243,141,128,
		195,78,66,215,61,156,180,

		151,160,137,91,90,15, 131,13,201,95,96,53,194,233,7,225,140,36,
		103,30,69,142,8,99,37,240, 21,10,23,190,6,148,247,120,234,75,0,
		26,197,62,94,252,219,203,117, 35,11,32,57,177,33,88,237,149,56,
		87,174,20,125,136,171,168,68,175,74,165,71,134,139,48,27,166,77,
		146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,
		40,244,102,143,54,65,25,63,161,1,216,80,73,209,76,132,187,208,89,
		18,169,200,196,135,130,116,188,159,86,164,100,109,198,173,186,3,
		64,52,217,226,250,124,123,5,202,38,147,118,126,255,82,85,212,
		207,206,59,227,47,16,58,17,182,189,28,42,223,183,170,213,119,248,
		152,2,44,154,163,70,221,153,101,155,167,43,172,9,129,22,39,253,
		19,98,108,110,79,113,224,232,178,185,112,104,218,246,97,228,251,
		34,242,193,238,210,144,12,191,179,162,241,81,51,145,235,249,14,
		239,107,49,192,214,31,181,199,106,157,184,84,204,176,115,121,50,
		45,127,4,150,254,138,236,205,93,222,114,67,29,24,72,243,141,128,
		195,78,66,215,61,156,180
	};

	// the vertices of a cube used to make 3D gradients
	static const int gradient3D[12][3] =
	{
		{1, 1, 0}, {-1, 1, 0}, {1, -1, 0}, {-1, -1, 0},
		{1, 0, 1}, {-1, 0, 1}, {1, 0, -1}, {-1, 0, -1},
		{0, 1, 1}, {0, -1, 1}, {0, 1, -1}, {0, -1, -1},
	};

	// the vertices of a hypercube used to make 4D gradients
	static const int gradient4D[32][4] =
	{
		{0, 1, 1, 1},	{0, 1, 1, -1},	{0, 1, -1, 1},	{0, 1, -1, -1},
		{0, -1, 1, 1},	{0, -1, 1, -1},	{0,-1, -1, 1},	{0, -1, -1, -1},
		{1, 0, 1, 1},	{1, 0, 1, -1},	{1, 0, -1, 1},	{1, 0, -1, -1},
		{-1, 0, 1, 1},	{-1, 0, 1, -1},	{-1, 0, -1, 1},	{-1, 0, -1, -1},
		{1, 1, 0, 1},	{1, 1, 0, -1},	{1, -1, 0, 1},	{1, -1, 0, -1},
		{-1, 1, 0, 1},	{-1, 1, 0, -1},	{-1, -1, 0, 1},	{-1, -1, 0, -1},
		{1, 1, 1, 0},	{1, 1, -1, 0},	{1, -1, 1, 0},	{1, -1, -1, 0},
		{-1, 1, 1, 0},	{-1, 1, -1, 0},	{-1, -1, 1, 0},	{-1, -1, -1, 0},
	};

	// A lookup table to traverse the simplex around a given point in 4D.
	static const int simplex[64][4] =
	{
		{0,1,2,3}, {0,1,3,2}, {0,0,0,0}, {0,2,3,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,2,3,0},
		{0,2,1,3}, {0,0,0,0}, {0,3,1,2}, {0,3,2,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,3,2,0},
		{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0},
		{1,2,0,3}, {0,0,0,0}, {1,3,0,2}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {2,3,0,1}, {2,3,1,0},
		{1,0,2,3}, {1,0,3,2}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {2,0,3,1}, {0,0,0,0}, {2,1,3,0},
		{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0},
		{2,0,1,3}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {3,0,1,2}, {3,0,2,1}, {0,0,0,0}, {3,1,2,0},
		{2,1,0,3}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {3,1,0,2}, {0,0,0,0}, {3,2,0,1}, {3,2,1,0}
	};
}

inline double lerp(double a, double b, double t)
{
	return a + t * (b - a);
}

inline double dot(const int g[], double x, double y, double z)
{
	return g[0] * x + g[1] * y + g[2] * z;
}

inline double fade(double t)
{
	return t * t * t * (t * (t * 6 - 15) + 10);
}

// PERLIN NOISE
//----------------------------------------------------------------------------------------------------

double perlin_noise(double x, double y, double z)
{
	// convert point to coordinates in a unit cube
	int X = floor(x);
    int Y = floor(y);
    int Z = floor(z);

	x -= X;
	y -= Y;
	z -= Z;

	X &= 255;
	Y &= 255;
	Z &= 255;

	// hash out grid indices of cube corners
	int gi000 = permutations[X		+ permutations[Y		+ permutations[Z	]]] % 12;
	int gi001 = permutations[X		+ permutations[Y		+ permutations[Z + 1]]] % 12;
	int gi010 = permutations[X		+ permutations[Y + 1	+ permutations[Z	]]] % 12;
	int gi011 = permutations[X		+ permutations[Y + 1	+ permutations[Z + 1]]] % 12;
	int gi100 = permutations[X + 1	+ permutations[Y		+ permutations[Z	]]] % 12;
	int gi101 = permutations[X + 1	+ permutations[Y		+ permutations[Z + 1]]] % 12;
	int gi110 = permutations[X + 1	+ permutations[Y + 1	+ permutations[Z	]]] % 12;
	int gi111 = permutations[X + 1	+ permutations[Y + 1	+ permutations[Z + 1]]] % 12;

	// choose gradient values based on original input point
	double n000 = dot(gradient3D[gi000], x, y, z);
	double n100 = dot(gradient3D[gi100], x - 1, y, z);
	double n010 = dot(gradient3D[gi010], x, y - 1, z);
	double n110 = dot(gradient3D[gi110], x - 1, y - 1, z);
	double n001 = dot(gradient3D[gi001], x, y, z - 1);
	double n101 = dot(gradient3D[gi101], x - 1, y, z - 1);
	double n011 = dot(gradient3D[gi011], x, y - 1, z - 1);
	double n111 = dot(gradient3D[gi111], x - 1, y - 1, z - 1);

	// compute fade curves for x, y, and z
	double u = fade(x);
	double v = fade(y);
	double w = fade(z);

	// use lerp to create actual gradiation between chosen values
	return	lerp(
			lerp(lerp(n000, n100, u), lerp(n010, n110, u), v),
			lerp(lerp(n001, n101, u), lerp(n011, n111, u), v),
			w);
}

// SIMPLEX NOISE
//----------------------------------------------------------------------------------------------------
// all simplex_noise functions return values in [-1.0, 1.0]
//
// This code was placed in the public domain by its original author, Stefan Gustavson.

double simplex_noise_2d(double x, double y)
{
    // First the input is skewed (by s) to determine which simplex cell
	// the point falls.
	// Skew is used to convert between square (x,y) space
	// and the equilateral triangular simplex space.
	// This maps a pair of triangles (or rhombus) in simplex space,
	// and skews it to fit a square cell
    double F2 = 0.5 * (sqrtf(3.0) - 1.0);
    double s = (x + y) * F2;

    int i = floor(x + s);
    int j = floor(y + s);

	// unskew the cell origin to (x,y) space
    double G2 = (3.0 - sqrtf(3.0)) / 6.0;
    double t = (i + j) * G2;

    double X0 = i - t;
    double Y0 = j - t;

    // The x,y distances from the cell origin
    double x0 = x - X0;
    double y0 = y - Y0;

	// Determine which simplex we are in.

	// Offsets for second (middle) corner of simplex
    int i1, j1;
    if(x0 > y0) { i1 = 1; j1 = 0; }
    else		{ i1 = 0; j1 = 1; }

    // A step of (1,0) in (i,j) means a step of (1-c,-c) in (x,y), and
    // a step of (0,1) in (i,j) means a step of (-c,1-c) in (x,y), where
    // c = (3-sqrt(3))/6

	// Offsets for middle corner in (x,y) unskewed coords
    double x1 = x0 - i1 + G2;
    double y1 = y0 - j1 + G2;

	// Offsets for last corner in (x,y) unskewed coords
    double x2 = x0 - 1.0 + 2.0 * G2;
    double y2 = y0 - 1.0 + 2.0 * G2;

    // determine hashed gradient indices of the three simplex corners
    int ii = i & 255;
    int jj = j & 255;
    int gi0 = permutations[ii      + permutations[jj	 ]] % 12;
    int gi1 = permutations[ii + i1 + permutations[jj + j1]] % 12;
    int gi2 = permutations[ii + 1  + permutations[jj +  1]] % 12;

    // Calculate the contribution from the three corners
	double n0, n1, n2;
    double t0 = 0.5 - x0 * x0 - y0 * y0;
	if(t0 < 0.0) { n0 = 0.0; }
    else
	{
        t0 *= t0;
        n0 = t0 * t0 * (gradient3D[gi0][0] * x0 +
						gradient3D[gi0][1] * y0);
    }

    double t1 = 0.5 - x1 * x1 - y1 * y1;
	if(t1 < 0.0) { n1 = 0.0; }
    else
	{
		t1 *= t1;
		n1 = t1 * t1 * (gradient3D[gi1][0] * x1 +
						gradient3D[gi1][1] * y1);
	}

    double t2 = 0.5 - x2 * x2 - y2 * y2;
	if(t2 < 0.0) { n2 = 0.0; }
    else
	{
        t2 *= t2;
        n2 = t2 * t2 * (gradient3D[gi2][0] * x2 +
						gradient3D[gi2][1] * y2);
    }

    return 70.0 * (n0 + n1 + n2);
}

double simplex_noise_3d(double x, double y, double z)
{
	// Skew the input space to determine in which simplex cell
	// the input point falls.
	// as with 2d simplex noise, cube cells are
	// mapped to six tetrahedron cells
	double F3 = 1.0 / 3.0;
	double s = (x + y + z) * F3;

	int i = floor(x + s);
	int j = floor(y + s);
	int k = floor(z + s);

	// Unskew the cell origin back to (x,y,z) space
	double G3 = 1.0 / 6.0;
	double t = (i + j + k) * G3;
	double X0 = i - t;
	double Y0 = j - t;
	double Z0 = k - t;

	// The x,y,z distances from the cell origin
	double x0 = x - X0; 
	double y0 = y - Y0;
	double z0 = z - Z0;

	// Determine which simplex tetrahedron we are in.

	int i1, j1, k1; // Offsets for second corner of simplex in (i,j,k) coords
	int i2, j2, k2; // Offsets for third corner of simplex in (i,j,k) coords

	if(x0 >= y0)
	{
		if(y0 >= z0)		{ i1 = 1; j1 = 0; k1 = 0; i2 = 1; j2 = 1; k2 = 0; } // X Y Z order
		else if(x0 >= z0)	{ i1 = 1; j1 = 0; k1 = 0; i2 = 1; j2 = 0; k2 = 1; } // X Z Y order
		else				{ i1 = 0; j1 = 0; k1 = 1; i2 = 1; j2 = 0; k2 = 1; } // Z X Y order
	}
	else
	{
		if(y0 < z0)			{ i1 = 0; j1 = 0; k1 = 1; i2 = 0; j2 = 1; k2 = 1; } // Z Y X order
		else if(x0 < z0)	{ i1 = 0; j1 = 1; k1 = 0; i2 = 0; j2 = 1; k2 = 1; } // Y Z X order
		else				{ i1 = 0; j1 = 1; k1 = 0; i2 = 1; j2 = 1; k2 = 0; } // Y X Z order
	}

	// A step of (1,0,0) in (i,j,k) means a step of (1-c,-c,-c) in (x,y,z),
	// a step of (0,1,0) in (i,j,k) means a step of (-c,1-c,-c) in (x,y,z), and
	// a step of (0,0,1) in (i,j,k) means a step of (-c,-c,1-c) in (x,y,z), where
	// c = 1/6.

	// Offsets for second corner in (x,y,z) coords
	double x1 = x0 - i1 + G3;
	double y1 = y0 - j1 + G3;
	double z1 = z0 - k1 + G3;

	// Offsets for third corner in (x,y,z) coords
	double x2 = x0 - i2 + 2.0 * G3;
	double y2 = y0 - j2 + 2.0 * G3;
	double z2 = z0 - k2 + 2.0 * G3;

	// Offsets for last corner in (x,y,z) coords
	double x3 = x0 - 1.0 + 3.0 * G3;
	double y3 = y0 - 1.0 + 3.0 * G3;
	double z3 = z0 - 1.0 + 3.0 * G3;

	// hash out the gradient indices of the four simplex corners
	int ii = i & 255;
	int jj = j & 255;
	int kk = k & 255;
	int gi0 = permutations[ii	   + permutations[jj	  + permutations[kk		]]] % 12;
	int gi1 = permutations[ii + i1 + permutations[jj + j1 + permutations[kk + k1]]] % 12;
	int gi2 = permutations[ii + i2 + permutations[jj + j2 + permutations[kk + k2]]] % 12;
	int gi3 = permutations[ii + 1  + permutations[jj + 1  + permutations[kk +  1]]] % 12;

	// Calculate the contribution from the four corners
	double n0, n1, n2, n3;

	double t0 = 0.5 - x0 * x0 - y0 * y0 - z0 * z0;
	if(t0 < 0.0) { n0 = 0.0; }
	else
	{
		t0 *= t0;
		n0 = t0 * t0 * (gradient3D[gi0][0] * x0 +
						gradient3D[gi0][1] * y0 +
						gradient3D[gi0][2] * z0);
	}

	double t1 = 0.5 - x1 * x1 - y1 * y1 - z1 * z1;
	if(t1 < 0.0) { n1 = 0.0; }
	else
	{
		t1 *= t1;
		n1 = t1 * t1 * (gradient3D[gi1][0] * x1 +
						gradient3D[gi1][1] * y1 +
						gradient3D[gi1][2] * z1);
	}

	double t2 = 0.5 - x2 * x2 - y2 * y2 - z2 * z2;
	if(t2 < 0.0) { n2 = 0.0; }
	else
	{
		t2 *= t2;
		n2 = t2 * t2 * (gradient3D[gi2][0] * x2 +
						gradient3D[gi2][1] * y2 +
						gradient3D[gi2][2] * z2);
	}

	double t3 = 0.5 - x3 * x3 - y3 * y3 - z3 * z3;
	if(t3 < 0.0) { n3 = 0.0; }
	else
	{
		t3 *= t3;
		n3 = t3 * t3 * (gradient3D[gi3][0] * x3 +
						gradient3D[gi3][1] * y3 +
						gradient3D[gi3][2] * z3);
	}

	return 32.0 * (n0 + n1 + n2 + n3);
}

double simplex_noise_4d(double x, double y, double z, double w)
{
	// Skew the (x,y,z,w) space to determine which cell of 24 simplices we're in
	double F4 = (sqrt(5.0) - 1.0) / 4.0;
	double s = (x + y + z + w) * F4;
	int i = floor(x + s);
	int j = floor(y + s);
	int k = floor(z + s);
	int l = floor(w + s);

	// Unskew the cell origin back to (x,y,z,w) space
	double G4 = (5.0 - sqrt(5.0)) / 20.0;
	double t = (i + j + k + l) * G4;
	double X0 = i - t;
	double Y0 = j - t;
	double Z0 = k - t;
	double W0 = l - t;

	// The x,y,z,w distances from the cell origin
	double x0 = x - X0;
	double y0 = y - Y0;
	double z0 = z - Z0;
	double w0 = w - W0;

	// For the 4D case, the simplex is a hypertetrahedron
	// To find out which of the 24 possible simplices we're in, we need to
	// determine the magnitude ordering of x0, y0, z0 and w0.
	// The method below is a good way of finding the ordering of x,y,z,w and
	// then find the correct traversal order for the simplex we're in.
	// First, six pair-wise comparisons are performed between each possible pair
	// of the four coordinates, and the results are used to add up binary bits
	// for an integer index.

	int c1 = (x0 > y0) ? 32 : 0;
	int c2 = (x0 > z0) ? 16 : 0;
	int c3 = (y0 > z0) ? 8 : 0;
	int c4 = (x0 > w0) ? 4 : 0;
	int c5 = (y0 > w0) ? 2 : 0;
	int c6 = (z0 > w0) ? 1 : 0;
	int c = c1 + c2 + c3 + c4 + c5 + c6;

	// simplex[c] is a 4-vector with the numbers 0, 1, 2 and 3 in some order.
	// Many values of c will never occur, since e.g. x>y>z>w makes x<z, y<w and x<w
	// impossible. Only the 24 indices which have non-zero entries make any sense.
	// We use a thresholding to set the coordinates in turn from the largest magnitude.
    
	// The integer offsets for the second simplex corner
	// The number 3 in the "simplex" array is at the position of the largest coordinate.
	int i1 = (simplex[c][0] >= 3) ? 1 : 0;
	int j1 = (simplex[c][1] >= 3) ? 1 : 0;
	int k1 = (simplex[c][2] >= 3) ? 1 : 0;
	int l1 = (simplex[c][3] >= 3) ? 1 : 0;

	// The integer offsets for the third simplex corner
	// The number 2 in the "simplex" array is at the second largest coordinate.
	int i2 = (simplex[c][0] >= 2) ? 1 : 0;
	int j2 = (simplex[c][1] >= 2) ? 1 : 0;
	int k2 = (simplex[c][2] >= 2) ? 1 : 0;
	int l2 = (simplex[c][3] >= 2) ? 1 : 0;

	// The integer offsets for the fourth simplex corner
	// The number 1 in the "simplex" array is at the second smallest coordinate.
	int i3 = (simplex[c][0] >= 1) ? 1 : 0;
	int j3 = (simplex[c][1] >= 1) ? 1 : 0;
	int k3 = (simplex[c][2] >= 1) ? 1 : 0;
	int l3 = (simplex[c][3] >= 1) ? 1 : 0;

	// The fifth corner has all coordinate offsets = 1, so no need to look that up.

	// Offsets for second corner in (x,y,z,w) coords
	double x1 = x0 - i1 + G4;
	double y1 = y0 - j1 + G4;
	double z1 = z0 - k1 + G4;
	double w1 = w0 - l1 + G4;

	// Offsets for third corner in (x,y,z,w) coords
	double x2 = x0 - i2 + 2.0 * G4;
	double y2 = y0 - j2 + 2.0 * G4;
	double z2 = z0 - k2 + 2.0 * G4;
	double w2 = w0 - l2 + 2.0 * G4;

	// Offsets for fourth corner in (x,y,z,w) coords
	double x3 = x0 - i3 + 3.0 * G4;
	double y3 = y0 - j3 + 3.0 * G4;
	double z3 = z0 - k3 + 3.0 * G4;
	double w3 = w0 - l3 + 3.0 * G4;

	// Offsets for last corner in (x,y,z,w) coords
	double x4 = x0 - 1.0 + 4.0 * G4;
	double y4 = y0 - 1.0 + 4.0 * G4;
	double z4 = z0 - 1.0 + 4.0 * G4;
	double w4 = w0 - 1.0 + 4.0 * G4;

	// hash out gradient indices of the five simplex corners
	int ii = i & 255;
	int jj = j & 255;
	int kk = k & 255;
	int ll = l & 255;
	int gi0 = permutations[ii	   + permutations[jj	  + permutations[kk		 + permutations[ll	   ]]]] % 32;
	int gi1 = permutations[ii + i1 + permutations[jj + j1 + permutations[kk + k1 + permutations[ll + l1]]]] % 32;
	int gi2 = permutations[ii + i2 + permutations[jj + j2 + permutations[kk + k2 + permutations[ll + l2]]]] % 32;
	int gi3 = permutations[ii + i3 + permutations[jj + j3 + permutations[kk + k3 + permutations[ll + l3]]]] % 32;
	int gi4 = permutations[ii + 1  + permutations[jj + 1  + permutations[kk + 1  + permutations[ll +  1]]]] % 32;

	// Calculate the contribution from the five corners
	double n0, n1, n2, n3, n4;

	double t0 = 0.5 - x0 * x0 - y0 * y0 - z0 * z0 - w0 * w0;
	if(t0 < 0.0) { n0 = 0.0; }
	else
	{
		t0 *= t0;
		n0 = t0 * t0 * (gradient4D[gi0][0] * x0 +
						gradient4D[gi0][1] * y0 +
						gradient4D[gi0][2] * z0 +
						gradient4D[gi0][3] * w0);
	}

	double t1 = 0.5 - x1 * x1 - y1 * y1 - z1 * z1 - w1 * w1;
	if(t1 < 0.0) { n1 = 0.0; }
	else
	{
		t1 *= t1;
		n1 = t1 * t1 * (gradient4D[gi1][0] * x1 +
						gradient4D[gi1][1] * y1 +
						gradient4D[gi1][2] * z1 +
						gradient4D[gi1][3] * w1);
	}

	double t2 = 0.5 - x2 * x2 - y2 * y2 - z2 * z2 - w2 * w2;
	if(t2 < 0.0) { n2 = 0.0; }
	else
	{
		t2 *= t2;
		n2 = t2 * t2 * (gradient4D[gi2][0] * x2 +
						gradient4D[gi2][1] * y2 +
						gradient4D[gi2][2] * z2 +
						gradient4D[gi2][3] * w2);
	}

	double t3 = 0.5 - x3 * x3 - y3 * y3 - z3 * z3 - w3 * w3;
	if(t3 < 0.0) { n3 = 0.0; }
	else
	{
		t3 *= t3;
		n3 = t3 * t3 * (gradient4D[gi3][0] * x3 +
						gradient4D[gi3][1] * y3 +
						gradient4D[gi3][2] * z3 +
						gradient4D[gi3][3] * w3);
	}

	double t4 = 0.5 - x4 * x4 - y4 * y4 - z4 * z4 - w4 * w4;
	if(t4 < 0.0) { n4 = 0.0; }
	else
	{
		t4 *= t4;
		n4 = t4 * t4 * (gradient4D[gi4][0] * x4 +
						gradient4D[gi4][1] * y4 +
						gradient4D[gi4][2] * z4 +
						gradient4D[gi4][3] * w4);
	}

	return 27.0 * (n0 + n1 + n2 + n3 + n4);
}

// The higher the persistence, interval [0.0, 1.0], the more of each succeeding octave will be added
double octave_simplex_noise_4d(double numOctaves, double persistence, double scale, double x, double y, double z, double w)
{
    double sum = 0.0;
	double frequency = scale;
    double amplitude = 1.0;

	double combinedAmplitude = 0.0;

    for(int i = 0; i < numOctaves; i++)
	{
        sum += amplitude * simplex_noise_4d(x * frequency, y * frequency, z * frequency, w * frequency);
        frequency *= 2.0;
        combinedAmplitude += amplitude;
        amplitude *= persistence;
    }

    return sum / combinedAmplitude;
}
