#!/usr/bin/env python3
# Copyright (C) 2023, Advanced Micro Devices, Inc. All rights reserved.
# SPDX-License-Identifier: MIT

import os
import sys
import numpy as np

OK = 1

if (len(sys.argv) < 5):
    OK = 0
    print("P1", file = sys.stderr)
elif (len(sys.argv) != int(sys.argv[3])+4):
    OK = 0
    print("P2", file = sys.stderr)

if (OK == 0):
    print ("Usage: CreateData NbDataPerLine Base Dimensions LengthDim1 [LengthDim2 ...]", file = sys.stderr)
    exit(1)

NDataPerLine = int(sys.argv[1])
Base = int(sys.argv[2])
NDims = int(sys.argv[3])
DimL = np.zeros([NDims],dtype = int)
CumL = np.zeros([NDims],dtype = int)

print("Numerical Base: ", int(Base), file = sys.stderr)
print("Number of Dimensions: ",int(NDims), file = sys.stderr)
for i in range(int(sys.argv[3])):
    DimL[i] = int(sys.argv[i+4])
    print("Length Dimension ", str(i), ": ",str(DimL[i]), file = sys.stderr)

L = 1;
for i in range(NDims):
    CumL[i] = L
    L = L*DimL[i]

print("\nTotal Length: ", str(L), file = sys.stderr)
Data = np.zeros([L], dtype=int)
Mod = np.zeros([NDims],dtype = int)


for i in range(L):
    x = i
    for j in range(NDims-1,-1,-1):
        Mod[j] = x/CumL[j]
        x = x - CumL[j]*Mod[j]


    x = 0
    b = 1
    for j in range(NDims):
        x = x + b*Mod[j]
        b = b*Base
    Data[i] = x


for i in range(L):
    if(i%NDataPerLine != NDataPerLine-1):
        print(str(Data[i]),end = "\t")
    else:
        print(str(Data[i]))

print("")
