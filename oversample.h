#include <complex>
#include "ap_fixed.h"
#include "ap_int.h"
#include <iostream>
using namespace std;

#define M 32            // Transform length
#define D 24            // Decimation rate
#define B 32            // Taps per branch
#define S 32            // Number of shift states
#define O (M*B-D)       // Offset
		
#define T (100*M*B)      // Length of input data
#define N ((T/D)*D)     // Truncate input data so N mod D = 0
#define F ((N-M*B)/D)   // # of data frames

typedef ap_fixed<16, 1, AP_RND_CONV, AP_SAT> iORq_t;

typedef iORq_t coeff_t;
typedef complex<iORq_t> indat_t;
typedef complex<iORq_t> outdat_t;

typedef ap_uint<8> apuint8_t;

typedef struct {
	indat_t val[D];
} iqgroupD_t;

typedef struct {
	indat_t val[M];
} iqgroupM_t;

typedef struct {
	iqgroupD_t data;
} opfbin_t;

typedef struct {
	iqgroupM_t data;
	apuint8_t user;
} opfbinout_t;



void osfilt(opfbin_t datain, opfbinout_t dataout, coeff_t coeffarr[M][B]);
