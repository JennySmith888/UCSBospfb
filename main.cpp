#include "oversample.h"
#include <iostream>
#include <stdio.h>


int main() {

    //Define dataset
    indat_t datain[N];
    FILE *input_data;
    input_data = fopen("in.dat","rb");
    fread(datain, N, 16, input_data);

    //Golden Data from MATLAB
    outdat_t dataout_expected[F*M];
    FILE *golden_data;
    golden_data = fopen("golden.dat", "rb");
    fread(dataout_expected, F*M, 16, golden_data);

    //Load coefficients
    coeff_t coeffarr[M][B] = {  // need to make this [M][B]
		#include "filt.dat"
      };
    coeff_t coeffarrflat[M*B];
	for(int row=0; row<M; row++){
		for(int col=0; col<B; col++)
			coeffarrflat[(row)*M+col]=coeffarr[row][col];
	};

    //Move data through function
    outdat_t dataout[N];
    opfbin_t buffin;
    opfbinout_t buffout;
    for (int i=0;i<F; i++) {
        for (int j=0;j<D; j++) buffin.data.val[j]=datain[O+j+i*D];
        osfilt(buffin, buffout, coeffarr);
        for (int j=0;j<D; j++) dataout[i*D+j]=buffout.data.val[j];
    };

    //Compare output with expected
    for(int i=0;i<N;i++) if (dataout[i]!=dataout_expected[i]) return 1;

    return 0;
}
