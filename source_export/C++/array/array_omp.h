/**
   \file array_omp.h
   \brief OpenMP Array Constructors.
   \author T.Streckenbach

   Array constructors using in omp loops.
   All the functions allocate memory by ompx_alloc(). 
*/

#ifndef ARRAY_OMP_H
#define ARRAY_OMP_H


#include "array.h"


PtrAWeak<DArray1> da1CreateOMP(int n1);
PtrAWeak<DArray2> da2CreateOMP(int n1, int n2);
PtrAWeak<DArray3> da3CreateOMP(int n1, int n2, int n3);
PtrAWeak<DArray4> da4CreateOMP(int n1, int n2, int n3, int n4);
PtrAWeak<DArray5> da5CreateOMP(int n1, int n2, int n3, int n4, int n5);

PtrAWeak<ZArray3> za3CreateOMP(int n1, int n2, int n3);

PtrAWeak<IArray1> ia1CreateOMP(int n1);
PtrAWeak<IArray2> ia2CreateOMP(int n1, int n2);
PtrAWeak<IArray3> ia3CreateOMP(int n1, int n2, int n3);


PtrAWeak<DArray2> da3FlattenOMP(const PtrA<DArray3>& a, int nrow, int ncol);
PtrAWeak<DArray3> da4FlattenOMP(const PtrA<DArray4>& a, int n1, int n2, int n3);


PtrAWeak<DArray1> da1PartOMP(const PtrA<DArray1>& a, int i1, int n1);
PtrAWeak<DArray2> da2PartOMP(const PtrA<DArray2>& a, int i1, int n1);



#endif

