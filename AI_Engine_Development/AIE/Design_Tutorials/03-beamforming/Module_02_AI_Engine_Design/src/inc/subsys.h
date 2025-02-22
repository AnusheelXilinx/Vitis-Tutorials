
/*
Copyright (C) 2024, Advanced Micro Devices, Inc. All rights reserved.
SPDX-License-Identifier: MIT
*/

#ifndef __SUBSYS_H__
#define __SUBSYS_H__

#include <adf.h>
#include "include.h"
#include "kernels.h"

using namespace adf;

//-------------------------------------
//  Beamforming Cascading Chain
//-------------------------------------
template <int xoff, int yoff, int len> 
class bfCascadeChain: public graph {
private:
  kernel core[len];
public:
   input_port din[len];
   input_port cin[len];
   output_port out;
  
  bfCascadeChain() {
  
		// First kernel in the chain
		core[(yoff&1)*(len-1)]     = kernel::create(bf8x8_fst);   source(core[(yoff&1)*(len-1)])      = "bf8x8_fst.cc" ;
		
		// Middle kernels
		for(unsigned i=1;i<len-1;i++){
			core[i] = kernel::create(bf8x8_mid); source(core[i])  = "bf8x8_mid.cc" ;
		}
		
		// Last kernel in the chain
		core[(len-1)*(1-(yoff&1))] = kernel::create(bf8x8_lst);   source(core[(len-1)*(1-(yoff&1))])  = "bf8x8_lst.cc" ;
		initialization_function(core[(len-1)*(1-(yoff&1))]) = "bf8x8_init";
				
		// Define run-time ratio
		for(unsigned i=0;i<len;i++) runtime<ratio>(core[i])=0.8;
	
	
		// Make connections for input data and coefficient
		for(unsigned i=0;i<len;i++){
			connect(cin[i], core[i].in[0]);
			dimensions(core[i].in[0])={512};
			connect(din[i], core[i].in[1]);
			dimensions(core[i].in[1])={768};
		}

		// Connect Output
		connect(core[(len-1)*(1-(yoff&1))].out[0], out);			
		dimensions(core[(len-1)*(1-(yoff&1))].out[0])={768};	
		// Connect Cascading Bus
		for(unsigned i=0; i<len-1; i++){
			connect<cascade>s(core[(yoff&1)*(len-1)+(1-(yoff&1)*2)*i].out[0], core[(yoff&1)*(len-1)+(1-(yoff&1)*2)*(i+1)].in[2]); 
		}	
			
		
		// location constraints
		for(int i=0;i<len;i++){
			location<kernel>(core[i])        =  tile(xoff+i, yoff);
			location<stack>(core[i])         =  bank(xoff+i, yoff, 0);
			location<buffer>(core[i].in[0])  = {bank(xoff+i, yoff, 0), bank(xoff+i, yoff, 1)};
			location<buffer>(core[i].in[1])  = {bank(xoff+i, yoff, 2), bank(xoff+i, yoff, 3)};
		}
		
		// specify last stage output buffer location
		location<buffer>(core[(len-1)*(1-(yoff&1))].out[0]) = {bank(xoff+ (len-1)*(1-(yoff&1)), yoff, 2), bank(xoff+ (len-1)*(1-(yoff&1)), yoff, 3)};
	
   } ; 

};  // end of bfcore





//-------------------------------------
//  DL 64 Antenna 32 Layer
//-------------------------------------
template <int xoff, int yoff> 
class DL64A32L: public graph {
private:
  bfCascadeChain<xoff,   yoff+0, 4> bf0;
  bfCascadeChain<xoff,   yoff+1, 4> bf1;
  bfCascadeChain<xoff,   yoff+2, 4> bf2;
  bfCascadeChain<xoff,   yoff+3, 4> bf3;
  bfCascadeChain<xoff+4, yoff+0, 4> bf4;
  bfCascadeChain<xoff+4, yoff+1, 4> bf5;
  bfCascadeChain<xoff+4, yoff+2, 4> bf6;
  bfCascadeChain<xoff+4, yoff+3, 4> bf7;
public:
  input_port din[4];
  input_port cin[32];
  output_port out[8];
  
  DL64A32L(){
  
		// connect input data
		for(unsigned k=0;k<4;k++){
			connect(din[k],bf0.din[k]); connect(din[k],bf1.din[k]);
			connect(din[k],bf2.din[k]); connect(din[k],bf3.din[k]);
			connect(din[k],bf4.din[k]); connect(din[k],bf5.din[k]);
			connect(din[k],bf6.din[k]); connect(din[k],bf7.din[k]);			
		}
		
		// connect coefficients
		for(unsigned k=0;k<4;k++){
			connect(cin[4*0+k],bf0.cin[k]);
			connect(cin[4*1+k],bf1.cin[k]);
			connect(cin[4*2+k],bf2.cin[k]);
			connect(cin[4*3+k],bf3.cin[k]);
			connect(cin[4*4+k],bf4.cin[k]);
			connect(cin[4*5+k],bf5.cin[k]);
			connect(cin[4*6+k],bf6.cin[k]);
			connect(cin[4*7+k],bf7.cin[k]);			
		}		

		// connect output
		connect(bf0.out,out[0]); connect(bf1.out,out[1]);
		connect(bf2.out,out[2]); connect(bf3.out,out[3]);
		connect(bf4.out,out[4]); connect(bf5.out,out[5]);
		connect(bf6.out,out[6]); connect(bf7.out,out[7]);	
	
  }; 

};  // end of DL64A32L



//-------------------------------------
//  UL 64 Antenna 32 Layer
//-------------------------------------
template <int xoff, int yoff> 
class UL64A32L: public graph {
private:
  bfCascadeChain<xoff,   yoff+0, 8> bf0;
  bfCascadeChain<xoff,   yoff+1, 8> bf1;
  bfCascadeChain<xoff,   yoff+2, 8> bf2;
  bfCascadeChain<xoff,   yoff+3, 8> bf3;
public:
  input_port din[8];
  input_port cin[32];
  output_port out[4];
  
  UL64A32L(){
  
		// connect input data
		for(unsigned k=0;k<8;k++){
			connect(din[k],bf0.din[k]);
			connect(din[k],bf1.din[k]);
			connect(din[k],bf2.din[k]);
			connect(din[k],bf3.din[k]);		
		}
		
		// connect coefficients
		for(unsigned k=0;k<8;k++){
			connect(cin[8*0+k],bf0.cin[k]);
			connect(cin[8*1+k],bf1.cin[k]);
			connect(cin[8*2+k],bf2.cin[k]);
			connect(cin[8*3+k],bf3.cin[k]);	
		}

		// connect output
		connect(bf0.out,out[0]); connect(bf1.out,out[1]);
		connect(bf2.out,out[2]); connect(bf3.out,out[3]);
	
  }; 

};  // end of UL64A32L



#endif //__SUBSYS_H__
