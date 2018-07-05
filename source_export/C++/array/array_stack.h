/**
   \file array_stack.h
   \brief Stack Array Constructors and Stack array class
   \author J. Fuhrmann

   Array constructors using only stack memory provided
   in a region of the stack previously reserved.
*/

#ifndef ARRAY_Stack_H
#define ARRAY_Stack_H


#include "array/array.h"
#include "stdcxx/stack.h"
#include "stdcxx/memacs.h"



PtrAWeak<DArray1> da1CreateStack(StackBase & s, int n1);
PtrAWeak<DArray2> da2CreateStack(StackBase & s, int n1, int n2);
PtrAWeak<DArray3> da3CreateStack(StackBase & s, int n1, int n2, int n3);

PtrAWeak<ZArray1> za1CreateStack(StackBase & s, int n1);
PtrAWeak<ZArray2> za2CreateStack(StackBase & s, int n1, int n2);
PtrAWeak<ZArray3> za3CreateStack(StackBase & s, int n1, int n2, int n3);

PtrAWeak<IArray1> ia1CreateStack(StackBase & s, int n1);
PtrAWeak<IArray2> ia2CreateStack(StackBase & s, int n1, int n2);
PtrAWeak<IArray3> ia3CreateStack(StackBase & s, int n1, int n2, int n3);

template <typename T> class array2;

template <typename T> class  array1
{
  friend class array2<T>;

  const int n;
  T *buf;
 public:
  array1(StackBase &s,const int n);
  ~array1();
  
  typedef T value_type;
  int nrow() const {return n;}
  int ncol() const {return 1;}
  int nall() const {return n;}
  T* vadr() { return buf+1;}

  T& operator () (const int i)
  {
    #if PDELIB_VARIANT_DBG
    memacs_rchk(i, n, 1,1);
    #endif    
    return buf[i];
  }
  T& operator [] (const int i)
  {
    #if PDELIB_VARIANT_DBG
    memacs_rchk(i, n, 1,1);
    #endif    
    return buf[i];
  }
  T operator=(double val)       { for (int i=1;i<=n;i++) buf[i]=val; return val;}
  array1& operator=(array1&u)   { for (int i=1;i<=n;i++) buf[i]=u.buf[i]; return u;}
  array1& operator-=(array1 &u) { for (int i=1;i<=n;i++) buf[i]-=u.buf[i]; return *this;}
  array1& operator+=(array1 &u) { for (int i=1;i<=n;i++) buf[i]+=u.buf[i]; return *this;}
  array1& operator*=(T v)       { for (int i=1;i<=n;i++) buf[i]*=v; return *this;}

  double l1norm(void);
};

template <typename T>
class array2
{

  const int n;
  const int m;
  const int nm;
  T *buf;
public:
  typedef T value_type;
  int nrow() const {return n;}
  int ncol() const {return m;}
  int nall() const {return nm;}
  T* vadr() { return buf+1;}

  array2(StackBase &s, const int n, const int m);
  ~array2();
  T& operator () (const int i, const int j)
  {
    #if PDELIB_VARIANT_DBG
    memacs_rchk(i, n, 1,2);
    memacs_rchk(j, m, 2,2);
    #endif    
    return buf[(i-1)*n+j];
  }
  T operator=(T val) { for (int i=1;i<=nm;i++) buf[i]=val; return val;}
  T* valaddr() { return buf+1;}
  array2& operator=(array2&u)   { for (int i=1;i<=nm;i++) buf[i]=u.buf[i]; return u;}
  array2& operator-=(array2 &u) { for (int i=1;i<=nm;i++) buf[i]-=u.buf[i]; return *this;}
  array2& operator+=(array2 &u) { for (int i=1;i<=nm;i++) buf[i]+=u.buf[i]; return *this;}
  array2& operator*=(T v)       { for (int i=1;i<=nm;i++) buf[i]*=v; return *this;}
  

};

typedef array1<double> darray1;
typedef array2<double> darray2;

typedef array1<int> iarray1;
typedef array2<int> iarray2;

typedef array1<double_complex> zarray1;
typedef array2<double_complex> zarray2;


template <class P,typename T> class newton
{
  P& _P;
  double _eps;
  int _maxsteps;
  double _damp;
  bool _log;
  bool _error;
public:
 newton(P& p):
  _P(p),
    _eps(1.0e-10),
    _maxsteps(10),
    _damp(1.0),
  _log(false),
    _error(false)
  {;};
  
  inline void eps(T eps) {_eps=eps;};
  inline void damp(T damp) {_damp=damp;};
  inline void log(bool log) {_log=log;};
  inline void maxsteps(int maxsteps) {_maxsteps=maxsteps;};
  inline bool error(void)  {return _error;};

  inline void solve(array1<T>&u, array1<T>&f, array2<T>& jac)
  {
    int n=u.nall();
    double damp=_damp;
    assert(n==f.nall());
    assert(n==jac.nrow());
    array1<T> res(u);
    array1<T> upd(u);
    double residual,firstresidual;
    int istep=1;
    _error=false;
    while (true)
      { 
	_P.apply_and_linearize(u,res,jac);
	residual=0.0;
	for (int i=1;i<=n;i++)
	  {
	    res(i)-=f(i);
	    residual+=fabs(res(i));
	  }
	if (istep==1) firstresidual=residual;
	if (_log) printf("solvenewton: step=%d cont=%g\n",istep,residual/firstresidual); 
	if (residual/firstresidual<_eps) 
	  {
	    if (_log) printf("solvenewton: success\n");
	    break;
	  }
	if (istep>_maxsteps) 
	  {
	    if (_log) printf("solvenewton: failed\n");
	    _error=true;
	    break;
	  }
     	jac.solve(upd,res);
	if (damp<1.0) upd*=damp;
	u-=upd;
	istep++;
	damp*=2.0;
	if (damp>1.0) damp=1.0;
      };
  }
};

#endif


