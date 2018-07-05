/** \file arraytools.h

    A number of tools  for working with arrays.
    
 */

#ifndef ARRAYTOOLS_H
#define ARRAYTOOLS_H

#include "array/array.h"

#include "array/linalgdens.h"

#include "luacxx/luacxx.h"

#include "stdsys/swigbind.h"

SwigBindBegin
/** 
   Subdivide the interval [x0,x1]  into n subintervals such
   that two neigboring intervals differ in size by factor alpha.

  It is the sequence:
  xx[j] = x0 + (x1-x0)*(1-alpha^{j-1})/(1-alpha^{n-1}) if alpha \ne 1
 or
  xx[j] = x0 + (x1-x0)*(j-1)/(n-1) if alpha \eq 1
*/

PtrA<DArray1> da1GeomSpace(double x0, double x1, int n, double alpha);


/**
   As linspace in matlab:
   Subdivide interval  [x0,x1] into n equally sized subintervals, return
   the sequence of interval boundary points.
 */
PtrA<DArray1> da1LinSpace(double x0, double x1, int n);


/**
   Concatenate two arrays: return a new array whose length is equal
   to the sum of the lengths of the input arrays.
 */
PtrA<DArray1> da1Concat(PtrA<DArray1> a, PtrA<DArray1> b);

/**
   Join two arrays: return a new array whose length is equal
   to the sum of the lengths of the input arrays diminished by 1,
   assuming that the last value of a and the first value of b
   differ at maximub by eps. Otherwise, return 0.

*/
PtrA<DArray1> da1Join(PtrA<DArray1> a, PtrA<DArray1> b, double eps);

SwigBindEnd


SwigBindObj


/**
 Working on arrays.
 
 For details about the solver see EigenSystemBase.
*/
SwigBind class EigenSystem : public EigenSystemBase {
  
  // overloading
  virtual void linamv(double* u, double* v);
  virtual void linaainv(double* u, double* v);
  virtual void linaminv(double* u, double* v);
  virtual void linaav(double* u, double* v);
  
 protected:
  
  // the array interface
  virtual void linamv(const PtrA<DArray1>& u, PtrA<DArray1> v) {}
  virtual void linaainv(const PtrA<DArray1>& u, PtrA<DArray1> v) {}
  virtual void linaminv(const PtrA<DArray1>& u, PtrA<DArray1> v) {}
  virtual void linaav(const PtrA<DArray1>& u, PtrA<DArray1> v) {}
  
SwigBindBegin
  
 public:
  
  EigenSystem();
  
  virtual ~EigenSystem();
};
SwigBindEnd
SwigBindPtr(EigenSystem)



/**
 Working with Lua functions.
 
 For details about the solver see EigenSystemBase.
*/
SwigBind class EigenSystemLua : public EigenSystem {
  
 protected:
  using EigenSystemBase::linamv;
  using EigenSystemBase::linaainv;
  using EigenSystemBase::linaminv;
  using EigenSystemBase::linaav;

  // overloading
  virtual void linamv(const PtrA<DArray1>& u, PtrA<DArray1> v);
  virtual void linaainv(const PtrA<DArray1>& u, PtrA<DArray1> v);
  virtual void linaminv(const PtrA<DArray1>& u, PtrA<DArray1> v);
  virtual void linaav(const PtrA<DArray1>& u, PtrA<DArray1> v);
  
  PtrWeak<LuaObjRef> es;
  
SwigBindBegin
  
 public:
  
  EigenSystemLua();
  
  /**
  
   Solves an eigensystem based on a table of Lua functions.
   
   \code
    es={
     linamv=<func>,
     linaainv=<func>,
     linaminv=<func>,
     linaav=<func>,
     vars={ <user-vars> },
     
     -- optional. otherwise set this directly in EigenSystemBase, and
     --  for the defaults are also specified in EigenSystemBase.
     n  =<int>,
     nev=<int>,
     ncv=<int>,
     
     sym   =<bool>,
     bmat  =<bool>,
     mode  =<int>,
     sigmar=<double>,
     sigmai=<double>,
     
     maxitr=<int>,
    }
    
    <func>=function(tab,u,v) ... end
   \endcode
   
   The function param 'tab' represents the table 'es' itself. So
   it is possible to pass local data.
  */
  bool es_solve(const LuaObjRef& eigenvalue_problem);
  
  // get eigen-comp j
  double       residual   (int j);
  double       eigenvaluer(int j); // real
  double       eigenvaluei(int j); // imag
  PtrA<DArray1> eigenvector(int j, bool normalize=true);
  
  virtual ~EigenSystemLua();
};
SwigBindEnd
SwigBindPtr(EigenSystemLua)


/**

 Creates EigenSystemLua.
*/
SwigBind inline Ptr<EigenSystemLua> laCreateEigenSystemLua() { return new EigenSystemLua(); }

SwigBind inline Ptr<EigenSystemBase> laCastToEigenSystemBase(Ptr<EigenSystemLua> ep) { return PtrCast<EigenSystemBase>(ep); }




#endif

