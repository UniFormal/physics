#include <config.h>
#include <math.h>
#include "stdcxx/sig.h"
#include "array_stack.h"
#include "stdsys/dprintf.h"
#include "linalgdens.h"


template<class TARRAY,typename T> inline
PtrAWeak<TARRAY> new_stack_array(StackBase &s,const MemAccess2 &acs, T*mem=NULL, int memoffset=0)
{
  size_t nelem=acs.n_elem()+1;
  if(!mem)  mem = (T*) new(s) T[nelem];
  TARRAY * a = new(s) TARRAY;
  PtrWeak<MemDynamicVIRTUAL<T> > vir=new(s)MemDynamicVIRTUAL<T> (nelem,mem,0);
  a->vir=vir;
  a->v=a->vir->v;
  a->acs=acs;
  a->set_vector_default();
  return a;
}



PtrAWeak<IArray1> ia1CreateStack(StackBase &s, int n1)
{
  MemAccess2  acs(n1);
  return new_stack_array<IArray1,int>(s,acs);
}

PtrAWeak<IArray2> ia2CreateStack(StackBase &s, int n1, int n2)
{
  MemAccess2  acs(n1,n2);
  return new_stack_array<IArray2,int>(s,acs);
}

PtrAWeak<IArray3> ia3CreateStack(StackBase &s, int n1, int n2, int n3)
{
  MemAccess2  acs(n1,n2,n3);
  return new_stack_array<IArray3,int>(s,acs);
}


PtrAWeak<DArray1> da1CreateStack(StackBase &s, int n1)
{
  MemAccess2  acs(n1);
  return new_stack_array<DArray1,double>(s,acs);
}

PtrAWeak<DArray2> da2CreateStack(StackBase &s, int n1, int n2)
{
  MemAccess2  acs(n1,n2);
  return new_stack_array<DArray2,double>(s,acs);
}

PtrAWeak<DArray3> da3CreateStack(StackBase &s, int n1, int n2, int n3)
{
  MemAccess2  acs(n1,n2,n3);
  return new_stack_array<DArray3,double>(s,acs);
}



PtrAWeak<ZArray1> za1CreateStack(StackBase &s, int n1)
{
  MemAccess2  acs(n1);
  return new_stack_array<ZArray1,double_complex>(s,acs);
}

PtrAWeak<ZArray2> za2CreateStack(StackBase &s, int n1, int n2)
{
  MemAccess2  acs(n1,n2);
  return new_stack_array<ZArray2,double_complex>(s,acs);
}

PtrAWeak<ZArray3> za3CreateStack(StackBase &s, int n1, int n2, int n3)
{
  MemAccess2  acs(n1,n2,n3);
  return new_stack_array<ZArray3,double_complex>(s,acs);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
extern "C"
{
  extern void  dgetrf_(int *n, int *m, double *a, int *lda, int* ipiv, int *info);
  extern void  dgetri_(int *n, double *a, int *lda, int* ipiv, double *work, int* lwork, int *info);
  extern void  zgetrf_(int *n, int *m, double_complex *a, int *lda, int* ipiv, int *info);
  extern void  zgetri_(int *n, double_complex *a, int *lda, int* ipiv, double_complex *work, int* lwork, int *info);
  extern void  dgetrs_( char *trans,int *n, const int *nrhs, double*a, int* lda, int *ipiv , double *b, int *ldb, int *info );
  extern void  zgetrs_( char *trans,int *n, const int *nrhs, double_complex *a, int* lda, int *ipiv , double_complex *b, int *ldb, int *info );
}


template<typename T>
array1<T>::array1(StackBase &S,const int n): n(n) { buf=new(S) T[n+1];};



template<typename T>
array1<T>::~array1()
{
  // delete for Stack is empty anyway.
  //  operator delete[](buf,S);
};

template<>
double array1<double>::l1norm(void) {double nm=0; for (int i=1;i<=n;i++) nm+=fabs(buf[i]); return nm;};

template<>
double array1<int>::l1norm(void) { double nm=0; for (int i=1;i<=n;i++) nm+=abs(buf[i]); return nm;};

template<>
double array1<double_complex>::l1norm(void) {double nm=0; for (int i=1;i<=n;i++) nm+=sqrt(buf[i].imag()*buf[i].imag()+buf[i].real()*buf[i].real()); return nm;};

template<typename T>
array2<T>::array2(StackBase &S, const int n, const int m):n(n), m(m),nm(n*m) {buf=new(S) T[nm+1];};

template<typename T>
array2<T>::~array2()
{
  // delete for Stack is empty anyway.
  // operator delete[](buf,S);
};


/*
template<>
void array2<double>::solve(array1<double>&u, array1<double>&f)
{ 
#if 0
  {
  array1<int> ipiv(S,n);
  array1<double> wrk(S,n);
  double d;

  lu_decmpT< array2<double> >(_n, *this,ipiv.vadr()-1,d,wrk.vadr()-1 );
  lu_solveT< array2<double>, array1<double>, array1<double> >(_n, *this,ipiv.vadr()-1, f,u);



  //  lu_decmpT(const int n, MAT& lu, int* ps, double& d, double* scales);
  //template<class MAT, class VEC1, class VEC2> inline
  //void lu_solveT(const int n, const MAT& lu, int* ps, const VEC1& b, VEC2& X)
  }
#endif


  char trans[3]={'N','\0'};
  array1<int> ipiv(_S,_n);
  int n=_n;
  int info;
  int one=1;
  u=f;
  dgetrf_(&n,&n,vadr(),&n,ipiv.vadr(),&info);
  dgetrs_(trans,&n,  &one, vadr(),&n,ipiv.vadr(),u.vadr(),&n,&info);
}

template<>
void array2<int>::solve(array1<int>&u, array1<int>&f)
{
  die("no linear solver for int");
}
template<>
void array2<double_complex>::solve(array1<double_complex>&u, array1<double_complex>&f)
{ 
  char trans[3]={'N','\0'};
  array1<int> ipiv(_S,_n);
  int n=_n;
  int info;
  int one=1;
  u=f;
  zgetrf_(&n,&n,vadr(),&n,ipiv.vadr(),&info);
  zgetrs_(trans,&n,  &one, vadr(),&n,ipiv.vadr(),u.vadr(),&n,&info);
}

*/

template class array1<double>;
template class array1<double_complex>;
template class array2<double>;
template class array2<double_complex>;
template class array1<int>;
template class array2<int>;


////////////////////////////////////////////////////////////////////////////////////////////////////




#ifdef wait
struct IArray2StackBase : public IArray2
{
  void *operator new( size_t sz) throw() {
    return ompx_alloc_secure(sz); 
  }
  void  operator delete( void *p, size_t sz ){
  }
};

struct IArray3StackBase : public IArray3
{
  void *operator new( size_t sz) throw() {
    return ompx_alloc_secure(sz); 
  }
  void  operator delete( void *p, size_t sz ){
  }
};


// ------------- StackBase DArray --------------- //

struct DArray1StackBase : public DArray1
{
  void *operator new( size_t sz ) throw() {
    return ompx_alloc_secure(sz); 
  }
  void  operator delete( void *p, size_t sz ){
  }
};

struct DArray2StackBase : public DArray2
{
  void *operator new( size_t sz ) throw() {
    return ompx_alloc_secure(sz); 
  }
  void  operator delete( void *p, size_t sz ){
  }
};

struct DArray3StackBase : public DArray3
{
  void *operator new( size_t sz) throw() {
    return ompx_alloc_secure(sz); 
  }
  void  operator delete( void *p, size_t sz ){
  }
};

struct DArray4StackBase : public DArray4
{
  void *operator new( size_t sz ) throw() {
    return ompx_alloc_secure(sz); 
  }
  void  operator delete( void *p, size_t sz ){
  }
};

struct DArray5StackBase : public DArray5
{
  void *operator new( size_t sz ) throw() {
    return ompx_alloc_secure(sz); 
  }
  void  operator delete( void *p, size_t sz ){
  }
};


// ------------- StackBase ZArray --------------- //

struct ZArray3StackBase : public ZArray3
{
  void *operator new( size_t sz ) throw() {
    return ompx_alloc_secure(sz); 
  }
  void  operator delete( void *p, size_t sz ){
  }
};



// ------------- StackBase Array Ctor --------------- //


PtrAWeak<IArray2> ia2CreateStackBase(int n1, int n2)
{
  MemAccess2  acs(n1,n2);
  return new_omp_array<IArray2,IArray2StackBase,int>(acs);
}

PtrAWeak<IArray3> ia3CreateStackBase(int n1, int n2, int n3)
{
  MemAccess2  acs(n1,n2,n3);
  return new_omp_array<IArray3,IArray3StackBase,int>(acs);
}


// ------------- Create StackBase DArray --------------- //

PtrAWeak<DArray1> da1CreateStackBase(int n1)
{
  MemAccess2  acs(n1);
  return new_omp_array<DArray1,DArray1StackBase,double>(acs);
}

PtrAWeak<DArray2> da2CreateStackBase(int n1, int n2)
{
  MemAccess2  acs(n1,n2);
  return new_omp_array<DArray2,DArray2StackBase,double>(acs);
}

PtrAWeak<DArray3> da3CreateStackBase(int n1, int n2, int n3)
{
  MemAccess2  acs(n1,n2,n3);
  return new_omp_array<DArray3,DArray3StackBase,double>(acs);
}

PtrAWeak<DArray4> da4CreateStackBase(int n1, int n2, int n3, int n4)
{
  MemAccess2  acs(n1,n2,n3,n4);
  return new_omp_array<DArray4,DArray4StackBase,double>(acs);
}

PtrAWeak<DArray5> da5CreateStackBase(int n1, int n2, int n3, int n4, int n5)
{
  MemAccess2  acs(n1,n2,n3,n4,n5);
  return new_omp_array<DArray5,DArray5StackBase,double>(acs);
}


// ------------- Create StackBase ZArray --------------- //

PtrAWeak<ZArray3> za3CreateStackBase(int n1, int n2, int n3)
{
  MemAccess2  acs(n1,n2,n3);
  return new_omp_array<ZArray3,ZArray3StackBase,double_complex>(acs);
}



// ------------- Flatten StackBase IArray --------------- //

PtrAWeak<DArray2> da3FlattenStackBase(const PtrA<DArray3>& a, int nrow, int ncol)
{
  MemAccess2 acs = a->acs;
  acs.calculate_multipliers(nrow,ncol);
  return new_omp_array<DArray2,DArray2StackBase,double>(acs);
}

PtrAWeak<DArray3> da4FlattenStackBase(const PtrA<DArray4>& a, int n1, int n2, int n3)
{
  MemAccess2 acs = a->acs;
  acs.calculate_multipliers(n1,n2,n3);
  return new_omp_array<DArray3,DArray3StackBase,double>(acs);
}


// ------------- Flatten StackBase IArray --------------- //

PtrAWeak<DArray1> da1PartStackBase(const PtrA<DArray1>& a, int i1, int n1)
{
  MemAccess2 acs(n1);
  PtrAWeak<DArray1> la;
  if (da1NAll(a)>0)
    {
      la=new_omp_array<DArray1,DArray1StackBase,double>(acs,&DA1(a,i1)-1,-1);
      assert( &DA1(a,i1)== &DA1(la,1) );
    }
  else
    la=new_omp_array<DArray1,DArray1StackBase,double>(acs,0);
  
  return la;
}

PtrAWeak<DArray2>  da2PartStackBase(const PtrA<DArray2>& a, int i1, int n1)
{
  int n2=da2NCol(a);
  
  MemAccess2 acs(n1,n2);
  PtrAWeak<DArray2> la=new_omp_array<DArray2,DArray2StackBase,double>(acs,&DA2(a,i1,1)-1,-1);
  
  assert( &DA2(a,i1,1)== &DA2(la,1,1));
  return la;
}



#endif
