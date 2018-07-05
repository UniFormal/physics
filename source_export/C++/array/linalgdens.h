/**
   \file linalgdens.h
   
   Yet another dense linear algebra and arpack interface
 */
#ifndef ARRAYLINALG_H
#define ARRAYLINALG_H

#include "array/array.h"
#include "array/array_stack.h"

#include "stdsys/swigbind.h"

////////////////////////////////////////
// Interface to blas for Array

extern "C"
{
  extern void  F77_FUNC(dgetrf,DGETRF)(int *n, int *m, double *a, int *lda, int* ipiv, int *info);
  extern void  F77_FUNC(dgetrs,DGETRS)(char *trans,int *n, const int *nrhs, double*a, int* lda, int *ipiv , double *b, int *ldb, int *info );
  extern void F77_FUNC(dgemm,DGEMM)(char *TRANSA, char *TRANSB, 
				    int *M, int * N, int *K, 
				    double *ALPHA, double *A, int *LDA, 
				    double *B, int *LDB, 
				    double *BETA,double * C, int *LDC);
  extern void F77_FUNC(dgesvd,DGESVD)(char *JOBU, char *JOBVT, int *M, int *N, double *A, int *LDA, double *S, double *U, int *LDU,
					double *VT, int *LDVT, double *WORK, int *LWORK, int *INFO);
}

SwigBindBegin

inline void lapack_svd(PtrA<DArray2> Mat, PtrA<DArray1> S, PtrA<DArray2> U, PtrA<DArray2> VT) {
  int m = da2NRow(Mat);
  int n = da2NCol(Mat);
  char jobu[2] = {'A','\0'};
  char jobvt[2] = {'A','\0'};
  double *mat = da2Val77(Mat);
  double *u = da2Val77(U);
  double *s = da1Val77(S);
  double *vt = da2Val77(VT);
  int lwork = -1;
  int info;
  double work;

  //Query the optimal workspace (lwork = -1)
  F77_FUNC(dgesvd,DGESVD)(jobu,jobvt,&m,&n,mat,&m,s,u,&m,vt,&n,&work,&lwork,&info);
  
  lwork = work;	//optimal workspacesize
  PtrA<DArray1> Work2 = new DArray1(lwork);
  double *work2 = da1Val77(Work2);

  //Compute SVD
  F77_FUNC(dgesvd,DGESVD)(jobu,jobvt,&m,&n,mat,&m,s,u,&m,vt,&n,work2,&lwork,&info);
}

inline void lapack_matmul(PtrA<DArray2> Mat, PtrA<DArray1> U, PtrA<DArray1> V)
{
  char transmat[2]={'T','\0'};
  char transvec[2]={'N','\0'};
  int n=da2NRow(Mat);
  double *mat=da2Val77(Mat);
  double *u=da1Val77(U);
  double *v=da1Val77(V);
  int ione=1;
  double done=1.0;
  double dzero=0.0;
  for (int i=0;i<n;i++) v[i]=0.0;
  F77_FUNC(dgemm,DGEMM)(transmat,transvec,
			&n,&ione,&n,
			&done,mat,&n,
			u,&n,
			&dzero,v,&n);
			
}

inline void lapack_lu_decmp(PtrA<DArray2> Mat, PtrA<IArray1> IPiv)
{
  int n=da2NRow(Mat);
  assert(ia1NAll(IPiv)>=n);
  double *mat=da2Val77(Mat);
  int *ipiv=ia1Val77(IPiv);
  int info;
  F77_FUNC(dgetrf,DGETRF)(&n,&n,mat,&n,ipiv,&info);
}

inline void lapack_lu_solve(PtrA<DArray2> Mat, PtrA<IArray1> IPiv, PtrA<DArray1> Sol, PtrA<DArray1> Rhs)
{
  char trans[2]={'T','\0'};
  int n=da2NRow(Mat);
  int one=1;
  int info;
  double *mat=da2Val77(Mat);
  int *ipiv=ia1Val77(IPiv);
  double *sol=da1Val77(Sol);
  double *rhs=da1Val77(Rhs);
  for (int i=0;i<n;i++) sol[i]=rhs[i];
  F77_FUNC(dgetrs,DGETRS)(trans,  &n,  &one,  mat,&         n,    ipiv,   sol,       &  n,&info);
}

SwigBindEnd

inline void lapack_svd(darray2& Mat, darray2& U, darray2& VT, darray1& S) {
  int n = Mat.nrow();
  int m = Mat.ncol();
  char jobu[2] = {'A','\0'};
  char jobvt[2] = {'A','\0'};
  double *mat = Mat.vadr();
  double *u = U.vadr();
  double *s = S.vadr();
  double *vt = VT.vadr();
  int lwork = -1;
  int info;
  double work;
  
  F77_FUNC(dgesvd,DGESVD)(jobu,jobvt,&n,&m,mat,&n,s,u,&n,vt,&m,&work,&lwork,&info);
  lwork = work;
  
  Ptr<DArray1> Work2 = new DArray1(lwork);
  DArray1& work2 = Work2;
  double *work_ = work2.vadr();

  F77_FUNC(dgesvd,DGESVD)(jobu,jobvt,&m,&n,mat,&m,s,u,&m,vt,&n,work_,&lwork,&info);
}

inline void lapack_matmul(darray2& Mat, darray1& U, darray1& V)
{
  char transmat[2]={'T','\0'};
  char transvec[2]={'N','\0'};
  int n=Mat.nrow();
  double *mat=Mat.vadr();
  double *u=U.vadr();
  double *v=V.vadr();
  int ione=1;
  double done=1.0;
  double dzero=0.0;
  for (int i=0;i<n;i++) v[i]=0.0;  
  F77_FUNC(dgemm,DGEMM)(transmat,transvec,
			&n,&ione,&n,
			&done,mat,&n,
			u,&n,
			&dzero,v,&n);
  
}

inline void lapack_lu_decmp(darray2& Mat, iarray1& IPiv)
{
  int n=Mat.nrow();
  assert(IPiv.nall()>=n);
  double *mat=Mat.vadr();
  int *ipiv=IPiv.vadr();
  int info;
  F77_FUNC(dgetrf,DGETRF)(&n,&n,mat,&n,ipiv,&info);
}

inline void lapack_lu_solve(darray2& Mat, iarray1& IPiv, darray1& Sol, darray1& Rhs)
{
  char trans[2]={'T','\0'};
  int n=Mat.nrow();
  int one=1;
  int info;
  double *mat=Mat.vadr();
  int *ipiv=IPiv.vadr();
  double *sol=Sol.vadr();
  double *rhs=Rhs.vadr();
  for (int i=0;i<n;i++) sol[i]=rhs[i];
  F77_FUNC(dgetrs,DGETRS)(trans,  &n,  &one,  mat,&n,    ipiv,   sol,       &  n,&info);
}



///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// lu_decmp()    Compute the LU decomposition of a matrix.                   //
//                                                                           //
// Compute the LU decomposition of a (non-singular) square matrix A using    //
// partial pivoting and implicit row exchanges.  The result is:              //
//     A = P * L * U,                                                        //
// where P is a permutation matrix, L is unit lower triangular, and U is     //
// upper triangular.  The factored form of A is used in combination with     //
// 'lu_solve()' to solve linear equations: Ax = b, or invert a matrix.       //
//                                                                           //
// The inputs are a square matrix 'lu[N..n+N-1][N..n+N-1]', it's size is 'n'.//
// On output, 'lu' is replaced by the LU decomposition of a rowwise permuta- //
// tion of itself, 'ps[N..n+N-1]' is an output vector that records the row   //
// permutation effected by the partial pivoting, effectively,  'ps' array    //
// tells the user what the permutation matrix P is; 'd' is output as +1/-1   //
// depending on whether the number of row interchanges was even or odd,      //
// respectively.                                                             //
//                                                                           //
// Return true if the LU decomposition is successfully computed, otherwise,  //
// return false in case that A is a singular matrix.                         //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

template<class MAT> inline
bool lu_decmpT(const int n, MAT& lu, int* ps, double& d, double* scales)
{
  const int N=0;
//double scales[4];
  double pivot, biggest, mult, tempf;
  int pivotindex = 0;
  int i, j, k;

  d = 1.0;                                      // No row interchanges yet.

  for (i = N; i < n + N; i++) {                             // For each row.
    // Find the largest element in each row for row equilibration
    biggest = 0.0;
    for (j = N; j < n + N; j++)
      if (biggest < (tempf = fabs(lu(i,j))))
        biggest  = tempf;
    if (biggest != 0.0)
      scales[i] = 1.0 / biggest;
    else {
      scales[i] = 0.0;
      return false;                            // Zero row: singular matrix.
    }
    ps[i] = i;                                 // Initialize pivot sequence.
  }

  for (k = N; k < n + N - 1; k++) {                      // For each column.
    // Find the largest element in each column to pivot around.
    biggest = 0.0;
    for (i = k; i < n + N; i++) {
      if (biggest < (tempf = fabs(lu(ps[i],k)) * scales[ps[i]])) {
        biggest = tempf;
        pivotindex = i;
      }
    }
    if (biggest == 0.0) {
      return false;                         // Zero column: singular matrix.
    }
    if (pivotindex != k) {                         // Update pivot sequence.
      j = ps[k];
      ps[k] = ps[pivotindex];
      ps[pivotindex] = j;
      d = -d;                          // ...and change the parity of d.
    }

    // Pivot, eliminating an extra variable  each time
    pivot = lu(ps[k],k);
    for (i = k + 1; i < n + N; i++) {
      lu(ps[i],k) = mult = lu(ps[i],k) / pivot;
      if (mult != 0.0) {
        for (j = k + 1; j < n + N; j++)
          lu(ps[i],j) -= mult * lu(ps[k],j);
      }
    }
  }

  // (lu[ps[n + N - 1]][n + N - 1] == 0.0) ==> A is singular.
  return lu(ps[n + N - 1],n + N - 1) != 0.0;
}

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// lu_solve()    Solves the linear equation:  Ax = b,  after the matrix A    //
//               has been decomposed into the lower and upper triangular     //
//               matrices L and U, where A = LU.                             //
//                                                                           //
// 'lu[N..n+N-1][N..n+N-1]' is input, not as the matrix 'A' but rather as    //
// its LU decomposition, computed by the routine 'lu_decmp'; 'ps[N..n+N-1]'  //
// is input as the permutation vector returned by 'lu_decmp';  'b[N..n+N-1]' //
// is input as the right-hand side vector, and returns with the solution     //
// vector. 'lu', 'n', and 'ps' are not modified by this routine and can be   //
// left in place for successive calls with different right-hand sides 'b'.   //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

template<class MAT, class VEC1, class VEC2> inline
void lu_solveT(const int n, const MAT& lu, int* ps, const VEC1& b, VEC2& X)
{
  const int N=0;
  int i, j;
  double dot;
  //double X[4];
  
  for (i = N; i < n + N; i++) X[i] = 0.0;

  // Vector reduction using U triangular matrix.
  for (i = N; i < n + N; i++) {
    dot = 0.0;
    for (j = N; j < i + N; j++)
      dot += lu(ps[i],j) * X[j];
    X[i] = b[ps[i]] - dot;
  }

  // Back substitution, in L triangular matrix.
  for (i = n + N - 1; i >= N; i--) {
    dot = 0.0;
    for (j = i + 1; j < n + N; j++)
      dot += lu(ps[i],j) * X[j];
    X[i] = (X[i] - dot) / lu(ps[i],i);
  }

  //for (i = N; i < n + N; i++) sol[i] = X[i];
}


bool LinAlgDenseExt_div(const DArray2& A, const DArray1& y, DArray1& x);
bool LinAlgDenseExt_mult(const DArray2& A, const DArray1& x, DArray1& y);




//////////////////////////////////////////////////////////////////////////////////////////////////
/**

 Notes on ARPACK interface:
 
 We consider only M and A real
 with A
  - (s) sym.
  - (n) nonsym.
 
 Use of dsaupd for (s), and dnaupd for (n).
 
 Types of EVP (the chars represents the values of 'bmat' in ARPACK)
  - (I) standard   $A x = lam x$
  - (M) genral     $A x = lam M x$, M sym. and pos. def.
 
 Three modes (the nums represents the values of 'iparam(7)' in ARPACK)
 - (1) regular
 - (2) regular-inverse
 - (3) shift-invert  (much better convergence)
 
 Use of equivalent EVPs (with same x and lam)
 
  - (E1) $(OP     - lam I       ) x = 0$
  - (E2) $(OP^-1 - (lam-sigma) I) x = 0$
 
 The EVPs transformations:
 
 \code
  drv type  mode  trans                       eq EVP
   d1   I    1     OP=A               , B=I    E1
   d2   I    3     OP=(A-sigma I)^-1  , B=I    E2
   d3   M    2     OP=M^-1 A          , B=M    E1
   d4   M    3     OP=(A-sigma I)^-1 M, B=M    E2
  \endcode
 The first column gives the link back to the ARPACK drivers.
  
 In the sym case (s) following selections are possible (the
 chars represents the values of 'which' in ARPACK)
  
  (LA) Largest algebraic EVs.
  (SA) Smallest algebraic EVs.
  (LM) EVs largest in magnitude.
  (SM) EVs smallest in magnitude.
  (BE) Compute nev EVs, half from
 	     each end of the spectrum. When nev
 	     is odd, compute one more from the
 	     high end than from the low end.
  
 In the nonsym case (n) following selections are possible (the
 chars represents the values of 'which' in ARPACK)
  
  (LM) EVs of largest magnitude. 	 
  (SM) EVs of smallest magnitude.
  (LR) EVs of largest real part.
  (SR) EVs of smallest real part.
  (LI) EVs of largest imaginary part.
  (SI) EVs of smallest imaginary part.  
  

*/
SwigBind class EigenSystemBase {
  
 protected:
  
  void ep_alloc();
  
  int workl_dim;
  int workd_dim;
  int workr_dim;
  
  double* p_workl;
  double* p_workd;
  double* p_workr;
  double* p_workev; // only non-sym case
  double* p_v;
  double* p_xx;
  double* p_lambdar;
  double* p_lambdai;
  double* p_res  ; // residual
  
  double& workl (int i       ) { return p_workl[i-1]; }
  double& workd (int i       ) { return p_workd[i-1]; }
  double& workr (int i       ) { return p_workr[i-1]; }
  double& xx    (int i, int j) { return p_xx[ (j-1)*n + i-1]; } // fortran access (reverse order)
  double& lambdar(int i       ) { return p_lambdar[i-1]; }
  double& lambdai(int i       ) { return p_lambdai[i-1]; }
  double& res   (int i       ) { return p_res[i-1]; }
  
  double workl (int i       ) const { return p_workl[i-1]; }
  double workd (int i       ) const { return p_workd[i-1]; }
  double workr (int i       ) const { return p_workr[i-1]; }
  double xx    (int i, int j) const { return p_xx[ (j-1)*n + i-1]; }
  double lambdar(int i       ) const { return p_lambdar[i-1]; }
  double lambdai(int i       ) const { return p_lambdai[i-1]; }
  double res   (int i       ) const { return p_res[i-1]; }
  
  int lworkl;
  
  int s_which;
  int n_which;
  
  bool do_s_extract();
  bool do_n_extract();
  
  void do_I_residual();
  void do_M_residual();
  void do_residual();
  
  int p_iparam[12];
  int p_ipntr[15];
  
  int iparam(int i) const { return p_iparam[i-1]; }
  int ipntr(int i) const { return p_ipntr[i-1]; }
  int& iparam(int i) { return p_iparam[i-1]; }
  int& ipntr(int i) { return p_ipntr[i-1]; }
  
  bool do_mode1();
  bool do_mode2();
  bool do_mode3();
  
  void str_s_which(char*) const;
  void str_n_which(char*) const;
  void str_bmat(char*) const;
  
SwigBindBegin
  
 public:
  
  // INPUT
  int n;   ///< size of eigenvector
  int nev; ///< no. eigenvalues
  int ncv; ///< no. Arnoldi vectors, it must hold 'nev < ncv <= n' (sym), '1 < ncv-nev <=n' (non-sym)
  
  bool sym;   // default: false
  bool bmat;  // default: false, true B=M, else B=I
  int  mode;  // default: 1    , iparam(7)
  double sigmar; // default: 1.0, shift
  double sigmai; // default: 1.0, shift
  
  double tol; ///< default: 0.0, convergence criterion
  int maxitr; ///< default: 300, maximum number of Arnoldi iterations allowed.
  
  void set_n(int n);     ///< sets n   and frees parts of the work space.
  void set_nev(int nev); ///< sets nev and frees parts of the work space.
  void set_ncv(int ncv); ///< sets ncv and frees parts of the work space.
  
  // for sym case (ascending order)
  void set_s_which_LA() { s_which=1; } ///< default
  void set_s_which_SA() { s_which=2; }
  void set_s_which_LM() { s_which=3; }
  void set_s_which_SM() { s_which=4; }
  void set_s_which_BE() { s_which=5; }
  bool is_s_which_LA() const { return s_which==1; }
  bool is_s_which_SA() const { return s_which==2; }
  bool is_s_which_LM() const { return s_which==3; }
  bool is_s_which_SM() const { return s_which==4; }
  bool is_s_which_BE() const { return s_which==5; }
  
  // for non sym case (ascending order)
  void set_n_which_LM() { n_which=1; } ///< default
  void set_n_which_SM() { n_which=2; }
  void set_n_which_LR() { n_which=3; }
  void set_n_which_SR() { n_which=4; }
  void set_n_which_LI() { n_which=5; }
  void set_n_which_SI() { n_which=6; }
  bool is_n_which_LM() const { return n_which==1; }
  bool is_n_which_SM() const { return n_which==2; }
  bool is_n_which_LR() const { return n_which==3; }
  bool is_n_which_SR() const { return n_which==4; }
  bool is_n_which_LI() const { return n_which==5; }
  bool is_n_which_SI() const { return n_which==6; }
  
  EigenSystemBase();
  
  virtual ~EigenSystemBase();
  
  double es_valr(int j) const { return lambdar(j); } ///< Get eigenvalue j
  double es_vali(int j) const { return lambdai(j); } ///< Get eigenvalue j
  double es_vec(int i, int j) const { return xx(i,j); } ///< Get eigenvector j with components i
  
  /**
   EVP: K a = lamda M a
   
   A=K-lamda*M
   
   A a = M 'U'  (what is U?)
   
   sigma: shift parameter
  */
  bool es_solve();
  
  int nconv; // (out) number of convergence
  
SwigBindEnd
 
 /* protected: */
  
  /**
   for mode: 2,3 (only if bmat=true)
   
   \param out  v = M u
  */
  virtual void linamv(double* u, double* v) { }
  
  /**
   for mode: 3
   
   \param out  v = A^-1 u
   
   !!!!! manchmal ist anfangs u==v
  */
  virtual void linaainv(double* u, double* v) { }

  /**
   for mode: 2 (only if bmat=true)
   
   \param out  v = M^-1 u
   
x   !!!!! manchmal ist anfangs u==v
  */
  virtual void linaminv(double* u, double* v) { }

  /**
   for mode: 1,2
   
   \param out  v = K u
  */
  virtual void linaav(double* u, double* v) { }
SwigBind };
SwigBindPtr(EigenSystemBase)






#endif

