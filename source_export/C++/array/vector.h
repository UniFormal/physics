/**
    \file vector.h

    \author Juergen Fuhrmann, T.Streckenbach

    \brief   Pure virtual base class for pdelib2 vectors.


    A vector is considered as an element of an Euclidean vector space. Upon a vector,
    we can perform addititon, subtraction, multiplication by scalars. We can calculate
    scalar products and norms. All these operations can be interfaced without referring
    to any of the components. The only other information about a vector we assume to be
    known is its dimension.

    pdelib2  defines a  pure virtual  base  class for  its vectors.  A
    vector  is  characterized  by  its dimension  and  by  the  vector
    routines which can be applied to it.


*/

#ifndef VECTOR_H
#define VECTOR_H

#include <math.h>
#include <stdio.h>
#include "stdsys/swigbind.h"
#include "stdsys/complex.h"
#include "stdcxx/ptr.h"
#include "stdcxx/sig.h"


class VectorLib;


/**
   \brief Vector data structure.
   Pure virtual base class.
 */
SwigBind struct Vector {
  PtrWeak<VectorLib> vlib; ///< Vector routine library
  // JF: This has to be a weak pointer, as by default it  is set
  // in the inner loops of parallel code, see also void TArray<T>::set_vector_default()
  // in array.cxx

  char vtype[16]; ///< Type signature
  Vector();
  
SwigBindBegin
  
  virtual int vdim() const = 0; ///< Dimension of vector space
  virtual int dim() const = 0;  ///< Tensor dimension
  virtual ~Vector();
  
  // Lua
  virtual double __getitem__V(int i) const { assert(0); return 0; } 
  virtual void __setitem__V(int i,double d) { assert(0); }
  virtual int __lenV(void*) { assert(0); return 1; }
  
  double __getitem__(int i) const { return __getitem__V(i); } // direct not possible (conflict in TArray2)
  void __setitem__(int i,double d) { __setitem__V(i,d); }
  int __len(void*) { return __lenV(NULL); }
};
SwigBindEnd
SwigBindPtr(Vector)

#if SwigExportOnly
SwigBindBegin

#ifdef SWIGLUA

%typemap(in,checkfn="lua_isuserdata") Ptr<Vector> ($&ltype argp)
%{
  if (!SWIG_IsOK(SWIG_ConvertPtr(L,$input,(void**)&argp,$&descriptor,0)))
  if (!SWIG_IsOK(SWIG_ConvertPtr(L,$input,(void**)&argp,SWIGTYPE_p_PtrAT_DArray1_t,0)))
  if (!SWIG_IsOK(SWIG_ConvertPtr(L,$input,(void**)&argp,SWIGTYPE_p_PtrAT_DArray2_t,0)))
  if (!SWIG_IsOK(SWIG_ConvertPtr(L,$input,(void**)&argp,SWIGTYPE_p_PtrAT_IArray1_t,0)))
  if (!SWIG_IsOK(SWIG_ConvertPtr(L,$input,(void**)&argp,SWIGTYPE_p_PtrAT_IArray2_t,0)))
  if (!SWIG_IsOK(SWIG_ConvertPtr(L,$input,(void**)&argp,SWIGTYPE_p_PtrAT_ZArray1_t,0)))
  if (!SWIG_IsOK(SWIG_ConvertPtr(L,$input,(void**)&argp,SWIGTYPE_p_PtrAT_ZArray2_t,0)))
     SWIG_fail_ptr("$symname",$argnum,$descriptor);
   $1 = *argp;
%}
%typemap(in) Ptr<Vector>& {
  
  // typemap(in) Ptr<Vector>&
  if (!SWIG_IsOK(SWIG_ConvertPtr(L,$input,(void**)&($1),SWIGTYPE_p_PtrT_Vector_t,0)))
  if (!SWIG_IsOK(SWIG_ConvertPtr(L,$input,(void**)&($1),SWIGTYPE_p_PtrAT_DArray1_t,0)))
  if (!SWIG_IsOK(SWIG_ConvertPtr(L,$input,(void**)&($1),SWIGTYPE_p_PtrAT_DArray2_t,0)))
  if (!SWIG_IsOK(SWIG_ConvertPtr(L,$input,(void**)&($1),SWIGTYPE_p_PtrAT_IArray1_t,0)))
  if (!SWIG_IsOK(SWIG_ConvertPtr(L,$input,(void**)&($1),SWIGTYPE_p_PtrAT_IArray2_t,0)))
  if (!SWIG_IsOK(SWIG_ConvertPtr(L,$input,(void**)&($1),SWIGTYPE_p_PtrAT_ZArray1_t,0)))
  if (!SWIG_IsOK(SWIG_ConvertPtr(L,$input,(void**)&($1),SWIGTYPE_p_PtrAT_ZArray2_t,0)))
   SWIG_fail_ptr("$symname",$input,SWIGTYPE_p_PtrT_Vector_t);
}
%typemap(in) Ptr<Vector> const & {
  
  // typemap(in) Ptr<Vector> const &
  if (!SWIG_IsOK(SWIG_ConvertPtr(L,$input,(void**)&($1),SWIGTYPE_p_PtrT_Vector_t,0)))
  if (!SWIG_IsOK(SWIG_ConvertPtr(L,$input,(void**)&($1),SWIGTYPE_p_PtrAT_DArray1_t,0)))
  if (!SWIG_IsOK(SWIG_ConvertPtr(L,$input,(void**)&($1),SWIGTYPE_p_PtrAT_DArray2_t,0)))
  if (!SWIG_IsOK(SWIG_ConvertPtr(L,$input,(void**)&($1),SWIGTYPE_p_PtrAT_IArray1_t,0)))
  if (!SWIG_IsOK(SWIG_ConvertPtr(L,$input,(void**)&($1),SWIGTYPE_p_PtrAT_IArray2_t,0)))
  if (!SWIG_IsOK(SWIG_ConvertPtr(L,$input,(void**)&($1),SWIGTYPE_p_PtrAT_ZArray1_t,0)))
  if (!SWIG_IsOK(SWIG_ConvertPtr(L,$input,(void**)&($1),SWIGTYPE_p_PtrAT_ZArray2_t,0)))
   SWIG_fail_ptr("$symname",$input,SWIGTYPE_p_PtrT_Vector_t);
}

#endif

SwigBindEnd
#endif


class VectorLib
{
 public:
  VectorLib(){};
  virtual ~VectorLib(){};

  virtual Ptr<Vector> clone(Ptr<Vector> original)=0;
  virtual void axpy(double_complex a, Ptr<Vector> x,Ptr<Vector> y)=0;// y=ax+y
  virtual void lin1(Ptr<Vector> x0,double_complex a, Ptr<Vector> x,Ptr<Vector> y)=0; // y=x0+ax
  virtual void lin2(Ptr<Vector> x0,double_complex a1,Ptr<Vector> x1,double_complex a2,Ptr<Vector> x2,Ptr<Vector> y)=0; // y=x0+a1x1+a2x2
  virtual void set (double_complex a, Ptr<Vector> x)=0;
  virtual void scal(double_complex a,Ptr<Vector> x)=0;
  virtual void copy(Ptr<Vector> x,Ptr<Vector> y)=0;
  virtual double_complex dot(Ptr<Vector> x, Ptr<Vector> y)=0;
  virtual double amax(Ptr<Vector> x)=0;
  virtual double nm2(Ptr<Vector> x)=0;
};


/**
   \brief Assert that vectors are of same type.

 */

SwigBind int vassert(Ptr<Vector> v1, Ptr<Vector> v2);

/**
   \brief  Return dimension of vector.
 */
SwigBind int vdim(Ptr<Vector> v);


/**
   \brief Return type signature string.
 */
SwigBind char* vtype(Ptr<Vector> v);



/**
   \brief Copy values from one vector to other

   \f$ y_i=x_i, i=1\dots{\rm vdim}\f$

   Runs in parallel if vector was created from space on partitioned grid.

   \param x source Vector
   \param y destination Vector

*/
SwigBind inline void vcopy(Ptr<Vector> x, Ptr<Vector> y) { ErrorCallStack ecs("vcopy(x,y)"); if(!x || !y) ErrorMsg("base",true,"empty x or y"); x->vlib->copy((x),(y)); }

/**
    \brief Clone vector.

    Passing information on a vector type to a module which has to create
    similar vectors, just is performed by passing a  vector of that type.

   Runs in parallel if vector was created from space on partitioned grid.

    \param  original Original vector
    \return Vector with same length, type signature and vector lib.
*/
SwigBind inline Ptr<Vector> vclone(Ptr<Vector> v) { ErrorCallStack ecs("vclone(v)"); if(!v) ErrorMsg("base",true,"empty v"); return v->vlib->clone(v); }


/**
   \brief One parameter linear combination

   \f$ y_i=x_{0,i} + a x_i, i=1\dots{\rm vdim}\f$
   Runs in parallel if vector was created from space on partitioned grid.


   \param x0 Vector
   \param a  scalar
   \param x  Vector
   \param y destination Vector

*/
SwigBind inline void vlin1(Ptr<Vector> x0, double a, Ptr<Vector> x, Ptr<Vector> y) { ErrorCallStack ecs("vlin1(x0,a,x,y)"); if(!x0 || !x || !y) ErrorMsg("base",true,"empty x0, x, or y"); x0->vlib->lin1((x0),a,(x),(y)); }
SwigBind inline void vlin1c(Ptr<Vector> x0, double_complex a, Ptr<Vector> x, Ptr<Vector> y) { ErrorCallStack ecs("vlin1c(x0,a,x,y)"); if(!x0 || !x || !y) ErrorMsg("base",true,"empty x0, x, or y"); x0->vlib->lin1((x0),a,(x),(y)); }

/**
   \brief Two  parameter linear combination

   \f$ y_i=x_{0,i} + a_1 x_{1,i}+ a_2 x_{2,i}, i=1\dots{\rm vdim}\f$

   Runs in parallel if vector was created from space on partitioned grid.

   \param x0 Vector
   \param a1 scalar
   \param x1  Vector
   \param a2 scalar
   \param x2  Vector
   \param y destination Vector

*/
SwigBind inline void vlin2(Ptr<Vector> x0,double a1,Ptr<Vector> x1,double a2,Ptr<Vector> x2,Ptr<Vector> y) { ErrorCallStack ecs("vlin2(x0,a1,x1,a2,x2,y)"); if(!x0 || !x1 || !x2 || !y) ErrorMsg("base",true,"empty x0, x1, x2, or y"); x0->vlib->lin2((x0),a1,(x1),a2,(x2),(y)); }
SwigBind inline void vlin2c(Ptr<Vector> x0,double_complex a1,Ptr<Vector> x1,double_complex a2,Ptr<Vector> x2,Ptr<Vector> y) {  ErrorCallStack ecs("vlin2c(x0,a1,x1,a2,x2,y)"); if(!x0 || !x1 || !x2 || !y) ErrorMsg("base",true,"empty x0, x1, x2, or y"); x0->vlib->lin2((x0),a1,(x1),a2,(x2),(y)); }

/**
   \brief Add scaled vector

   \f$ y_i=y_i + a x_i, i=1\dots{\rm vdim}\f$

   Runs in parallel if vector was created from space on partitioned grid.

   \param a  scalar
   \param x  Vector
   \param y destination Vector
*/

SwigBind inline void vaxpy(double a,Ptr<Vector> x,Ptr<Vector> y) { ErrorCallStack ecs("vaxpy(a,x,y)"); if(!x || !y) ErrorMsg("base",true,"empty x or y"); x->vlib->axpy(a,(x),(y)); }
SwigBind inline void vaxpyc(double_complex a,Ptr<Vector> x,Ptr<Vector> y) { ErrorCallStack ecs("vaxpyc(a,x,y)"); if(!x || !y) ErrorMsg("base",true,"empty x or y"); x->vlib->axpy(a,(x),(y)); }

/**
   \brief Dot product of two vectors

   Runs in parallel if vector was created from space on partitioned grid.

   \param x  Vector
   \param y Vector
   \return \f$ x\cdot y =\sum_{i=1}^{\rm vdim} x_i y_i\f$
*/
SwigBind inline double_complex vdotc(Ptr<Vector> x,Ptr<Vector> y) { ErrorCallStack ecs("vdotc(x,y)"); if(!x || !y) ErrorMsg("base",true,"empty x or y"); return x->vlib->dot(x,y); }
SwigBind inline double         vdot (Ptr<Vector> x,Ptr<Vector> y) { ErrorCallStack ecs("vdot(x,y)"); if(!x || !y) ErrorMsg("base",true,"empty x or y"); return x->vlib->dot(x,y).real(); }


/**
   \brief Euclidean norm fo vector
*/
SwigBind inline double vnm2(Ptr<Vector> x) { ErrorCallStack ecs("vnm2(x)"); if(!x) ErrorMsg("base",true,"empty x"); return sqrt(vdot(x,x)); }

/**
   \brief Set value for all vector components


   \f$ x_i=a, i=1\dots{\rm vdim}\f$

   Runs in parallel if vector was created from space on partitioned grid.

   \param a value
   \param x  Vector

*/
SwigBind inline void vset(double a,Ptr<Vector> x) { ErrorCallStack ecs("vset(a,x)"); if(!x) ErrorMsg("base",true,"empty x"); x->vlib->set(a,(x)); }
SwigBind inline void vsetc(double_complex a,Ptr<Vector> x) { ErrorCallStack ecs("vsetc(a,x)"); if(!x) ErrorMsg("base",true,"empty x"); x->vlib->set(a,(x)); }

/**
   \brief Multiply vector by a scalar

   \f$ x_i=ax_i, i=1\dots{\rm vdim}\f$

   Runs in parallel if vector was created from space on partitioned grid.

   \param a value
   \param x  Vector

*/
SwigBind inline void vscal(double a,Ptr<Vector> x) { ErrorCallStack ecs("vscal(a,x)"); if(!x) ErrorMsg("base",true,"empty x"); x->vlib->scal(a,(x)); }
SwigBind inline void vscalc(double_complex a,Ptr<Vector> x) { ErrorCallStack ecs("vscalc(a,x)"); if(!x) ErrorMsg("base",true,"empty x"); x->vlib->scal(a,(x)); }

/**
   \brief Maximum absolute value of the vector components

    Runs in parallel if vector was created from space on partitioned grid.

   \param x  Vector

   \return \f$ \max_{i=1}^{\rm vdim} |x_i|\f$
*/
SwigBind inline double vamax(Ptr<Vector> x) { ErrorCallStack ecs("vamax(x)"); if(!x) ErrorMsg("base",true,"empty x"); return x->vlib->amax((x)); }




#endif

