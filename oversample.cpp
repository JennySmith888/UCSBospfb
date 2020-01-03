#include <stdio.h>
#include <iostream>
#include "oversample.h"

#define BUFFSIZE M //TODO

void circbuff(indat_t buff[BUFFSIZE], int shift){

	indat_t temp[BUFFSIZE];

	for(int i=0;i<shift;i++)
		temp[BUFFSIZE+i-shift]=buff[i];

	for(int i=shift; i<BUFFSIZE;i++)
		temp[i-shift]=buff[i];
	
	for(int i=0;i<BUFFSIZE;i++)
		buff[i]=temp[i];
}

void circbuff_switch(indat_t buff[BUFFSIZE], int shift){

	indat_t temp[BUFFSIZE];
	switch (shift) {
		case 0:
			return;
			break;
		case 2:
			for(int i=0;i<2;i++) temp[BUFFSIZE+i-2]=buff[i];
			for(int i=2; i<BUFFSIZE;i++) temp[i-2]=buff[i];
			break;
		case 4:
			for(int i=0;i<4;i++) temp[BUFFSIZE+i-4]=buff[i];
			for(int i=4; i<BUFFSIZE;i++) temp[i-4]=buff[i];
			break;
		case 8:
			for(int i=0;i<8;i++) temp[BUFFSIZE+i-8]=buff[i];
			for(int i=8; i<BUFFSIZE;i++) temp[i-8]=buff[i];
			break;
		default:
			return;
	}

	for(int i=0;i<BUFFSIZE;i++)
		buff[i]=temp[i];
}


void vector_multiply2d(indat_t a[M][B], coeff_t b[M][B], indat_t out[M][B]) {
	//multiply reg1 by filter & store in filt out
	vecmul_col: for(int col=0; col<B; col++){
		vecmul_row: for(int row =0; row<M; row++) {
			out[row][col] = a[row][col]*b[row][col];
		}
	}
}

void vector_multiply(indat_t a[M*B], coeff_t b[M*B], indat_t out[M*B]) {
	//multiply reg1 by filter & store in filt out
	vecmul: for(int i=0; i<B*M; i++){
		out[i] = a[i]*b[i];
	}
}

void sum_rows_and_flip(indat_t a[M][B], indat_t out[M]){
	//sum along the rows and flip result up/down
	sumrow: for(int row=0; row<M; row++){
		indat_t sum;
		sum=0;
		sumcol: for(int col=0; col<B; col++) sum += a[M-1-row][col];
		out[row]=sum;
	}
}

void sum_rows_and_flip_1d(indat_t a[M*B], indat_t out[M]){
	//sum along the rows and flip result up/down
	//If matrix has size, n by m [i.e. i goes from 0 to (n-1) and j from 0 to (m-1) ], then:
	//A[N][M]: A[ i ][ j ] = array[ i*m + j ].
	indat_t sum;
	sumrow: for(int row=0; row<M; row++){
		sum=0;
		sumcol: for(int col=0; col<B; col++) {
			sum += a[(M-1-row)*M+col];
		}
		out[row]=sum;
	}
}

void reshape2d(indat_t a[M*B], indat_t out[M][B]) {
	int j=0;
	reshapecol: for (int col=0;col<B;col++){
		reshaperow: for (int row=0;row<M;row++){
			out[row][col]=a[j];
			j++;
		}
	}
}

void osfilt(opfbin_t datain, opfbinout_t dataout, coeff_t coeffarr[M*B]) {
#pragma HLS PIPELINE II=1
#pragma HLS INTERFACE axis port=datain
#pragma HLS INTERFACE axis port=dataout
#pragma HLS DATA_PACK variable=datain
#pragma HLS DATA_PACK variable=dataout
#pragma HLS ARRAY_PARTITION variable=coeffarr block factor=24 dim=1  //maybe this

	static apuint8_t group_cnt=0;
	static indat_t datarr[M*B]; //af_in_vec
	//indat_t reg1[M][B];
	indat_t filtout[M*B];
	indat_t circbuffin[M];
	static ap_uint<4> shiftstate[S]={0,2,4,8};
	static ap_uint<3> state_idx=0;
	//indat_t circbuffout[M];


	// move the D samples forward
	samplesforward: for(int i=0;i<(M*B)-D;i++) datarr[D+i]=datarr[i];

	// load in the next D samples (this will be done F times)
	loadnew: for(int i=0;i<D; i++) datarr[i]=datain.data.val[i];

//  reshape data array into reg1
//	reshape2d(datarr, reg1);
//	vector_multiply2d(reg1, coeffarr, filtout);
//	sum_rows_and_flip(filtout, circbuffin);

	vector_multiply(datarr, coeffarr, filtout);
	sum_rows_and_flip_1d(filtout, circbuffin);

	//shift samples
	circbuff_switch(circbuffin, shiftstate[state_idx]);
	
	dataout: for(int i=0;i<M;i++) dataout.data.val[i]=circbuffin[i];
	dataout.user=group_cnt++;

	group_cnt = group_cnt >= apuint8_t(3) ?  apuint8_t(0) : apuint8_t(group_cnt+apuint8_t(1));
	
	//increment state index
	if (state_idx >= S) state_idx=0;
	else state_idx++;

	
}
