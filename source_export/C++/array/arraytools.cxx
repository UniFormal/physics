#include <config.h>

#include "arraytools.h"

#include "stdcxx/sig.h"

#include "luacxx/luacxxswig.h"

using namespace std;

PtrA<DArray1> da1GeomSpace(double x0, double x1, int n, double alpha)
{
  if (n<=2) 
    {
      n=2;
      alpha=1;
    }

  if (alpha==1.0)
    return da1LinSpace(x0,x1,n);

  PtrA<DArray1> X=da1Create(n);
  DArray1 & x=X;
  double alphax=1.0;
  for (int i=1; i<n;i++)
    alphax=alphax*alpha;
  double h=(x1-x0)*(1.0-alpha)/(1.0-alphax);
    
  x(1)=x0;
  for (int i=1; i<n; i++)
    {
      x(i+1)=x(i)+h;
      h=h*alpha;
    }
  x(n)=x1;
  
  return X;
}


PtrA<DArray1> da1LinSpace(double x0, double x1, int n)
{
  if (n<=2) n=2;
  PtrA<DArray1> X=da1Create(n);
  DArray1 & x=X;
  if (n==2)
    {
      x(1)=x0;
      x(n)=x1;
      return X;
    }
  double h=(x1-x0)/(((double)n)-1.0);
  x(1)=x0;
  for (int i=1; i<n; i++)
    x(i+1)=x(i)+h;
  x(n)=x1;
  return X;
}

PtrA<DArray1> da1Concat(PtrA<DArray1> A, PtrA<DArray1> B)
{
  int nA=da1NAll(A);
  int nB=da1NAll(B);

  PtrA<DArray1> X=da1Create(nA+nB);
  int iX=1;
  for (int iA=1; iA<=nA; iA++,iX++)
    DA1(X,iX)=DA1(A,iA);
  for (int iB=1; iB<=nB; iB++,iX++)
    DA1(X,iX)=DA1(B,iB);

  return X;
}

PtrA<DArray1> da1Join(PtrA<DArray1> A, PtrA<DArray1> B, double eps)
{
  int nA=da1NAll(A);
  int nB=da1NAll(B);
  
  if (fabs(DA1(A,nA)-DA1(B,1))>eps) return 0;

  PtrA<DArray1> X=da1Create(nA+nB-1);
  int iX=1;
  for (int iA=1; iA<=nA; iA++,iX++)
    DA1(X,iX)=DA1(A,iA);
  for (int iB=2; iB<=nB; iB++,iX++)
    DA1(X,iX)=DA1(B,iB);

  return X;
}

/*
  da1Refine

 */
 
// ------------- EigenSystem


inline PtrA<DArray1> new_DArray1(int n, double* u)
{
  PtrA<DArray1> U=new DArray1(n,u-1,0,NULL);
  return U;
}

inline PtrA<DArray2> new_DArray2(int m, int n, double* u)
{
  PtrA<DArray2> U=new DArray2(m,n,u-1,0,NULL);
  return U;
}

EigenSystem::EigenSystem()
  : EigenSystemBase()
{
}

EigenSystem::~EigenSystem()
{
}

// overloading
void EigenSystem::linamv(double* u, double* v)
{
  linamv(new_DArray1(n,u),new_DArray1(n,v));
}

void EigenSystem::linaainv(double* u, double* v)
{
  linaainv(new_DArray1(n,u),new_DArray1(n,v));
}

void EigenSystem::linaminv(double* u, double* v)
{
  linaminv(new_DArray1(n,u),new_DArray1(n,v));
}

void EigenSystem::linaav(double* u, double* v)
{
  linaav(new_DArray1(n,u),new_DArray1(n,v));
}


// ------------- EigenSystemLua


EigenSystemLua::EigenSystemLua()
  : EigenSystem()
{
}

EigenSystemLua::~EigenSystemLua()
{
}


// Lua
void EigenSystemLua::linamv(const PtrA<DArray1>& u, PtrA<DArray1> v)
{
  assert(es);
  if(!(**es)["linamv"].is_function()) { ErrorMsg("base","missing Lua func evp.linamv(..)"); }
  LuaList args;
  args[1]=(**es)["vars"];
  args[2]=new LuaArray<DArray1>(u);
  args[3]=new LuaArray<DArray1>(v);
  (**es)["linamv"](args);
}

void EigenSystemLua::linaainv(const PtrA<DArray1>& u, PtrA<DArray1> v)
{
  assert(es);
  if(!(**es)["linaainv"].is_function()) { ErrorMsg("base","missing Lua func evp.linaainv(..)"); }
  LuaList args;
  args[1]=(**es)["vars"];
  args[2]=new LuaArray<DArray1>(u);
  args[3]=new LuaArray<DArray1>(v);
  (**es)["linaainv"](args);
}

void EigenSystemLua::linaminv(const PtrA<DArray1>& u, PtrA<DArray1> v)
{
  assert(es);
  if(!(**es)["linaminv"].is_function()) { ErrorMsg("base","missing Lua func evp.linaminv(..)"); }
  LuaList args;
  args[1]=(**es)["vars"];
  args[2]=new LuaArray<DArray1>(u);
  args[3]=new LuaArray<DArray1>(v);
  (**es)["linaminv"](args);
}

void EigenSystemLua::linaav(const PtrA<DArray1>& u, PtrA<DArray1> v)
{
  assert(es);
  if(!(**es)["linaav"].is_function()) { ErrorMsg("base","missing Lua func evp.linaav(..)"); }
  LuaList args;
  args[1]=(**es)["vars"];
  args[2]=new LuaArray<DArray1>(u);
  args[3]=new LuaArray<DArray1>(v);
  (**es)["linaav"](args);
}

bool EigenSystemLua::es_solve(const LuaObjRef& es_)
{ ErrorCallStack ecs("es_solve(ep)");
  
  es=(LuaObjRef*)&es_;
  
  LuaObjGet(n  ,(*es_)["n"  ]);
  LuaObjGet(nev,(*es_)["nev"]);
  LuaObjGet(ncv,(*es_)["ncv"]);
  
  LuaObjGet(sym   ,(*es_)["sym"   ]);
  LuaObjGet(bmat  ,(*es_)["bmat"  ]);
  LuaObjGet(mode  ,(*es_)["mode"  ]);
  LuaObjGet(sigmar,(*es_)["sigmar"]);
  LuaObjGet(sigmai,(*es_)["sigmai"]);
  LuaObjGet(maxitr,(*es_)["maxitr"]);  
  
  
  if(n<=0) { ErrorMsg("base","n<=0"); return false; }
  
  bool b=EigenSystemBase::es_solve();
  
  es=0;
  
  return b;
  
//  residual    =new_DArray1(n,    p_res   );
//  eigenvalues =new_DArray1(  nev,p_lambdar);
//  eigenvectors=new_DArray2(n,nev,p_xx    );
}

double EigenSystemLua::residual(int j)
{
  int nconv = iparam(5); // no convergence
  if(j>nconv) { ErrorMsg("base","j>nconv (no convergence)"); return 0; }
  
  return res(j);
}

PtrA<DArray1> EigenSystemLua::eigenvector(int j, bool normalize)
{
  PtrA<DArray1> v=da1Create(n);
  if(!normalize)
  {
    for(int i=1;i<=n;++i)
     (*v)(i)=xx(i,j);
  } else
  {
    double norm=0.0;
    for(int i=1;i<=n;++i)
    {
      const double xxij=xx(i,j);
      (*v)(i)=xxij;
      norm+=xxij*xxij;
    }
    norm=sqrt(norm);
    for(int i=1;i<=n;++i)
    {
      (*v)(i)/=norm;
    }
  }
  return v;
}

double EigenSystemLua::eigenvaluer(int j)
{
  return lambdar(j);
}

double EigenSystemLua::eigenvaluei(int j)
{
  return lambdai(j);
}









