///
/// \file operator.h
///
/// Abstract functor base classes for operators between vector
/// 

#ifndef OPERATOR_H
#define OPERATOR_H

#include "vector.h"


///
/// Functor class for operator application. 
/// 
/// Abstract base class for 
///
///      v=op(u)
///
class opApply
{
  virtual void apply( Ptr<Vector>  u, Ptr<Vector> v) {};
 public:
  virtual void operator () (Ptr<Vector>  u, Ptr<Vector> v) { apply(u,v);};
  virtual ~opApply() {}
};

///
/// Functor class for solver operator
/// 
/// Abstract base class for solution of 
///
///      op(u)=v
///
class opSolve
{
  virtual void solve( Ptr<Vector>  u, Ptr<Vector> v)=0;
 public:
  inline void operator () (Ptr<Vector>  u, Ptr<Vector> v) { solve(u,v);};
  virtual ~opSolve() {}
};


///
/// Functor class for scalar product
/// 
/// Abstract base class for 
///
///       scalpro=(u,v)
///
class opScalarProduct
{
  virtual double scalar_product(Ptr<Vector>  u, Ptr<Vector> v)=0;
 public:
  inline double operator () (Ptr<Vector>  u, Ptr<Vector> v) { return scalar_product(u,v);}
  virtual ~opScalarProduct() {}
};


///
/// Functor class for norm
/// 
/// Abstract base class for 
///
///       norm=|u|
///
class opNorm
{
  virtual double norm(Ptr<Vector> u) = 0;
 public:
  inline double  operator()(Ptr<Vector> u) { return norm(u);};
  virtual ~opNorm(){}
};

#endif
