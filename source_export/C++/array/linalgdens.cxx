#include <config.h>

#include "linalgdens.h"


#ifdef PACKAGE_ARPACK

extern "C" {

// blas

#define F77_daxpy F77_FUNC(daxpy,DAXPY)
void F77_daxpy(int *n1
             , double *a
             , double *x, int *sx, 
               double *y, int *sy);

#define F77_dnrm2 F77_FUNC(dnrm2,DNRM2)
double F77_dnrm2(int* n, double* x, int* incx);


// arpack

#define F77_dsaupd F77_FUNC(dsaupd,DSAUPD)
void F77_dsaupd( int* ido, const char* bmat, int* n, const char* which,
            int* nev, double* tol, double* workr,
            int* ncv, double* v, int* ldv, int* iparam, int* ipntr, double* workd, double* workl,
            int* lworkl, int* info );

#define F77_dnaupd F77_FUNC(dnaupd,DNAUPD)
void F77_dnaupd( int* ido, const char* bmat, int* n, const char* which,
            int* nev, double* tol, double* workr,
            int* ncv, double* v, int* ldv, int* iparam, int* ipntr, double* workd, double* workl,
            int* lworkl, int* info );

#define F77_dseupd F77_FUNC(dseupd,DSEUPD)
void F77_dseupd(int* rvec, const char* howmany, int* select,
          double* lambda,
          double* z, int* ldz, // hier auch v
          double* sigma,
          const char* bmat,
          int* n,
          const char* which,
          int* nev,
          double* tol,
          double* workr,
          int* ncv,
          double* v, int* ldv,
          int* iparam,int* ipntr,
          double* workd, double* workl, int* lworkl,
          int* ierr);
  
#define F77_dneupd F77_FUNC(dneupd,DNEUPD)
void F77_dneupd(int* rvec, const char* howmany, int* select,
          double* lambdar,double* lambdai,
          double* z, int* ldz, // hier auch v
          double* sigmar,double* sigmai,
          double* workev,
          const char* bmat,
          int* n,
          const char* which,
          int* nev,
          double* tol,
          double* workr,
          int* ncv,
          double* v, int* ldv,
          int* iparam,int* ipntr,
          double* workd, double* workl, int* lworkl,
          int* ierr);
}

#endif

static void daxpy(int n, double a, double* x, int sx, double* y, int sy)
{
  #ifdef PACKAGE_ARPACK
  
  F77_daxpy(&n, &a, x, &sx, y, &sy);
  
  #endif
}

static double dnrm2(int n, double* x, int incx)
{
  #ifdef PACKAGE_ARPACK
  
  return F77_dnrm2(&n, x, &incx);
  
  #endif
}

static char* dsaupd_info_err(int info)
{
  static char static_dsaupd_err[1024];
  
  char* s=static_dsaupd_err;
  strcpy(s,"unknown info in dsaupd_info_err()\n");
  
   if(info== -1   ) strcpy(s,"N must be positive.");
   if(info== -2   ) strcpy(s,"NEV must be positive.");
   if(info== -3   ) strcpy(s,"NCV must be greater than NEV and less than or equal to N.\n \
                     (it must hold: nev < ncv <= n)\n");
   if(info== -4   ) strcpy(s,"The maximum number of Arnoldi update iterations allowed\n \
                    must be greater than zero.");
   if(info== -5   ) strcpy(s,"WHICH must be one of 'LM', 'SM', 'LA', 'SA' or 'BE'.");
   if(info== -6   ) strcpy(s,"BMAT must be one of 'I' or 'G'.");
   if(info== -7   ) strcpy(s,"Length of private work array WORKL is not sufficient.");
   if(info== -8   ) strcpy(s,"Error return from trid. eigenvalue calculation;\n \
                    Informational error from LAPACK routine dsteqr.");
   if(info== -9   ) strcpy(s,"Starting vector is zero.");
   if(info== -10  ) strcpy(s,"IPARAM(7) must be 1,2,3,4,5.");
   if(info== -11  ) strcpy(s,"IPARAM(7) = 1 and BMAT = 'G' are incompatable.");
   if(info== -12  ) strcpy(s,"IPARAM(1) must be equal to 0 or 1.");
   if(info== -13  ) strcpy(s,"NEV and WHICH = 'BE' are incompatable.");
   if(info== -9999) strcpy(s,"Could not build an Arnoldi factorization.\n \
            IPARAM(5) returns the size of the current Arnoldi\n \
            factorization. The user is advised to check that\n \
            enough workspace and array storage has been allocated.");
  return s;
}

static char* dnaupd_info_err(int info)
{
  static char static_dnaupd_err[1024];
  
  char* s=static_dnaupd_err;
  strcpy(s,"unknown info in dnaupd_info_err()\n");
  
   if(info== -1   ) strcpy(s,"N must be positive.");
   if(info== -2   ) strcpy(s,"NEV must be positive.");
   if(info== -3   ) strcpy(s,"NCV-NEV >= 2 and less than or equal to N.\n \
                     (it must hold: 1 < ncv-nev <=n )\n");
   if(info== -4   ) strcpy(s,"The maximum number of Arnoldi update iterations allowed\n \
                    must be greater than zero.");
   if(info== -5   ) strcpy(s,"WHICH must be one of 'LM', 'SM', 'LR', 'SR', 'LI', 'SI'.");
   if(info== -6   ) strcpy(s,"BMAT must be one of 'I' or 'G'.");
   if(info== -7   ) strcpy(s,"Length of private work array WORKL is not sufficient.");
   if(info== -8   ) strcpy(s,"Error return from LAPACK. eigenvalue calculation;\n \
                    Informational error from LAPACK routine dsteqr.");
   if(info== -9   ) strcpy(s,"Starting vector is zero.");
   if(info== -10  ) strcpy(s,"IPARAM(7) must be 1,2,3,4.");
   if(info== -11  ) strcpy(s,"IPARAM(7) = 1 and BMAT = 'G' are incompatable.");
   if(info== -12  ) strcpy(s,"IPARAM(1) must be equal to 0 or 1.");
   if(info== -13  ) strcpy(s,"NEV and WHICH = 'BE' are incompatable.");
   if(info== -9999) strcpy(s,"Could not build an Arnoldi factorization.\n \
            IPARAM(5) returns the size of the current Arnoldi\n \
            factorization. The user is advised to check that\n \
            enough workspace and array storage has been allocated.");
  return s;
}









bool EigenSystemBase::do_mode1()
{ // derived from dsdrv1,dndrv1
  // regular
  
  #ifdef PACKAGE_ARPACK
  
  char str_bmat_[10];
  char str_which_[10];
  
  str_bmat(str_bmat_);
  str_s_which(str_which_);
  
  int ido=0, info=0;
  
  int ldv=n;
  
  // calc
  while(true)
  {
/*
same as dnaupd
call dsaupd 
   ( IDO, BMAT, N, WHICH, 
     NEV, TOL, RESID, 
     NCV, V, LDV, IPARAM, IPNTR, WORKD, WORKL,
     LWORKL, INFO )
IDO     Integer.  (INPUT/OUTPUT)
BMAT    Character*1.  (INPUT)
N       Integer.  (INPUT)
WHICH   Character*2.  (INPUT)
NEV     Integer.  (INPUT)
TOL     Double precision scalar.  (INPUT)
RESID   Double precision array of length N.  (INPUT/OUTPUT)
NCV     Integer.  (INPUT)
        Number of columns of the matrix V (less than or equal to N).
V       Double precision N by NCV array.  (OUTPUT)
LDV     Integer.  (INPUT)
IPARAM  Integer array of length 11.  (INPUT/OUTPUT)
IPNTR   Integer array of length 11.  (OUTPUT)
WORKD   Double precision work array of length 3*N.  (REVERSE COMMUNICATION)
WORKL   Double precision work array of length LWORKL.  (OUTPUT/WORKSPACE)
LWORKL  Integer.  (INPUT)
INFO    Integer.  (INPUT/OUTPUT)
*/
    if(sym)
     F77_dsaupd( &ido, str_bmat_, &n, str_which_,
            &nev, &tol, p_workr,
            &ncv, p_v, &ldv, p_iparam, p_ipntr, p_workd, p_workl,
            &lworkl, &info );
    else
     F77_dnaupd( &ido, str_bmat_, &n, str_which_,
            &nev, &tol, p_workr,
            &ncv, p_v, &ldv, p_iparam, p_ipntr, p_workd, p_workl,
            &lworkl, &info );
      
    if (ido == -1 || ido == 1)
    {
      linaav(&workd(ipntr(1)), &workd(ipntr(2)));
    } else
     break;
  }
  // info
  // 1: Maximum number of iterations reached
  // 3: No shifts could be applied during implicit, Arnoldi update, try increasing NCV.
  if(info < 0)
  {
    printf("ARPACK info: %s\n",sym?dsaupd_info_err(info):dnaupd_info_err(info));
    ErrorMsg("base",true,"error solve info =%i",info); return false;
  }
  #endif
  return true;
}

bool EigenSystemBase::do_mode2()
{ // derived from dsdrv3,dndrv3
  // regular-inverse
  
  #ifdef PACKAGE_ARPACK
  
  char str_bmat_[10];
  char str_which_[10];
  
  str_bmat(str_bmat_);
  str_s_which(str_which_);
  
  int ido=0, info=0;
  
  int ldv=n;
  
  // calc
  while(true)
  {
    if(sym)
     F77_dsaupd( &ido, str_bmat_, &n, str_which_,
            &nev, &tol, p_workr,
            &ncv, p_v, &ldv, p_iparam, p_ipntr, p_workd, p_workl,
            &lworkl, &info );
    else
     F77_dnaupd( &ido, str_bmat_, &n, str_which_,
            &nev, &tol, p_workr,
            &ncv, p_v, &ldv, p_iparam, p_ipntr, p_workd, p_workl,
            &lworkl, &info );
      
    if (ido == -1 || ido == 1)
    {
      linaav  (&workd(ipntr(1)), &workd(ipntr(2)));
      memcpy  (&workd(ipntr(1)), &workd(ipntr(2)), (n+1)*sizeof(double) ); // dst,src
      linaminv(&workd(ipntr(2)), &workd(ipntr(2))); /// TODO: clone
    } else if(ido==2)
    {
      linamv  (&workd(ipntr(1)), &workd(ipntr(2)));
    } else
     break;
  }
  // info
  // 1: Maximum number of iterations reached
  // 3: No shifts could be applied during implicit, Arnoldi update, try increasing NCV.
  if(info < 0)
  {
    printf("ARPACK info: %s\n",sym?dsaupd_info_err(info):dnaupd_info_err(info));
    ErrorMsg("base",true,"error solve info =%i",info); return false;
  }
  #endif
  return true;
}

bool EigenSystemBase::do_mode3()
{ // derived from dsdrv4,dndrv4 (bmat) and dsdrv2,dndrv2 (no bmat)
  // shift-invert
  
  #ifdef PACKAGE_ARPACK
  
  char str_bmat_[10];
  char str_which_[10];
  
  str_bmat(str_bmat_);
    if(sym)
      str_s_which(str_which_);
    else
      str_n_which(str_which_);
  printf("which: %s\n",str_which_);
  int ido=0, info=0;
  
  int ldv=n;
  
  // calc
  while(ido!=99)
  {
    if(sym)
     F77_dsaupd( &ido, str_bmat_, &n, str_which_,
            &nev, &tol, p_workr,
            &ncv, p_v, &ldv, p_iparam, p_ipntr, p_workd, p_workl,
            &lworkl, &info );
    else
     F77_dnaupd( &ido, str_bmat_, &n, str_which_,
            &nev, &tol, p_workr,
            &ncv, p_v, &ldv, p_iparam, p_ipntr, p_workd, p_workl,
            &lworkl, &info );
      
    if(bmat)
    {
      if (ido == -1)
      {
        linamv  (&workd(ipntr(1)), &workd(ipntr(2)));
        linaainv(&workd(ipntr(2)), &workd(ipntr(2))); /// TODO: clone
      } else if(ido == 1)
      {
        memcpy  (&workd(ipntr(2)), &workd(ipntr(3)), (n+1)*sizeof(double) ); // dst,src
        linaainv(&workd(ipntr(2)), &workd(ipntr(2)));
      } else if(ido==2)
      {
        linamv  (&workd(ipntr(1)), &workd(ipntr(2)));
      } else
       break;
    } else
      {
	if (ido == -1)
	  {
	    memcpy  (&workd(ipntr(2)), &workd(ipntr(1)), (n+1)*sizeof(double) );
	    linaainv(&workd(ipntr(2)), &workd(ipntr(2))); /// TODO: clone
	  }
	if (ido == 1)
	  {
	    linaainv(&workd(ipntr(2)), &workd(ipntr(3))); /// TODO: clone
	  }
      }
    
  }
  // info
  // 1: Maximum number of iterations reached
  // 3: No shifts could be applied during implicit, Arnoldi update, try increasing NCV.
  if(info < 0)
  {
    printf("ARPACK info: %s\n",sym?dsaupd_info_err(info):dnaupd_info_err(info));
    ErrorMsg("base",true,"error solve info =%i",info); return false;
  }
  #endif
  return true;
}








void EigenSystemBase::str_s_which(char* str_which) const
{
  if(is_s_which_LA()) strcpy(str_which,"LA");
  if(is_s_which_SA()) strcpy(str_which,"SA");
  if(is_s_which_LM()) strcpy(str_which,"LM");
  if(is_s_which_SM()) strcpy(str_which,"SM");
  if(is_s_which_BE()) strcpy(str_which,"BE");
}

void EigenSystemBase::str_n_which(char* str_which) const
{
  if(is_n_which_LM()) strcpy(str_which,"LM");
  if(is_n_which_SM()) strcpy(str_which,"SM");
  if(is_n_which_LR()) strcpy(str_which,"LR");
  if(is_n_which_SR()) strcpy(str_which,"SR");
  if(is_n_which_LI()) strcpy(str_which,"LI");
  if(is_n_which_SI()) strcpy(str_which,"SI");
}

void EigenSystemBase::str_bmat(char* str_bmat_) const
{
  if(bmat) strcpy(str_bmat_,"G");
   else    strcpy(str_bmat_,"I");
}








bool EigenSystemBase::do_s_extract()
{
  #ifdef PACKAGE_ARPACK
  
  char str_how_[10];
  char str_bmat_[10];
  char str_which_[10];
  
  strcpy(str_how_,"A");
  str_bmat(str_bmat_);
  str_s_which(str_which_);
  
  int ierr=0;
  int rvec=true;
  int* select=(int*)malloc((ncv+1000)*sizeof(int));
  for(int i=0;i<ncv;++i)
   select[i]=0;
  
/*
call dseupd ( rvec, 'All', select, d, z, ldz, sigma, 
     bmat, n, which, nev, tol, resid, ncv, v, ldv, 
     iparam, ipntr, workd, workl, lworkl, ierr )
RVEC    LOGICAL  (INPUT)
HOWMNY  Character*1  (INPUT)
SELECT  Logical array of dimension NCV.  (INPUT/WORKSPACE)
D       Double precision array of dimension NEV.  (OUTPUT)
Z       Double precision N by NEV array if HOWMNY = 'A'.  (OUTPUT)
LDZ     Integer.  (INPUT)
SIGMA   Double precision  (INPUT)
...
same as in the last call of dsaupd()
...
IERR

*/
  F77_dseupd(&rvec, str_how_, select,
              p_lambdar,
              p_xx, &n, 
             &sigmar,
              str_bmat_,
             &n,
              str_which_,
             &nev,
             &tol,
              p_workr,
             &ncv,
              p_v, &n,
              p_iparam, p_ipntr,
              p_workd, p_workl, &lworkl,
             &ierr);
  free(select);
  if(ierr)
  {
    ErrorMsg("base",true,"sym error extract ierr =%i",ierr); return false;
  }
  #endif
  return true;
}

bool EigenSystemBase::do_n_extract()
{
  #ifdef PACKAGE_ARPACK
  
  char str_how_[10];
  char str_bmat_[10];
  char str_which_[10];
  
  strcpy(str_how_,"A");
  str_bmat(str_bmat_);
  str_n_which(str_which_);
  
  int ierr=0;
  int rvec=true;
  int* select=(int*)malloc((ncv+1000)*sizeof(int));
  memset(select,0,(ncv+1)*sizeof(int));
  
/*
SUBROUTINE DNEUPD(RVEC, HOWMNY, SELECT, DR, DI, Z, LDZ, SIGMAR, SIGMAI,
                  WORKEV, BMAT, N, WHICH, NEV, TOL, RESID, NCV, V, LDV,
                  IPARAM, IPNTR, WORKD, WORKL, LWORKL, INFO )

WORKEV  Double precision work array of dimension 3*NCV.  (WORKSPACE)
*/
  F77_dneupd(&rvec, str_how_, select,
              p_lambdar,p_lambdai,
              p_xx, &n, 
             &sigmar,&sigmai,
              p_workev,
              str_bmat_,
             &n,
              str_which_,
             &nev,
             &tol,
              p_workr,
             &ncv,
              p_v, &n,
              p_iparam, p_ipntr,
              p_workd, p_workl, &lworkl,
             &ierr);
  free(select);
  if(ierr)
  {
    ErrorMsg("base",true,"nsym error extract ierr =%i",ierr); return false;
  }
  #endif
  return true;
}

void EigenSystemBase::do_I_residual()
{
  #ifdef PACKAGE_ARPACK
  
  assert(nconv);
  
  // residual:  ||  A*x - lambda*x ||
  
  for(int j=1;j<=nconv;++j)
  {
//    if( lambdar(j) == 0 ) TODO
//    {
      // Ritz value is real
      linaav(&xx(1,j), p_workd);
      daxpy(n, -lambdar(j), &xx(1,j), 1, p_workd, 1);
      res(j)  = dnrm2(n, p_workd, 1);
      res(j) /= fabs(lambdar(j));
//    } else
//    {
/*
 if (first) then
c
c                  %------------------------%
c                  | Ritz value is complex. |
c                  | Residual of one Ritz   |
c                  | value of the conjugate |
c                  | pair is computed.      | 
c                  %------------------------%
c        
                   call av(nx, v(1,j), ax)
                   call daxpy(n, -d(j,1), v(1,j), 1, ax, 1)
                   call daxpy(n, d(j,2), v(1,j+1), 1, ax, 1)
                   d(j,3) = dnrm2(n, ax, 1)
                   call av(nx, v(1,j+1), ax)
                   call daxpy(n, -d(j,2), v(1,j), 1, ax, 1)
                   call daxpy(n, -d(j,1), v(1,j+1), 1, ax, 1)
                   d(j,3) = dlapy2( d(j,3), dnrm2(n, ax, 1) )
                   d(j,3) = d(j,3) / dlapy2(d(j,1),d(j,2))
                   d(j+1,3) = d(j,3)
                   first = .false.
                else
                   first = .true.
                end if
*/
//    }
  }
  
  #endif
}

void EigenSystemBase::do_M_residual()
{
  #ifdef PACKAGE_ARPACK
  
  // residual:  ||  A*x - lambda*B*x ||
  
  // Ritz value is real
  for(int j=1;j<=nconv;++j)
  {
    linaav(&xx(1,j), &workd(1));
    linamv(&xx(1,j), &workd(n+1));
    daxpy (n, -lambdar(j), &workd(n+1), 1, &workd(1), 1);
    res(j)  =  dnrm2(n, &workd(1), 1);
    res(j) /= fabs(lambdar(j));
  }
  
  #endif
}

void EigenSystemBase::do_residual()
{
  if(bmat) do_M_residual();
   else do_I_residual();
}


EigenSystemBase::EigenSystemBase()
  : workl_dim(0)
  , workd_dim(0)
  , workr_dim(0)
  , p_workl (NULL)
  , p_workd (NULL)
  , p_workr (NULL)
  , p_workev(NULL)
  , p_v     (NULL)
  , p_xx    (NULL)
  , p_lambdar(NULL)
  , p_lambdai(NULL)
  , p_res   (NULL)
  , lworkl(0)
  , s_which(1)
  , n_which(1)
  , n   (0)
  , nev (0)
  , ncv (0)
  , sym (false)
  , bmat(false)
  , mode(1)
  , sigmar(0.0)
  , sigmai(0.0)
  , tol(0.0)
  , maxitr(300)
  , nconv(0)
{
  memset(p_iparam,0,(12)*sizeof(int));
  memset(p_ipntr ,0,(15)*sizeof(int)); 
}

EigenSystemBase::~EigenSystemBase()
{
  if(p_workl  ) free(p_workl );
  if(p_workd  ) free(p_workd );
  if(p_workr  ) free(p_workr );
  if(p_workev ) free(p_workev);
  if(p_v      ) free(p_v     );
  if(p_xx     ) free(p_xx    );
  if(p_lambdar) free(p_lambdar);
  if(p_lambdai) free(p_lambdai);
  if(p_res    ) free(p_res   );

}

void EigenSystemBase::ep_alloc()
{
  workl_dim=ncv*(ncv+8);
  workd_dim=3*n;
  workr_dim=n;
  
  // speicher !!!!!!!!!!!!!!!! nur mit +1 crash bei n=3, n=2 ok
  if(!p_workl  ) p_workl  =(double*)malloc( (workl_dim+10000)*sizeof(double) );
  if(!p_workd  ) p_workd  =(double*)malloc( (workd_dim+10000)*sizeof(double) );
  if(!p_workr  ) p_workr  =(double*)malloc( (workr_dim+10000)*sizeof(double) );
  if(!p_workev ) p_workev =(double*)malloc( (ncv      +10000)*sizeof(double) ); // only non-sym case
  if(!p_v      ) p_v      =(double*)malloc( (    ncv*n+10000)*sizeof(double) );
  if(!p_xx     ) p_xx     =(double*)malloc( (    nev*n+10000)*sizeof(double) );
  if(!p_lambdar) p_lambdar=(double*)malloc( (nev      +10000)*sizeof(double) );
  if(!p_lambdai) p_lambdai=(double*)malloc( (nev      +10000)*sizeof(double) );
  if(!p_res    ) p_res    =(double*)malloc( (nev      +10000)*sizeof(double) );
}

void EigenSystemBase::set_n(int n_)
{
  if(p_workd) { free(p_workd); p_workd=NULL; }
  if(p_workr) { free(p_workr); p_workr=NULL; }
  if(p_v    ) { free(p_v    ); p_v    =NULL; }
  if(p_xx   ) { free(p_xx   ); p_xx   =NULL; }
  
  n=n_;
}

void EigenSystemBase::set_nev(int nev_)
{
  if(p_xx     ) { free(p_xx     ); p_xx     =NULL; }
  if(p_lambdar) { free(p_lambdar); p_lambdar=NULL; }
  if(p_lambdai) { free(p_lambdai); p_lambdai=NULL; }
  if(p_res    ) { free(p_res    ); p_res    =NULL; }
  
  nev=nev_;
}

void EigenSystemBase::set_ncv(int ncv_)
{
  if(p_workl ) { free(p_workl ); p_workl =NULL; }
  if(p_v     ) { free(p_v     ); p_v     =NULL; }
  if(p_workev) { free(p_workev); p_workev=NULL; }
  
  ncv=ncv_;
}


bool EigenSystemBase::es_solve()
{ ErrorCallStack ecs("EigenSystemBase::es_solve()");
  
  #ifndef PACKAGE_ARPACK
   ErrorMsg("base","missing ARPACK");
   return false;
  #else
  
  ep_alloc();
  
  memset(p_workl,0,(workl_dim+1000)*sizeof(double));
  memset(p_workd,0,(workd_dim+1000)*sizeof(double));
  for(int i=0;i<n+100;++i) p_workr[i]=0.0; // starting residual vector (0.0==random)
  memset(p_workev,0,(ncv+1000)*sizeof(double)); // only non-sym case
  
  memset(p_v ,0,(ncv*n+1000)*sizeof(double));
  memset(p_xx,0,(nev*n+1000)*sizeof(double));
  
  nconv=0;
  
  
  // 1,3,7 all input
  iparam(1)=1; // exact shifts with respect to the current Hessenberg matrix
  iparam(3) = maxitr;
  iparam(7) = mode;
  
  if(sym)
  {
    lworkl = ncv*(ncv+8); // drv all
  //maxncv=1000;
    
  } else
  {
    lworkl = (3*ncv)*(3*ncv)+6*ncv; // drv all
  //maxn=256; // drv4
  //maxncv=25;
    
  }
  
  // ------- calc -------- //
  switch(mode)
  {
    case 1:
     if(!do_mode1())
      return false;
     break;
    case 2:
     if(!do_mode2())
      return false;
     break;
    case 3:
     if(!do_mode3())
      return false;
     break;
    default:
    ErrorMsg("base","mode not implemented"); return false;
    break;
  }
  
  // ------- extract -------- //
  if(sym)
  {
    if(!do_s_extract())
     return false;
  } else
  {
    if(!do_n_extract())
     return false;
  }
  
  nconv = iparam(5); // no convergence
  
  
  // ------- error -------- //
  do_residual();
    
  return true;
  #endif
}


//////////////////////////////////////////////////////////////


bool LinAlgDenseExt_mult(const DArray2& A, const DArray1& x, DArray1& y)
{ ErrorCallStack ecs("operator*(A,x)");
  
  if(A.ncol()!=x.nall()) { ErrorMsg("base","x is not compatible to A"); return false; }
  
  const int m=A.nrow();
  const int n=A.ncol();

  for(int i=1;i<=m;++i)
  {
    double a=0;
    
    for(int j=1;j<=n;++j)
     a+=A(i,j)*x(j);
    
    y(i)=a;
  }
  return true;
}


struct DArray1_lu
{
  DArray1& V;
  DArray1_lu(DArray1& v) : V(v) {}
  double& operator[](int i) { return V(i+1); }
  double operator[](int i) const { return V(i+1); }
};

struct const_DArray1_lu
{
  const DArray1& V;
  const_DArray1_lu(const DArray1& v) : V(v) {}
  double operator[](int i) const { return V(i+1); }
};

struct DArray2_lu
{
  DArray2& V;
  DArray2_lu(DArray2& v) : V(v) {}
  double& operator()(int i, int j) { return V(i+1,j+1); }
  double operator()(int i, int j) const { return V(i+1,j+1); }
};

struct const_DArray2_lu
{
  const DArray2& V;
  const_DArray2_lu(const DArray2& v) : V(v) {}
  double operator()(int i, int j) const { return V(i+1,j+1); }
};


DA_LU_decom::DA_LU_decom(const DArray2& A_)
  : A(A_)
  , dim(A.nrow())
  , scales(dim)
  , I(dim)
  , L(dim,dim)
{
  L.copy(A);
  decom_ok=decom();
}

bool DA_LU_decom::decom()
{
  if(A.n(1)!=A.n(2)) {
    ErrorCallStack ecs("DA_LU_decom::decom()");
    ErrorMsg("base","dim(A)!=(n,n)");
    return false;
  }
  DArray2_lu l(L);
  double d;
  return lu_decmpT<DArray2_lu>(dim,l,I.getVal77(),d,scales.getVal77());
}

bool DA_LU_decom::solve(const DArray1& y, DArray1& x)
{
  if(!is_decom_ok()) return false;
  
  if(dim!=y.nall()) { ErrorCallStack ecs("DA_LU_decom::solve(y,x)"); ErrorMsg("base","y is not compatible to A"); return false; }
  if(dim!=x.nall()) { ErrorCallStack ecs("DA_LU_decom::solve(y,x)"); ErrorMsg("base","x is not compatible to A"); return false; }
  
  DArray2_lu l(L);
  const_DArray1_lu ly(y);
  DArray1_lu lx(x);
  
  lu_solveT<DArray2_lu,const_DArray1_lu,DArray1_lu>(dim,l,I.getVal77(),ly,lx);
  return true;
}


bool LinAlgDenseExt_div(const DArray2& A, const DArray1& y, DArray1& x)
{ ErrorCallStack ecs("operator/(A,y)");
  
  DA_LU_decom lu(A);
  
  return lu.solve(y,x);
}


// #define USE_GAUSS_JORDAN

#ifdef USE_GAUSS_JORDAN
/******************************************************************************/
/* Perform Gauss-Jordan elimination with row-pivoting to obtain the solution to 
 * the system of linear equations
 * A X = B
 * 
 * Arguments:
 * 		lhs		-	left-hand side of the equation, matrix A
 * 		rhs		-	right-hand side of the equation, matrix B
 * 		nrows	-	number of rows in the arrays lhs and rhs
 * 		ncolsrhs-	number of columns in the array rhs
 * 
 * The function uses Gauss-Jordan elimination with pivoting.  The solution X to 
 * the linear system winds up stored in the array rhs; create a copy to pass to
 * the function if you wish to retain the original RHS array.
 * 
 * Passing the identity matrix as the rhs argument results in the inverse of 
 * matrix A, if it exists.
 * 
 * No library or header dependencies, but requires the function swaprows, which 
 * is included here.
 */

//  swaprows - exchanges the contents of row0 and row1 in a 2d array
inline void swaprows(double** arr, int row0, int row1)
{
    double* temp;
    temp=arr[row0];
    arr[row0]=arr[row1];
    arr[row1]=temp;
}

//	gjelim 
static void da_gjelim(double** lhs, double** rhs, int nrows, int ncolsrhs)
{

    //	augment lhs array with rhs array and store in arr2
    double** arr2=new double*[nrows];
    for (int row=0; row<nrows; ++row)
        arr2[row]=new double[nrows+ncolsrhs];

    for (int row=0; row<nrows; ++row) {
        for (int col=0; col<nrows; ++col) {
            arr2[row][col]=lhs[row][col];
        }
        for (int col=nrows; col<nrows+ncolsrhs; ++col) {
            arr2[row][col]=rhs[row][col-nrows];
        }
    }

    //	perform forward elimination to get arr2 in row-echelon form
    for (int dindex=0; dindex<nrows; ++dindex) {
        //	run along diagonal, swapping rows to move zeros in working position 
        //	(along the diagonal) downwards
        if ( (dindex==(nrows-1)) && (arr2[dindex][dindex]==0)) {
            return; //  no solution
        } else if (arr2[dindex][dindex]==0) {
            swaprows(arr2, dindex, dindex+1);
        }
        //	divide working row by value of working position to get a 1 on the
        //	diagonal
        if (arr2[dindex][dindex] == 0.0) {
            return;
        } else {
            double tempval=arr2[dindex][dindex];
            for (int col=0; col<nrows+ncolsrhs; ++col) {
                arr2[dindex][col]/=tempval;
            }
        }

        //	eliminate value below working position by subtracting a multiple of 
        //	the current row
        for (int row=dindex+1; row<nrows; ++row) {
            double wval=arr2[row][dindex];
            for (int col=0; col<nrows+ncolsrhs; ++col) {
                arr2[row][col]-=wval*arr2[dindex][col];
            }
        }
    }

    //	backward substitution steps
    for (int dindex=nrows-1; dindex>=0; --dindex) {
        //	eliminate value above working position by subtracting a multiple of 
        //	the current row
        for (int row=dindex-1; row>=0; --row) {
            double wval=arr2[row][dindex];
            for (int col=0; col<nrows+ncolsrhs; ++col) {
                arr2[row][col]-=wval*arr2[dindex][col];
            }
        }
    }

    //	assign result to replace rhs
    for (int row=0; row<nrows; ++row) {
        for (int col=0; col<ncolsrhs; ++col) {
            rhs[row][col]=arr2[row][col+nrows];
        }
    }

    for (int row=0; row<nrows; ++row)
        delete[] arr2[row];
    delete[] arr2;
}
#endif


PtrA<DArray2> da2Inverse(PtrA<DArray2> A)
{
  assert(A->nrow()==A->ncol());
  
  const int nc=A->ncol();
  const int nr=nc;
  
  PtrA<DArray2> I=da2Create(nr,nc);
  
  #ifdef USE_GAUSS_JORDAN
  
  int r,c;
  
  double** aa=new double*[nr];
  for(r=0; r<nr; ++r)
   aa[r]=new double[nr+nc];
  for(r=0; r<nr; ++r)
   for(c=0; c<nc; ++c)
    aa[r][c]=(*A)(r+1,c+1);
  
  double** ee=new double*[nr];
  for(r=0; r<nr; ++r)
   ee[r]=new double[nr+nc];
  
  for(r=0; r<nr; ++r)
   for(c=0; c<nc; ++c)
    ee[r][c]=bool(r==c);
  
  da_gjelim( aa, ee, nr, nc);
  
  for(r=0; r<nr; ++r)
   for(c=0; c<nc; ++c)
    (*I)(r+1,c+1)=ee[r][c];
  
  for(r=0; r<nr; ++r)
   delete[] aa[r];
  delete[] aa;
  
  for(r=0; r<nr; ++r)
   delete[] ee[r];
  delete[] ee;
  
  #else
  
  PtrA<DArray1> e=da1Create(nr);
  e->set0();
  
  PtrA<DArray1> x=da1Create(nr);
  
  DA_LU_decom lu(A);
  if(!lu.is_decom_ok()) return 0;
  
  for(int c=1;c<=nc;++c)
  {
    e(c)=1.0;
    
    lu.solve(e,x);
    
    for(int r=1;r<=nr;++r)
     (*I)(r,c)=x(r);
      
    e(c)=0.0;
  }
  
  #endif
  return I;
}
