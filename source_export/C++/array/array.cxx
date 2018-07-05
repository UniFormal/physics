#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#ifdef NEED_STDINT 
#include <stdint.h>
#endif

#include <complex>

#ifdef WITH_SWIGWRAP
#include "luacxx/swigext_lua.h"
#endif

#include "stdsys/dprintf.h"
#include "stdsys/mathext.h"
#include "linalgdens.h"
#include "array.h"
#include "arrayprint.h"
#include <typeinfo>


#ifndef NO_XDR
#include <stdio.h>
#ifdef HAVE_FSEEKO
#define fseek fseeko
#define ftell ftello
#endif
#include <rpc/rpc.h>
#endif



using namespace std;

static int  info=1;
static int  error=1;
int ld_init_array(void);

LinAlgDenseExt default_LinAlgDenseExt;

template <class T> int TArray<T>::jl_array_offset=0;


void aSet_print_test_raw(bool b)
{
  static_array_print_test_raw=b;
}
void aSet_print_test_val(bool b)
{
  static_array_print_test_val=b;
}

void aSet_memacs_stop_on_error(bool on)
{
  memacs_stop_on_error=on;
}


template<typename T>
cTypePerformer<T>::cTypePerformer()
{
  if(typeid(VTYPE)==typeid(int)) {
   basetype=a_int;
   strcpy(vtype_str,"I");
   return;
  }
  if(typeid(VTYPE)==typeid(double)) {
   basetype=a_double;
   strcpy(vtype_str,"D");
   return;
  }
  if(typeid(VTYPE)==typeid(double_complex)) {
   basetype=a_double_complex;
   strcpy(vtype_str,"Z");
   return;
  }
  if(typeid(VTYPE)==typeid(char)) {
   basetype=a_char;
   strcpy(vtype_str,"B");
   return;
  }
  assert(0);
}

template struct cTypePerformer<double_complex>;
template struct cTypePerformer<double>;
template struct cTypePerformer<int>;
template struct cTypePerformer<char>;



// ------------------------------------------------------- //


// ------------------------------------------------------- //

template<typename T>
void TArray<T>::renew_mem(value_type * mem, int memoffset, free_func mf)
{
   buf=0;
   vir=0;
   if(mem) {
     vir=new vir_type(acs.n_elem(),mem,memoffset,mf);
     v=vir->v;
   } else { 
     buf=new buf_type(acs.n_elem());
     v=buf->v;
   }
}

template<typename T>
void TArray<T>::set0()
{
  switch(dim())
  {
  case 1: memset(&v[acs(1)],0,vdim()*sizeof(value_type)); break;
  case 2: memset(&v[acs(1,1)],0,vdim()*sizeof(value_type)); break;
  case 3: memset(&v[acs(1,1,1)],0,vdim()*sizeof(value_type)); break;
  case 4: memset(&v[acs(1,1,1,1)],0,vdim()*sizeof(value_type)); break;
  case 5: memset(&v[acs(1,1,1,1,1)],0,vdim()*sizeof(value_type)); break;
  }
}

template<typename T>
void TArray<T>::copy_v(const self& src)
{
  if(src.nall()>nall())
  {
    printf("error in TArray<T>::copy_v(src): incompatible dimensions\n");
    assert(0);
  }
  if(!src.vdim()) return;
  
  const T* const src_v=src.v;
  switch(dim())
  {
  case 1: memcpy(&v[acs(1)],&src_v[acs(1)],src.vdim()*sizeof(value_type)); break;
  case 2: memcpy(&v[acs(1,1)],&src_v[acs(1,1)],src.vdim()*sizeof(value_type)); break;
  case 3: memcpy(&v[acs(1,1,1)],&src_v[acs(1,1,1)],src.vdim()*sizeof(value_type)); break;
  case 4: memcpy(&v[acs(1,1,1,1)],&src_v[acs(1,1,1,1)],src.vdim()*sizeof(value_type)); break;
  case 5: memcpy(&v[acs(1,1,1,1,1)],&src_v[acs(1,1,1,1,1)],src.vdim()*sizeof(value_type)); break;
  }
}



template<typename T>
bool TArray<T>::extend(size_t* ii)
{
  // new size
  bool recalc=false;
  MemAccess a = maCreate(acs);
  for(int d=1;d<=dim();++d)
   if(ii[d]>(size_t)(a.n[d])) recalc=true; // extend
    else ii[d]=a.n[d];
  
  // update access
  if(recalc)
    a.calculate_multipliers(ii);
  
  // blow up
  if(a.n_elem()>acs.n_elem()) {
    if(vir)
     return false;
    assert(buf);
    //buf->resize((size_t)(1.2*(a.n_elem()+1)));
    buf->resize(a.n_elem());
    v=buf->v;
  }
  
  if(recalc)
    acs=maCreate(a);
  
  return true;
}

template<typename T>
bool TArray<T>::resize(const size_t* ii)
{
  // new size
  bool recalc=false;
  MemAccess a = maCreate(acs);
  for(int d=1;d<=dim();++d)
   if(ii[d]!=(size_t)(a.n[d])) recalc=true;
  
  // update access
  if(recalc)
    a.calculate_multipliers(ii);
  
  // resize
  if(a.n_elem()!=acs.n_elem()) {
    if(vir)
     return false;
    assert(buf);
    buf->resize((size_t)(a.n_elem()));
    v=buf->v;
  }
  
  if(recalc)
    acs=maCreate(a);
  
  return true;
}


template<typename T>
void TArray<T>::set_vector_vtype()
{
  sprintf(vtype,"%sArray%i", tp.vtype_str, type_dim());
}



template<typename T>
void TArray<T>::copy_from_init_list( const init_list_type_1d& il)
{
  int n1=1;
  for (auto j1 = il.begin(); j1 != il.end(); j1++)
   v[acs(n1++)] = *j1;
}


inline void sz_assert(size_t jsz_, int n, int d)
{
  #if PDELIB_VARIANT_DBG
  int jsz = int(jsz_);
  if(jsz!=n)
   ErrorMsg("base",memacs_stop_on_error,"Array dimension not rectangular: expect n(%i)=%i, but got %i\n",d,n,jsz);
  #endif    
}

template<typename T>
void TArray<T>::copy_from_init_list( const init_list_type_2d& il)
{
  int n1=1;
  for (auto j1 = il .begin(); j1 != il .end(); j1++, n1++) { int n2=1; sz_assert(j1->size(),n(2),2);
  for (auto j2 = j1->begin(); j2 != j1->end(); j2++      )
   v[acs(n1,n2++)] = *j2;
  }
}


template<typename T>
void TArray<T>::copy_from_init_list( const init_list_type_3d& il)
{
  int n1=1;
  for (auto j1 = il .begin(); j1 != il .end(); j1++, n1++) { int n2=1; sz_assert(j1->size(),n(2),2);
  for (auto j2 = j1->begin(); j2 != j1->end(); j2++, n2++) { int n3=1; sz_assert(j2->size(),n(3),3);
  for (auto j3 = j2->begin(); j3 != j2->end(); j3++      )
   v[acs(n1,n2,n3++)] = *j3;
  }}
}


template<typename T>
void TArray<T>::copy_from_init_list( const init_list_type_4d& il)
{
  int n1=1;
  for (auto j1 = il .begin(); j1 != il .end(); j1++, n1++) { int n2=1; sz_assert(j1->size(),n(2),2);
  for (auto j2 = j1->begin(); j2 != j1->end(); j2++, n2++) { int n3=1; sz_assert(j2->size(),n(3),3);
  for (auto j3 = j2->begin(); j3 != j2->end(); j3++, n3++) { int n4=1; sz_assert(j3->size(),n(4),4);
  for (auto j4 = j3->begin(); j4 != j3->end(); j4++      )
   v[acs(n1,n2,n3,n4++)] = *j4;
  }}}
}


template<typename T>
void TArray<T>::copy_from_init_list( const init_list_type_5d& il)
{
  int n1=1;
  for (auto j1 = il .begin(); j1 != il .end(); j1++, n1++) { int n2=1; sz_assert(j1->size(),n(2),2);
  for (auto j2 = j1->begin(); j2 != j1->end(); j2++, n2++) { int n3=1; sz_assert(j2->size(),n(3),3);
  for (auto j3 = j2->begin(); j3 != j2->end(); j3++, n3++) { int n4=1; sz_assert(j3->size(),n(4),4);
  for (auto j4 = j3->begin(); j4 != j3->end(); j4++, n4++) { int n5=1; sz_assert(j4->size(),n(5),5);
  for (auto j5 = j4->begin(); j5 != j4->end(); j5++      )
   v[acs(n1,n2,n3,n4,n5++)] = *j5;
  }}}}
}

  









// ------------------------------------------------------- //

template<class TARRAY> inline
int aCheck(const Ptr<TARRAY> & a, int dim, const char *file, int line)
{
  int rc=1;
  if (a->dim()!=dim) {
    ld_init_array();
    dprintf(error)("%s(%d): %p: wrong array dim %d. Expected: %d\n", file,line,a.ptr(),a->dim(),dim);
    rc=0;
  }
  if(a->buf)
   rc=a->buf->check();
  return rc;
}

int da1Check0(const PtrA<DArray1> & a, const char *file, int line) { return aCheck(a,1,file,line); }
int da2Check0(const PtrA<DArray2> & a, const char *file, int line) { return aCheck(a,2,file,line); }
int da3Check0(const PtrA<DArray3> & a, const char *file, int line) { return aCheck(a,3,file,line); }
int da4Check0(const PtrA<DArray4> & a, const char *file, int line) { return aCheck(a,4,file,line); }
int da5Check0(const PtrA<DArray5> & a, const char *file, int line) { return aCheck(a,5,file,line); }

int ia1Check0(const PtrA<IArray1> & a, const char *file, int line) { return aCheck(a,1,file,line); }
int ia2Check0(const PtrA<IArray2> & a, const char *file, int line) { return aCheck(a,2,file,line); }
int ia3Check0(const PtrA<IArray3> & a, const char *file, int line) { return aCheck(a,3,file,line); }
int ia4Check0(const PtrA<IArray4> & a, const char *file, int line) { return aCheck(a,4,file,line); }
int ia5Check0(const PtrA<IArray5> & a, const char *file, int line) { return aCheck(a,5,file,line); }

int za1Check0(const PtrA<ZArray1> & a, const char *file, int line) { return aCheck(a,1,file,line); }
int za2Check0(const PtrA<ZArray2> & a, const char *file, int line) { return aCheck(a,2,file,line); }
int za3Check0(const PtrA<ZArray3> & a, const char *file, int line) { return aCheck(a,3,file,line); }
int za4Check0(const PtrA<ZArray4> & a, const char *file, int line) { return aCheck(a,4,file,line); }
int za5Check0(const PtrA<ZArray5> & a, const char *file, int line) { return aCheck(a,5,file,line); }

// ------------------------------------------------------- //

template<typename T>
void TArray<T>::print_block( int numcol, const int lo[], const int hi[],const char*name ) const
{
   Ptr<ArrayFormat> af;
   if(tp.basetype==a_int   ) af = new ArrayFormatI((int   *)(this->v));
   if(tp.basetype==a_double) af = new ArrayFormatD((double*)(this->v));
   if(tp.basetype==a_double_complex) af = new ArrayFormatZ((double_complex*)(this->v));
   if(tp.basetype==a_char) af = new ArrayFormatB((char*)(this->v));
   
   generic_print_block(this,af,name,numcol,lo,hi);
}
  
template<typename T>
void TArray<T>::print_rows(int low,int high,const char *name) const
{
  int lo[3], hi[3];
  lo[1]=low ; lo[2]=1;
  hi[1]=high; hi[2]=acs.n2;
  
  if(acs.dim == 1) print_block(1     ,lo,hi,name);
  if(acs.dim == 2) print_block(acs.n2,lo,hi,name); // ncol=n[2]
}


void da1PrintBlock(const PtrA<DArray1> & v, int numcol, int l1,int n1,const char *name )
{
  int lo[2], hi[2];
  lo[1]=l1;
  hi[1]=n1;
  
  v->print_block(numcol,lo,hi,name);
}

void da2PrintBlock(const PtrA<DArray2> & v, int numcol, int l1,int n1, int l2,int n2,const char *name )
{
  int lo[3], hi[3];
  lo[1]=l1; lo[2]=l2;
  hi[1]=n1; hi[2]=n2;
  
  v->print_block(numcol,lo,hi,name);
}

void ia1PrintBlock(const PtrA<IArray1> & v, int numcol, int l1,int n1,const char *name )
{
  int lo[2], hi[2];
  lo[1]=l1;
  hi[1]=n1;
  
  v->print_block(numcol,lo,hi,name);
}

void ia2PrintBlock(const PtrA<IArray2> & v, int numcol, int l1,int n1, int l2,int n2,const char *name )
{
  int lo[3], hi[3];
  lo[1]=l1; lo[2]=l2;
  hi[1]=n1; hi[2]=n2;
  
  v->print_block(numcol,lo,hi,name);
}

// ------------------------------------------------------- //

// linehandler
inline string PtrAOfT(const char*T) { return "PtrA<"+string(T)+">*"; }
template<class T> inline
T* swigext_acast(swig_lua_userdata* usr, const char *type)
{
  PtrA<T>* p =(PtrA<T>*)_swigext_cast(usr,PtrAOfT(type).c_str());
  if(p)
   return p->ptr();
  return 0;
}


template<class DST> inline
DST* ldf_userdata_cast(void*usr, const char*type)
{
#ifdef WITH_SWIGWRAP
  DST * dst=swigext_acast<DST>((swig_lua_userdata*)usr,type);
  assert(dst!=0);
  return dst;
#else
 assert(0);
#endif
}

struct a2f { double operator()(const char *s) { return atof(s); } };
struct a2i {    int operator()(const char *s) { return atoi(s); } };


template<class TARRAY,class ATOX> inline
int ta1LineHandlerF(TARRAY* a1, int iline, int argc, const char *argv[],ATOX atox)
{
  (*a1)(iline)=atox(argv[0]);
  return 0;
}

int da1LineHandlerF(void* a, int iline, int argc, const char *argv[]) { return ta1LineHandlerF(ldf_userdata_cast<DArray1>(a,"DArray1"),iline,argc,argv,a2f()); }
int ia1LineHandlerF(void* a, int iline, int argc, const char *argv[]) { return ta1LineHandlerF(ldf_userdata_cast<IArray1>(a,"IArray1"),iline,argc,argv,a2i()); }

template<class TARRAY,class ATOX> inline
int ta2LineHandlerF(TARRAY* a2, int irow, int argc, const char *argv[],ATOX atox)
{
  int i;
  typename TARRAY::value_type *row= &((*a2)(irow,1));
  for (i=0;i<a2->acs.n2;i++)
    {
      row[i]=atox(argv[i]);
    }
  return 0;
}
int da2LineHandlerF(void* a, int irow, int argc, const char *argv[]) { return ta2LineHandlerF(ldf_userdata_cast<DArray2>(a,"DArray2"),irow,argc,argv,a2f()); }
int ia2LineHandlerF(void* a, int irow, int argc, const char *argv[]) { return ta2LineHandlerF(ldf_userdata_cast<IArray2>(a,"IArray2"),irow,argc,argv,a2i()); }


template<class TARRAY,class ATOX> inline
int ta1LineHandlerV(TARRAY* a1, int iline, int argc, const char *argv[],ATOX atox)
{
  size_t nn[]={0,(size_t) iline}; 
  if(a1->extend(nn))
   (*a1)(iline)=atox(argv[0]);
  return 0;
}
int da1LineHandlerV(void* a, int iline, int argc, const char *argv[]) { return ta1LineHandlerV(ldf_userdata_cast<DArray1>(a,"DArray1"),iline,argc,argv,a2f()); }
int ia1LineHandlerV(void* a, int iline, int argc, const char *argv[]) { return ta1LineHandlerV(ldf_userdata_cast<IArray1>(a,"IArray1"),iline,argc,argv,a2i()); }

template<class TARRAY,class ATOX> inline
int ta2LineHandlerV(TARRAY* a2, int irow, int argc, const char *argv[],ATOX atox)
{
  int i;
  typename TARRAY::value_type *row;
  size_t nn[]={0,(size_t) irow,(size_t) argc}; 
  if(a2->extend(nn)) {
    row= &((*a2)(irow,1));
    for (i=0;i<a2->acs.n2;i++)
     row[i]=atox(argv[i]);
  }
  
  return 0;
}
int da2LineHandlerV(void* a, int irow, int argc, const char *argv[]) { return ta2LineHandlerV(ldf_userdata_cast<DArray2>(a,"DArray2"),irow,argc,argv,a2f()); }
int ia2LineHandlerV(void* a, int irow, int argc, const char *argv[]) { return ta2LineHandlerV(ldf_userdata_cast<IArray2>(a,"IArray2"),irow,argc,argv,a2i()); }

// ------------------------------------------------------- //

#ifdef NO_XDR
typedef void* xdrproc_t;
double*xdr_double=NULL;
int*xdr_int=NULL;
#endif

#define xdr_double_complex xdr_double
/*TODO: tweak for complex !!! */

template<class TARRAY> inline
int tamXDRHandler(TARRAY* a, FILE * f, xdrproc_t xdrproc)
{
#ifndef NO_XDR
  XDR xdr;
  xdrstdio_create(&xdr,f,XDR_DECODE);
  assert(xdr_vector(&xdr,(char *)(a->v+1),a->acs.vdim,sizeof(typename TARRAY::value_type),(xdrproc_t) xdrproc));
  xdr_destroy(&xdr);
#else
  assert(fread(a->v+1,sizeof(typename TARRAY::value_type),a->acs.vdim,f)==size_t(a->acs.vdim));
#endif
  return 0;
}
int da1XDRHandler(void* a, FILE *f) { return tamXDRHandler(ldf_userdata_cast<DArray1>(a,"DArray1"),f,(xdrproc_t)xdr_double); }
int da2XDRHandler(void* a, FILE *f) { return tamXDRHandler(ldf_userdata_cast<DArray2>(a,"DArray2"),f,(xdrproc_t)xdr_double); }
int da3XDRHandler(void* a, FILE *f) { return tamXDRHandler(ldf_userdata_cast<DArray3>(a,"DArray3"),f,(xdrproc_t)xdr_double); }
int da4XDRHandler(void* a, FILE *f) { return tamXDRHandler(ldf_userdata_cast<DArray4>(a,"DArray4"),f,(xdrproc_t)xdr_double); }
int da5XDRHandler(void* a, FILE *f) { return tamXDRHandler(ldf_userdata_cast<DArray5>(a,"DArray5"),f,(xdrproc_t)xdr_double); }

int ia1XDRHandler(void* a, FILE *f) { return tamXDRHandler(ldf_userdata_cast<IArray1>(a,"IArray1"),f,(xdrproc_t)xdr_int); }
int ia2XDRHandler(void* a, FILE *f) { return tamXDRHandler(ldf_userdata_cast<IArray2>(a,"IArray2"),f,(xdrproc_t)xdr_int); }
int ia3XDRHandler(void* a, FILE *f) { return tamXDRHandler(ldf_userdata_cast<IArray3>(a,"IArray3"),f,(xdrproc_t)xdr_int); }
int ia4XDRHandler(void* a, FILE *f) { return tamXDRHandler(ldf_userdata_cast<IArray4>(a,"IArray4"),f,(xdrproc_t)xdr_int); }
int ia5XDRHandler(void* a, FILE *f) { return tamXDRHandler(ldf_userdata_cast<IArray5>(a,"IArray5"),f,(xdrproc_t)xdr_int); }

// ------------------------------------------------------- //

template<class TARRAY> inline
double tamXDump(const Ptr<TARRAY> & a, FILE *f, xdrproc_t xdrproc)
{
  double pos;
  pos=(double) ftell(f);
#ifndef NO_XDR
  {
    XDR xdr;
    xdrstdio_create(&xdr,f,XDR_ENCODE);
    assert(xdr_vector(&xdr,(char*) (a->v+1),a->acs.vdim,sizeof(typename TARRAY::value_type),(xdrproc_t) xdrproc));
    xdr_destroy(&xdr);
  }
#else
  assert(fwrite(a->v+1,sizeof(typename TARRAY::value_type),a->acs.vdim,f)==size_t(a->acs.vdim));
#endif
  return pos;
}

double da1XDump(const PtrA<DArray1> & a, FILE *f) { return tamXDump(a,f,(xdrproc_t)xdr_double); }
double da2XDump(const PtrA<DArray2> & a, FILE *f) { return tamXDump(a,f,(xdrproc_t)xdr_double); }
double da3XDump(const PtrA<DArray3> & a, FILE *f) { return tamXDump(a,f,(xdrproc_t)xdr_double); }
double da4XDump(const PtrA<DArray4> & a, FILE *f) { return tamXDump(a,f,(xdrproc_t)xdr_double); }
double da5XDump(const PtrA<DArray5> & a, FILE *f) { return tamXDump(a,f,(xdrproc_t)xdr_double); }

double ia1XDump(const PtrA<IArray1> & a, FILE *f) { return tamXDump(a,f,(xdrproc_t)xdr_int); }
double ia2XDump(const PtrA<IArray2> & a, FILE *f) { return tamXDump(a,f,(xdrproc_t)xdr_int); }
double ia3XDump(const PtrA<IArray3> & a, FILE *f) { return tamXDump(a,f,(xdrproc_t)xdr_int); }
double ia4XDump(const PtrA<IArray4> & a, FILE *f) { return tamXDump(a,f,(xdrproc_t)xdr_int); }
double ia5XDump(const PtrA<IArray5> & a, FILE *f) { return tamXDump(a,f,(xdrproc_t)xdr_int); }



template<class TARRAY> inline
void tamXLoad(const Ptr<TARRAY> & a, FILE *f, double pos, xdrproc_t xdrproc)
{
  // We must define _FILE_OFFSET_BITS=64 in order to allow reading
  // of files > than 2GB on 32bit systems. The position is conveniently
  // stored in a double, the automatic cast does what necessary to convert it
  // into some 64 integer type.
  fseek(f,(size_t)pos,SEEK_SET);
  tamXDRHandler(a.ptr(),f,xdrproc);
}

void da1XLoad(const PtrA<DArray1> & a, FILE *f,double pos) { return tamXLoad(a,f,pos,(xdrproc_t)xdr_double); }
void da2XLoad(const PtrA<DArray2> & a, FILE *f,double pos) { return tamXLoad(a,f,pos,(xdrproc_t)xdr_double); }
void da3XLoad(const PtrA<DArray3> & a, FILE *f,double pos) { return tamXLoad(a,f,pos,(xdrproc_t)xdr_double); }
void da4XLoad(const PtrA<DArray4> & a, FILE *f,double pos) { return tamXLoad(a,f,pos,(xdrproc_t)xdr_double); }
void da5XLoad(const PtrA<DArray5> & a, FILE *f,double pos) { return tamXLoad(a,f,pos,(xdrproc_t)xdr_double); }

void ia1XLoad(const PtrA<IArray1> & a, FILE *f,double pos) { return tamXLoad(a,f,pos,(xdrproc_t)xdr_int); }
void ia2XLoad(const PtrA<IArray2> & a, FILE *f,double pos) { return tamXLoad(a,f,pos,(xdrproc_t)xdr_int); }
void ia3XLoad(const PtrA<IArray3> & a, FILE *f,double pos) { return tamXLoad(a,f,pos,(xdrproc_t)xdr_int); }
void ia4XLoad(const PtrA<IArray4> & a, FILE *f,double pos) { return tamXLoad(a,f,pos,(xdrproc_t)xdr_int); }
void ia5XLoad(const PtrA<IArray5> & a, FILE *f,double pos) { return tamXLoad(a,f,pos,(xdrproc_t)xdr_int); }


static const int  A_LINELENGTH=4096;              // maximum length of input line
static const int  A_MAXTOK=A_LINELENGTH/2+1;  // maximum number of tokens per input line
static const char* A_DELIMITERS=" \t\r\n,;";    // default token delimiter in linehandler
static bool  isdelim[256];


inline void tokenize(char *line, int& argc, char* argv[])
{
  argc=0;
  int i=0;
  while(1) // tokenize
    {
      while((line[i]!='\0') && isdelim[(unsigned int)line[i]]) i++;
      if (line[i]=='\0') break;
      argv[argc++]=&line[i];
      while((line[i]!='\0') && !isdelim[(unsigned int)line[i]]) i++;
      if (line[i]=='\0') break;
      line[i]='\0';
      i++;
    }      

}

PtrA<DArray1> da1ALoad(FILE *f)
{
  PtrA<DArray1> a=new DArray1(0);
  char line[A_LINELENGTH];
  int irow=0;
  while (!feof(f))
    {
      char* rc=fgets(line,  A_LINELENGTH, f);
      if (rc==0) break;
      if (line[0]=='#') continue;
      irow++;
      da1Insert(a,irow,atof(line));
    }
  return a;
}

PtrA<DArray1> da1ALoad(const char *fname)
{
  FILE *f=fopen(fname,"r");
  if (f==0)
    {
      dprintf(error)("unable to open file %s\n",fname);
      return 0;
    }
  PtrA<DArray1> a=da1ALoad(f);
  fclose(f);
  return a;
}

PtrA<IArray1> ia1ALoad(FILE *f)
{
  PtrA<IArray1> a=new IArray1(0);
  char line[A_LINELENGTH];
  int irow=0;
  while (!feof(f))
    {
      char* rc=fgets(line,  A_LINELENGTH, f);
      if (rc==0) break;
      if (line[0]=='#') continue;
      irow++;
      ia1Insert(a,irow,atof(line));
    }
  return a;
}

PtrA<IArray1> ia1ALoad(const char *fname)
{
  FILE *f=fopen(fname,"r");
  if (f==0)
    {
      dprintf(error)("unable to open file %s\n",fname);
      return 0;
    }
  PtrA<IArray1> a=ia1ALoad(f);
  fclose(f);
  return a;
}

PtrA<DArray2> da2ALoad(FILE *f)
{
  PtrA<DArray2> a=new DArray2(0,0);
  char line[A_LINELENGTH];
  char* argv[A_MAXTOK];
  int  argc;
  int irow=0;
  while (!feof(f))
    {
      char* rc=fgets(line,  A_LINELENGTH, f);
      if (rc==0) break;
      if (line[0]=='#') continue;
      irow++;
      tokenize(line,argc,argv);
      for(int icol=1;icol<=argc;icol++)
	da2Insert(a,irow,icol, atof(argv[icol-1]));
    }
  return a;
}

PtrA<DArray2> da2ALoad(const char *fname)
{
  FILE *f=fopen(fname,"r");
  if (f==0)
    {
      dprintf(error)("unable to open file %s\n",fname);
      return 0;
    }
  PtrA<DArray2> a=da2ALoad(f);
  fclose(f);
  return a;
}

PtrA<IArray2> ia2ALoad(FILE *f)
{
  PtrA<IArray2> a=new IArray2(0,0);
  char line[A_LINELENGTH];
  char* argv[A_MAXTOK];
  int  argc;
  int irow=0;
  while (!feof(f))
    {
      char* rc=fgets(line,  A_LINELENGTH, f);
      if (rc==0) break;
      if (line[0]=='#') continue;
      irow++;
      tokenize(line,argc,argv);
      for(int icol=1;icol<=argc;icol++)
	ia2Insert(a,irow,icol, atof(argv[icol-1]));
    }
  return a;
}

PtrA<IArray2> ia2ALoad(const char *fname)
{
  FILE *f=fopen(fname,"r");
  if (f==0)
    {
      dprintf(error)("unable to open file %s\n",fname);
      return 0;
    }
  PtrA<IArray2> a=ia2ALoad(f);
  fclose(f);
  return a;
}





template<class TARRAY> inline
double tamXAppend(const Ptr<TARRAY> & a , const char *fname, xdrproc_t xdrproc)
{
  FILE *f;
  double pos;
  assert(f=fopen(fname,"a"));
  pos=tamXDump(a,f,xdrproc);
  fclose(f);
  return pos;
}

double da1XAppend(const PtrA<DArray1> & a,const char *fname) { return tamXAppend(a,fname,(xdrproc_t)xdr_double); }
double da2XAppend(const PtrA<DArray2> & a,const char *fname) { return tamXAppend(a,fname,(xdrproc_t)xdr_double); }
double da3XAppend(const PtrA<DArray3> & a,const char *fname) { return tamXAppend(a,fname,(xdrproc_t)xdr_double); }
double da4XAppend(const PtrA<DArray4> & a,const char *fname) { return tamXAppend(a,fname,(xdrproc_t)xdr_double); }
double da5XAppend(const PtrA<DArray5> & a,const char *fname) { return tamXAppend(a,fname,(xdrproc_t)xdr_double); }

double ia1XAppend(const PtrA<IArray1> & a,const char *fname) { return tamXAppend(a,fname,(xdrproc_t)xdr_int); }
double ia2XAppend(const PtrA<IArray2> & a,const char *fname) { return tamXAppend(a,fname,(xdrproc_t)xdr_int); }
double ia3XAppend(const PtrA<IArray3> & a,const char *fname) { return tamXAppend(a,fname,(xdrproc_t)xdr_int); }
double ia4XAppend(const PtrA<IArray4> & a,const char *fname) { return tamXAppend(a,fname,(xdrproc_t)xdr_int); }
double ia5XAppend(const PtrA<IArray5> & a,const char *fname) { return tamXAppend(a,fname,(xdrproc_t)xdr_int); }


template<class TARRAY> inline
void tamXRestore(const Ptr<TARRAY> & a, const char *fname, double pos, xdrproc_t xdrproc)
{
  FILE *f;
  assert(f=fopen(fname,"r"));
  tamXLoad(a,f,pos,xdrproc);
  fclose(f);
}

void da1XRestore(const PtrA<DArray1> & a,const char *fname,double pos) { return tamXRestore(a,fname,pos,(xdrproc_t)xdr_double); }
void da2XRestore(const PtrA<DArray2> & a,const char *fname,double pos) { return tamXRestore(a,fname,pos,(xdrproc_t)xdr_double); }
void da3XRestore(const PtrA<DArray3> & a,const char *fname,double pos) { return tamXRestore(a,fname,pos,(xdrproc_t)xdr_double); }
void da4XRestore(const PtrA<DArray4> & a,const char *fname,double pos) { return tamXRestore(a,fname,pos,(xdrproc_t)xdr_double); }
void da5XRestore(const PtrA<DArray5> & a,const char *fname,double pos) { return tamXRestore(a,fname,pos,(xdrproc_t)xdr_double); }

void ia1XRestore(const PtrA<IArray1> & a,const char *fname,double pos) { return tamXRestore(a,fname,pos,(xdrproc_t)xdr_int); }
void ia2XRestore(const PtrA<IArray2> & a,const char *fname,double pos) { return tamXRestore(a,fname,pos,(xdrproc_t)xdr_int); }
void ia3XRestore(const PtrA<IArray3> & a,const char *fname,double pos) { return tamXRestore(a,fname,pos,(xdrproc_t)xdr_int); }
void ia4XRestore(const PtrA<IArray4> & a,const char *fname,double pos) { return tamXRestore(a,fname,pos,(xdrproc_t)xdr_int); }
void ia5XRestore(const PtrA<IArray5> & a,const char *fname,double pos) { return tamXRestore(a,fname,pos,(xdrproc_t)xdr_int); }


// ------------------------------------------------------- //

double ia1ADump(PtrA<IArray1> a , FILE *f)
{
  int i,n=ia1N(a,1);
  for (i=1;i<=n;i++)
    fprintf(f,"%d\n",IA1(a,i));
  return (double) ftell(f);
}

void ia1ADumpLDF(PtrA<IArray1> a , FILE *f, const char *name)
{
  fprintf(f,"%s=ia1Create(%d)\n",name,ia1N(a,1));
  fprintf(f,"Data{%s}\n$\n",name);
  ia1ADump(a,f);
  fprintf(f,"$\n");
}

double da1ADump(PtrA<DArray1> a , FILE *f)
{
  int i,n=da1N(a,1);
  for (i=1;i<=n;i++)
    fprintf(f,"%.15e\n",DA1(a,i));
  return (double) ftell(f);
}

void da1ADumpLDF(PtrA<DArray1> a , FILE *f, const char *name)
{
  fprintf(f,"%s=da1Create(%d)\n",name,da1N(a,1));
  fprintf(f,"Data{%s}\n$\n",name);
  da1ADump(a,f);
  fprintf(f,"$\n");
}

double ia2ADump(PtrA<IArray2> a , FILE *f)
{
  int i,j;
  int nrow=ia2N(a,1);
  int ncol=ia2N(a,2);
  for (i=1;i<=nrow;i++)
    {
      for (j=1;j<=ncol;j++)
	fprintf(f," %d",IA2(a,i,j));
      fprintf(f,"\n");
    }
  return (double) ftell(f);
}

void ia2ADumpLDF(PtrA<IArray2> a , FILE *f, const char *name)
{
  fprintf(f,"%s=ia2Create(%d,%d)\n",name,ia2N(a,1),ia2N(a,2));
  fprintf(f,"Data{%s}\n$\n",name);
  ia2ADump(a,f);
  fprintf(f,"$\n");
}

double da2ADump(PtrA<DArray2> a , FILE *f)
{
  int i,j;
  int nrow=da2N(a,1);
  int ncol=da2N(a,2);
  for (i=1;i<=nrow;i++)
    {
      for (j=1;j<=ncol;j++)
	fprintf(f," %.15e",DA2(a,i,j));
      fprintf(f,"\n");
    }
  return (double) ftell(f);
}
void da2ADumpLDF(PtrA<DArray2> a , FILE *f, const char *name)
{
  fprintf(f,"%s=da2Create(%d,%d)\n",name,da2N(a,1),da2N(a,2));
  fprintf(f,"Data{%s}\n$\n",name);
  da2ADump(a,f);
  fprintf(f,"$\n");
}



void ia1ADump(PtrA<IArray1> a , const char *fname)
{
  FILE *f=fopen(fname,"w");
  ia1ADump(a,f);
  fclose(f);
}

void ia2ADump(PtrA<IArray2> a , const char *fname)
{
  FILE *f=fopen(fname,"w");
  ia2ADump(a,f);
  fclose(f);
}

void da1ADump(PtrA<DArray1> a , const char *fname)
{
  FILE *f=fopen(fname,"w");
  da1ADump(a,f);
  fclose(f);

}

void da2ADump(PtrA<DArray2> a , const char *fname)
{
  FILE *f=fopen(fname,"w");
  da2ADump(a,f);
  fclose(f);

}




void ia1XDumpLDF(PtrA<IArray1> a , FILE *f, const char *name)
{
  fprintf(f,"%s=ia1Create(%d)\n",name,ia1N(a,1));
  fprintf(f,"Data{%s,encoding='xdr'}\n$\n",name);
  ia1XDump(a,f);
  fprintf(f,"$\n");
}


void da1XDumpLDF(PtrA<DArray1> a , FILE *f, const char *name)
{
  fprintf(f,"%s=da1Create(%d)\n",name,da1N(a,1));
  fprintf(f,"Data{%s,encoding='xdr'}\n$\n",name);
  da1XDump(a,f);
  fprintf(f,"$\n");
}

void ia2XDumpLDF(PtrA<IArray2> a , FILE *f, const char *name)
{
  fprintf(f,"%s=ia2Create(%d,%d)\n",name,ia2N(a,1),ia2N(a,2));
  fprintf(f,"Data{%s,encoding='xdr'}\n$\n",name);
  ia2XDump(a,f);
  fprintf(f,"$\n");
}

void da2XDumpLDF(PtrA<DArray2> a , FILE *f, const char *name)
{
  fprintf(f,"%s=da2Create(%d,%d)\n",name,da2N(a,1),da2N(a,2));
  fprintf(f,"Data{%s, encoding='xdr'}\n$\n",name);
  da2XDump(a,f);
  fprintf(f,"$\n");
}


template < typename T> 
struct TALib: public VectorLib
{
  virtual ~TALib(){};

  void   axpy  (double_complex za,  Ptr<Vector> x_,Ptr<Vector> y_ )
  {
    double a=za.real();
    Ptr< TArray <T> >x=PtrCast< TArray <T> >(x_);
    Ptr< TArray <T> >y=PtrCast< TArray <T> >(y_);
    int i, n=x->acs.vdim;
    T *xx=(T *)x->v;
    T *yy=(T *)y->v;
    for (i=1;i<=n;i++) yy[i]=yy[i]+a*xx[i];
  };
  
  
  void   lin1  (Ptr<Vector> x0_, double_complex za,  Ptr<Vector> x_,Ptr<Vector> y_ )
  {
    double a=za.real();
    Ptr< TArray <T> >x0=PtrCast< TArray <T> >(x0_);
    Ptr< TArray <T> >x =PtrCast< TArray <T> >(x_);
    Ptr< TArray <T> >y =PtrCast< TArray <T> >(y_);
    int i, n=x0->acs.vdim;
    T *xx0=(T *)x0->v;
    T *xx=(T *)x->v;
    T *yy=(T *)y->v;
    for (i=1;i<=n;i++) yy[i]=xx0[i]+a*xx[i];
  }
  
  void   lin2  (Ptr<Vector> x0_, double_complex za1, Ptr<Vector> x1_,double_complex za2, Ptr<Vector> x2_,Ptr<Vector> y_ )
  {
    double a1=za1.real();
    double a2=za2.real();
    Ptr< TArray <T> >x0=PtrCast< TArray <T> >(x0_);
    Ptr< TArray <T> >x1=PtrCast< TArray <T> >(x1_);
    Ptr< TArray <T> >x2=PtrCast< TArray <T> >(x2_);
    Ptr< TArray <T> >y =PtrCast< TArray <T> >(y_);
    int i, n=x0->acs.vdim;
    T *xx0=(T *)x0->v;
    T *xx1=(T *)x1->v;
    T *xx2=(T *)x2->v;
    T *yy=(T *)y->v;
    for (i=1;i<=n;i++) yy[i]=xx0[i]+a1*xx1[i]+a2*xx2[i];
  }
  
  void   set   (double_complex za,Ptr<Vector> x_)
  {
    double a=za.real();
    Ptr< TArray <T> >x=PtrCast< TArray <T> >(x_);
    int i;
    const int n=x->acs.vdim;
    T *xx=((T *)x->v)+1;
    for (i=1;i<=n;i++,xx++) *xx=a;
  }
  
  void   scal  (double_complex za,Ptr<Vector> x_)
  {
    
    double a=za.real();
    Ptr< TArray <T> >x=PtrCast< TArray <T> >(x_);
    int i;
    int n=x->acs.vdim;
    T *xx=((T *)x->v)+1;
    for (i=1;i<=n;i++,xx++) *xx*=a;
    
  }
  
  void   copy  (Ptr<Vector> x_, Ptr<Vector> y_)
  {
    Ptr< TArray <T> >x=PtrCast< TArray <T> >(x_);
    Ptr< TArray <T> >y=PtrCast< TArray <T> >(y_);
    int i;
    int n=x->acs.vdim;
    T *xx=(T *)x->v;
    T *yy=(T *)y->v;
    for (i=1;i<=n;i++) yy[i]=xx[i];
  }
  
  double_complex dot   (Ptr<Vector> x_, Ptr<Vector> y_)
  {
    Ptr< TArray <T> >x=PtrCast< TArray <T> >(x_);
    Ptr< TArray <T> >y=PtrCast< TArray <T> >(y_);
    int i;
    int n=x->acs.vdim;
    T *xx=(T *)x->v;
    T *yy=(T *)y->v;
    double_complex dot=double_complex(0.0,0.0);
    for (i=1;i<=n;i++) dot+=yy[i]*xx[i];
    return dot;
  }

  double amax  (Ptr<Vector> x_);
  double nm2  (Ptr<Vector> x_);

  
  Ptr<Vector> clone (Ptr<Vector> x);
};


template<> double TALib<double>::amax(Ptr<Vector> x_)
{
  Ptr< TArray <double> >x=PtrCast< TArray <double> >(x_);
  int i;
  int n=x->acs.vdim;
  double *xx=(double *)x->v;
  double max=fabs(xx[1]);
  for (i=1;i<=n;i++)
    {
      double xmax=fabs(xx[i]);
      if (max<xmax) max=xmax;
    }
  return max;
}

template<> double TALib<double>::nm2(Ptr<Vector> x_)
{
  Ptr< TArray <double> >x=PtrCast< TArray <double> >(x_);
  int i;
  int n=x->acs.vdim;
  double *xx=(double *)x->v;
  double nm2q=0.0;
  for (i=1;i<=n;i++) nm2q+=xx[i]*xx[i];
  return sqrt(nm2q);
}

template<> double TALib<double_complex>::amax(Ptr<Vector> x_)
{
  Ptr< TArray <double_complex> >x=PtrCast< TArray <double_complex> >(x_);
  int i;
  int n=x->acs.vdim;
  double_complex *xx=(double_complex *)x->v;
  double max=abs(xx[1]);
  for (i=1;i<=n;i++)
    {
      double xmax=abs(xx[i]);
      if (max<xmax) max=xmax;
    }
  return max;
}

template<> double TALib<double_complex>::nm2(Ptr<Vector> x_)
{
  Ptr< TArray <double_complex> >x=PtrCast< TArray <double_complex> >(x_);
  int i;
  int n=x->acs.vdim;
  double_complex *xx=(double_complex *)x->v;
  double nm2q=0.0;
  for (i=1;i<=n;i++) nm2q+=abs(xx[i])*abs(xx[i]);
  return sqrt(nm2q);
}

template<> double TALib<int>::amax(Ptr<Vector> x_)
{
  Ptr< TArray <int> >x=PtrCast< TArray <int> >(x_);
  int i;
  int n=x->acs.vdim;
  int *xx=(int *)x->v;
  double max=fabs(xx[1]);
  for (i=1;i<=n;i++)
    {
      int xmax=fabs(xx[i]);
      if (max<xmax) max=xmax;
    }
  return max;
}

template<> double TALib<int>::nm2(Ptr<Vector> x_)
{
  Ptr< TArray <int> >x=PtrCast< TArray <int> >(x_);
  int i;
  int n=x->acs.vdim;
  int *xx=(int *)x->v;
  double nm2q=0.0;
  for (i=1;i<=n;i++) nm2q+=xx[i]*xx[i];
  return sqrt(nm2q);
}


template<> Ptr<Vector> TALib<double>::clone(Ptr<Vector> x)
{
  typedef double T;
  Ptr< TArray <T> > x_ = PtrCast< TArray <T> >(x);
  switch(x_->dim())
    {
    case 1: return PtrCast<Vector>(da1Clone(PtrCast< TArray1 <T> >(x)));
    case 2: return PtrCast<Vector>(da2Clone(PtrCast< TArray2 <T> >(x)));
    case 3: return PtrCast<Vector>(da3Clone(PtrCast< TArray3 <T> >(x)));
    case 4: return PtrCast<Vector>(da4Clone(PtrCast< TArray4 <T> >(x)));
    case 5: return PtrCast<Vector>(da5Clone(PtrCast< TArray5 <T> >(x)));
    }
  return NULL;
};

template<> Ptr<Vector> TALib<double_complex>::clone(Ptr<Vector> x)
{
  typedef double_complex T;
  Ptr< TArray <T> > x_ = PtrCast< TArray <T> >(x);
  switch(x_->dim())
    {
    case 1: return PtrCast<Vector>(za1Clone(PtrCast< TArray1 <T> >(x)));
    case 2: return PtrCast<Vector>(za2Clone(PtrCast< TArray2 <T> >(x)));
    case 3: return PtrCast<Vector>(za3Clone(PtrCast< TArray3 <T> >(x)));
    case 4: return PtrCast<Vector>(za4Clone(PtrCast< TArray4 <T> >(x)));
    case 5: return PtrCast<Vector>(za5Clone(PtrCast< TArray5 <T> >(x)));
    }
  return NULL;
};

template<> Ptr<Vector> TALib<int>::clone(Ptr<Vector> x)
{
  typedef int T;
  Ptr< TArray <T> > x_ = PtrCast< TArray <T> >(x);
  switch(x_->dim())
    {
    case 1: return PtrCast<Vector>(ia1Clone(PtrCast< TArray1 <T> >(x)));
    case 2: return PtrCast<Vector>(ia2Clone(PtrCast< TArray2 <T> >(x)));
    case 3: return PtrCast<Vector>(ia3Clone(PtrCast< TArray3 <T> >(x)));
    case 4: return PtrCast<Vector>(ia4Clone(PtrCast< TArray4 <T> >(x)));
    case 5: return PtrCast<Vector>(ia5Clone(PtrCast< TArray5 <T> >(x)));
    }
  return NULL;
};


static Ptr< TALib <int> > ialib= new TALib<int>;
static Ptr< TALib <double> > dalib= new TALib<double>;
static Ptr< TALib <double_complex> > zalib= new TALib<double_complex>;

template<typename T>
void TArray<T>::set_vector_default()
{
  // JF: Need a weak cast here because this may be called where 
  // ptr_allow_smart=0. 
  if(tp.basetype==a_double        ) this->vlib=PtrWeakCast<VectorLib>(dalib);
  if(tp.basetype==a_int           ) this->vlib=PtrWeakCast<VectorLib>(ialib);
  if(tp.basetype==a_double_complex) this->vlib=PtrWeakCast<VectorLib>(zalib);
  set_vector_vtype();
  
  if (ptr_allow_smart())
    {
      if(tp.basetype==a_double && (acs.dim==1 || acs.dim==2)) 
	{
	  if(!ext.get_extension<LinAlgDenseExt>())
	    ext.add_extension(default_LinAlgDenseExt.clone());
	}
    }
}



void jl_array_offset(int off) 
{ 
  TArray<int>::jl_array_offset=off;
  TArray<double>::jl_array_offset=off;
}


static int initialized=0;
int ld_init_array(void)
{
  if (initialized) return 0;
  dbgRegister("array_info",&info);
  dbgRegister("array_error",&error);
  
  default_LinAlgDenseExt.solve_func=LinAlgDenseExt_div;
  default_LinAlgDenseExt.multi_func=LinAlgDenseExt_mult;

  for (int i=0;i<256;i++)
    isdelim[i]=false;
  for (int i=0; A_DELIMITERS[i]!='\0';i++)
    {
      isdelim[(int)A_DELIMITERS[i]]=true;
    }
  jl_array_offset(0);

  initialized=1;
  return 0;
}


template struct TArray<double>;
template struct TArray<int>;
template struct TArray<double_complex>;
template struct TArray<char>;


