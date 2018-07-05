#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "array_omp.h"

#include "stdsys/ompx.h"
#include "stdcxx/sig.h"


static void* ompx_alloc_secure(size_t n)
{
  void *ptr;
  ptr=ompx_alloc(n);

  if(ptr)
   return ptr;

  ErrorMsg("base","Thread memory size exceeded");
  return NULL;
}


// ------------- OMP IArray --------------- //

struct IArray1OMP : public IArray1
{
  void *operator new( size_t sz) throw() {
    return ompx_alloc_secure(sz); 
  }
  void  operator delete( void *p, size_t sz ){
  }
};

struct IArray2OMP : public IArray2
{
  void *operator new( size_t sz) throw() {
    return ompx_alloc_secure(sz); 
  }
  void  operator delete( void *p, size_t sz ){
  }
};

struct IArray3OMP : public IArray3
{
  void *operator new( size_t sz) throw() {
    return ompx_alloc_secure(sz); 
  }
  void  operator delete( void *p, size_t sz ){
  }
};


// ------------- OMP DArray --------------- //

struct DArray1OMP : public DArray1
{
  void *operator new( size_t sz ) throw() {
    return ompx_alloc_secure(sz); 
  }
  void  operator delete( void *p, size_t sz ){
  }
};

struct DArray2OMP : public DArray2
{
  void *operator new( size_t sz ) throw() {
    return ompx_alloc_secure(sz); 
  }
  void  operator delete( void *p, size_t sz ){
  }
};

struct DArray3OMP : public DArray3
{
  void *operator new( size_t sz) throw() {
    return ompx_alloc_secure(sz); 
  }
  void  operator delete( void *p, size_t sz ){
  }
};

struct DArray4OMP : public DArray4
{
  void *operator new( size_t sz ) throw() {
    return ompx_alloc_secure(sz); 
  }
  void  operator delete( void *p, size_t sz ){
  }
};

struct DArray5OMP : public DArray5
{
  void *operator new( size_t sz ) throw() {
    return ompx_alloc_secure(sz); 
  }
  void  operator delete( void *p, size_t sz ){
  }
};


// ------------- OMP ZArray --------------- //

struct ZArray3OMP : public ZArray3
{
  void *operator new( size_t sz ) throw() {
    return ompx_alloc_secure(sz); 
  }
  void  operator delete( void *p, size_t sz ){
  }
};



// ------------- OMP Array Ctor --------------- //

template <typename T>
struct MemOMP : public MemDynamicVIRTUAL<T>
{
  MemOMP<T>( int sz, T * mem )
    : MemDynamicVIRTUAL<T>(sz,mem,0)
  {
  }
  void *operator new( size_t sz) throw() {
    return ompx_alloc_secure(sz); 
  }
  void  operator delete( void *p, size_t sz ){
  }
};


template<class TARRAY_BASE,class TARRAY, typename T> inline
PtrAWeak<TARRAY_BASE> new_omp_array(const MemAccess2 &acs, T*mem=NULL, int memoffset=0)
{
  size_t nelem=acs.n_elem()+1;
  if(!mem)
      mem = (T*)ompx_alloc_secure(nelem*sizeof(typename TARRAY_BASE::value_type));
  TARRAY_BASE * a = new TARRAY();
  PtrWeak<MemDynamicVIRTUAL<T> > vir=(MemOMP<T>*)new MemOMP<T>(nelem,mem);
  a->vir=vir;
  a->v=a->vir->v;
  a->acs=acs;
  a->set_vector_default();
  return PtrAWeak<TARRAY_BASE>(a);
}


// ------------- Create OMP IArray --------------- //

PtrAWeak<IArray1> ia1CreateOMP(int n1)
{
  MemAccess2  acs(n1);
  return new_omp_array<IArray1,IArray1OMP,int>(acs);
}

PtrAWeak<IArray2> ia2CreateOMP(int n1, int n2)
{
  MemAccess2  acs(n1,n2);
  return new_omp_array<IArray2,IArray2OMP,int>(acs);
}

PtrAWeak<IArray3> ia3CreateOMP(int n1, int n2, int n3)
{
  MemAccess2  acs(n1,n2,n3);
  return new_omp_array<IArray3,IArray3OMP,int>(acs);
}


// ------------- Create OMP DArray --------------- //

PtrAWeak<DArray1> da1CreateOMP(int n1)
{
  MemAccess2  acs(n1);
  return new_omp_array<DArray1,DArray1OMP,double>(acs);
}

PtrAWeak<DArray2> da2CreateOMP(int n1, int n2)
{
  MemAccess2  acs(n1,n2);
  return new_omp_array<DArray2,DArray2OMP,double>(acs);
}

PtrAWeak<DArray3> da3CreateOMP(int n1, int n2, int n3)
{
  MemAccess2  acs(n1,n2,n3);
  return new_omp_array<DArray3,DArray3OMP,double>(acs);
}

PtrAWeak<DArray4> da4CreateOMP(int n1, int n2, int n3, int n4)
{
  MemAccess2  acs(n1,n2,n3,n4);
  return new_omp_array<DArray4,DArray4OMP,double>(acs);
}

PtrAWeak<DArray5> da5CreateOMP(int n1, int n2, int n3, int n4, int n5)
{
  MemAccess2  acs(n1,n2,n3,n4,n5);
  return new_omp_array<DArray5,DArray5OMP,double>(acs);
}


// ------------- Create OMP ZArray --------------- //

PtrAWeak<ZArray3> za3CreateOMP(int n1, int n2, int n3)
{
  MemAccess2  acs(n1,n2,n3);
  return new_omp_array<ZArray3,ZArray3OMP,double_complex>(acs);
}



// ------------- Flatten OMP IArray --------------- //

PtrAWeak<DArray2> da3FlattenOMP(const PtrA<DArray3>& a, int nrow, int ncol)
{
  MemAccess2 acs = a->acs;
  acs.calculate_multipliers(nrow,ncol);
  return new_omp_array<DArray2,DArray2OMP,double>(acs);
}

PtrAWeak<DArray3> da4FlattenOMP(const PtrA<DArray4>& a, int n1, int n2, int n3)
{
  MemAccess2 acs = a->acs;
  acs.calculate_multipliers(n1,n2,n3);
  return new_omp_array<DArray3,DArray3OMP,double>(acs);
}


// ------------- Flatten OMP IArray --------------- //

PtrAWeak<DArray1> da1PartOMP(const PtrA<DArray1>& a, int i1, int n1)
{
  MemAccess2 acs(n1);
  PtrAWeak<DArray1> la;
  if (da1NAll(a)>0)
    {
      la=new_omp_array<DArray1,DArray1OMP,double>(acs,&DA1(a,i1)-1,-1);
      assert( &DA1(a,i1)== &DA1(la,1) );
    }
  else
    la=new_omp_array<DArray1,DArray1OMP,double>(acs,0);
  
  return la;
}

PtrAWeak<DArray2>  da2PartOMP(const PtrA<DArray2>& a, int i1, int n1)
{
  int n2=da2NCol(a);
  
  MemAccess2 acs(n1,n2);
  PtrAWeak<DArray2> la=new_omp_array<DArray2,DArray2OMP,double>(acs,&DA2(a,i1,1)-1,-1);
  
  assert( &DA2(a,i1,1)== &DA2(la,1,1));
  return la;
}



