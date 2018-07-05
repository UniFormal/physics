#ifndef ARRAY_H
#define ARRAY_H
/**
   \file array.h

   \brief Array API.

   \author J.Fuhrmann, H.Langmach, T. Streckenbach

   
   All basic information within pdelib2 is stored in arrays which start
   counting from 1. This is also
   true  for more complex objects like   grids.
   This decision has several advantages:
   - Consistency between array based mathematical formulation and implementation
   - Easy passing of data to other languages, e.g. Fortran
   - Comparably easy port of matlab code
   - Better memory access patterns and optimization possibilities than e.g. with
   linked lists.

   All access methods are inlined,  and benchmarks have shown that the
   overhead of  the array access methods  lies in the range  of 10-20%
   compared to plain C++ arrays  or corresponding Fortran code (for 1D
   arrays).  This  however strongly depends on  compiler optimization.
   Assuming  strict  ANSI  aliasing   is  mandatory  for  satisfactory
   performance, but  for some  compilers (e.g. intel)  this has  to be
   turned  on  explicity  even  when using  the  highest  optimization
   level. The pdelib build system tries to follow this rule.

   If  the  last  bit  of  speed really  matters,  due  to  structural
   compatibility,  time  critical  loops could  be  re-implemented  in
   Fortran. This  task is be eased  by the fact that  the array access
   syntax based  on operator() is  similar for pdelib  arrays, Fortran
   and matlab.
   
   \section arraydt Array data types implemented in pdelib
   Array data types are characterized by the \e base \e type and the
   \e dimension.  They follow the following naming scheme: an array with
   dimension \p m and  base type \p _t_type  has the associated type name 
   \p _T_Array_m_ 
   where \p _T_ is taken from the following list:
   <table>
   <tr><th>_t_type   </th><th>     _t_ </th><th>  _T_</th></tr>
   <tr><td>int       </td><td>      i  </td><td>   I</td></tr>
   <tr><td>double    </td><td>      d  </td><td>   D</td></tr>
   <tr><td>double_complex</td><td>  z  </td><td>  Z</td></tr>
   </table>
   The dimension \p _m_ currently ranges from 1 to 5.
   The component access for array  type \p _T_Array_m_ is  is provided via 
   the overloaded operator() method or \p _T_A_m_(), and all all array specific function names
   are prefixed by \p _t_a_m_. The type  \p _T_Array_m_ is  a typedef for \p TArray_m_, and thus both
   are interoperable.

   E.g. a  two-dimensional double array  has the type name  DArray2. A
   one-dimensional  integer  array has  the  type  IArray1. Arrays  in
   pdelib2 \b always count from 1 in all dimensions.

   We have implemented
   \code
   DArray1 ... DArray5
   IArray1 ... IArray5
   ZArray1 ... ZArray5
   \endcode

   \section arraycompacc Array component access

   Array  component  access  is   organized  via  overloading  of  the
   operator() method  of the  array class or  an array  specific smart
   pointer.  Due to c++ syntax restrictions, for the different ways of
   declaring/passing an array object it can be used as follows:

   - Array specific smart pointer to array. Array specific smart pointers
     are derived from smart pointers and now provide the most clean syntax along
     with smart-pointer typical memory bookeeping. The use of this idiom is 
     \b strongly \b encouraged.
     \code 
     PtrA<DArray2> a=new DArray2(10,10);
     a(3,5) =1;
     \endcode

   - Reference to array.  Perfectly \b supported. Easy to use in combination with 
     non-array specific smart pointers to arrays at the expense of an additional
     declaration and initialization of the reference.
     \code 
     Ptr<DArray2> A=new DArray2(10,10);
     DArray2 &a=A;
     a(3,5) =1;
     \endcode



   - Array declared on stack - possible but \b untypical \b use in pdelib. 
     When using this, be aware that the array data is nevertheless allocated
     on the heap. A stack based array constructor is delivered in array_stack.h
    \code 
    DArray2  a(10,10);
    a(3,5)=1;
    \endcode

   - \link ptr.h Smart pointer \endlink to array. 
      \b Supported but obviously inconvenient and hard to read/write.
   \code 
     Ptr<DArray2> a=new DArray2(10,10);
    (*a)(3,5) =1;
    \endcode


   - Legacy access methods.  This array class evolved  from an initial
     implementation in C, and  therefore several pdelib2 modules still
     use the legacy access method which evolved from C macros:
     \code 
     Ptr<DArray2> a=new DArray2(10,10);
     DA2(a,3,5) =1;
     \endcode

   - Pointer to array  - \b strongly  \b disencouraged, as memory bookkeeping
     is  a mess (never forget the delete!)....
    \code 
    DArray2 *a=new DArray2(10,10);
    (*a)(3,5) =1;
    delete a;
    \endcode
    
   

   A   usage   example   is    found   in 
   \ref   examples/base/array.cxx.
  
   
*/
////////////////////////////////////////////////////
////////////////////////////////////////////////////
// FILE CONTENTS
//
//  1. Includes
//  2. Preparatory definitions
//  3. Main template class definition
//  4. Constructors & casts
//  5. Dimension specific class templates
//  6. Type specializations
//  *7. Array specific smart pointers
//  8. Lua specific part 
//  9. Ptr <-> PtrA casts
//  10. Ptr based API
//    10.1 Constructors of new arrays, with allocating new data space
//    10.2 Create identical copies of the array structure. 
//    10.3 Constructors of array "views", 
//    10.4 Create a one-dimensional array with the _same_ value
//    10.5 Create a row array pointing to the _same_ field of values as a.
//    10.6 Create a column array pointing to the _same_ field of values as a.
//    10.7 Ptr<Array> <-> Vector cast
//    10.8 Integrity check
//    10.9 Print Array entries from the selected block (subarray) to standard output.
//    10.10 Print the whole contents of array a to the standard output.
//    10.11 Size information retrieval 
//    10.12 Stride
//    10.13 Number of elements in the i-th dimension. 
//    10.14 Legacy access functions
//    10.15 Get/set API for component access
//    10.16 Size extension
//    10.17 Size adjustion
//    10.18 Offset 0 direct  data memory access 
//    10.19 Offset 1 direct  data memory access  (for passing to fortran 77)
//    10.20 Set context information
//    10.21 Array constructors for data memory allocated elsewhere, free() to free memory
//    10.22 Array constructors for data memory allocated elsewhere, delete() to free memory
//    10.23 Load/store data to files
// 11. PtrA based API
//    11.1 Constructors
//    11.2 Short cut  constructors
//    11.3 Dimension information
//    11.4 Load/store
// 12. ArrayView classes  
// 13. BLAS wrappers
// 14. Dense linear algebra extension


// X linalgdenseext

////////////////////////////////////////////////////
////////////////////////////////////////////////////
// 1. Includes

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>

#include <initializer_list>

#include "stdsys/swigbind.h"
#include "stdcxx/ptrx.h"
#include "stdcxx/mem.h"
#include "stdcxx/memacs.h"
#include "stdcxx/sig.h"
#include "stdsys/complex.h"
#include "array/vector.h"

////////////////////////////////////////////////////
////////////////////////////////////////////////////
// 2. Preparatory definitions


/** 
    \internal
    Simple class to hold information of a array coming from a external     
    array library,  for example numpy
 */
class BaseClassObj {
  public:
    BaseClassObj() {}
    virtual ~BaseClassObj() {}
} ;


/** 
    \brief  Enumeration data type to hold information
    on the base type of an array
*/
typedef enum 
{
  a_int,
  a_double,
  a_double_complex,
  a_char
} aBaseType;

  
/**
   \brief Base class for pointer holding additional context information
*/
struct ArrayClassBase
{
  virtual ~ArrayClassBase() {}
};


/**
   \brief  Class for providing various representations  of 
   base type
*/
template<typename T>
struct cTypePerformer {
  typedef T VTYPE;    /**< value type: int, double, complex, ... */
  char vtype_str[3];  /**< value type short: D, I, DC, ... */
  aBaseType basetype; /**< value type ID for: int, double, complex, ... */
  cTypePerformer();
};

typedef cTypePerformer<double_complex> cTypePerformerC;
typedef cTypePerformer<double>         cTypePerformerD;
typedef cTypePerformer<int>            cTypePerformerI;
typedef cTypePerformer<char>           cTypePerformerB;


////////////////////////////////////////////////////
////////////////////////////////////////////////////
// 3. Main template class definition

/**
    \brief Base structure of _m_-dimensional arrays with values of type _t_type_.
*/

template<class T>
struct TArray : public Vector {

  
  /// Base class object for external array classes
  /// e.g. from numpy
  Ptr<BaseClassObj> base_obj ; 
  


  ////////////////
  // typedefs for allowing shorter notation in class definition

  typedef T value_type;
  typedef TArray<value_type> self;
  typedef MemDynamicPAT<value_type> buf_type;
  typedef MemDynamicVIRTUAL<value_type> vir_type;
  typedef typename vir_type::free_func free_func;



  ////////////////
  // Type & context information

    /// Type information for object
  cTypePerformer<value_type> tp;  
  
  /// Tensor dimension
  virtual int type_dim() const { return 0; }
  
  /// Set type information in base class
  void set_vector_vtype();

  /// Set default information in base class
  void set_vector_default();
  
  /// refers to context information for arrays; e.g., a vector space.
  Ptr<ArrayClassBase> aclass; 

  /// Hook for additional data to  be  associated with object
  ExtensionList ext;
  


  ////////////////
  // Data memory

  ///defines data memory  access pattern on v 
  MemAccess2 acs; 

  ///holds data memory for v 
  Ptr<buf_type> buf; 

  /// holds "virtual" data memory for v 
  Ptr<vir_type> vir;
  
  /// points to array-data, v[1] = first data 
  value_type *v;   

  /// detect if data memory is owned by the object
  bool mem_owner() const { return buf || ( vir && vir->memfree ); }

  /// size of memory of array
  int memsize() const { return buf?sizeof(self)+buf->bufsize():sizeof(self)+vir->bufsize(); }
  
  /// re-allocate data memory (if necessary) and extend with size n
  bool extend(      size_t* n);
  
  /// re-allocate data memory (if necessary) and force to size n 
  bool resize(const size_t* n);
  
  /// renew data memory
  void renew_mem(value_type * mem=NULL, int memoffset=0, free_func mf=NULL);
  
  /// Set content to zero
  void set0();

  /// Copy array content
  void copy_v(const self& src);

  /// Get raw data pointer.  Points to the zero-th element.
  /// For access replacement in C/C++ based loops.
  /// So for v=a->getVal,  v[i] and a(i) point to the same location
  inline T* getVal() const { return v ;} 

  /// Get raw data pointer.  Points to the first element.
  /// For passing to Fortran subroutines
  /// For v=a->getVal;  fortsub_(v),  a(i) and v(i) in the 
  /// Fortran subroutine point to the same location.
  inline T* vadr() const { return v+1 ;}
  inline T* getVal77() const { return vadr() ;} 
  


  ///////////////
  /// Dimension information

  /// array tensor dimension 
  virtual int  dim() const { return acs. dim; }

  /// numpy like: number of dimensions
  int ndim() const {return dim();}

  /// Dimension of vector space of all array elements
  virtual int vdim() const { return acs.vdim; } 

  /// numpy like: size of the array
  int size() const {return vdim();}

  /// Number of all elements of the array. 
  int nall() const { return acs.vdim; }

  /// Size in  in i-th dimension
  int n(const int i) const { switch(i) {case 1: return acs.n1; case 2: return acs.n2; case 3: return acs.n3; case 4: return acs.n4 ; default: return acs.n5;}}
  
  /// numpy like: size in i-th dimension
  int shape(const int i) { return n(i);}
  
  /// Number of rows 
  int nrow() const { return acs.n1; }
  
  /// number of columns
  int ncol() const {
    if(dim()==1)
     return 1;
    return acs.n2;
  }


  //////////////////////
  /// Print 
  
  /// print a sub-block of the array
  void print_block(int numcol, const int lo[], const int hi[], const char*name) const;

  /// print array rows
  void print_rows(int low,int high,const char *name) const;
  
  /// print all array content
  void print(const char *name) const { print_rows(1,acs.n1,name); }
  
  /// print all array content
  void print() const { print_rows(1,acs.n1,""); }
  
  /////////////////////////
  // Constructors

  
  /// copy constructor (flat copy)
  template<class TARRAY>
  void copy(const TARRAY& o) {
    // omit elements created in std-ctor
    aclass=o.aclass;
    ext.clear_extensions();
    ext.add_sharing_allowed(o.ext);
    ext.add_not_sharing_allowed_by_clone(o.ext);
    acs=o.acs;
    buf=o.buf;
    vir=o.vir;
    v=o.v;
    vlib=o.vlib;
    
    //set_vector_vtype();
#if defined _WIN32 && !defined MINGW
    strcpy_s(vtype,o.vtype);
#else
    strcpy(vtype,o.vtype);
#endif
  }
  
  
  /// copy constructor
  TArray(const TArray& o)
    : acs(o.acs)
  {
    renew_mem();
#if defined _WIN32 && !defined MINGW
    strcpy_s(vtype,o.vtype);
#else
    strcpy(vtype,o.vtype);
#endif
    copy_v(o);
    aclass=o.aclass;
  }


  static int jl_array_offset;
  
  //////////////
protected:
  
  //////////////
  // Empty constructor
  
 TArray():Vector() { }
  
  //////////////
  // Constructors allocating memory
 TArray(int n1,int n2,int n3,int n4,int n5) : acs(n1,n2,n3,n4,n5) { renew_mem();set_vector_default();} ///< 5D Array constructor
 TArray(int n1,int n2,int n3,int n4       ) : acs(n1,n2,n3,n4   ) { renew_mem();set_vector_default();} ///< 4D Array constructor
 TArray(int n1,int n2,int n3              ) : acs(n1,n2,n3      ) { renew_mem();set_vector_default();} ///< 3D Array constructor
 TArray(int n1,int n2                     ) : acs(n1,n2         ) { renew_mem();set_vector_default();} ///< 2D Array constructor
 TArray(int n1                            ) : acs(n1            ) { renew_mem();set_vector_default();} ///< 1D Array constructor
 TArray(const MemAccess2 & acs_           ) : acs(acs_          ) { renew_mem();set_vector_default();} 
  
  //////////////
  // Constructors creating memory views
 TArray(int n1,int n2,int n3,int n4,int n5, value_type* mem,int memoffset,free_func ff=NULL) : acs(n1,n2,n3,n4,n5) { assert(mem); renew_mem(mem,memoffset,ff);set_vector_default();}
 TArray(int n1,int n2,int n3,int n4       , value_type* mem,int memoffset,free_func ff=NULL) : acs(n1,n2,n3,n4   ) { assert(mem); renew_mem(mem,memoffset,ff);set_vector_default();}
 TArray(int n1,int n2,int n3              , value_type* mem,int memoffset,free_func ff=NULL) : acs(n1,n2,n3      ) { assert(mem); renew_mem(mem,memoffset,ff);set_vector_default();}
 TArray(int n1,int n2                     , value_type* mem,int memoffset,free_func ff=NULL) : acs(n1,n2         ) { assert(mem); renew_mem(mem,memoffset,ff);set_vector_default();}
 TArray(int n1                            , value_type* mem,int memoffset,free_func ff=NULL) : acs(n1            ) { assert(mem); renew_mem(mem,memoffset,ff);set_vector_default();}
 TArray(const MemAccess2 & acs_           , value_type* mem,int memoffset,free_func ff=NULL) : acs(acs_          ) { assert(mem); renew_mem(mem,memoffset,ff);set_vector_default();}

  //////////////
  // Constructors filling from initializer_list
  typedef std::initializer_list< T                 > init_list_type_1d;
  typedef std::initializer_list< init_list_type_1d > init_list_type_2d;
  typedef std::initializer_list< init_list_type_2d > init_list_type_3d;
  typedef std::initializer_list< init_list_type_3d > init_list_type_4d;
  typedef std::initializer_list< init_list_type_4d > init_list_type_5d;
  
  void copy_from_init_list( const init_list_type_1d& il);
  void copy_from_init_list( const init_list_type_2d& il);
  void copy_from_init_list( const init_list_type_3d& il);
  void copy_from_init_list( const init_list_type_4d& il);
  void copy_from_init_list( const init_list_type_5d& il);
  
 TArray( init_list_type_5d il ) : acs(il.size()
                                     ,il.begin()->size()
                                     ,il.begin()->begin()->size()
                                     ,il.begin()->begin()->begin()->size()
                                     ,il.begin()->begin()->begin()->begin()->size()
                                     ) { renew_mem();set_vector_default();copy_from_init_list(il);} ///< 5D Array constructor
 TArray( init_list_type_4d il ) : acs(il.size()
                                     ,il.begin()->size()
                                     ,il.begin()->begin()->size()
                                     ,il.begin()->begin()->begin()->size()
                                     ) { renew_mem();set_vector_default();copy_from_init_list(il);} ///< 4D Array constructor
 TArray( init_list_type_3d il ) : acs(il.size()
                                     ,il.begin()->size()
                                     ,il.begin()->begin()->size()
                                     ) { renew_mem();set_vector_default();copy_from_init_list(il);} ///< 3D Array constructor
 TArray( init_list_type_2d il ) : acs(il.size()
                                     ,il.begin()->size()
                                     ) { renew_mem();set_vector_default();copy_from_init_list(il);} ///< 2D Array constructor
 TArray( init_list_type_1d il ) : acs(il.size()
                                     ) { renew_mem();set_vector_default();copy_from_init_list(il);} ///< 1D Array constructor
 
  /////////////
  // Element access
 

 T& operator()(int n1,int n2,int n3,int n4,int n5) { return v[acs(n1,n2,n3,n4,n5)]; } ///< 5D Array access 
 T& operator()(int n1,int n2,int n3,int n4       ) { return v[acs(n1,n2,n3,n4   )]; } ///< 4D Array access  
 T& operator()(int n1,int n2,int n3              ) { return v[acs(n1,n2,n3      )]; } ///< 3D Array access
 T& operator()(int n1,int n2                     ) { return v[acs(n1,n2         )]; } ///< 2D Array access  
 T& operator()(int n1                            ) { return v[acs(n1            )]; } ///< 1D Array access  
 
 T& operator()(int n1,int n2,int n3,int n4,int n5) const { return v[acs(n1,n2,n3,n4,n5)]; } ///< 5D Array access 
 T& operator()(int n1,int n2,int n3,int n4       ) const { return v[acs(n1,n2,n3,n4   )]; } ///< 4D Array access 
 T& operator()(int n1,int n2,int n3              ) const { return v[acs(n1,n2,n3      )]; } ///< 3D Array access 
 T& operator()(int n1,int n2                     ) const { return v[acs(n1,n2         )]; } ///< 2D Array access 
 T& operator()(int n1                            ) const { return v[acs(n1            )]; } ///< 1D Array access 
 
 // Lua forward
  virtual double __getitem__V(int i) const { assert(0); return 0; } 
  virtual void __setitem__V(int i,double d) { assert(0); }
  virtual int __lenV(void*) { assert(0); return 1; }

};


////////////////////////////////////////////////////
////////////////////////////////////////////////////
// 4. Array specific smart pointer 

/** 
 \brief Special smartpointer class for array types derived from TArray.
 
 This class allows a direct index access to the array elements, and it is
 also compatible with the more general smartpointer classes Ptr and PtrWeak.
 
 Eexample:
 \code
  void my_code(PtrA<DArray1> a)
  {
    a(1)=2.0;
  }
  
  Ptr<DArray1> A=...
  
  my_code(A);
 \endcode
 
*/
template<class A>
class PtrA : public Ptr<A>
{
  template<class DST> PtrA<DST> friend PtrACastToArray(const Ptr<Vector>&o);
  template<class SRC> Ptr<Vector> friend PtrACastToVector(const PtrA<SRC>&o);

  void doref( A* t ) { Ptr<A>::doref(t); }

  
 public:
  
  typedef A*       pointer;
  typedef A&       reference;
  typedef const A& const_reference;
 
  typedef Ptr<A>   base;
  typedef PtrA<A>  self;
 
  typedef typename A::value_type value_type;
  
  /* --- construct --- */
  
  PtrA() {}
  
  PtrA(pointer new_p ) : Ptr<A>(new_p) {  }
  
  PtrA(const_reference own_r ) : Ptr<A>(own_r) {  } ///< Create a weak smart pointer to wrap an object.
  
  PtrA(pointer new_p, PtrType t ) : base(new_p,t) { } ///< Create a weak smart pointer
  
  
  /* --- implicit typecast --- */
  
  operator Ptr<Vector>() const
  {
    return PtrACastToVector(*this); //PtrCast<Vector>(*this);
  }
  
  /* --- copy --- */
  
  PtrA(const self & o)
   : base(o)
  {
  }
  
  PtrA(const base & o)
   : base(o)
  {
  }
  
  /* --- assign --- */
  
  self& operator=(A* new_p)
  {
    return (self&)base::operator=(base(new_p));
  }
  
  /* --- array access --- */
  
  value_type  operator()(int n1) const { return this->ref()(n1); }
  value_type& operator()(int n1)       { return this->ref()(n1); }

  value_type  operator()(int n1, int n2) const { return this->ref()(n1,n2); }
  value_type& operator()(int n1, int n2)       { return this->ref()(n1,n2); }

  value_type  operator()(int n1, int n2, int n3) const { return this->ref()(n1,n2,n3); }
  value_type& operator()(int n1, int n2, int n3)       { return this->ref()(n1,n2,n3); }

  value_type  operator()(int n1, int n2, int n3, int n4) const { return this->ref()(n1,n2,n3,n4); }
  value_type& operator()(int n1, int n2, int n3, int n4)       { return this->ref()(n1,n2,n3,n4); }

  value_type  operator()(int n1, int n2, int n3, int n4, int n5) const { return this->ref()(n1,n2,n3,n4,n5); }
  value_type& operator()(int n1, int n2, int n3, int n4, int n5)       { return this->ref()(n1,n2,n3,n4,n5); }

  /* Array data */
  int nrow() const { return this->ptr()->nrow();}
  int ncol() const { return this->ptr()->ncol();}
  int nall() const { return this->ptr()->nall();}
  value_type* vadr() const { return this->ptr()->vadr();}
  
  /* --- access --- */
  
  pointer   ptr()        const { return Ptr<A>::ptr(); }
  reference ref()        const { return Ptr<A>::ref(); }
  const_reference const_ref() const { return Ptr<A>::ref(); }
};



/** 
 \brief Special weak smartpointer class for array types derived from TArray.
 
 This class allows a direct index access to the array elements, and it is
 also compatible with the more general smartpointer classes Ptr and PtrWeak.
 
 Eexample:
 \code
  void my_code(PtrAWeak<DArray1> a)
  {
    a(1)=2.0;
  }
  
  Ptr<DArray1> A=...
  
  my_code(A);
 \endcode
*/
template<class A>
class PtrAWeak : public PtrA<A>
{
  typedef PtrA<A>   base;
  typedef PtrAWeak<A>  self;
  
public:
  
  typedef typename A::value_type value_type;
  
  /* --- construct --- */
  
  PtrAWeak() {}
  PtrAWeak(typename base::pointer new_p) : base(new_p,PtrNoSmart) {}
  
  
  /* --- copy --- */
  
  PtrAWeak(const self & o)
   : base(o)
  {
  }
  
  PtrAWeak(const base & o)
   : base(&*o,PtrNoSmart)
//  : base(o)
  {
  }
  
  /* --- assign --- */
  
  self& operator=(const self & o)
  {
    this->p=o.p;
    return *this;
  }
  
  self& operator=(typename base::pointer new_p)
  {
    return operator=(self(new_p));
  }
  
  ///self& operator=(A* new_p)
  ///{
  ///  return (self&)base::operator=(base(new_p));
  ///}
  
  /* --- array access --- */
  
  value_type  operator()(int n1) const { return this->ref()(n1); }
  value_type& operator()(int n1)       { return this->ref()(n1); }

  value_type  operator()(int n1, int n2) const { return this->ref()(n1,n2); }
  value_type& operator()(int n1, int n2)       { return this->ref()(n1,n2); }
  
  value_type  operator()(int n1, int n2, int n3) const { return this->ref()(n1,n2,n3); }
  value_type& operator()(int n1, int n2, int n3)       { return this->ref()(n1,n2,n3); }

  value_type  operator()(int n1, int n2, int n3, int n4) const { return this->ref()(n1,n2,n3,n4); }
  value_type& operator()(int n1, int n2, int n3, int n4)       { return this->ref()(n1,n2,n3,n4); }

  value_type  operator()(int n1, int n2, int n3, int n4, int n5) const { return this->ref()(n1,n2,n3,n4,n5); }
  value_type& operator()(int n1, int n2, int n3, int n4, int n5)       { return this->ref()(n1,n2,n3,n4,n5); }
};

// ----------------------------------------------------------------------------

// useful if mem-alloc without calloc

template<size_t sz,class T> inline
void set_ptr(PtrA<T> t[sz]) {
  for(size_t i=0; i<sz;++i)
   t[i]=PtrA<T>();
}

template<size_t sz1,size_t sz2,class T> inline
void set_ptr(PtrA<T> t[sz1][sz2],size_t sz) {
 for(size_t i=0; i<sz;++i)
  for(size_t j=0; j<sz2;++j)
   t[i][j]=PtrA<T>();
}




// ------------------------------------------------------------------------------//

template<class T>
struct VectorOfPtrA : public std::vector<PtrA<T> > {
  VectorOfPtrA(int n) : std::vector<PtrA<T> >(n) {}
  VectorOfPtrA() : std::vector<PtrA<T> >() {}
};


template<class T> inline
bool is_item(const Ptr<VectorOfPtrA<T> >& v, const PtrA<T>& item)
{
  if(!v) return false;

  typename VectorOfPtrA<T>::iterator
    i=v->begin()
  , e=v->end();
  for(;i!=e;++i)
    if(item==*i)
     return true;
  return false;
}

template<class T> inline
int add_item(Ptr<VectorOfPtrA<T> >& v, const PtrA<T>& item, int at_index=-1)
{
  if(!item) return -1;
  if(!v) v=new VectorOfPtrA<T>();

  if(0<=at_index){
    // force position (*v)[at_index]=item
    int sz = at_index - (v->size()-1);
    for(int i=1;i<=sz;++i)
     v->push_back(0);
    (*v)[at_index]=item;
    return at_index;
  }

  // insert in free position
  typename VectorOfPtrA<T>::iterator
    i=v->begin()
  , e=v->end();
  int idx=0;
  for(;i!=e;++i,++idx)
   if(!*i) {
     *i=item;
     return idx;
   }

  // append
  v->push_back(item);
  return v->size()-1;
}

template<class T> inline
void append_item(Ptr<VectorOfPtrA<T> >& v, const PtrA<T>& item)
{
  if(!item) return;
  if(!v) v=new VectorOfPtrA<T>();

  // append
  v->push_back(item);
}


template<class T> inline
void del_item(Ptr<VectorOfPtrA<T> >& v, const PtrA<T>& item)
{
  if(!item) return;
  if(!v) return;

  typename VectorOfPtrA<T>::iterator
    i=v->begin()
  , e=v->end();
  for(;i!=e;++i)
   if(*i==item) {
     *i=0;
     return;
   }
}


template<class T> inline
void del_item(Ptr<VectorOfPtrA<T> >& v, const char*name)
{
  if(!name) return;
  if(!v) return;

  typename VectorOfPtrA<T>::iterator
    i=v->begin()
  , e=v->end();
  for(;i!=e;++i)
   if(*i && strcmp(name,(*i)->name)==0) {
     *i=0;
     return;
   }
}

template<class T> inline
PtrA<T> get_item(const Ptr<VectorOfPtrA<T> >& v, const char*name)
{
  if(!name) return 0;
  if(!v) return 0;

  typename VectorOfPtrA<T>::iterator
    i=v->begin()
  , e=v->end();
  for(;i!=e;++i)
    if(*i && strcmp(name,(*i)->name)==0)
     return *i;
  return 0;
}


template<class T> inline
PtrA<T> get_item(const Ptr<VectorOfPtrA<T> >& v, size_t idx)
{
  if(!v) return 0;

  if(idx<v->size())
   return (*v)[idx];

  return 0;
}

template<class T> inline
bool get_item(const Ptr<VectorOfPtrA<T> >& v, size_t idx, PtrA<T>& item)
{
  if(!v) return 0;

  if(idx<v->size()) {
   item=(*v)[idx];
   return true;
  }

  return false;
}

template<class T> inline
bool get_item(const Ptr<VectorOfPtrA<T> >& v, size_t idx, PtrAWeak<T>& item)
{
  if(!v) return 0;

  if(idx<v->size()) {
   item=(*v)[idx];
   return true;
  }

  return false;
}



/**
  Explicit typecast between smartpointers. If the cast failed, the Ptr::p becomes NULL.
*/
template<class DST> inline
PtrA<DST> PtrACastToArray(const Ptr<Vector>&o)
{
  DST* p=ptr_dynamic_cast<DST>(o);
  if(!p)
   return NULL;
  PtrA<DST> d;
  d.refcount=o.refcount;
  d.doref((DST*)(o.ptr()));
  d.p=p;
  return d;
}

/**
  Explicit typecast between smartpointers. If the cast failed, the Ptr::p becomes NULL.
*/
template<class SRC> inline
Ptr<Vector> PtrACastToVector(const PtrA<SRC>&o)
{
  #if 1
  Vector* p=ptr_dynamic_cast<Vector>(o);
  if(!p)
   return NULL;
  Ptr<Vector> d;
  d.ctor_cast(o.refcount,(Vector*)o.ptr());
  
  //d.refcount=o.refcount;
  //d.doref((Vector*)(o.ptr()));
  //d.p=p;
  return d;
  #else
  return (Ptr<Vector>)o;
  #endif
}

/// template<class ARRAY> inline
/// Ptr<ARRAY> PtrACast(const PtrA<ARRAY>&o)
/// {
///   return PtrACastToArray<ARRAY>((Ptr<Vector>)o);
/// }




////////////////////////////////////////////////////
////////////////////////////////////////////////////
// 5. Constructors and casts


/**
   Copy constructor (flat copy).
 */
template<class DST,class SRC> inline
PtrA<DST> aCopy(const PtrA<SRC> & a)
{
  PtrA<DST> c = new DST();
  c->copy(a.ref());
  return c;
}

/**
   Clone constructor (structurally identical to a, but with own uninitialized mem).
 */
template<class TARRAY> inline
PtrA<TARRAY> aClone(const PtrA<TARRAY> & a)
{
  PtrA<TARRAY> c = aCopy<TARRAY>(a);
  c->vir=0;
  c->acs.recalc();
  c->renew_mem();
  return c;
}

/**
   Clone copy (deep copy).
 */
template<class TARRAY> inline
PtrA<TARRAY> aCloneCopy(const PtrA<TARRAY> & a)
{
  if (!a) {
   return 0;
  }
  
 PtrA<TARRAY>
 c = aClone<TARRAY>(a);
 c->copy_v(*a);
 return c;
}


/**
   Array reformatting
 */
#if 1
template<class DST,class SRC> inline
PtrA<DST> aView(const PtrA<SRC> & a, const MemAccess2 & acs)
{
  if(acs.n_elem()!=a->acs.n_elem())
   return 0;
  PtrA<DST> c = aCopy<DST>(a);
  c->acs=acs;
  c->set_vector_default();
//assert(!c->mem_owner());
  return c;
}
#else
template<class DST,class SRC> inline
PtrA<DST> aView(const PtrA<SRC> & a, const MemAccess2 & acs)
{
  if(acs.n_elem()!=a->acs.n_elem())
   return 0;
  PtrA<DST> c = aCopy<DST>(a);
  c->acs=acs;
  c->renew_mem(a->v,0,NULL);
  c->set_vector_default();
  
  assert(!c->mem_owner());
  return c;
}
#endif


/**
   Cast vector to array
 */
template<class TARRAY> inline
PtrA<TARRAY> aCast(const Ptr<Vector> & a_,int dim)
{
  cTypePerformer<typename TARRAY::value_type> tp;
  PtrA<TARRAY> a = PtrACastToArray<TARRAY>(a_);
  if (a && a->dim()==dim && a->tp.basetype==tp.basetype)
    return a;
  return 0;
}

/**
   Check if vector can be casted to array
 */
template<class TARRAY> inline
int aValid(const Ptr<Vector> & a_,int dim)
{
  cTypePerformer<typename TARRAY::value_type> tp;
  PtrA<TARRAY> a = PtrACastToArray<TARRAY>(a_);
  if (a && a->dim()==dim && a->tp.basetype==tp.basetype)
    return 1;
  else
    return 0;
}


#if 0
template<class DST,class SRC> inline
Ptr<DST> aRow(const SRC* a, int irow)
{
  Ptr<DST> c = new DST();
  c->copy((SRC&)(*a));
  
  typename SRC::value_type* v=a->v;
  const MemAccess2& acs=a->acs;
  switch(acs.dim) {
   case 5: c->v=&(v[acs(irow,1,1,1,1)])-1; break;
   case 4: c->v=&(v[acs(irow,1,1,1  )])-1; break;
   case 3: c->v=&(v[acs(irow,1,1    )])-1; break;
   case 2: c->v=&(v[acs(irow,1      )])-1; break;
   default: assert(0);
  };
  c->acs=maRow(a->acs);
//c->set_vector_vtype();
  c->set_vector_default();
  c->aclass=0;
  //assert(!c->mem_owner());
  return c;
}



template<class DST,class SRC> inline
PtrA<DST> aCol(const SRC* a, int icol)
{
  PtrA<DST> c = new DST();
  c->copy((SRC&)(*a));
  
  typename SRC::value_type* v=a->v;
  const MemAccess2& acs=a->acs;
  switch(acs.dim) {
   case 5: c->v=&(v[acs(1,icol,1,1,1)])-1; break;
   case 4: c->v=&(v[acs(1,icol,1,1  )])-1; break;
   case 3: c->v=&(v[acs(1,icol,1    )])-1; break;
   case 2: c->v=&(v[acs(1,icol      )])-1; break;
   default: assert(0);
  };
  assert(0);
///  c->acs=maCol(a->acs);
//c->set_vector_vtype();
  c->set_vector_default();
  c->aclass=a->aclass;
  //assert(!c->mem_owner());
  return c; 
}
#endif



////////////////////////////////////////////////////
////////////////////////////////////////////////////
// 6. Dimension specific class templates


template <class T> struct TArray2;


/**
   One dimensional array template
 */
template<class T>
struct TArray1 : public TArray<T> {
 typedef typename TArray<T>::free_func free_func;
 typedef typename TArray<T>::init_list_type_1d init_list_type_1d;
 int type_dim() const { return 1; }
 TArray1() { this->acs.dim=type_dim(); TArray<T>::set_vector_vtype();  }
 TArray1(int n1)
   : TArray<T>(n1) {TArray<T>::set_vector_vtype();}  ///< Array constructor
 TArray1(int n1, T* mem,int memoffset,free_func ff=NULL)
       : TArray<T>(n1,mem,memoffset,ff) {TArray<T>::set_vector_vtype();}
 TArray1(const MemAccess2 & acs_)
       : TArray<T>(acs_) { assert(acs_.dim==1); TArray<T>::set_vector_vtype();}
 TArray1(const MemAccess2 & acs_, T* mem,int memoffset,free_func ff=NULL)
       : TArray<T>(acs_,mem,memoffset,ff) { assert(acs_.dim==1); TArray<T>::set_vector_vtype();}
 TArray1( init_list_type_1d il ) : TArray<T>(il) { TArray<T>::set_vector_vtype();} ///< Array constructor
 T& operator()(int n1) { return TArray<T>::operator()(n1); }         ///< Array access
 T& operator()(int n1) const { return TArray<T>::operator()(n1); }   ///< Array access  
 
 PtrA< TArray1<T> > operator+(const PtrA< TArray1<T> >& R)
 {
   PtrA< TArray1<T> > V=aClone(R);
   vlin1(PtrAWeak< TArray1<T> >(this)
       ,  1.0
       , PtrCast<Vector>(R)
       , PtrCast<Vector>(V));
   return V;
 }  ///< Arithmetic operator overload
 PtrA< TArray1<T> > operator-(const PtrA< TArray1<T> >& R)
 {
   PtrA< TArray1<T> > V=aClone(R);
   vlin1(PtrAWeak< TArray1<T> >(this)
      , -1.0
      , PtrCast<Vector>(R)
      , PtrCast<Vector>(V));
   return V;
 }  ///< Arithmetic operator overload
 PtrA< TArray1<T> > operator*(double s)
 {
   PtrA< TArray1<T> > V=aClone(PtrA< TArray1<T> >( PtrAWeak< TArray1<T> >(this) ));
   V->copy_v(*this);
   vscal(s,V);
   return V;
 }  ///< Arithmetic operator overload
 
 PtrA<TArray1<T> > operator/(const PtrA<TArray2<T> >& A); ///< Arithmetic operator overload
 
 // swig -lua
 T __getitem__(int i) const { return operator()(i+TArray<T>::jl_array_offset); } 
 void __setitem__(int i,T d) { operator()(i+TArray<T>::jl_array_offset)=d; } 
 int __len(void*) { return TArray<T>::nall(); }
 void view(T** data, int* length) const // use for swig-python
 {
   *data = this->v+1;
   *length = this->n(1);
 }
 

  virtual double __getitem__V(int i) const { return (double)(double_complex(__getitem__(i)).real()); } 
  virtual void __setitem__V(int i,double d) { __setitem__(i,(T)d); }
  virtual int __lenV(void*) { return __len(NULL); }
 
 inline void insert(int i,T v){size_t nn[]={0,(size_t)i}; assert(this->extend(nn)); (*this)(i)=v;}; ///< Value insertion (possible reallocation)

};

template<class T> inline
PtrA<TArray1<T> > operator*(PtrA<TArray1<T> > a, double s)
{
  return a->operator*(s);
}



/**
   Two dimensional array template
 */

template<class T>
struct TArray2 : public TArray<T> {
 typedef typename TArray<T>::free_func free_func;
 typedef typename TArray<T>::init_list_type_2d init_list_type_2d;
 int type_dim() const { return 2; }
 TArray2() { this->acs.dim=type_dim(); TArray<T>::set_vector_vtype();  }
 TArray2(int n1,int n2)
   : TArray<T>(n1,n2) {TArray<T>::set_vector_vtype();} ///< Array constructor
 TArray2(int n1,int n2, T* mem,int memoffset,free_func ff=NULL)
       : TArray<T>(n1,n2,mem,memoffset,ff) {TArray<T>::set_vector_vtype();}
 TArray2(const MemAccess2 & acs_)
       : TArray<T>(acs_) { assert(acs_.dim==2); TArray<T>::set_vector_vtype();}
 TArray2(const MemAccess2 & acs_, T* mem,int memoffset,free_func ff=NULL)
       : TArray<T>(acs_,mem,memoffset,ff) { assert(acs_.dim==2); TArray<T>::set_vector_vtype();}
 TArray2( init_list_type_2d il ) : TArray<T>(il) { TArray<T>::set_vector_vtype();} ///< Array constructor
 T& operator()(int n1,int n2) { return TArray<T>::operator()(n1,n2); }  ///< Array access
 T& operator()(int n1,int n2) const { return TArray<T>::operator()(n1,n2); } ///< Array access
 
  // This is an elegant way to write a(5) to get 5th row...
 T* operator()(int n1) { return &TArray<T>::operator()(n1,1); }
 T* operator()(int n1) const { return &TArray<T>::operator()(n1,1); }
 
 PtrA<TArray2<T> > operator+(const PtrA<TArray2<T> >& R)
 {
   PtrA<TArray2<T> > V=aClone(R);
   vlin1(PtrAWeak< TArray2<T> >(this)
       ,  1.0
       , PtrCast<Vector>(R)
       , PtrCast<Vector>(V));
   return V;
 }  ///< Arithmetic operator overload
 PtrA<TArray2<T> > operator-(const PtrA<TArray2<T> >& R)
 {
   PtrA<TArray2<T> > V=aClone(R);
   vlin1(PtrAWeak< TArray2<T> >(this)
     , -1.0
     , PtrCast<Vector>(R)
     , PtrCast<Vector>(V));
   return V;
 } ///< Arithmetic operator overload
 PtrA<TArray2<T> > operator*(double s)
 {
   PtrA< TArray2<T> > V=aClone(PtrA< TArray2<T> >( PtrAWeak< TArray2<T> >(this) ));
   V->copy_v(*this);
   vscal(s,PtrCast<Vector>(V));
   return V;
 } ///< Arithmetic operator overload
 PtrA<TArray1<T> > operator*(const PtrA<TArray1<T> >& x); ///< Arithmetic operator overload
 

 PtrA< TArray1<T> > row_copy(int irow) const
 {
   const TArray2<T>&  a = *this;
   const int n = a.ncol();
   PtrA< TArray1<T> > c = new TArray1<T>(n);
   
   for(int i=1;i<=n;++i)
     c(i)=a(irow,i);
   
   return c;
 }
 
 PtrA< TArray1<T> > col_copy(int icol) const
 {
   const TArray2<T>&  a = *this;
   const int n = a.nrow();
   PtrA< TArray1<T> > c = new TArray1<T>(n);
   
   for(int i=1;i<=n;++i)
     c(i)=a(i,icol);
 
   c->aclass=a.aclass;
   return c; 
 }
 
 
 // swig -lua
 PtrA<TArray1<T> > row(int irow) const
 {
   const TArray2<T>&  a = *this;
   PtrA < TArray1<T> > c = new TArray1<T>();
   c->copy((const TArray2<T>&)(a));

   T* v=a.v;
   const MemAccess2& acs=a.acs;
   switch(acs.dim) {
    case 5: c->v=&(v[acs(irow,1,1,1,1)])-1; break;
    case 4: c->v=&(v[acs(irow,1,1,1  )])-1; break;
    case 3: c->v=&(v[acs(irow,1,1    )])-1; break;
    case 2: c->v=&(v[acs(irow,1      )])-1; break;
    default: assert(0);
   };
   c->acs=maRow(a.acs);
 //c->set_vector_vtype();
   c->set_vector_default();
   c->aclass=0;
   //assert(!c->mem_owner());
   return c;
 }
 PtrA<TArray1<T> > col(int icol) const { return col_copy(icol); }
 PtrA<TArray1<T> > __getitem__(int irow) const { return row(irow+TArray<T>::jl_array_offset); }
 int __len(void*) { return TArray<T>::nrow(); }
 void view(int* nrows, int* ncols, T** data) const // use for swig-python
 {
   *data = this->v+1;
   *nrows = this->nrow();
   *ncols = this->ncol();
 }

  virtual double __getitem__V(int i) const { return (double)__getitem__(i); } 
  virtual int __lenV(void*) { return __len(NULL); }
 
 // add row for covenient initialization etc.
 
 inline void insert(int i, int j, T v){size_t nn[]={0,(size_t)i,(size_t)j}; assert(this->extend(nn)); (*this)(i,j)=v;};  ///< Value insertion (possible reallocation)
 inline void insertrow(int i,T v1                    ){size_t nn[]={0,(size_t)i,1}; assert(this->extend(nn)); (*this)(i,1)=v1;} ///<  Convenient row insertion (possible reallocation)
 inline void insertrow(int i,T v1,T v2               ){size_t nn[]={0,(size_t)i,2}; assert(this->extend(nn)); (*this)(i,1)=v1;(*this)(i,2)=v2;} ///<  Convenient row insertion (possible reallocation)
 inline void insertrow(int i,T v1,T v2,T v3          ){size_t nn[]={0,(size_t)i,3}; assert(this->extend(nn)); (*this)(i,1)=v1;(*this)(i,2)=v2;(*this)(i,3)=v3;} ///<  Convenient row insertion (possible reallocation)
 inline void insertrow(int i,T v1,T v2,T v3,T v4     ){size_t nn[]={0,(size_t)i,4}; assert(this->extend(nn)); (*this)(i,1)=v1;(*this)(i,2)=v2;(*this)(i,3)=v3;(*this)(i,4)=v4;} ///<  Convenient row insertion (possible reallocation)
 inline void insertrow(int i,T v1,T v2,T v3,T v4,T v5){size_t nn[]={0,(size_t)i,5}; assert(this->extend(nn)); (*this)(i,1)=v1;(*this)(i,2)=v2;(*this)(i,3)=v3;(*this)(i,4)=v4;(*this)(i,5)=v5;} ///<  Convenient row insertion (possible reallocation)
 inline void insertrow(int i,T v1,T v2,T v3,T v4,T v5,T v6){size_t nn[]={0,(size_t)i,6}; assert(this->extend(nn)); (*this)(i,1)=v1;(*this)(i,2)=v2;(*this)(i,3)=v3;(*this)(i,4)=v4;(*this)(i,5)=v5;(*this)(i,6)=v6;} ///<  Convenient row insertion (possible reallocation)
};

template<class T> inline
PtrA<TArray2<T> > operator*(PtrA<TArray2<T> > a, double s)
{
  return a->operator*(s);
}

template<class T> inline
bool mult2(PtrA<TArray2<T> > b, const PtrA<TArray2<T> >& a, const PtrA<TArray2<T> >& x)
{
  const int am=a->nrow();
  const int an=a->ncol();
  
  const int xm=x->nrow();
  const int xn=x->ncol();
  
  if(an!=xm) return false;
  
  if( b->nrow() != am ) return false;
  if( b->ncol() != xn ) return false;
  
//PtrA< TArray2<T> > b = new TArray2<T>( am, xn );
  
  for(int d=1;d<=am;++d)
  {
    for(int l=1;l<=xn;++l)
    {
      double& entry = b(d,l);
      entry=0;
      for(int k=1;k<=an;++k)
       entry+=a(d,k)*x(k,l);
    }
  }
  return true;
}



/**
   3 dimensional array template
 */

template<class T>
struct TArray3 : public TArray<T> {
 typedef typename TArray<T>::free_func free_func;
 typedef typename TArray<T>::init_list_type_3d init_list_type_3d;
 int type_dim() const { return 3; }
 TArray3() { this->acs.dim=type_dim(); TArray<T>::set_vector_vtype();  }
 TArray3(int n1,int n2,int n3)
       : TArray<T>(n1,n2,n3) {TArray<T>::set_vector_vtype();} ///< Array constructor
 TArray3(int n1,int n2,int n3, T* mem,int memoffset,free_func ff=NULL)
       : TArray<T>(n1,n2,n3,mem,memoffset,ff) {TArray<T>::set_vector_vtype();}
 TArray3(const MemAccess2 & acs_)
       : TArray<T>(acs_) { assert(acs_.dim==3); TArray<T>::set_vector_vtype();}
 TArray3(const MemAccess2 & acs_, T* mem,int memoffset,free_func ff=NULL)
       : TArray<T>(acs_,mem,memoffset,ff) { assert(acs_.dim==3); TArray<T>::set_vector_vtype();}
 TArray3( init_list_type_3d il ) : TArray<T>(il) { TArray<T>::set_vector_vtype();} ///< Array constructor
 T& operator()(int n1,int n2,int n3) { return TArray<T>::operator()(n1,n2,n3); } ///< Array access
 T& operator()(int n1,int n2,int n3) const { return TArray<T>::operator()(n1,n2,n3); } ///< Array access

 PtrA< TArray1<T> > col_copy(int icol, int jcol) const
 {
   const TArray3<T>&  a = *this;
   const int n = a.nrow();
   PtrA< TArray1<T> > c = new TArray1<T>(n);
   
   for(int i=1;i<=n;++i)
     c(i)=a(i,icol,jcol);
 
   c->aclass=a.aclass;
   return c; 
 }

};


/**
   4 dimensional array template
 */
template<class T>
struct TArray4 : public TArray<T> {
 typedef typename TArray<T>::free_func free_func;
 typedef typename TArray<T>::init_list_type_4d init_list_type_4d;
 int type_dim() const { return 4; }
 TArray4() { this->acs.dim=type_dim(); TArray<T>::set_vector_vtype();  }
 TArray4(int n1,int n2,int n3,int n4) 
       : TArray<T>(n1,n2,n3,n4) {TArray<T>::set_vector_vtype();} ///< Array constructor
 TArray4(int n1,int n2,int n3,int n4, T* mem,int memoffset,free_func ff=NULL) 
       : TArray<T>(n1,n2,n3,n4,mem,memoffset,ff) {TArray<T>::set_vector_vtype();}
 TArray4(const MemAccess2 & acs_)
       : TArray<T>(acs_) { assert(acs_.dim==4); TArray<T>::set_vector_vtype();}
 TArray4(const MemAccess2 & acs_, T* mem,int memoffset,free_func ff=NULL)
       : TArray<T>(acs_,mem,memoffset,ff) { assert(acs_.dim==4); TArray<T>::set_vector_vtype();}
 TArray4( init_list_type_4d il ) : TArray<T>(il) { TArray<T>::set_vector_vtype();} ///< Array constructor
 T& operator()(int n1,int n2,int n3,int n4) { return TArray<T>::operator()(n1,n2,n3,n4); } ///< Array access
 T& operator()(int n1,int n2,int n3,int n4) const { return TArray<T>::operator()(n1,n2,n3,n4); } ///< Array access
};

/**
   5 dimensional array template
 */
template<class T>
struct TArray5 : public TArray<T> {
 typedef typename TArray<T>::free_func free_func;
 typedef typename TArray<T>::init_list_type_5d init_list_type_5d;
 int type_dim() const { return 5; }
 TArray5() { this->acs.dim=type_dim(); TArray<T>::set_vector_vtype(); }
 TArray5(int n1,int n2,int n3,int n4,int n5)
       : TArray<T>(n1,n2,n3,n4,n5) {TArray<T>::set_vector_vtype();} ///< Array constructor
 TArray5(int n1,int n2,int n3,int n4,int n5, T* mem,int memoffset, free_func ff=NULL) 
       : TArray<T>(n1,n2,n3,n4,n5,mem,memoffset,ff) {TArray<T>::set_vector_vtype();}
 TArray5(const MemAccess2 & acs_)
       : TArray<T>(acs_) { assert(acs_.dim==5); TArray<T>::set_vector_vtype();}
 TArray5(const MemAccess2 & acs_, T* mem,int memoffset,free_func ff=NULL)
       : TArray<T>(acs_,mem,memoffset,ff) { assert(acs_.dim==5); TArray<T>::set_vector_vtype();}
 TArray5( init_list_type_5d il ) : TArray<T>(il) { TArray<T>::set_vector_vtype();} ///< Array constructor
 T& operator()(int n1,int n2,int n3,int n4,int n5) { return TArray<T>::operator()(n1,n2,n3,n4,n5); } ///< Array access
 T& operator()(int n1,int n2,int n3,int n4,int n5) const { return TArray<T>::operator()(n1,n2,n3,n4,n5); }///< Array access
};


////////////////////////////////////////////////////
////////////////////////////////////////////////////
// 7. Type specializations

typedef TArray1<double> DArray1;  ///< Typedef for 1D double array
typedef TArray2<double> DArray2;  ///< Typedef for 2D double array
typedef TArray3<double> DArray3;  ///< Typedef for 3D double array
typedef TArray4<double> DArray4;  ///< Typedef for 4D double array
typedef TArray5<double> DArray5;  ///< Typedef for 5D double array

typedef TArray1<int> IArray1; ///< Typedef for 1D int array
typedef TArray2<int> IArray2; ///< Typedef for 2D int array
typedef TArray3<int> IArray3; ///< Typedef for 3D int array
typedef TArray4<int> IArray4; ///< Typedef for 4D int array
typedef TArray5<int> IArray5; ///< Typedef for 5D int array

typedef TArray1<double_complex> ZArray1;  ///< Typedef for 1D double_complex array
typedef TArray2<double_complex> ZArray2;  ///< Typedef for 2D double_complex array
typedef TArray3<double_complex> ZArray3;  ///< Typedef for 3D double_complex array
typedef TArray4<double_complex> ZArray4;  ///< Typedef for 4D double_complex array
typedef TArray5<double_complex> ZArray5;  ///< Typedef for 5D double_complex array

typedef TArray1<char> BArray1; ///< Typedef for 1D bayte array
typedef TArray2<char> BArray2; ///< Typedef for 2D bayte array
typedef TArray3<char> BArray3; ///< Typedef for 3D bayte array
typedef TArray4<char> BArray4; ///< Typedef for 4D bayte array
typedef TArray5<char> BArray5; ///< Typedef for 5D bayte array


typedef TArray<double> DArray;
typedef TArray<double_complex> ZArray;
typedef TArray<int> IArray;
typedef TArray<char> BArray;


// inline Ptr<DArray1> da1Cast(const Ptr<DArray1>& a) { return a; }



////////////////////////////////////////////////////
////////////////////////////////////////////////////
// 8. Lua specific part 

SwigBindBegin
void jl_array_offset(int off);
SwigBindEnd

#if SwigExportOnly

SwigBindBegin
struct DArray1 { 
  double __getitem__(int i) const; 
  void __setitem__(int i,double d);
  int __len(void*);
  void view(double** data, int* length) const; // use for swig-python
  int nall();
  int size();
  int shape(const int i);
  int ndim();
  void print();
  void print(char *name);
  PtrA<DArray1> operator+(const PtrA<DArray1>& R); 
  PtrA<DArray1> operator-(const PtrA<DArray1>& R); 
  PtrA<DArray1> operator*(double s);
  PtrA<DArray1> operator/(const PtrA<DArray2>& R) { return ::operator/(PtrAWeak<DArray1>(self),R); }
};


#ifdef SWIGLUA
/////////////////////////
//
// Input typemaps, which also try to cast the pointer from lua state L at stack position $input
// to Ptr<Vector>.
//
%typemap(in) PtrA<DArray1>& {
  
  // typemap(in) PtrA<DArray1>&
  if (!SWIG_IsOK(SWIG_ConvertPtr(L,$input,(void**)&($1),SWIGTYPE_p_PtrAT_DArray1_t,0))){
    if (!SWIG_IsOK(SWIG_ConvertPtr(L,$input,(void**)&($1),SWIGTYPE_p_PtrT_Vector_t,0)))
    {
      SWIG_fail_ptr("$symname",$input,SWIGTYPE_p_PtrAT_DArray1_t);
    } else
    {
      if(!da1Cast(*$1))
       SWIG_fail_ptr("$symname, wrong Ptr<Vector> in",$input,SWIGTYPE_p_PtrAT_DArray1_t);
    }
  }
}
%typemap(in) PtrA<DArray1> const & {
  
  // typemap(in) PtrA<DArray1> const &
  if (!SWIG_IsOK(SWIG_ConvertPtr(L,$input,(void**)&($1),SWIGTYPE_p_PtrAT_DArray1_t,0))){
    if (!SWIG_IsOK(SWIG_ConvertPtr(L,$input,(void**)&($1),SWIGTYPE_p_PtrT_Vector_t,0)))
    {
      SWIG_fail_ptr("$symname",$input,SWIGTYPE_p_PtrAT_DArray1_t);
    } else
    {
      if(!da1Cast(*$1))
       SWIG_fail_ptr("$symname, wrong Ptr<Vector> in",$input,SWIGTYPE_p_PtrAT_DArray1_t);
    }
  }
}



%typemap(in) PtrA<DArray2>& {
  
  // typemap(in) PtrA<DArray2>&
  if (!SWIG_IsOK(SWIG_ConvertPtr(L,$input,(void**)&($1),SWIGTYPE_p_PtrAT_DArray2_t,0))){
    if (!SWIG_IsOK(SWIG_ConvertPtr(L,$input,(void**)&($1),SWIGTYPE_p_PtrT_Vector_t,0)))
    {
      SWIG_fail_ptr("$symname",$input,SWIGTYPE_p_PtrAT_DArray2_t);
    } else
    {
      if(!da2Cast(*$1))
       SWIG_fail_ptr("$symname, wrong Ptr<Vector> in",$input,SWIGTYPE_p_PtrAT_DArray2_t);
    }
  }
}
%typemap(in) PtrA<DArray2> const & {
  
  // typemap(in) PtrA<DArray2> const &
  if (!SWIG_IsOK(SWIG_ConvertPtr(L,$input,(void**)&($1),SWIGTYPE_p_PtrAT_DArray2_t,0))){
    if (!SWIG_IsOK(SWIG_ConvertPtr(L,$input,(void**)&($1),SWIGTYPE_p_PtrT_Vector_t,0)))
    {
      SWIG_fail_ptr("$symname",$input,SWIGTYPE_p_PtrAT_DArray2_t);
    } else
    {
      if(!da2Cast(*$1))
       SWIG_fail_ptr("$symname, wrong Ptr<Vector> in",$input,SWIGTYPE_p_PtrAT_DArray2_t);
    }
  }
}


/////////////////////////
//
// For a new input typemaps, we have to define a new typechecking rule as well (in case of
// overloaded methods). This code will be inserted into the dynamic dispatcher.
//
// Important: Do not shadow SWIG's own precedence values!
// SWIG first defines a general type hierarchy with precedence values beginning with
// SWIG_TYPECHECK_POINTER=0, next is SWIG_TYPECHECK_VOIDPTR=10, and the last one is
// SWIG_TYPECHECK_STRING_ARRAY=114.
//
%typemap(typecheck,precedence=1) PtrA<DArray1>& {
  void *ptr;
  if (lua_isuserdata(L,$input)==0 || SWIG_ConvertPtr(L,$input, (void **) &ptr, $1_descriptor, 0)) {
    $1 = 0;
    if(SWIG_IsOK(SWIG_ConvertPtr(L,$input,(void**)&ptr,SWIGTYPE_p_PtrT_Vector_t,0)))
     $1 = 1;
  } else {
    $1 = 1;
  }
}

%typemap(typecheck,precedence=2) PtrA<DArray1> const& {
  void *ptr;
  if (lua_isuserdata(L,$input)==0 || SWIG_ConvertPtr(L,$input, (void **) &ptr, $1_descriptor, 0)) {
    $1 = 0;
    if(SWIG_IsOK(SWIG_ConvertPtr(L,$input,(void**)&ptr,SWIGTYPE_p_PtrT_Vector_t,0)))
     $1 = 1;
  } else {
    $1 = 1;
  }
}




%typemap(typecheck,precedence=1) PtrA<DArray2>& {
  void *ptr;
  if (lua_isuserdata(L,$input)==0 || SWIG_ConvertPtr(L,$input, (void **) &ptr, $1_descriptor, 0)) {
    $1 = 0;
    if(SWIG_IsOK(SWIG_ConvertPtr(L,$input,(void**)&ptr,SWIGTYPE_p_PtrT_Vector_t,0)))
     $1 = 1;
  } else {
    $1 = 1;
  }
}

%typemap(typecheck,precedence=2) PtrA<DArray2> const& {
  void *ptr;
  if (lua_isuserdata(L,$input)==0 || SWIG_ConvertPtr(L,$input, (void **) &ptr, $1_descriptor, 0)) {
    $1 = 0;
    if(SWIG_IsOK(SWIG_ConvertPtr(L,$input,(void**)&ptr,SWIGTYPE_p_PtrT_Vector_t,0)))
     $1 = 1;
  } else {
    $1 = 1;
  }
}

#endif




struct ZArray1 { 
 double_complex __getitem__(int i) const; 
 void __setitem__(int i,double_complex d); 
 int __len(void*);
 int nall();
 void print();
 void print(char *name);
};
struct IArray1 { 
  int __getitem__(int i) const; 
  void __setitem__(int i,int d); 
  int __len(void*);
  int nall();
  int size();
  int shape(const int i);
  int ndim();
  void print();
 void print(char *name);
};

struct DArray2 { 
  PtrA<DArray1> row(int irow) const; 
  PtrA<DArray1> col(int icol) const;	
  PtrA<DArray1> __getitem__(int irow) const;
  int __len(void*);
  void view(int* nrows, int* ncols, double** data) const; // use for swig-python
  PtrA<DArray2> operator+(const PtrA<DArray2>& R); 
  PtrA<DArray2> operator-(const PtrA<DArray2>& R); 
  PtrA<DArray2> operator*(double s);
  PtrA<DArray1> operator*(const PtrA<DArray1>& R) { return ::operator*(PtrAWeak<DArray2>(self),R); }
  PtrA<DArray1> col_copy(int icol);
  int nall();
  int nrow();
  int ncol();
  int size();
  int shape(const int i);
  int ndim();  
  void print();
  void print(char *name);
};

struct DArray3 { 
  PtrA<DArray1> col_copy(int icol, int jcol);
  int nall();
  int n(int i);
  int dim();
  int size();
  int shape(const int i);
  int ndim();
  int nrow();
  int ncol();
};

struct ZArray2 { 
  PtrA<ZArray1> row(int irow) const; 
  PtrA<ZArray1> col(int icol) const;	
  PtrA<ZArray1> __getitem__(int irow) const;
  int __len(void*);
  int nall();
  int nrow();
  int ncol();
  int size();
  int shape(const int i);
  int ndim();
  void print();
  void print(char *name);
};
struct IArray2 { 
  PtrA<IArray1> row(int irow) const; 
  PtrA<IArray1> col(int icol) const;	
  PtrA<IArray1> __getitem__(int irow) const; 
  int __len(void*);
  int nall();
  int nrow();
  int ncol();
  int size();
  int shape(const int i);
  int ndim();
  void print();
  void print(char *name);
};
SwigBindEnd
//SwigBindPtrA(IArray1)
//SwigBindPtrA(IArray2)
//SwigBindPtrA(IArray3)
//SwigBindPtrA(DArray1)
//SwigBindPtrA(DArray2)
//SwigBindPtrA(DArray3)
//SwigBindPtrA(ZArray1)
//SwigBindPtrA(ZArray2)
//SwigBindPtrA(ZArray3)

// Vector -> Array
/// SwigBindPtrCast(DArray1,Vector)
/// SwigBindPtrCast(DArray2,Vector)
/// SwigBindPtrCast(DArray3,Vector)
/// 
/// SwigBindPtrCast(ZArray1,Vector)
/// SwigBindPtrCast(ZArray2,Vector)
/// SwigBindPtrCast(ZArray3,Vector)
/// 
/// SwigBindPtrCast(IArray1,Vector)
/// SwigBindPtrCast(IArray2,Vector)
/// SwigBindPtrCast(IArray3,Vector)
   
// Array -> Vector
/// SwigBindPtrCast(Vector,DArray1)
/// SwigBindPtrCast(Vector,DArray2)
/// SwigBindPtrCast(Vector,DArray3)
/// 
/// SwigBindPtrCast(Vector,ZArray1)
/// SwigBindPtrCast(Vector,ZArray2)
/// SwigBindPtrCast(Vector,ZArray3)
/// 
/// SwigBindPtrCast(Vector,IArray1)
/// SwigBindPtrCast(Vector,IArray2)
/// SwigBindPtrCast(Vector,IArray3)




SwigBindBegin

template<class A> struct PtrA
{
  A* operator->() const;
};

template<class DST> PtrA<DST> PtrACastToArray(const Ptr<Vector>&s);

template<class SRC> Ptr<Vector> PtrACastToVector(const PtrA<SRC>&s);

/// template<class ARRAY> Ptr<ARRAY> PtrACast(const PtrA<ARRAY>&s);


///< SwigBindPtrA
%template(PtrAIArray1) PtrA<IArray1>;
%template(PtrAIArray2) PtrA<IArray2>;
%template(PtrAIArray3) PtrA<IArray3>;

%template(PtrADArray1) PtrA<DArray1>;
%template(PtrADArray2) PtrA<DArray2>;
%template(PtrADArray3) PtrA<DArray3>;

%template(PtrAZArray1) PtrA<ZArray1>;
%template(PtrAZArray2) PtrA<ZArray2>;
%template(PtrAZArray3) PtrA<ZArray3>;


///< SwigBindPtrCast(to,from)
%template(PtrCast_DArray1_Vector) PtrACastToArray<DArray1>;
%template(PtrCast_DArray2_Vector) PtrACastToArray<DArray2>;
%template(PtrCast_DArray3_Vector) PtrACastToArray<DArray3>;

%template(PtrCast_Vector_DArray1) PtrACastToVector<DArray1>;
%template(PtrCast_Vector_DArray2) PtrACastToVector<DArray2>;
%template(PtrCast_Vector_DArray3) PtrACastToVector<DArray3>;


%template(PtrCast_IArray1_Vector) PtrACastToArray<IArray1>;
%template(PtrCast_IArray2_Vector) PtrACastToArray<IArray2>;
%template(PtrCast_IArray3_Vector) PtrACastToArray<IArray3>;

%template(PtrCast_Vector_IArray1) PtrACastToVector<IArray1>;
%template(PtrCast_Vector_IArray2) PtrACastToVector<IArray2>;
%template(PtrCast_Vector_IArray3) PtrACastToVector<IArray3>;


%template(PtrCast_ZArray1_Vector) PtrACastToArray<ZArray1>;
%template(PtrCast_ZArray2_Vector) PtrACastToArray<ZArray2>;
%template(PtrCast_ZArray3_Vector) PtrACastToArray<ZArray3>;

%template(PtrCast_Vector_ZArray1) PtrACastToVector<ZArray1>;
%template(PtrCast_Vector_ZArray2) PtrACastToVector<ZArray2>;
%template(PtrCast_Vector_ZArray3) PtrACastToVector<ZArray3>;





/// %template(PtrACast_VectorToDArray1) PtrACastToArray<DArray1>;
/// %template(PtrACast_VectorToDArray2) PtrACastToArray<DArray2>;
/// %template(PtrACast_VectorToDArray3) PtrACastToArray<DArray3>;
/// 
/// %template(PtrACast_VectorToIArray1) PtrACastToArray<IArray1>;
/// %template(PtrACast_VectorToIArray2) PtrACastToArray<IArray2>;
/// %template(PtrACast_VectorToIArray3) PtrACastToArray<IArray3>;
/// 
/// %template(PtrACast_DArray1ToVector) PtrACastToVector<DArray1>;
/// %template(PtrACast_DArray2ToVector) PtrACastToVector<DArray2>;
/// %template(PtrACast_DArray3ToVector) PtrACastToVector<DArray3>;
/// 
/// %template(PtrACast_IArray1ToVector) PtrACastToVector<IArray1>;
/// %template(PtrACast_IArray2ToVector) PtrACastToVector<IArray2>;
/// %template(PtrACast_IArray3ToVector) PtrACastToVector<IArray3>;


/// %template(PtrACast_DArray1) PtrACast<DArray1>;
/// %template(PtrACast_DArray2) PtrACast<DArray2>;
/// %template(PtrACast_DArray3) PtrACast<DArray3>;
/// 
/// %template(PtrACast_IArray1) PtrACast<IArray1>;
/// %template(PtrACast_IArray2) PtrACast<IArray2>;
/// %template(PtrACast_IArray3) PtrACast<IArray3>;
/// 
/// %template(PtrACast_ZArray1) PtrACast<ZArray1>;
/// %template(PtrACast_ZArray2) PtrACast<ZArray2>;
/// %template(PtrACast_ZArray3) PtrACast<ZArray3>;


SwigBindEnd
#endif // SwigExportOnly



////////////////////////////////////////////////////
////////////////////////////////////////////////////
// 9. Ptr <-> PtrA casts

SwigBind inline PtrA<DArray1> aCast(Ptr<DArray1> a) { return a ;}
SwigBind inline PtrA<DArray2> aCast(Ptr<DArray2> a) { return a ;}
SwigBind inline PtrA<DArray3> aCast(Ptr<DArray3> a) { return a ;}
SwigBind inline PtrA<DArray4> aCast(Ptr<DArray4> a) { return a ;}
SwigBind inline PtrA<DArray5> aCast(Ptr<DArray5> a) { return a ;}

SwigBind inline Ptr<DArray1>  aCast(PtrA<DArray1> a) { return a ;}
SwigBind inline Ptr<DArray2>  aCast(PtrA<DArray2> a) { return a ;}
SwigBind inline Ptr<DArray3>  aCast(PtrA<DArray3> a) { return a ;}
SwigBind inline Ptr<DArray4>  aCast(PtrA<DArray4> a) { return a ;}
SwigBind inline Ptr<DArray5>  aCast(PtrA<DArray5> a) { return a ;}

SwigBind inline PtrA<IArray1> aCast(Ptr<IArray1> a) { return a ;}
SwigBind inline PtrA<IArray2> aCast(Ptr<IArray2> a) { return a ;}
SwigBind inline PtrA<IArray3> aCast(Ptr<IArray3> a) { return a ;}
SwigBind inline PtrA<IArray4> aCast(Ptr<IArray4> a) { return a ;}
SwigBind inline PtrA<IArray5> aCast(Ptr<IArray5> a) { return a ;}

SwigBind inline Ptr<IArray1>  aCast(PtrA<IArray1> a) { return a ;}
SwigBind inline Ptr<IArray2>  aCast(PtrA<IArray2> a) { return a ;}
SwigBind inline Ptr<IArray3>  aCast(PtrA<IArray3> a) { return a ;}
SwigBind inline Ptr<IArray4>  aCast(PtrA<IArray4> a) { return a ;}
SwigBind inline Ptr<IArray5>  aCast(PtrA<IArray5> a) { return a ;}



////////////////////////////////////////////////////
////////////////////////////////////////////////////
// 10. Ptr based API

///////////////
// 10.1 Constructors of new arrays, with allocating new data space

SwigBind inline PtrA<DArray1> da1Create(int n1				    ) { return new DArray1(n1		 ); } ///< Create 1D double array
SwigBind inline PtrA<DArray2> da2Create(int n1, int n2			    ) { return new DArray2(n1,n2	 ); } ///< Create 2D double array
SwigBind inline PtrA<DArray3> da3Create(int n1, int n2, int n3		    ) { return new DArray3(n1,n2,n3	 ); } ///< Create 3D double array
SwigBind inline PtrA<DArray4> da4Create(int n1, int n2, int n3, int n4	    ) { return new DArray4(n1,n2,n3,n4	 ); } ///< Create 4D double array
SwigBind inline PtrA<DArray5> da5Create(int n1, int n2, int n3, int n4, int n5) { return new DArray5(n1,n2,n3,n4,n5); } ///< Create 5D double array

SwigBind inline PtrA<IArray1> ia1Create(int n1				    ) { return new IArray1(n1		 ); } ///< Create 1D int array
SwigBind inline PtrA<IArray2> ia2Create(int n1, int n2			    ) { return new IArray2(n1,n2	 ); } ///< Create 2D int array
SwigBind inline PtrA<IArray3> ia3Create(int n1, int n2, int n3		    ) { return new IArray3(n1,n2,n3	 ); } ///< Create 3D int array
SwigBind inline PtrA<IArray4> ia4Create(int n1, int n2, int n3, int n4	    ) { return new IArray4(n1,n2,n3,n4	 ); } ///< Create 4D int array
SwigBind inline PtrA<IArray5> ia5Create(int n1, int n2, int n3, int n4, int n5) { return new IArray5(n1,n2,n3,n4,n5); } ///< Create 5D int array

SwigBind inline PtrA<ZArray1> za1Create(int n1				    ) { return new ZArray1(n1		 ); } ///< Create 1D double_complex array
SwigBind inline PtrA<ZArray2> za2Create(int n1, int n2			    ) { return new ZArray2(n1,n2	 ); } ///< Create 2D double_complex array
SwigBind inline PtrA<ZArray3> za3Create(int n1, int n2, int n3		    ) { return new ZArray3(n1,n2,n3	 ); } ///< Create 3D double_complex array
SwigBind inline PtrA<ZArray4> za4Create(int n1, int n2, int n3, int n4	    ) { return new ZArray4(n1,n2,n3,n4	 ); } ///< Create 4D double_complex array
SwigBind inline PtrA<ZArray5> za5Create(int n1, int n2, int n3, int n4, int n5) { return new ZArray5(n1,n2,n3,n4,n5); } ///< Create 5D double_complex array

///////////////
// 10.2 Create identical copies of the array structure. 


SwigBind inline PtrA<DArray1> da1Clone(const PtrA<DArray1> & a) { return aClone(a); }   ///< Clone 1D double array (without copying the values but with a new value field)         
SwigBind inline PtrA<DArray2> da2Clone(const PtrA<DArray2> & a) { return aClone(a); }   ///< Clone 2D double array (without copying the values but with a new value field)         
SwigBind inline PtrA<DArray3> da3Clone(const PtrA<DArray3> & a) { return aClone(a); }   ///< Clone 3D double array (without copying the values but with a new value field)         
SwigBind inline PtrA<DArray4> da4Clone(const PtrA<DArray4> & a) { return aClone(a); }   ///< Clone 4D double array (without copying the values but with a new value field)         
SwigBind inline PtrA<DArray5> da5Clone(const PtrA<DArray5> & a) { return aClone(a); }   ///< Clone 5D double array (without copying the values but with a new value field)         
									                                          
SwigBind inline PtrA<IArray1> ia1Clone(const PtrA<IArray1> & a) { return aClone(a); }   ///< Clone 1D int array  (without copying the values but with a new value field)		 
SwigBind inline PtrA<IArray2> ia2Clone(const PtrA<IArray2> & a) { return aClone(a); }   ///< Clone 2D int array  (without copying the values but with a new value field)		 
SwigBind inline PtrA<IArray3> ia3Clone(const PtrA<IArray3> & a) { return aClone(a); }   ///< Clone 3D int array  (without copying the values but with a new value field)		 
SwigBind inline PtrA<IArray4> ia4Clone(const PtrA<IArray4> & a) { return aClone(a); }   ///< Clone 4D int array  (without copying the values but with a new value field)		 
SwigBind inline PtrA<IArray5> ia5Clone(const PtrA<IArray5> & a) { return aClone(a); }   ///< Clone 5D int array  (without copying the values but with a new value field)            
										                                          
SwigBind inline PtrA<ZArray1> za1Clone(const PtrA<ZArray1> & a) { return aClone(a); }   ///< Clone 1D double_complex array   (without copying the values but with a new value field)
SwigBind inline PtrA<ZArray2> za2Clone(const PtrA<ZArray2> & a) { return aClone(a); }   ///< Clone 2D double_complex array	  (without copying the values but with a new value field)
SwigBind inline PtrA<ZArray3> za3Clone(const PtrA<ZArray3> & a) { return aClone(a); }   ///< Clone 3D double_complex array	  (without copying the values but with a new value field)
SwigBind inline PtrA<ZArray4> za4Clone(const PtrA<ZArray4> & a) { return aClone(a); }   ///< Clone 4D double_complex array	  (without copying the values but with a new value field)
SwigBind inline PtrA<ZArray5> za5Clone(const PtrA<ZArray5> & a) { return aClone(a); }   ///< Clone 5D double_complex array	  (without copying the values but with a new value field)


///////////////
// 10.3  Constructors of array "views", 
//   without allocating new data space 



SwigBind inline PtrA<DArray2> da2Format(const PtrA<DArray1> & a, int n1, int n2                        ) { return aView<DArray2>(a,MemAccess2(a->acs,n1,n2         )); } ///< Create 2D double array with the same value buffer as \p a \return Returns NULL if n1*n2*...*nN != aNAll(a)
SwigBind inline PtrA<DArray3> da3Format(const PtrA<DArray1> & a, int n1, int n2, int n3		    ) { return aView<DArray3>(a,MemAccess2(a->acs,n1,n2,n3	)); } ///< Create 3D double array with the same value buffer as \p a \return Returns NULL if n1*n2*...*nN != aNAll(a)
SwigBind inline PtrA<DArray4> da4Format(const PtrA<DArray1> & a, int n1, int n2, int n3, int n4	    ) { return aView<DArray4>(a,MemAccess2(a->acs,n1,n2,n3,n4	)); } ///< Create 4D double array with the same value buffer as \p a \return Returns NULL if n1*n2*...*nN != aNAll(a)
SwigBind inline PtrA<DArray5> da5Format(const PtrA<DArray1> & a, int n1, int n2, int n3, int n4, int n5) { return aView<DArray5>(a,MemAccess2(a->acs,n1,n2,n3,n4,n5)); } ///< Create 5D double array with the same value buffer as \p a \return Returns NULL if n1*n2*...*nN != aNAll(a)

SwigBind inline PtrA<IArray2> ia2Format(const PtrA<IArray1> & a, int n1, int n2                        ) { return aView<IArray2>(a,MemAccess2(a->acs,n1,n2         )); } ///<  Create 2D int array with the same value buffer as \p a \return Returns NULL if n1*n2*...*nN != aNAll(a)
SwigBind inline PtrA<IArray3> ia3Format(const PtrA<IArray1> & a, int n1, int n2, int n3		    ) { return aView<IArray3>(a,MemAccess2(a->acs,n1,n2,n3	)); } ///<  Create 3D int array with the same value buffer as \p a \return Returns NULL if n1*n2*...*nN != aNAll(a)
SwigBind inline PtrA<IArray4> ia4Format(const PtrA<IArray1> & a, int n1, int n2, int n3, int n4	    ) { return aView<IArray4>(a,MemAccess2(a->acs,n1,n2,n3,n4	)); } ///<  Create 4D int array with the same value buffer as \p a \return Returns NULL if n1*n2*...*nN != aNAll(a)
SwigBind inline PtrA<IArray5> ia5Format(const PtrA<IArray1> & a, int n1, int n2, int n3, int n4, int n5) { return aView<IArray5>(a,MemAccess2(a->acs,n1,n2,n3,n4,n5)); } ///<  Create 5D int array with the same value buffer as \p a \return Returns NULL if n1*n2*...*nN != aNAll(a)

SwigBind inline PtrA<ZArray2> za2Format(const PtrA<ZArray1> & a, int n1, int n2                        ) { return aView<ZArray2>(a,MemAccess2(a->acs,n1,n2         )); } ///<  Create 2D double_complex array with the same value buffer as \p a \return Returns NULL if n1*n2*...*nN != aNAll(a)
SwigBind inline PtrA<ZArray3> za3Format(const PtrA<ZArray1> & a, int n1, int n2, int n3		    ) { return aView<ZArray3>(a,MemAccess2(a->acs,n1,n2,n3	)); } ///<  Create 3D double_complex array with the same value buffer as \p a \return Returns NULL if n1*n2*...*nN != aNAll(a)
SwigBind inline PtrA<ZArray4> za4Format(const PtrA<ZArray1> & a, int n1, int n2, int n3, int n4	    ) { return aView<ZArray4>(a,MemAccess2(a->acs,n1,n2,n3,n4	)); } ///<  Create 4D double_complex array with the same value buffer as \p a \return Returns NULL if n1*n2*...*nN != aNAll(a)
SwigBind inline PtrA<ZArray5> za5Format(const PtrA<ZArray1> & a, int n1, int n2, int n3, int n4, int n5) { return aView<ZArray5>(a,MemAccess2(a->acs,n1,n2,n3,n4,n5)); } ///<  Create 5D double_complex array with the same value buffer as \p a \return Returns NULL if n1*n2*...*nN != aNAll(a)


///////////////
//  10.4 Create a one-dimensional array with the _same_ value
//   field as \p a. 
//   One could see it as different view of a.


SwigBind inline PtrA<DArray1> da2All(const PtrA<DArray2> & a) { return aView<DArray1>(a,MemAccess2(a->acs,a->acs.vdim)); }///<  Create 2D double array with the same value buffer as \p a
SwigBind inline PtrA<DArray1> da3All(const PtrA<DArray3> & a) { return aView<DArray1>(a,MemAccess2(a->acs,a->acs.vdim)); }///<	Create 3D double array with the same value buffer as \p a
SwigBind inline PtrA<DArray1> da4All(const PtrA<DArray4> & a) { return aView<DArray1>(a,MemAccess2(a->acs,a->acs.vdim)); }///<	Create 4D double array with the same value buffer as \p a
SwigBind inline PtrA<DArray1> da5All(const PtrA<DArray5> & a) { return aView<DArray1>(a,MemAccess2(a->acs,a->acs.vdim)); }///<	Create 5D double array with the same value buffer as \p a
																							      
SwigBind inline PtrA<IArray1> ia2All(const PtrA<IArray2> & a) { return aView<IArray1>(a,MemAccess2(a->acs,a->acs.vdim)); }///<	Create 2D int array with the same value buffer as \p a 
SwigBind inline PtrA<IArray1> ia3All(const PtrA<IArray3> & a) { return aView<IArray1>(a,MemAccess2(a->acs,a->acs.vdim)); }///<	Create 3D int array with the same value buffer as \p a 
SwigBind inline PtrA<IArray1> ia4All(const PtrA<IArray4> & a) { return aView<IArray1>(a,MemAccess2(a->acs,a->acs.vdim)); }///<	Create 4D int array with the same value buffer as \p a 
SwigBind inline PtrA<IArray1> ia5All(const PtrA<IArray5> & a) { return aView<IArray1>(a,MemAccess2(a->acs,a->acs.vdim)); }///<	Create 5D int array with the same value buffer as \p a 
																							      
SwigBind inline PtrA<ZArray1> za2All(const PtrA<ZArray2> & a) { return aView<ZArray1>(a,MemAccess2(a->acs,a->acs.vdim)); }///<	Create 2D double_complex array with the same value buffer as \p a
SwigBind inline PtrA<ZArray1> za3All(const PtrA<ZArray3> & a) { return aView<ZArray1>(a,MemAccess2(a->acs,a->acs.vdim)); }///<	Create 3D double_complex array with the same value buffer as \p a
SwigBind inline PtrA<ZArray1> za4All(const PtrA<ZArray4> & a) { return aView<ZArray1>(a,MemAccess2(a->acs,a->acs.vdim)); }///<	Create 4D double_complex array with the same value buffer as \p a
SwigBind inline PtrA<ZArray1> za5All(const PtrA<ZArray5> & a) { return aView<ZArray1>(a,MemAccess2(a->acs,a->acs.vdim)); }///< ;Create 5D double_complex array with the same value buffer as a
																						      

template  <typename T>
inline PtrA<TArray1 <T> > ta2All(const PtrA<TArray2 <T> > & a) { return aView<TArray1 <T> >(a,MemAccess2(a->acs,a->acs.vdim)); };


///////////////
// 10.5 Create a row array pointing to the _same_ field of values as a.

///SwigBind inline Ptr<DArray4> da5Row(const Ptr<DArray5> & a, int irow) { return aRow<DArray4>(a.ptr(),irow); } ///< Extract array row (pointing to the same value buffer)
///SwigBind inline Ptr<DArray3> da4Row(const Ptr<DArray4> & a, int irow) { return aRow<DArray3>(a.ptr(),irow); } ///< Extract array row (pointing to the same value buffer)
///SwigBind inline Ptr<DArray2> da3Row(const Ptr<DArray3> & a, int irow) { return aRow<DArray2>(a.ptr(),irow); } ///< Extract array row (pointing to the same value buffer)
///SwigBind inline Ptr<DArray1> da2Row(const Ptr<DArray2> & a, int irow) { return aRow<DArray1>(a.ptr(),irow); } ///< Extract array row (pointing to the same value buffer)
///
///SwigBind inline Ptr<IArray4> ia5Row(const Ptr<IArray5> & a, int irow) { return aRow<IArray4>(a.ptr(),irow); } ///< Extract array row (pointing to the same value buffer)
///SwigBind inline Ptr<IArray3> ia4Row(const Ptr<IArray4> & a, int irow) { return aRow<IArray3>(a.ptr(),irow); } ///< Extract array row (pointing to the same value buffer)
///SwigBind inline Ptr<IArray2> ia3Row(const Ptr<IArray3> & a, int irow) { return aRow<IArray2>(a.ptr(),irow); } ///< Extract array row (pointing to the same value buffer)
///SwigBind inline Ptr<IArray1> ia2Row(const Ptr<IArray2> & a, int irow) { return aRow<IArray1>(a.ptr(),irow); } ///< Extract array row (pointing to the same value buffer)
///
///SwigBind inline Ptr<ZArray4> za5Row(const Ptr<ZArray5> & a, int irow) { return aRow<ZArray4>(a.ptr(),irow); } ///< Extract array row (pointing to the same value buffer)
///SwigBind inline Ptr<ZArray3> za4Row(const Ptr<ZArray4> & a, int irow) { return aRow<ZArray3>(a.ptr(),irow); } ///< Extract array row (pointing to the same value buffer)
///SwigBind inline Ptr<ZArray2> za3Row(const Ptr<ZArray3> & a, int irow) { return aRow<ZArray2>(a.ptr(),irow); } ///< Extract array row (pointing to the same value buffer)
///SwigBind inline Ptr<ZArray1> za2Row(const Ptr<ZArray2> & a, int irow) { return aRow<ZArray1>(a.ptr(),irow); } ///< Extract array row (pointing to the same value buffer)

///////////////
//  10.5.1 Create a row array as a subset copy of a.

SwigBind inline PtrA<DArray1> da2RowCopy(const PtrA<DArray2> & a, int irow) { return a->row_copy(irow); }  ///< Extract array row (copy)
                                                                                       	                                                              
SwigBind inline PtrA<IArray1> ia2RowCopy(const PtrA<IArray2> & a, int irow) { return a->row_copy(irow); }  ///< Extract array row (copy)
													                                                              
SwigBind inline PtrA<ZArray1> za2RowCopy(const PtrA<ZArray2> & a, int irow) { return a->row_copy(irow); }  ///< Extract array row (copy)



///////////////
//  10.6 Create a column array as a subset copy of a.

SwigBind inline PtrA<DArray1> da2ColCopy(const PtrA<DArray2> & a, int icol) { return a->col_copy(icol); }  ///< Extract array column (copy)
                                                                                       	                                                              
SwigBind inline PtrA<IArray1> ia2ColCopy(const PtrA<IArray2> & a, int icol) { return a->col_copy(icol); }  ///< Extract array column (copy)
													                                                              
SwigBind inline PtrA<ZArray1> za2ColCopy(const PtrA<ZArray2> & a, int icol) { return a->col_copy(icol); }  ///< Extract array column (copy)


///////////////
//  10.7 Ptr<Array> <-> Vector cast

SwigBind inline PtrA<DArray1> da1Cast(const Ptr<Vector> & a) { return aCast<DArray1>(a,1); } ///< Cast vector to array
SwigBind inline PtrA<DArray2> da2Cast(const Ptr<Vector> & a) { return aCast<DArray2>(a,2); } ///< Cast vector to array
SwigBind inline PtrA<DArray3> da3Cast(const Ptr<Vector> & a) { return aCast<DArray3>(a,3); } ///< Cast vector to array
inline PtrA<DArray4> da4Cast(const Ptr<Vector> & a) { return aCast<DArray4>(a,4); } ///< Cast vector to array
inline PtrA<DArray5> da5Cast(const Ptr<Vector> & a) { return aCast<DArray5>(a,5); } ///< Cast vector to array

SwigBind inline PtrA<IArray1> ia1Cast(const Ptr<Vector> & a) { return aCast<IArray1>(a,1); } ///< Cast vector to array 
SwigBind inline PtrA<IArray2> ia2Cast(const Ptr<Vector> & a) { return aCast<IArray2>(a,2); } ///< Cast vector to array 
SwigBind inline PtrA<IArray3> ia3Cast(const Ptr<Vector> & a) { return aCast<IArray3>(a,3); } ///< Cast vector to array 
inline PtrA<IArray4> ia4Cast(const Ptr<Vector> & a) { return aCast<IArray4>(a,4); } ///< Cast vector to array 
inline PtrA<IArray5> ia5Cast(const Ptr<Vector> & a) { return aCast<IArray5>(a,5); } ///< Cast vector to array 

inline PtrA<ZArray1> za1Cast(const Ptr<Vector> & a) { return aCast<ZArray1>(a,1); } ///< Cast vector to array 
inline PtrA<ZArray2> za2Cast(const Ptr<Vector> & a) { return aCast<ZArray2>(a,2); } ///< Cast vector to array 
inline PtrA<ZArray3> za3Cast(const Ptr<Vector> & a) { return aCast<ZArray3>(a,3); } ///< Cast vector to array 
inline PtrA<ZArray4> za4Cast(const Ptr<Vector> & a) { return aCast<ZArray4>(a,4); } ///< Cast vector to array 
inline PtrA<ZArray5> za5Cast(const Ptr<Vector> & a) { return aCast<ZArray5>(a,5); } ///< Cast vector to array 


template <typename T> inline PtrA<TArray1 <T> > ta1Cast(const Ptr<Vector> & a) { return aCast<TArray1 <T> >(a,1); }  ///< Cast vector to array 
template <typename T> inline PtrA<TArray2 <T> > ta2Cast(const Ptr<Vector> & a) { return aCast<TArray2 <T> >(a,2); }  ///< Cast vector to array 
template <typename T> inline PtrA<TArray3 <T> > ta3Cast(const Ptr<Vector> & a) { return aCast<TArray3 <T> >(a,3); }  ///< Cast vector to array 
template <typename T> inline PtrA<TArray4 <T> > ta4Cast(const Ptr<Vector> & a) { return aCast<TArray4 <T> >(a,4); }  ///< Cast vector to array 
template <typename T> inline PtrA<TArray5 <T> > ta5Cast(const Ptr<Vector> & a) { return aCast<TArray5 <T> >(a,5); }  ///< Cast vector to array 


inline int da1Valid(const Ptr<Vector> & a) { return aValid<DArray1>(a,1); } ///< Check if vector can be casted to array of type
inline int da2Valid(const Ptr<Vector> & a) { return aValid<DArray2>(a,2); } ///< Check if vector can be casted to array of type
inline int da3Valid(const Ptr<Vector> & a) { return aValid<DArray3>(a,3); } ///< Check if vector can be casted to array of type
inline int da4Valid(const Ptr<Vector> & a) { return aValid<DArray4>(a,4); } ///< Check if vector can be casted to array of type
inline int da5Valid(const Ptr<Vector> & a) { return aValid<DArray5>(a,5); } ///< Check if vector can be casted to array of type

inline int ia1Valid(const Ptr<Vector> & a) { return aValid<IArray1>(a,1); } ///< Check if vector can be casted to array of type
inline int ia2Valid(const Ptr<Vector> & a) { return aValid<IArray2>(a,2); } ///< Check if vector can be casted to array of type
inline int ia3Valid(const Ptr<Vector> & a) { return aValid<IArray3>(a,3); } ///< Check if vector can be casted to array of type
inline int ia4Valid(const Ptr<Vector> & a) { return aValid<IArray4>(a,4); } ///< Check if vector can be casted to array of type
inline int ia5Valid(const Ptr<Vector> & a) { return aValid<IArray5>(a,5); } ///< Check if vector can be casted to array of type

inline int za1Valid(const Ptr<Vector> & a) { return aValid<ZArray1>(a,1); } ///< Check if vector can be casted to array of type
inline int za2Valid(const Ptr<Vector> & a) { return aValid<ZArray2>(a,2); } ///< Check if vector can be casted to array of type
inline int za3Valid(const Ptr<Vector> & a) { return aValid<ZArray3>(a,3); } ///< Check if vector can be casted to array of type
inline int za4Valid(const Ptr<Vector> & a) { return aValid<ZArray4>(a,4); } ///< Check if vector can be casted to array of type
inline int za5Valid(const Ptr<Vector> & a) { return aValid<ZArray5>(a,5); } ///< Check if vector can be casted to array of type

int  _t_a_m_Valid(Ptr<Vector> a);


///////////////
// 10.8 Integrity check

int da1Check0(const PtrA<DArray1> & a, const char *file, int line);
int da2Check0(const PtrA<DArray2> & a, const char *file, int line);
int da3Check0(const PtrA<DArray3> & a, const char *file, int line);
int da4Check0(const PtrA<DArray4> & a, const char *file, int line);
int da5Check0(const PtrA<DArray5> & a, const char *file, int line);

int ia1Check0(const PtrA<IArray1> & a, const char *file, int line);
int ia2Check0(const PtrA<IArray2> & a, const char *file, int line);
int ia3Check0(const PtrA<IArray3> & a, const char *file, int line);
int ia4Check0(const PtrA<IArray4> & a, const char *file, int line);
int ia5Check0(const PtrA<IArray5> & a, const char *file, int line);

int za1Check0(const PtrA<ZArray1> & a, const char *file, int line);
int za2Check0(const PtrA<ZArray2> & a, const char *file, int line);
int za3Check0(const PtrA<ZArray3> & a, const char *file, int line);
int za4Check0(const PtrA<ZArray4> & a, const char *file, int line);
int za5Check0(const PtrA<ZArray5> & a, const char *file, int line);

SwigBind inline int da1Check(const PtrA<DArray1> & a) { return da1Check0(a,__FILE__,__LINE__); }   ///< Fence post check for array bounds
SwigBind inline int da2Check(const PtrA<DArray2> & a) { return da2Check0(a,__FILE__,__LINE__); }	 ///< Fence post check for array bounds
SwigBind inline int da3Check(const PtrA<DArray3> & a) { return da3Check0(a,__FILE__,__LINE__); }	 ///< Fence post check for array bounds
SwigBind inline int da4Check(const PtrA<DArray4> & a) { return da4Check0(a,__FILE__,__LINE__); }	 ///< Fence post check for array bounds
SwigBind inline int da5Check(const PtrA<DArray5> & a) { return da5Check0(a,__FILE__,__LINE__); }	 ///< Fence post check for array bounds
												                                       
SwigBind inline int ia1Check(const PtrA<IArray1> & a) { return ia1Check0(a,__FILE__,__LINE__); }	 ///< Fence post check for array bounds
SwigBind inline int ia2Check(const PtrA<IArray2> & a) { return ia2Check0(a,__FILE__,__LINE__); }	 ///< Fence post check for array bounds
SwigBind inline int ia3Check(const PtrA<IArray3> & a) { return ia3Check0(a,__FILE__,__LINE__); }	 ///< Fence post check for array bounds
SwigBind inline int ia4Check(const PtrA<IArray4> & a) { return ia4Check0(a,__FILE__,__LINE__); }	 ///< Fence post check for array bounds
SwigBind inline int ia5Check(const PtrA<IArray5> & a) { return ia5Check0(a,__FILE__,__LINE__); }	 ///< Fence post check for array bounds
												                                       
SwigBind inline int za1Check(const PtrA<ZArray1> & a) { return za1Check0(a,__FILE__,__LINE__); }	 ///< Fence post check for array bounds
SwigBind inline int za2Check(const PtrA<ZArray2> & a) { return za2Check0(a,__FILE__,__LINE__); }	 ///< Fence post check for array bounds
SwigBind inline int za3Check(const PtrA<ZArray3> & a) { return za3Check0(a,__FILE__,__LINE__); }	 ///< Fence post check for array bounds
SwigBind inline int za4Check(const PtrA<ZArray4> & a) { return za4Check0(a,__FILE__,__LINE__); }	 ///< Fence post check for array bounds
SwigBind inline int za5Check(const PtrA<ZArray5> & a) { return za5Check0(a,__FILE__,__LINE__); }	 ///< Fence post check for array bounds


///////////////
//  10.9 Print Array entries from the selected block (subarray) to standard output.
//  
//    E.g., 
//    _t_a2Print(a, 2,7, 31,31)	 will print 
//    the values from row 2 until row 7 of column 31;
//    _t_a2Print(a, 2,7, 5,32)	will print
//    the values from row 2 until row 7 of column  5 to  32,
//    etc.

SwigBind  void da1PrintBlock(const PtrA<DArray1> & v, int numcol, int l1,int n1, const char *name );
SwigBind  void da2PrintBlock(const PtrA<DArray2> & v, int numcol, int l1,int n1, int l2,int n2, const char *name );

SwigBind  void ia1PrintBlock(const PtrA<IArray1> & v, int numcol, int l1,int n1, const char *name );
SwigBind  void ia2PrintBlock(const PtrA<IArray2> & v, int numcol, int l1,int n1, int l2,int n2, const char *name );

SwigBind inline void da1PrintRows(const PtrA<DArray1> & a,int low,int high) { a->print_rows(low,high,""); }
SwigBind inline void da2PrintRows(const PtrA<DArray2> & a,int low,int high) { a->print_rows(low,high,""); }
												       
SwigBind inline void za1PrintRows(const PtrA<ZArray1> & a,int low,int high) { a->print_rows(low,high,""); }
SwigBind inline void za2PrintRows(const PtrA<ZArray2> & a,int low,int high) { a->print_rows(low,high,""); }
												       
SwigBind inline void ia1PrintRows(const PtrA<IArray1> & a,int low,int high) { a->print_rows(low,high,""); }
SwigBind inline void ia2PrintRows(const PtrA<IArray2> & a,int low,int high) { a->print_rows(low,high,""); }

SwigBind inline void da1PrintRows(const PtrA<DArray1> & a,int low,int high,const char *name) { a->print_rows(low,high,name); }
SwigBind inline void da2PrintRows(const PtrA<DArray2> & a,int low,int high,const char *name) { a->print_rows(low,high,name); }
																
SwigBind inline void za1PrintRows(const PtrA<ZArray1> & a,int low,int high,const char *name) { a->print_rows(low,high,name); }
SwigBind inline void za2PrintRows(const PtrA<ZArray2> & a,int low,int high,const char *name) { a->print_rows(low,high,name); }
																
SwigBind inline void ia1PrintRows(const PtrA<IArray1> & a,int low,int high,const char *name) { a->print_rows(low,high,name); }
SwigBind inline void ia2PrintRows(const PtrA<IArray2> & a,int low,int high,const char *name) { a->print_rows(low,high,name); }


///////////////
// 10.10  Print the whole contents of array a to the standard output.

SwigBind inline void da1Print(const PtrA<DArray1> & a) { da1PrintRows(a,1,a->acs.n1); } ///< Print the contents of array a to the standard output
SwigBind inline void da2Print(const PtrA<DArray2> & a) { da2PrintRows(a,1,a->acs.n1); } ///< Print the contents of array a to the standard output

SwigBind inline void za1Print(const PtrA<ZArray1> & a) { za1PrintRows(a,1,a->acs.n1); } ///< Print the contents of array a to the standard output
SwigBind inline void za2Print(const PtrA<ZArray2> & a) { za2PrintRows(a,1,a->acs.n1); } ///< Print the contents of array a to the standard output

SwigBind inline void ia1Print(const PtrA<IArray1> & a) { ia1PrintRows(a,1,a->acs.n1); } ///< Print the contents of array a to the standard output
SwigBind inline void ia2Print(const PtrA<IArray2> & a) { ia2PrintRows(a,1,a->acs.n1); } ///< Print the contents of array a to the standard output

SwigBind inline void da1Print(const PtrA<DArray1> & a,const char *name) { da1PrintRows(a,1,a->acs.n1,name); } ///< Print the contents of array a to the standard output
SwigBind inline void da2Print(const PtrA<DArray2> & a,const char *name) { da2PrintRows(a,1,a->acs.n1,name); } ///< Print the contents of array a to the standard output
													  
SwigBind inline void za1Print(const PtrA<ZArray1> & a,const char *name) { za1PrintRows(a,1,a->acs.n1,name); } ///< Print the contents of array a to the standard output
SwigBind inline void za2Print(const PtrA<ZArray2> & a,const char *name) { za2PrintRows(a,1,a->acs.n1,name); } ///< Print the contents of array a to the standard output
													  
SwigBind inline void ia1Print(const PtrA<IArray1> & a,const char *name) { ia1PrintRows(a,1,a->acs.n1,name); } ///< Print the contents of array a to the standard output
SwigBind inline void ia2Print(const PtrA<IArray2> & a,const char *name) { ia2PrintRows(a,1,a->acs.n1,name); } ///< Print the contents of array a to the standard output


SwigBind inline void da1PrintAll(const PtrA<DArray1> & a) { da1PrintRows(a,1,a->acs.n1); } ///< Print the contents of array a to the standard output
SwigBind inline void da2PrintAll(const PtrA<DArray2> & a) { da2PrintRows(a,1,a->acs.n1); } ///< Print the contents of array a to the standard output

SwigBind inline void za1PrintAll(const PtrA<ZArray1> & a) { za1PrintRows(a,1,a->acs.n1); } ///< Print the contents of array a to the standard output
SwigBind inline void za2PrintAll(const PtrA<ZArray2> & a) { za2PrintRows(a,1,a->acs.n1); } ///< Print the contents of array a to the standard output

SwigBind inline void ia1PrintAll(const PtrA<IArray1> & a) { ia1PrintRows(a,1,a->acs.n1); } ///< Print the contents of array a to the standard output
SwigBind inline void ia2PrintAll(const PtrA<IArray2> & a) { ia2PrintRows(a,1,a->acs.n1); } ///< Print the contents of array a to the standard output

SwigBind inline void da1PrintAll(const PtrA<DArray1> & a,const char *name) { da1PrintRows(a,1,a->acs.n1,name); } ///< Print the contents of array a to the standard output
SwigBind inline void da2PrintAll(const PtrA<DArray2> & a,const char *name) { da2PrintRows(a,1,a->acs.n1,name); } ///< Print the contents of array a to the standard output
													  
SwigBind inline void za1PrintAll(const PtrA<ZArray1> & a,const char *name) { za1PrintRows(a,1,a->acs.n1,name); } ///< Print the contents of array a to the standard output
SwigBind inline void za2PrintAll(const PtrA<ZArray2> & a,const char *name) { za2PrintRows(a,1,a->acs.n1,name); } ///< Print the contents of array a to the standard output
													  
SwigBind inline void ia1PrintAll(const PtrA<IArray1> & a,const char *name) { ia1PrintRows(a,1,a->acs.n1,name); } ///< Print the contents of array a to the standard output
SwigBind inline void ia2PrintAll(const PtrA<IArray2> & a,const char *name) { ia2PrintRows(a,1,a->acs.n1,name); } ///< Print the contents of array a to the standard output


/**
 \brief Replaces the row index in all array 'Print' functions with the tag "PDELIB-TEST-RAW:".
*/
SwigBind void aSet_print_test_raw(bool b);
SwigBind void aSet_print_test_val(bool b);

/**
 \brief Fence post check in Debug configuration
 
 Default is on. For non debug configuration this function has no effect.
*/
SwigBind void aSet_memacs_stop_on_error(bool on);


/////////////////////
// 10.11 Size information retrieval 

SwigBind inline int da1NAll(const PtrA<DArray1> & a) { return a->nall(); } ///< Number of elements in array
SwigBind inline int da2NAll(const PtrA<DArray2> & a) { return a->nall(); } ///< Number of elements in array
SwigBind inline int da3NAll(const PtrA<DArray3> & a) { return a->nall(); } ///< Number of elements in array
SwigBind inline int da4NAll(const PtrA<DArray4> & a) { return a->nall(); } ///< Number of elements in array
SwigBind inline int da5NAll(const PtrA<DArray5> & a) { return a->nall(); } ///< Number of elements in array

SwigBind inline int ia1NAll(const PtrA<IArray1> & a) { return a->nall(); } ///< Number of elements in array
SwigBind inline int ia2NAll(const PtrA<IArray2> & a) { return a->nall(); } ///< Number of elements in array
SwigBind inline int ia3NAll(const PtrA<IArray3> & a) { return a->nall(); } ///< Number of elements in array
SwigBind inline int ia4NAll(const PtrA<IArray4> & a) { return a->nall(); } ///< Number of elements in array
SwigBind inline int ia5NAll(const PtrA<IArray5> & a) { return a->nall(); } ///< Number of elements in array

SwigBind inline int za1NAll(const PtrA<ZArray1> & a) { return a->nall(); } ///< Number of elements in array
SwigBind inline int za2NAll(const PtrA<ZArray2> & a) { return a->nall(); } ///< Number of elements in array
SwigBind inline int za3NAll(const PtrA<ZArray3> & a) { return a->nall(); } ///< Number of elements in array
SwigBind inline int za4NAll(const PtrA<ZArray4> & a) { return a->nall(); } ///< Number of elements in array
SwigBind inline int za5NAll(const PtrA<ZArray5> & a) { return a->nall(); } ///< Number of elements in array


/////////////////////
// 10.12 Stride

SwigBind  inline int da1Stride(const PtrA<DArray1> & a) { return 1; /* a->acs.stride */; } ///< Stride of array
SwigBind  inline int da2Stride(const PtrA<DArray2> & a) { return 1; /* a->acs.stride */; } ///< Stride of array
SwigBind  inline int da3Stride(const PtrA<DArray3> & a) { return 1; /* a->acs.stride */; } ///< Stride of array
SwigBind  inline int da4Stride(const PtrA<DArray4> & a) { return 1; /* a->acs.stride */; } ///< Stride of array
SwigBind  inline int da5Stride(const PtrA<DArray5> & a) { return 1; /* a->acs.stride */; } ///< Stride of array

SwigBind  inline int ia1Stride(const PtrA<IArray1> & a) { return 1; /* a->acs.stride */; } ///< Stride of array
SwigBind  inline int ia2Stride(const PtrA<IArray2> & a) { return 1; /* a->acs.stride */; } ///< Stride of array
SwigBind  inline int ia3Stride(const PtrA<IArray3> & a) { return 1; /* a->acs.stride */; } ///< Stride of array
SwigBind  inline int ia4Stride(const PtrA<IArray4> & a) { return 1; /* a->acs.stride */; } ///< Stride of array
SwigBind  inline int ia5Stride(const PtrA<IArray5> & a) { return 1; /* a->acs.stride */; } ///< Stride of array

SwigBind  inline int za1Stride(const PtrA<ZArray1> & a) { return 1; /* a->acs.stride */; } ///< Stride of array
SwigBind  inline int za2Stride(const PtrA<ZArray2> & a) { return 1; /* a->acs.stride */; } ///< Stride of array
SwigBind  inline int za3Stride(const PtrA<ZArray3> & a) { return 1; /* a->acs.stride */; } ///< Stride of array
SwigBind  inline int za4Stride(const PtrA<ZArray4> & a) { return 1; /* a->acs.stride */; } ///< Stride of array
SwigBind  inline int za5Stride(const PtrA<ZArray5> & a) { return 1; /* a->acs.stride */; } ///< Stride of array


/////////////////////
// 10.13    Number of elements in the i-th dimension. 
/*    E.g., 
    da2N(a,1) is the number of rows,
    da2N(a,2) is the number of columns of a two-dimensional array a.       
*/

SwigBind inline int da1N(const PtrA<DArray1> & a, int i) { return a->n(i); } ///< Number of elements in \p i-th dimension
SwigBind inline int da2N(const PtrA<DArray2> & a, int i) { return a->n(i); } ///< Number of elements in \p i-th dimension
SwigBind inline int da3N(const PtrA<DArray3> & a, int i) { return a->n(i); } ///< Number of elements in \p i-th dimension
SwigBind inline int da4N(const PtrA<DArray4> & a, int i) { return a->n(i); } ///< Number of elements in \p i-th dimension
SwigBind inline int da5N(const PtrA<DArray5> & a, int i) { return a->n(i); } ///< Number of elements in \p i-th dimension

SwigBind inline int ia1N(const PtrA<IArray1> & a, int i) { return a->n(i); } ///< Number of elements in \p i-th dimension
SwigBind inline int ia2N(const PtrA<IArray2> & a, int i) { return a->n(i); } ///< Number of elements in \p i-th dimension
SwigBind inline int ia3N(const PtrA<IArray3> & a, int i) { return a->n(i); } ///< Number of elements in \p i-th dimension
SwigBind inline int ia4N(const PtrA<IArray4> & a, int i) { return a->n(i); } ///< Number of elements in \p i-th dimension
SwigBind inline int ia5N(const PtrA<IArray5> & a, int i) { return a->n(i); } ///< Number of elements in \p i-th dimension

SwigBind inline int za1N(const PtrA<ZArray1> & a, int i) { return a->n(i); } ///< Number of elements in \p i-th dimension
SwigBind inline int za2N(const PtrA<ZArray2> & a, int i) { return a->n(i); } ///< Number of elements in \p i-th dimension
SwigBind inline int za3N(const PtrA<ZArray3> & a, int i) { return a->n(i); } ///< Number of elements in \p i-th dimension
SwigBind inline int za4N(const PtrA<ZArray4> & a, int i) { return a->n(i); } ///< Number of elements in \p i-th dimension
SwigBind inline int za5N(const PtrA<ZArray5> & a, int i) { return a->n(i); } ///< Number of elements in \p i-th dimension



SwigBind inline int da1NCol(const PtrA<DArray1> & a) { return a->ncol(); } ///< Number of columns
SwigBind inline int da2NCol(const PtrA<DArray2> & a) { return a->ncol(); } ///< Number of columns
SwigBind inline int da3NCol(const PtrA<DArray3> & a) { return a->ncol(); } ///< Number of columns
SwigBind inline int da4NCol(const PtrA<DArray4> & a) { return a->ncol(); } ///< Number of columns
SwigBind inline int da5NCol(const PtrA<DArray5> & a) { return a->ncol(); } ///< Number of columns

SwigBind inline int ia1NCol(const PtrA<IArray1> & a) { return a->ncol(); } ///< Number of columns
SwigBind inline int ia2NCol(const PtrA<IArray2> & a) { return a->ncol(); } ///< Number of columns
SwigBind inline int ia3NCol(const PtrA<IArray3> & a) { return a->ncol(); } ///< Number of columns
SwigBind inline int ia4NCol(const PtrA<IArray4> & a) { return a->ncol(); } ///< Number of columns
SwigBind inline int ia5NCol(const PtrA<IArray5> & a) { return a->ncol(); } ///< Number of columns

SwigBind inline int za1NCol(const PtrA<ZArray1> & a) { return a->ncol(); } ///< Number of columns
SwigBind inline int za2NCol(const PtrA<ZArray2> & a) { return a->ncol(); } ///< Number of columns
SwigBind inline int za3NCol(const PtrA<ZArray3> & a) { return a->ncol(); } ///< Number of columns
SwigBind inline int za4NCol(const PtrA<ZArray4> & a) { return a->ncol(); } ///< Number of columns
SwigBind inline int za5NCol(const PtrA<ZArray5> & a) { return a->ncol(); } ///< Number of columns

SwigBind inline int da1NRow(const PtrA<DArray1> & a) { return a->nrow(); } ///< Number of rows
SwigBind inline int da2NRow(const PtrA<DArray2> & a) { return a->nrow(); } ///< Number of rows
SwigBind inline int da3NRow(const PtrA<DArray3> & a) { return a->nrow(); } ///< Number of rows
SwigBind inline int da4NRow(const PtrA<DArray4> & a) { return a->nrow(); } ///< Number of rows
SwigBind inline int da5NRow(const PtrA<DArray5> & a) { return a->nrow(); } ///< Number of rows
                   
SwigBind inline int ia1NRow(const PtrA<IArray1> & a) { return a->nrow(); } ///< Number of rows
SwigBind inline int ia2NRow(const PtrA<IArray2> & a) { return a->nrow(); } ///< Number of rows
SwigBind inline int ia3NRow(const PtrA<IArray3> & a) { return a->nrow(); } ///< Number of rows
SwigBind inline int ia4NRow(const PtrA<IArray4> & a) { return a->nrow(); } ///< Number of rows
SwigBind inline int ia5NRow(const PtrA<IArray5> & a) { return a->nrow(); } ///< Number of rows

SwigBind inline int za1NRow(const PtrA<ZArray1> & a) { return a->nrow(); } ///< Number of rows
SwigBind inline int za2NRow(const PtrA<ZArray2> & a) { return a->nrow(); } ///< Number of rows
SwigBind inline int za3NRow(const PtrA<ZArray3> & a) { return a->nrow(); } ///< Number of rows
SwigBind inline int za4NRow(const PtrA<ZArray4> & a) { return a->nrow(); } ///< Number of rows
SwigBind inline int za5NRow(const PtrA<ZArray5> & a) { return a->nrow(); } ///< Number of rows




/////////////////////
// 10.14 Legacy access functions
// All are now valid only for stride 1.           

inline double& DA1S(const PtrA<DArray1> & a, int i1                            ) { return (*a)(i1); }             ///< Element acess for array of stride >=1
inline double& DA2S(const PtrA<DArray2> & a, int i1,int i2                     ) { return (*a)(i1,i2); }          ///< Element acess for array of stride >=1
inline double& DA3S(const PtrA<DArray3> & a, int i1,int i2,int i3              ) { return (*a)(i1,i2,i3); }       ///< Element acess for array of stride >=1
inline double& DA4S(const PtrA<DArray4> & a, int i1,int i2,int i3,int i4       ) { return (*a)(i1,i2,i3,i4); }    ///< Element acess for array of stride >=1
inline double& DA5S(const PtrA<DArray5> & a, int i1,int i2,int i3,int i4,int i5) { return (*a)(i1,i2,i3,i4,i5); } ///< Element acess for array of stride >=1

inline int& IA1S(const PtrA<IArray1> & a, int i1                            ) { return (*a)(i1); }                ///< Element acess for array of stride >=1  
inline int& IA2S(const PtrA<IArray2> & a, int i1,int i2                     ) { return (*a)(i1,i2); }		 ///< Element acess for array of stride >=1
inline int& IA3S(const PtrA<IArray3> & a, int i1,int i2,int i3              ) { return (*a)(i1,i2,i3); }		 ///< Element acess for array of stride >=1
inline int& IA4S(const PtrA<IArray4> & a, int i1,int i2,int i3,int i4       ) { return (*a)(i1,i2,i3,i4); }	 ///< Element acess for array of stride >=1
inline int& IA5S(const PtrA<IArray5> & a, int i1,int i2,int i3,int i4,int i5) { return (*a)(i1,i2,i3,i4,i5); }    ///< Element acess for array of stride >=1

inline double_complex& ZA1S(const PtrA<ZArray1> & a, int i1                            ) { return (*a)(i1); }             ///< Element acess for array of stride >=1
inline double_complex& ZA2S(const PtrA<ZArray2> & a, int i1,int i2		      ) { return (*a)(i1,i2); }		 ///< Element acess for array of stride >=1
inline double_complex& ZA3S(const PtrA<ZArray3> & a, int i1,int i2,int i3	      ) { return (*a)(i1,i2,i3); }	 ///< Element acess for array of stride >=1
inline double_complex& ZA4S(const PtrA<ZArray4> & a, int i1,int i2,int i3,int i4	      ) { return (*a)(i1,i2,i3,i4); }	 ///< Element acess for array of stride >=1
inline double_complex& ZA5S(const PtrA<ZArray5> & a, int i1,int i2,int i3,int i4,int i5) { return (*a)(i1,i2,i3,i4,i5); } ///< Element acess for array of stride >=1


inline double& DA1(const PtrA<DArray1> & a, int i1                            ) { return (*a)(i1); }             ///< Default element acess for array of stride >=1
inline double& DA2(const PtrA<DArray2> & a, int i1,int i2                     ) { return (*a)(i1,i2); }          ///< Default element acess for array of stride >=1
inline double& DA3(const PtrA<DArray3> & a, int i1,int i2,int i3              ) { return (*a)(i1,i2,i3); }       ///< Default element acess for array of stride >=1
inline double& DA4(const PtrA<DArray4> & a, int i1,int i2,int i3,int i4       ) { return (*a)(i1,i2,i3,i4); }    ///< Default element acess for array of stride >=1
inline double& DA5(const PtrA<DArray5> & a, int i1,int i2,int i3,int i4,int i5) { return (*a)(i1,i2,i3,i4,i5); } ///< Default element acess for array of stride >=1

inline int& IA1(const PtrA<IArray1> & a, int i1                            ) { return (*a)(i1); }                ///< Default element acess for array of stride >=1  
inline int& IA2(const PtrA<IArray2> & a, int i1,int i2                     ) { return (*a)(i1,i2); }		 ///< Default element acess for array of stride >=1
inline int& IA3(const PtrA<IArray3> & a, int i1,int i2,int i3              ) { return (*a)(i1,i2,i3); }		 ///< Default element acess for array of stride >=1
inline int& IA4(const PtrA<IArray4> & a, int i1,int i2,int i3,int i4       ) { return (*a)(i1,i2,i3,i4); }	 ///< Default element acess for array of stride >=1
inline int& IA5(const PtrA<IArray5> & a, int i1,int i2,int i3,int i4,int i5) { return (*a)(i1,i2,i3,i4,i5); }    ///< Default element acess for array of stride >=1

inline double_complex& ZA1(const PtrA<ZArray1> & a, int i1                            ) { return (*a)(i1); }             ///< Default element acess for array of stride >=1
inline double_complex& ZA2(const PtrA<ZArray2> & a, int i1,int i2		      ) { return (*a)(i1,i2); }		 ///< Default element acess for array of stride >=1
inline double_complex& ZA3(const PtrA<ZArray3> & a, int i1,int i2,int i3	      ) { return (*a)(i1,i2,i3); }	 ///< Default element acess for array of stride >=1
inline double_complex& ZA4(const PtrA<ZArray4> & a, int i1,int i2,int i3,int i4	      ) { return (*a)(i1,i2,i3,i4); }	 ///< Default element acess for array of stride >=1
inline double_complex& ZA5(const PtrA<ZArray5> & a, int i1,int i2,int i3,int i4,int i5) { return (*a)(i1,i2,i3,i4,i5); } ///< Default element acess for array of stride >=1


inline double& DA1C(const PtrA<DArray1> & a, int i1                            ) { return (*a)(i1); }               ///< Element access for arrays with mandatory stride 1
inline double& DA2C(const PtrA<DArray2> & a, int i1,int i2                     ) { return (*a)(i1,i2); }            ///< Element access for arrays with mandatory stride 1
inline double& DA3C(const PtrA<DArray3> & a, int i1,int i2,int i3              ) { return (*a)(i1,i2,i3); }         ///< Element access for arrays with mandatory stride 1  
inline double& DA4C(const PtrA<DArray4> & a, int i1,int i2,int i3,int i4       ) { return (*a)(i1,i2,i3,i4); }      ///< Element access for arrays with mandatory stride 1
inline double& DA5C(const PtrA<DArray5> & a, int i1,int i2,int i3,int i4,int i5) { return (*a)(i1,i2,i3,i4,i5); }   ///< Element access for arrays with mandatory stride 1

inline int& IA1C(const PtrA<IArray1> & a, int i1                            ) { return (*a)(i1); }                   ///< Element access for arrays with mandatory stride 1 
inline int& IA2C(const PtrA<IArray2> & a, int i1,int i2                     ) { return (*a)(i1,i2); }		      ///< Element access for arrays with mandatory stride 1 
inline int& IA3C(const PtrA<IArray3> & a, int i1,int i2,int i3              ) { return (*a)(i1,i2,i3); }	      ///< Element access for arrays with mandatory stride 1 
inline int& IA4C(const PtrA<IArray4> & a, int i1,int i2,int i3,int i4       ) { return (*a)(i1,i2,i3,i4); }	      ///< Element access for arrays with mandatory stride 1 
inline int& IA5C(const PtrA<IArray5> & a, int i1,int i2,int i3,int i4,int i5) { return (*a)(i1,i2,i3,i4,i5); }	      ///< Element access for arrays with mandatory stride 1 

inline double_complex& ZA1C(const PtrA<ZArray1> & a, int i1                            ) { return (*a)(i1); }                ///< Element access for arrays with mandatory stride 1 
inline double_complex& ZA2C(const PtrA<ZArray2> & a, int i1,int i2                     ) { return (*a)(i1,i2); }	      ///< Element access for arrays with mandatory stride 1 
inline double_complex& ZA3C(const PtrA<ZArray3> & a, int i1,int i2,int i3              ) { return (*a)(i1,i2,i3); }	      ///< Element access for arrays with mandatory stride 1 
inline double_complex& ZA4C(const PtrA<ZArray4> & a, int i1,int i2,int i3,int i4	      ) { return (*a)(i1,i2,i3,i4); }	      ///< Element access for arrays with mandatory stride 1 
inline double_complex& ZA5C(const PtrA<ZArray5> & a, int i1,int i2,int i3,int i4,int i5) { return (*a)(i1,i2,i3,i4,i5); }    ///< Element access for arrays with mandatory stride 1 


//////////////////
// 10.15 Get/set API for component access

SwigBind inline double da1Get(const PtrA<DArray1> & a, int i1                            ) { return DA1(a,i1            ); }  ///< Get element value for array with arbitrary stride
SwigBind inline double da2Get(const PtrA<DArray2> & a, int i1,int i2                     ) { return DA2(a,i1,i2         ); }  ///< Get element value for array with arbitrary stride
SwigBind inline double da3Get(const PtrA<DArray3> & a, int i1,int i2,int i3              ) { return DA3(a,i1,i2,i3      ); }  ///< Get element value for array with arbitrary stride
SwigBind inline double da4Get(const PtrA<DArray4> & a, int i1,int i2,int i3,int i4       ) { return DA4(a,i1,i2,i3,i4   ); }  ///< Get element value for array with arbitrary stride
SwigBind inline double da5Get(const PtrA<DArray5> & a, int i1,int i2,int i3,int i4,int i5) { return DA5(a,i1,i2,i3,i4,i5); }  ///< Get element value for array with arbitrary stride

SwigBind inline int ia1Get(const PtrA<IArray1> & a, int i1                            ) { return IA1(a,i1            ); } ///< Get element value for array with arbitrary stride
SwigBind inline int ia2Get(const PtrA<IArray2> & a, int i1,int i2                     ) { return IA2(a,i1,i2         ); }	///< Get element value for array with arbitrary stride
SwigBind inline int ia3Get(const PtrA<IArray3> & a, int i1,int i2,int i3              ) { return IA3(a,i1,i2,i3      ); }	///< Get element value for array with arbitrary stride
SwigBind inline int ia4Get(const PtrA<IArray4> & a, int i1,int i2,int i3,int i4       ) { return IA4(a,i1,i2,i3,i4   ); }	///< Get element value for array with arbitrary stride
SwigBind inline int ia5Get(const PtrA<IArray5> & a, int i1,int i2,int i3,int i4,int i5) { return IA5(a,i1,i2,i3,i4,i5); }	///< Get element value for array with arbitrary stride

SwigBind inline double_complex& za1Get(const PtrA<ZArray1> & a, int i1                            ) { return ZA1(a,i1            ); } ///< Get element value for array with arbitrary stride
SwigBind inline double_complex& za2Get(const PtrA<ZArray2> & a, int i1,int i2                     ) { return ZA2(a,i1,i2	       ); } ///< Get element value for array with arbitrary stride
SwigBind inline double_complex& za3Get(const PtrA<ZArray3> & a, int i1,int i2,int i3              ) { return ZA3(a,i1,i2,i3      ); } ///< Get element value for array with arbitrary stride
SwigBind inline double_complex& za4Get(const PtrA<ZArray4> & a, int i1,int i2,int i3,int i4       ) { return ZA4(a,i1,i2,i3,i4   ); } ///< Get element value for array with arbitrary stride
SwigBind inline double_complex& za5Get(const PtrA<ZArray5> & a, int i1,int i2,int i3,int i4,int i5) { return ZA5(a,i1,i2,i3,i4,i5); } ///< Get element value for array with arbitrary stride



SwigBind inline void da1Set(const PtrA<DArray1> & a, int i1                            , double val) { DA1(a,i1            )=val; } ///< Set element value for array with arbitrary stride
SwigBind inline void da2Set(const PtrA<DArray2> & a, int i1,int i2                     , double val) { DA2(a,i1,i2         )=val; } ///< Set element value for array with arbitrary stride
SwigBind inline void da3Set(const PtrA<DArray3> & a, int i1,int i2,int i3              , double val) { DA3(a,i1,i2,i3      )=val; } ///< Set element value for array with arbitrary stride
SwigBind inline void da4Set(const PtrA<DArray4> & a, int i1,int i2,int i3,int i4	     , double val) { DA4(a,i1,i2,i3,i4	 )=val; } ///< Set element value for array with arbitrary stride
SwigBind inline void da5Set(const PtrA<DArray5> & a, int i1,int i2,int i3,int i4,int i5, double val) { DA5(a,i1,i2,i3,i4,i5)=val; } ///< Set element value for array with arbitrary stride

SwigBind inline void ia1Set(const PtrA<IArray1> & a, int i1                            , int val) { IA1(a,i1            )=val; }  ///< Set element value for array with arbitrary stride 
SwigBind inline void ia2Set(const PtrA<IArray2> & a, int i1,int i2                     , int val) { IA2(a,i1,i2         )=val; }	///< Set element value for array with arbitrary stride 
SwigBind inline void ia3Set(const PtrA<IArray3> & a, int i1,int i2,int i3              , int val) { IA3(a,i1,i2,i3      )=val; }	///< Set element value for array with arbitrary stride 
SwigBind inline void ia4Set(const PtrA<IArray4> & a, int i1,int i2,int i3,int i4	     , int val) { IA4(a,i1,i2,i3,i4   )=val; }	///< Set element value for array with arbitrary stride 
SwigBind inline void ia5Set(const PtrA<IArray5> & a, int i1,int i2,int i3,int i4,int i5, int val) { IA5(a,i1,i2,i3,i4,i5)=val; }	///< Set element value for array with arbitrary stride 

SwigBind inline void za1Set(const PtrA<ZArray1> & a, int i1                            , double_complex& val) { ZA1(a,i1            )=val; }  ///< Set element value for array with arbitrary stride 
SwigBind inline void za2Set(const PtrA<ZArray2> & a, int i1,int i2                     , double_complex& val) { ZA2(a,i1,i2         )=val; }  ///< Set element value for array with arbitrary stride 
SwigBind inline void za3Set(const PtrA<ZArray3> & a, int i1,int i2,int i3              , double_complex& val) { ZA3(a,i1,i2,i3      )=val; }  ///< Set element value for array with arbitrary stride 
SwigBind inline void za4Set(const PtrA<ZArray4> & a, int i1,int i2,int i3,int i4	     , double_complex& val) { ZA4(a,i1,i2,i3,i4	  )=val; }  ///< Set element value for array with arbitrary stride 
SwigBind inline void za5Set(const PtrA<ZArray5> & a, int i1,int i2,int i3,int i4,int i5, double_complex& val) { ZA5(a,i1,i2,i3,i4,i5)=val; }  ///< Set element value for array with arbitrary stride 


//////////////////
// 10.16 Size extension

/*
  Re-allocates (if necessary) the array in order to allow write access to
  all elements between (1,...) and (n1,...). 
  All array elements remain their indices. Returns false if 'a' based on
  "virtual" memory or 'a' is only a "view" on an array.
  E.g.: a2=da2Create(4,4)
        da2Extend(a2,3,5)
        now dim(a2)=(4,5) !
*/

SwigBind inline bool da1Extend(const PtrA<DArray1> & a, size_t n1                            ) { size_t nn[]={0,n1            }; return a->extend(nn); }              ///< Reallocate array buffer in order to extend array. \return Returns false if buffer is only view of another array.
SwigBind inline bool da2Extend(const PtrA<DArray2> & a, size_t n1,size_t n2                     ) { size_t nn[]={0,n1,n2         }; return a->extend(nn); }           ///< Reallocate array buffer in order to extend array. \return Returns false if buffer is only view of another array.     
SwigBind inline bool da3Extend(const PtrA<DArray3> & a, size_t n1,size_t n2,size_t n3              ) { size_t nn[]={0,n1,n2,n3      }; return a->extend(nn); }        ///< Reallocate array buffer in order to extend array. \return Returns false if buffer is only view of another array.
SwigBind inline bool da4Extend(const PtrA<DArray4> & a, size_t n1,size_t n2,size_t n3,size_t n4       ) { size_t nn[]={0,n1,n2,n3,n4   }; return a->extend(nn); }     ///< Reallocate array buffer in order to extend array. \return Returns false if buffer is only view of another array. 
SwigBind inline bool da5Extend(const PtrA<DArray5> & a, size_t n1,size_t n2,size_t n3,size_t n4,size_t n5) { size_t nn[]={0,n1,n2,n3,n4,n5}; return a->extend(nn); }  ///< Reallocate array buffer in order to extend array. \return Returns false if buffer is only view of another array. 

SwigBind inline bool ia1Extend(const PtrA<IArray1> & a, size_t n1                            ) { size_t nn[]={0,n1            }; return a->extend(nn); }               ///< Reallocate array buffer in order to extend array. \return Returns false if buffer is only view of another array.
SwigBind inline bool ia2Extend(const PtrA<IArray2> & a, size_t n1,size_t n2                     ) { size_t nn[]={0,n1,n2         }; return a->extend(nn); }	     ///< Reallocate array buffer in order to extend array. \return Returns false if buffer is only view of another array.
SwigBind inline bool ia3Extend(const PtrA<IArray3> & a, size_t n1,size_t n2,size_t n3              ) { size_t nn[]={0,n1,n2,n3      }; return a->extend(nn); }	     ///< Reallocate array buffer in order to extend array. \return Returns false if buffer is only view of another array.
SwigBind inline bool ia4Extend(const PtrA<IArray4> & a, size_t n1,size_t n2,size_t n3,size_t n4       ) { size_t nn[]={0,n1,n2,n3,n4   }; return a->extend(nn); }	     ///< Reallocate array buffer in order to extend array. \return Returns false if buffer is only view of another array.
SwigBind inline bool ia5Extend(const PtrA<IArray5> & a, size_t n1,size_t n2,size_t n3,size_t n4,size_t n5) { size_t nn[]={0,n1,n2,n3,n4,n5}; return a->extend(nn); }   ///< Reallocate array buffer in order to extend array. \return Returns false if buffer is only view of another array.

SwigBind inline bool za1Extend(const PtrA<ZArray1> & a, size_t n1                            ) { size_t nn[]={0,n1            }; return a->extend(nn); }                ///< Reallocate array buffer in order to extend array. \return Returns false if buffer is only view of another array.
SwigBind inline bool za2Extend(const PtrA<ZArray2> & a, size_t n1,size_t n2		      ) { size_t nn[]={0,n1,n2	       }; return a->extend(nn); }	      ///< Reallocate array buffer in order to extend array. \return Returns false if buffer is only view of another array.
SwigBind inline bool za3Extend(const PtrA<ZArray3> & a, size_t n1,size_t n2,size_t n3		 ) { size_t nn[]={0,n1,n2,n3	  }; return a->extend(nn); }	      ///< Reallocate array buffer in order to extend array. \return Returns false if buffer is only view of another array.
SwigBind inline bool za4Extend(const PtrA<ZArray4> & a, size_t n1,size_t n2,size_t n3,size_t n4	    ) { size_t nn[]={0,n1,n2,n3,n4   }; return a->extend(nn); }	      ///< Reallocate array buffer in order to extend array. \return Returns false if buffer is only view of another array.
SwigBind inline bool za5Extend(const PtrA<ZArray5> & a, size_t n1,size_t n2,size_t n3,size_t n4,size_t n5) { size_t nn[]={0,n1,n2,n3,n4,n5}; return a->extend(nn); }    ///< Reallocate array buffer in order to extend array. \return Returns false if buffer is only view of another array.

SwigBind inline void da1Insert(const PtrA<DArray1> & a, size_t n1                            ,double val) { if(da1Extend(a,n1            )) (*a)(n1            )=val; }              ///< Insert value into array. Extend it if necessary.
SwigBind inline void da2Insert(const PtrA<DArray2> & a, size_t n1,size_t n2                     ,double val) { if(da2Extend(a,n1,n2         )) (*a)(n1,n2         )=val; }           ///< Insert value into array. Extend it if necessary.           
SwigBind inline void da3Insert(const PtrA<DArray3> & a, size_t n1,size_t n2,size_t n3              ,double val) { if(da3Extend(a,n1,n2,n3      )) (*a)(n1,n2,n3      )=val; }        ///< Insert value into array. Extend it if necessary.              
SwigBind inline void da4Insert(const PtrA<DArray4> & a, size_t n1,size_t n2,size_t n3,size_t n4       ,double val) { if(da4Extend(a,n1,n2,n3,n4   )) (*a)(n1,n2,n3,n4   )=val; }     ///< Insert value into array. Extend it if necessary.                
SwigBind inline void da5Insert(const PtrA<DArray5> & a, size_t n1,size_t n2,size_t n3,size_t n4,size_t n5,double val) { if(da5Extend(a,n1,n2,n3,n4,n5)) (*a)(n1,n2,n3,n4,n5)=val; }  ///< Insert value into array. Extend it if necessary.                    

SwigBind inline void ia1Insert(const PtrA<IArray1> & a, size_t n1                            ,int val) { if(ia1Extend(a,n1            )) (*a)(n1            )=val; }             ///< Insert value into array. Extend it if necessary.               
SwigBind inline void ia2Insert(const PtrA<IArray2> & a, size_t n1,size_t n2                     ,int val) { if(ia2Extend(a,n1,n2         )) (*a)(n1,n2         )=val; }          ///< Insert value into array. Extend it if necessary.             
SwigBind inline void ia3Insert(const PtrA<IArray3> & a, size_t n1,size_t n2,size_t n3              ,int val) { if(ia3Extend(a,n1,n2,n3      )) (*a)(n1,n2,n3      )=val; }       ///< Insert value into array. Extend it if necessary.                
SwigBind inline void ia4Insert(const PtrA<IArray4> & a, size_t n1,size_t n2,size_t n3,size_t n4       ,int val) { if(ia4Extend(a,n1,n2,n3,n4   )) (*a)(n1,n2,n3,n4   )=val; }    ///< Insert value into array. Extend it if necessary.                   
SwigBind inline void ia5Insert(const PtrA<IArray5> & a, size_t n1,size_t n2,size_t n3,size_t n4,size_t n5,int val) { if(ia5Extend(a,n1,n2,n3,n4,n5)) (*a)(n1,n2,n3,n4,n5)=val; } ///< Insert value into array. Extend it if necessary.                      

SwigBind inline void za1Insert(const PtrA<ZArray1> & a, size_t n1                            ,double_complex& val) { if(za1Extend(a,n1            )) (*a)(n1            )=val; }             ///< Insert value into array. Extend it if necessary.                     
SwigBind inline void za2Insert(const PtrA<ZArray2> & a, size_t n1,size_t n2                     ,double_complex& val) { if(za2Extend(a,n1,n2         )) (*a)(n1,n2         )=val; }          ///< Insert value into array. Extend it if necessary.             
SwigBind inline void za3Insert(const PtrA<ZArray3> & a, size_t n1,size_t n2,size_t n3              ,double_complex& val) { if(za3Extend(a,n1,n2,n3      )) (*a)(n1,n2,n3      )=val; }       ///< Insert value into array. Extend it if necessary.                
SwigBind inline void za4Insert(const PtrA<ZArray4> & a, size_t n1,size_t n2,size_t n3,size_t n4       ,double_complex& val) { if(za4Extend(a,n1,n2,n3,n4   )) (*a)(n1,n2,n3,n4   )=val; }    ///< Insert value into array. Extend it if necessary.                   
SwigBind inline void za5Insert(const PtrA<ZArray5> & a, size_t n1,size_t n2,size_t n3,size_t n4,size_t n5,double_complex& val) { if(za5Extend(a,n1,n2,n3,n4,n5)) (*a)(n1,n2,n3,n4,n5)=val; } ///< Insert value into array. Extend it if necessary.                      

//////////////////
// 10.17 Size adjustion
/*
    Adjust allocated memory to actual size.
    
    The functions are using realloc(). So the behavior is as follows:
    The content of the memory block is preserved up to the lesser of the
    new and old sizes, even if the block is moved.
    If the new size is larger, the value of the newly allocated portion is indeterminate.
    
    The functions are returning true on success. If an array is a virtual one, then
    the adjust will fail.
*/

SwigBind inline bool da1Adjust(const PtrA<DArray1> & a, size_t n1                            ) { size_t nn[]={0,n1            }; return a->resize(nn); }              ///< Adjust allocated memory to given size      
SwigBind inline bool da2Adjust(const PtrA<DArray2> & a, size_t n1,size_t n2                     ) { size_t nn[]={0,n1,n2         }; return a->resize(nn); }           ///< Adjust allocated memory to given size            
SwigBind inline bool da3Adjust(const PtrA<DArray3> & a, size_t n1,size_t n2,size_t n3              ) { size_t nn[]={0,n1,n2,n3      }; return a->resize(nn); }        ///< Adjust allocated memory to given size               
SwigBind inline bool da4Adjust(const PtrA<DArray4> & a, size_t n1,size_t n2,size_t n3,size_t n4       ) { size_t nn[]={0,n1,n2,n3,n4   }; return a->resize(nn); }     ///< Adjust allocated memory to given size                  
SwigBind inline bool da5Adjust(const PtrA<DArray5> & a, size_t n1,size_t n2,size_t n3,size_t n4,size_t n5) { size_t nn[]={0,n1,n2,n3,n4,n5}; return a->resize(nn); }  ///< Adjust allocated memory to given size                     

SwigBind inline bool ia1Adjust(const PtrA<IArray1> & a, size_t n1                            ) { size_t nn[]={0,n1            }; return a->resize(nn); }              ///< Adjust allocated memory to given size        
SwigBind inline bool ia2Adjust(const PtrA<IArray2> & a, size_t n1,size_t n2                     ) { size_t nn[]={0,n1,n2         }; return a->resize(nn); }           ///< Adjust allocated memory to given size           
SwigBind inline bool ia3Adjust(const PtrA<IArray3> & a, size_t n1,size_t n2,size_t n3              ) { size_t nn[]={0,n1,n2,n3      }; return a->resize(nn); }        ///< Adjust allocated memory to given size              
SwigBind inline bool ia4Adjust(const PtrA<IArray4> & a, size_t n1,size_t n2,size_t n3,size_t n4       ) { size_t nn[]={0,n1,n2,n3,n4   }; return a->resize(nn); }     ///< Adjust allocated memory to given size                 
SwigBind inline bool ia5Adjust(const PtrA<IArray5> & a, size_t n1,size_t n2,size_t n3,size_t n4,size_t n5) { size_t nn[]={0,n1,n2,n3,n4,n5}; return a->resize(nn); }  ///< Adjust allocated memory to given size                    

SwigBind inline bool za1Adjust(const PtrA<ZArray1> & a, size_t n1                            ) { size_t nn[]={0,n1            }; return a->resize(nn); }              ///< Adjust allocated memory to given size     
SwigBind inline bool za2Adjust(const PtrA<ZArray2> & a, size_t n1,size_t n2                     ) { size_t nn[]={0,n1,n2         }; return a->resize(nn); }           ///< Adjust allocated memory to given size           
SwigBind inline bool za3Adjust(const PtrA<ZArray3> & a, size_t n1,size_t n2,size_t n3              ) { size_t nn[]={0,n1,n2,n3      }; return a->resize(nn); }        ///< Adjust allocated memory to given size              
SwigBind inline bool za4Adjust(const PtrA<ZArray4> & a, size_t n1,size_t n2,size_t n3,size_t n4       ) { size_t nn[]={0,n1,n2,n3,n4   }; return a->resize(nn); }     ///< Adjust allocated memory to given size                 
SwigBind inline bool za5Adjust(const PtrA<ZArray5> & a, size_t n1,size_t n2,size_t n3,size_t n4,size_t n5) { size_t nn[]={0,n1,n2,n3,n4,n5}; return a->resize(nn); }  ///< Adjust allocated memory to given size                    



//////////////////
// 10.18 Offset 0 direct  data memory access 

inline double * da1Val(const PtrA<DArray1> & a) { return a->v; }  ///< Retrieve value pointer (values counting from 1)
inline double * da2Val(const PtrA<DArray2> & a) { return a->v; }  ///< Retrieve value pointer (values counting from 1)
inline double * da3Val(const PtrA<DArray3> & a) { return a->v; }  ///< Retrieve value pointer (values counting from 1)
inline double * da4Val(const PtrA<DArray4> & a) { return a->v; }  ///< Retrieve value pointer (values counting from 1)
inline double * da5Val(const PtrA<DArray5> & a) { return a->v; }  ///< Retrieve value pointer (values counting from 1)

inline int * ia1Val(const PtrA<IArray1> & a) { return a->v; }  ///< Retrieve value pointer (values counting from 1)
inline int * ia2Val(const PtrA<IArray2> & a) { return a->v; } ///< Retrieve value pointer (values counting from 1)
inline int * ia3Val(const PtrA<IArray3> & a) { return a->v; } ///< Retrieve value pointer (values counting from 1)
inline int * ia4Val(const PtrA<IArray4> & a) { return a->v; } ///< Retrieve value pointer (values counting from 1)
inline int * ia5Val(const PtrA<IArray5> & a) { return a->v; } ///< Retrieve value pointer (values counting from 1)

inline double_complex * za1Val(const PtrA<ZArray1> & a) { return a->v; } ///< Retrieve value pointer (values counting from 1)
inline double_complex * za2Val(const PtrA<ZArray2> & a) { return a->v; } ///< Retrieve value pointer (values counting from 1)
inline double_complex * za3Val(const PtrA<ZArray3> & a) { return a->v; } ///< Retrieve value pointer (values counting from 1)
inline double_complex * za4Val(const PtrA<ZArray4> & a) { return a->v; } ///< Retrieve value pointer (values counting from 1)
inline double_complex * za5Val(const PtrA<ZArray5> & a) { return a->v; } ///< Retrieve value pointer (values counting from 1)

template <typename T> inline T * ta1Val(const PtrA<TArray1 <T> > & a) { return a->v; } ///< Retrieve value pointer (values counting from 1)
template <typename T> inline T * ta2Val(const PtrA<TArray2 <T> > & a) { return a->v; } ///< Retrieve value pointer (values counting from 1)
template <typename T> inline T * ta3Val(const PtrA<TArray3 <T> > & a) { return a->v; } ///< Retrieve value pointer (values counting from 1)
template <typename T> inline T * ta4Val(const PtrA<TArray4 <T> > & a) { return a->v; } ///< Retrieve value pointer (values counting from 1)
template <typename T> inline T * ta5Val(const PtrA<TArray5 <T> > & a) { return a->v; } ///< Retrieve value pointer (values counting from 1)

//////////////////
// 10.19 Offset 1 direct  data memory access  (for passing to fortran 77)

inline double * da1Val77(const PtrA<DArray1> & a) { return a->v+1; } ///< Retrieve value pointer (values counting from 0)
inline double * da2Val77(const PtrA<DArray2> & a) { return a->v+1; } ///< Retrieve value pointer (values counting from 0)
inline double * da3Val77(const PtrA<DArray3> & a) { return a->v+1; } ///< Retrieve value pointer (values counting from 0)
inline double * da4Val77(const PtrA<DArray4> & a) { return a->v+1; } ///< Retrieve value pointer (values counting from 0)
inline double * da5Val77(const PtrA<DArray5> & a) { return a->v+1; } ///< Retrieve value pointer (values counting from 0)

inline int * ia1Val77(const PtrA<IArray1> & a) { return a->v+1; } ///< Retrieve value pointer (values counting from 0)
inline int * ia2Val77(const PtrA<IArray2> & a) { return a->v+1; } ///< Retrieve value pointer (values counting from 0)
inline int * ia3Val77(const PtrA<IArray3> & a) { return a->v+1; } ///< Retrieve value pointer (values counting from 0)
inline int * ia4Val77(const PtrA<IArray4> & a) { return a->v+1; } ///< Retrieve value pointer (values counting from 0)
inline int * ia5Val77(const PtrA<IArray5> & a) { return a->v+1; } ///< Retrieve value pointer (values counting from 0)

inline double_complex * za1Val77(const PtrA<ZArray1> & a) { return a->v+1; } ///< Retrieve value pointer (values counting from 0)
inline double_complex * za2Val77(const PtrA<ZArray2> & a) { return a->v+1; } ///< Retrieve value pointer (values counting from 0)
inline double_complex * za3Val77(const PtrA<ZArray3> & a) { return a->v+1; } ///< Retrieve value pointer (values counting from 0)
inline double_complex * za4Val77(const PtrA<ZArray4> & a) { return a->v+1; } ///< Retrieve value pointer (values counting from 0)
inline double_complex * za5Val77(const PtrA<ZArray5> & a) { return a->v+1; } ///< Retrieve value pointer (values counting from 0)

inline double * da2RowVal77(const PtrA<DArray2> & a, int irow) { return &(DA2(a ,irow,1)); }       ///< Retrieve row value pointer (values counting from 0)  
inline double * da3RowVal77(const PtrA<DArray3> & a, int irow) { return &(DA3(a ,irow,1,1)); }     ///< Retrieve row value pointer (values counting from 0)
inline double * da4RowVal77(const PtrA<DArray4> & a, int irow) { return &(DA4(a ,irow,1,1,1)); }   ///< Retrieve row value pointer (values counting from 0)
inline double * da5RowVal77(const PtrA<DArray5> & a, int irow) { return &(DA5(a ,irow,1,1,1,1)); } ///< Retrieve row value pointer (values counting from 0)

inline int * ia2RowVal77(const PtrA<IArray2> & a, int irow) { return &(IA2(a ,irow,1)); }       ///< Retrieve row value pointer (values counting from 0)
inline int * ia3RowVal77(const PtrA<IArray3> & a, int irow) { return &(IA3(a ,irow,1,1)); }     ///< Retrieve row value pointer (values counting from 0)
inline int * ia4RowVal77(const PtrA<IArray4> & a, int irow) { return &(IA4(a ,irow,1,1,1)); }   ///< Retrieve row value pointer (values counting from 0)
inline int * ia5RowVal77(const PtrA<IArray5> & a, int irow) { return &(IA5(a ,irow,1,1,1,1)); } ///< Retrieve row value pointer (values counting from 0)

inline double_complex * za2RowVal77(const PtrA<ZArray2> & a, int irow) { return &(ZA2(a ,irow,1)); }        ///< Retrieve row value pointer (values counting from 0)
inline double_complex * za3RowVal77(const PtrA<ZArray3> & a, int irow) { return &(ZA3(a ,irow,1,1)); }	   ///< Retrieve row value pointer (values counting from 0)
inline double_complex * za4RowVal77(const PtrA<ZArray4> & a, int irow) { return &(ZA4(a ,irow,1,1,1)); }	   ///< Retrieve row value pointer (values counting from 0)
inline double_complex * za5RowVal77(const PtrA<ZArray5> & a, int irow) { return &(ZA5(a ,irow,1,1,1,1)); }  ///< Retrieve row value pointer (values counting from 0)

inline double * da2ColVal77(const PtrA<DArray2> & a, int icol) { return &(DA2(a ,1,icol)); }       ///< Retrieve column value pointer (values counting from 0)
inline double * da3ColVal77(const PtrA<DArray3> & a, int icol) { return &(DA3(a ,1,icol,1)); }	  ///< Retrieve column value pointer (values counting from 0)
inline double * da4ColVal77(const PtrA<DArray4> & a, int icol) { return &(DA4(a ,1,icol,1,1)); }	  ///< Retrieve column value pointer (values counting from 0)
inline double * da5ColVal77(const PtrA<DArray5> & a, int icol) { return &(DA5(a ,1,icol,1,1,1)); } ///< Retrieve column value pointer (values counting from 0)

inline int * ia2ColVal77(const PtrA<IArray2> & a, int icol) { return &(IA2(a ,1,icol)); }            ///< Retrieve column value pointer (values counting from 0)
inline int * ia3ColVal77(const PtrA<IArray3> & a, int icol) { return &(IA3(a ,1,icol,1)); }	    ///< Retrieve column value pointer (values counting from 0)
inline int * ia4ColVal77(const PtrA<IArray4> & a, int icol) { return &(IA4(a ,1,icol,1,1)); }	    ///< Retrieve column value pointer (values counting from 0)
inline int * ia5ColVal77(const PtrA<IArray5> & a, int icol) { return &(IA5(a ,1,icol,1,1,1)); }	    ///< Retrieve column value pointer (values counting from 0)

inline double_complex * za2ColVal77(const PtrA<ZArray2> & a, int icol) { return &(ZA2(a ,1,icol)); }       ///< Retrieve column value pointer (values counting from 0)
inline double_complex * za3ColVal77(const PtrA<ZArray3> & a, int icol) { return &(ZA3(a ,1,icol,1)); }	  ///< Retrieve column value pointer (values counting from 0)
inline double_complex * za4ColVal77(const PtrA<ZArray4> & a, int icol) { return &(ZA4(a ,1,icol,1,1)); }	  ///< Retrieve column value pointer (values counting from 0)
inline double_complex * za5ColVal77(const PtrA<ZArray5> & a, int icol) { return &(ZA5(a ,1,icol,1,1,1)); } ///< Retrieve column value pointer (values counting from 0)

inline double * da2RowVal(const PtrA<DArray2> & a, int irow) { return &(DA2(a ,irow,1))-1; }        ///< Retrieve row value pointer (values counting from 1)	
inline double * da3RowVal(const PtrA<DArray3> & a, int irow) { return &(DA3(a ,irow,1,1))-1; }	   ///< Retrieve row value pointer (values counting from 1)
inline double * da4RowVal(const PtrA<DArray4> & a, int irow) { return &(DA4(a ,irow,1,1,1))-1; }	   ///< Retrieve row value pointer (values counting from 1)
inline double * da5RowVal(const PtrA<DArray5> & a, int irow) { return &(DA5(a ,irow,1,1,1,1))-1; }  ///< Retrieve row value pointer (values counting from 1)

inline int * ia2RowVal(const PtrA<IArray2> & a, int irow) { return &(IA2(a ,irow,1))-1; }           ///< Retrieve row value pointer (values counting from 0)
inline int * ia3RowVal(const PtrA<IArray3> & a, int irow) { return &(IA3(a ,irow,1,1))-1; }	   ///< Retrieve row value pointer (values counting from 0)
inline int * ia4RowVal(const PtrA<IArray4> & a, int irow) { return &(IA4(a ,irow,1,1,1))-1; }	   ///< Retrieve row value pointer (values counting from 0)
inline int * ia5RowVal(const PtrA<IArray5> & a, int irow) { return &(IA5(a ,irow,1,1,1,1))-1; }	   ///< Retrieve row value pointer (values counting from 0)  

inline double_complex * za2RowVal(const PtrA<ZArray2> & a, int irow) { return &(ZA2(a ,irow,1))-1; }       ///< Retrieve row value pointer (values counting from 0)
inline double_complex * za3RowVal(const PtrA<ZArray3> & a, int irow) { return &(ZA3(a ,irow,1,1))-1; }	  ///< Retrieve row value pointer (values counting from 0)
inline double_complex * za4RowVal(const PtrA<ZArray4> & a, int irow) { return &(ZA4(a ,irow,1,1,1))-1; }	  ///< Retrieve row value pointer (values counting from 0)
inline double_complex * za5RowVal(const PtrA<ZArray5> & a, int irow) { return &(ZA5(a ,irow,1,1,1,1))-1; } ///< Retrieve row value pointer (values counting from 0)

inline double * da2ColVal(const PtrA<DArray2> & a, int icol) { return &(DA2(a ,1,icol))-1; }          ///< Retrieve column value pointer (values counting from 1)
inline double * da3ColVal(const PtrA<DArray3> & a, int icol) { return &(DA3(a ,1,icol,1))-1; }	     ///< Retrieve column value pointer (values counting from 1)
inline double * da4ColVal(const PtrA<DArray4> & a, int icol) { return &(DA4(a ,1,icol,1,1))-1; }	     ///< Retrieve column value pointer (values counting from 1)
inline double * da5ColVal(const PtrA<DArray5> & a, int icol) { return &(DA5(a ,1,icol,1,1,1))-1; }    ///< Retrieve column value pointer (values counting from 1)

inline int * ia2ColVal(const PtrA<IArray2> & a, int icol) { return &(IA2(a ,1,icol))-1; }          ///< Retrieve column value pointer (values counting from 1) 
inline int * ia3ColVal(const PtrA<IArray3> & a, int icol) { return &(IA3(a ,1,icol,1))-1; }	  ///< Retrieve column value pointer (values counting from 1) 
inline int * ia4ColVal(const PtrA<IArray4> & a, int icol) { return &(IA4(a ,1,icol,1,1))-1; }	  ///< Retrieve column value pointer (values counting from 1) 
inline int * ia5ColVal(const PtrA<IArray5> & a, int icol) { return &(IA5(a ,1,icol,1,1,1))-1; }	  ///< Retrieve column value pointer (values counting from 1) 

inline double_complex * za2ColVal(const PtrA<ZArray2> & a, int icol) { return &(ZA2(a ,1,icol))-1; }       ///< Retrieve column value pointer (values counting from 1) 
inline double_complex * za3ColVal(const PtrA<ZArray3> & a, int icol) { return &(ZA3(a ,1,icol,1))-1; }	  ///< Retrieve column value pointer (values counting from 1) 
inline double_complex * za4ColVal(const PtrA<ZArray4> & a, int icol) { return &(ZA4(a ,1,icol,1,1))-1; }	  ///< Retrieve column value pointer (values counting from 1) 
inline double_complex * za5ColVal(const PtrA<ZArray5> & a, int icol) { return &(ZA5(a ,1,icol,1,1,1))-1; } ///< Retrieve column value pointer (values counting from 1) 


//////////////////
// 10.20  Set context information
inline void da1SetClass(const PtrA<DArray1> & a, const Ptr<ArrayClassBase> & aclass) { a->aclass=aclass; } ///< Set array class (for storing information on grid etc.)
inline void da2SetClass(const PtrA<DArray2> & a, const Ptr<ArrayClassBase> & aclass) { a->aclass=aclass; } ///< Set array class (for storing information on grid etc.)
inline void da3SetClass(const PtrA<DArray3> & a, const Ptr<ArrayClassBase> & aclass) { a->aclass=aclass; } ///< Set array class (for storing information on grid etc.)
inline void da4SetClass(const PtrA<DArray4> & a, const Ptr<ArrayClassBase> & aclass) { a->aclass=aclass; } ///< Set array class (for storing information on grid etc.)
inline void da5SetClass(const PtrA<DArray5> & a, const Ptr<ArrayClassBase> & aclass) { a->aclass=aclass; } ///< Set array class (for storing information on grid etc.)

inline void ia1SetClass(const PtrA<IArray1> & a, const Ptr<ArrayClassBase> & aclass) { a->aclass=aclass; } ///< Set array class (for storing information on grid etc.)
inline void ia2SetClass(const PtrA<IArray2> & a, const Ptr<ArrayClassBase> & aclass) { a->aclass=aclass; } ///< Set array class (for storing information on grid etc.)
inline void ia3SetClass(const PtrA<IArray3> & a, const Ptr<ArrayClassBase> & aclass) { a->aclass=aclass; } ///< Set array class (for storing information on grid etc.)
inline void ia4SetClass(const PtrA<IArray4> & a, const Ptr<ArrayClassBase> & aclass) { a->aclass=aclass; } ///< Set array class (for storing information on grid etc.)
inline void ia5SetClass(const PtrA<IArray5> & a, const Ptr<ArrayClassBase> & aclass) { a->aclass=aclass; } ///< Set array class (for storing information on grid etc.)

inline void za1SetClass(const PtrA<ZArray1> & a, const Ptr<ArrayClassBase> & aclass) { a->aclass=aclass; } ///< Set array class (for storing information on grid etc.)
inline void za2SetClass(const PtrA<ZArray2> & a, const Ptr<ArrayClassBase> & aclass) { a->aclass=aclass; } ///< Set array class (for storing information on grid etc.)
inline void za3SetClass(const PtrA<ZArray3> & a, const Ptr<ArrayClassBase> & aclass) { a->aclass=aclass; } ///< Set array class (for storing information on grid etc.)
inline void za4SetClass(const PtrA<ZArray4> & a, const Ptr<ArrayClassBase> & aclass) { a->aclass=aclass; } ///< Set array class (for storing information on grid etc.)
inline void za5SetClass(const PtrA<ZArray5> & a, const Ptr<ArrayClassBase> & aclass) { a->aclass=aclass; } ///< Set array class (for storing information on grid etc.)



///////////////
// 10.21 Array constructors for data memory allocated elsewhere, free() to free memory

SwigBind inline PtrA<DArray1> da1Attach(int n1                                ,double* val,int memoffset) { return new DArray1(n1            ,val,memoffset); }
SwigBind inline PtrA<DArray2> da2Attach(int n1, int n2                        ,double* val,int memoffset) { return new DArray2(n1,n2         ,val,memoffset); }
SwigBind inline PtrA<DArray3> da3Attach(int n1, int n2, int n3                ,double* val,int memoffset) { return new DArray3(n1,n2,n3      ,val,memoffset); }
SwigBind inline PtrA<DArray4> da4Attach(int n1, int n2, int n3, int n4        ,double* val,int memoffset) { return new DArray4(n1,n2,n3,n4   ,val,memoffset); }
SwigBind inline PtrA<DArray5> da5Attach(int n1, int n2, int n3, int n4, int n5,double* val,int memoffset) { return new DArray5(n1,n2,n3,n4,n5,val,memoffset); }

SwigBind inline PtrA<IArray1> ia1Attach(int n1                                ,int* val,int memoffset) { return new IArray1(n1            ,val,memoffset); }
SwigBind inline PtrA<IArray2> ia2Attach(int n1, int n2                        ,int* val,int memoffset) { return new IArray2(n1,n2         ,val,memoffset); }
SwigBind inline PtrA<IArray3> ia3Attach(int n1, int n2, int n3                ,int* val,int memoffset) { return new IArray3(n1,n2,n3      ,val,memoffset); }
SwigBind inline PtrA<IArray4> ia4Attach(int n1, int n2, int n3, int n4        ,int* val,int memoffset) { return new IArray4(n1,n2,n3,n4   ,val,memoffset); }
SwigBind inline PtrA<IArray5> ia5Attach(int n1, int n2, int n3, int n4, int n5,int* val,int memoffset) { return new IArray5(n1,n2,n3,n4,n5,val,memoffset); }

SwigBind inline PtrA<ZArray1> za1Attach(int n1                                ,double_complex* val,int memoffset) { return new ZArray1(n1            ,val,memoffset); }
SwigBind inline PtrA<ZArray2> za2Attach(int n1, int n2                        ,double_complex* val,int memoffset) { return new ZArray2(n1,n2         ,val,memoffset); }
SwigBind inline PtrA<ZArray3> za3Attach(int n1, int n2, int n3                ,double_complex* val,int memoffset) { return new ZArray3(n1,n2,n3      ,val,memoffset); }
SwigBind inline PtrA<ZArray4> za4Attach(int n1, int n2, int n3, int n4        ,double_complex* val,int memoffset) { return new ZArray4(n1,n2,n3,n4   ,val,memoffset); }
SwigBind inline PtrA<ZArray5> za5Attach(int n1, int n2, int n3, int n4, int n5,double_complex* val,int memoffset) { return new ZArray5(n1,n2,n3,n4,n5,val,memoffset); }


///////////////
// 10.22 Array constructors for data memory allocated elsewhere, delete() to free memory


PtrA<DArray1> da1AttachXDelete(int n1, double *v, int offset);
PtrA<DArray2> da2AttachXDelete(int n1, int n2, double *v, int offset);

PtrA<IArray1> ia1AttachXDelete(int n1, int  *v, int offset);
PtrA<IArray2> ia2AttachXDelete(int n1, int n2, int *v, int offset);


// uses free[] to destroy val
PtrA<DArray1> da1AttachXFree(int n1, double *v, int offset);
PtrA<DArray2> da2AttachXFree(int n1, int n2, double *v, int offset);

PtrA<IArray1> ia1AttachXFree(int n1, int  *v, int offset);
PtrA<IArray2> ia2AttachXFree(int n1, int n2, int *v, int offset);



//////////////////
// 10.23  Load/store data to files

  SwigBind double da1XDump(const PtrA<DArray1> & a, FILE *f);
  SwigBind double da2XDump(const PtrA<DArray2> & a, FILE *f);
  SwigBind double da3XDump(const PtrA<DArray3> & a, FILE *f);
  SwigBind double da4XDump(const PtrA<DArray4> & a, FILE *f);
  SwigBind double da5XDump(const PtrA<DArray5> & a, FILE *f);
  
  SwigBind double ia1XDump(const PtrA<IArray1> & a, FILE *f);
  SwigBind double ia2XDump(const PtrA<IArray2> & a, FILE *f);
  SwigBind double ia3XDump(const PtrA<IArray3> & a, FILE *f);
  SwigBind double ia4XDump(const PtrA<IArray4> & a, FILE *f);
  SwigBind double ia5XDump(const PtrA<IArray5> & a, FILE *f);
  
  
  SwigBind void da1XLoad(const PtrA<DArray1> & a, FILE *f,double pos);
  SwigBind void da2XLoad(const PtrA<DArray2> & a, FILE *f,double pos);
  SwigBind void da3XLoad(const PtrA<DArray3> & a, FILE *f,double pos);
  SwigBind void da4XLoad(const PtrA<DArray4> & a, FILE *f,double pos);
  SwigBind void da5XLoad(const PtrA<DArray5> & a, FILE *f,double pos);
  
  SwigBind void ia1XLoad(const PtrA<IArray1> & a, FILE *f,double pos);
  SwigBind void ia2XLoad(const PtrA<IArray2> & a, FILE *f,double pos);
  SwigBind void ia3XLoad(const PtrA<IArray3> & a, FILE *f,double pos);
  SwigBind void ia4XLoad(const PtrA<IArray4> & a, FILE *f,double pos);
  SwigBind void ia5XLoad(const PtrA<IArray5> & a, FILE *f,double pos);
  
  
  SwigBind double da1XAppend(const PtrA<DArray1> & a,const char *fname);
  SwigBind double da2XAppend(const PtrA<DArray2> & a,const char *fname);
  SwigBind double da3XAppend(const PtrA<DArray3> & a,const char *fname);
  SwigBind double da4XAppend(const PtrA<DArray4> & a,const char *fname);
  SwigBind double da5XAppend(const PtrA<DArray5> & a,const char *fname);
  
  SwigBind double ia1XAppend(const PtrA<IArray1> & a,const char *fname);
  SwigBind double ia2XAppend(const PtrA<IArray2> & a,const char *fname);
  SwigBind double ia3XAppend(const PtrA<IArray3> & a,const char *fname);
  SwigBind double ia4XAppend(const PtrA<IArray4> & a,const char *fname);
  SwigBind double ia5XAppend(const PtrA<IArray5> & a,const char *fname);
  
  
  SwigBind void da1XRestore(const PtrA<DArray1> & a,const char *fname,double pos);
  SwigBind void da2XRestore(const PtrA<DArray2> & a,const char *fname,double pos);
  SwigBind void da3XRestore(const PtrA<DArray3> & a,const char *fname,double pos);
  SwigBind void da4XRestore(const PtrA<DArray4> & a,const char *fname,double pos);
  SwigBind void da5XRestore(const PtrA<DArray5> & a,const char *fname,double pos);
  
  SwigBind void ia1XRestore(const PtrA<IArray1> & a,const char *fname,double pos);
  SwigBind void ia2XRestore(const PtrA<IArray2> & a,const char *fname,double pos);
  SwigBind void ia3XRestore(const PtrA<IArray3> & a,const char *fname,double pos);
  SwigBind void ia4XRestore(const PtrA<IArray4> & a,const char *fname,double pos);
  SwigBind void ia5XRestore(const PtrA<IArray5> & a,const char *fname,double pos);


/**
   \brief Dump array data into open ASCII file.
 */
SwigBind double ia1ADump(PtrA<IArray1> a , FILE *f);

/**
   \brief Dump array data into open ASCII file.
 */
SwigBind double ia2ADump(PtrA<IArray2> a , FILE *f);

/**
   \brief Dump array data into open ASCII file.
 */
SwigBind double da1ADump(PtrA<DArray1> a , FILE *f);

/**
   \brief Dump array data into open ASCII file.
 */
SwigBind double da2ADump(PtrA<DArray2> a , FILE *f);


/**
   \brief Dump array data into ASCII file.
 */
SwigBind void ia1ADump(PtrA<IArray1> a , const char *fname);

/**
   \brief Dump array data into ASCII file.
 */
SwigBind void ia2ADump(PtrA<IArray2> a , const char *fname);

/**
   \brief Dump array data into ASCII file.
 */
SwigBind void da1ADump(PtrA<DArray1> a , const char *fname);

/**
   \brief Dump array data into ASCII file.
 */
SwigBind void da2ADump(PtrA<DArray2> a , const char *fname);


/**
   \brief Dump array data into open mixed ASCII LDF 
   file labelling it with the specified name.
 */
SwigBind void ia1ADumpLDF(PtrA<IArray1> a , FILE *f, const char *name);
/**
   \brief Dump array data into open mixed ASCII LDF 
   file labelling it with the specified name.
 */
SwigBind void ia2ADumpLDF(PtrA<IArray2> a , FILE *f, const char *name);

/**
   \brief Dump array data into open mixed ASCII LDF 
   file labelling it with the specified name.
 */
SwigBind void da1ADumpLDF(PtrA<DArray1> a , FILE *f, const char *name);
/**
   \brief Dump array data into open mixed ASCII LDF 
   file labelling it with the specified name.
 */
SwigBind void da2ADumpLDF(PtrA<DArray2> a , FILE *f, const char *name);

/**
   \brief Dump array data into open mixed ASCII/binary LDF 
   file labelling it with the specified name.
 */
SwigBind void ia1XDumpLDF(PtrA<IArray1> a , FILE *f, const char *name);
/**
   \brief Dump array data into open mixed ASCII/binary LDF 
   file labelling it with the specified name.
 */
SwigBind void ia2XDumpLDF(PtrA<IArray2> a , FILE *f, const char *name);

/**
   \brief Dump array data into open mixed ASCII/binary LDF 
   file labelling it with the specified name.
 */
SwigBind void da1XDumpLDF(PtrA<DArray1> a , FILE *f, const char *name);
/**
   \brief Dump array data into open mixed ASCII/binary LDF 
   file labelling it with the specified name.
 */
SwigBind void da2XDumpLDF(PtrA<DArray2> a , FILE *f, const char *name);



SwigBind PtrA<DArray1> da1ALoad(FILE *f);
SwigBind PtrA<DArray1> da1ALoad(const char *fname);
SwigBind PtrA<IArray1> ia1ALoad(FILE *f);
SwigBind PtrA<IArray1> ia1ALoad(const char *fname);
SwigBind PtrA<DArray2> da2ALoad(FILE *f);
SwigBind PtrA<DArray2> da2ALoad(const char *fname);
SwigBind PtrA<IArray2> ia2ALoad(FILE *f);
SwigBind PtrA<IArray2> ia2ALoad(const char *fname);



////////////////////////////////////////////////////
////////////////////////////////////////////////////
// 11. PtrA based API
// For pure C++ code it would be  not necessary to double the API.
// It is necessary in Lua, as in the Lua binding, automatic  casts do not work.

//////////////
// 11.1 Constructors

SwigBind inline PtrA<DArray1> da1New(int n1                                ) { return new DArray1(n1            ); } ///< Create 1D double array
SwigBind inline PtrA<DArray2> da2New(int n1, int n2                        ) { return new DArray2(n1,n2         ); } ///< Create 2D double array
SwigBind inline PtrA<DArray3> da3New(int n1, int n2, int n3                ) { return new DArray3(n1,n2,n3      ); } ///< Create 3D double array
SwigBind inline PtrA<DArray4> da4New(int n1, int n2, int n3, int n4        ) { return new DArray4(n1,n2,n3,n4   ); } ///< New 4D double array
SwigBind inline PtrA<DArray5> da5New(int n1, int n2, int n3, int n4, int n5) { return new DArray5(n1,n2,n3,n4,n5); } ///< New 5D double array

SwigBind inline PtrA<IArray1> ia1New(int n1                                ) { return new IArray1(n1            ); } ///< New 1D int array
SwigBind inline PtrA<IArray2> ia2New(int n1, int n2                        ) { return new IArray2(n1,n2         ); } ///< New 2D int array
SwigBind inline PtrA<IArray3> ia3New(int n1, int n2, int n3                ) { return new IArray3(n1,n2,n3      ); } ///< New 3D int array
SwigBind inline PtrA<IArray4> ia4New(int n1, int n2, int n3, int n4        ) { return new IArray4(n1,n2,n3,n4   ); } ///< New 4D int array
SwigBind inline PtrA<IArray5> ia5New(int n1, int n2, int n3, int n4, int n5) { return new IArray5(n1,n2,n3,n4,n5); } ///< New 5D int array

SwigBind inline PtrA<ZArray1> za1New(int n1                                ) { return new ZArray1(n1            ); } ///< New 1D double_complex array
SwigBind inline PtrA<ZArray2> za2New(int n1, int n2                        ) { return new ZArray2(n1,n2         ); } ///< New 2D double_complex array
SwigBind inline PtrA<ZArray3> za3New(int n1, int n2, int n3                ) { return new ZArray3(n1,n2,n3      ); } ///< New 3D double_complex array
SwigBind inline PtrA<ZArray4> za4New(int n1, int n2, int n3, int n4        ) { return new ZArray4(n1,n2,n3,n4   ); } ///< New 4D double_complex array
SwigBind inline PtrA<ZArray5> za5New(int n1, int n2, int n3, int n4, int n5) { return new ZArray5(n1,n2,n3,n4,n5); } ///< New 5D double_complex array



// SwigBind inline PtrA<DArray1> da1Clone(const PtrA<DArray1> & a) { return aClone(a); }   ///< Clone 1D double array (without copying the values but with a new value field)         
// SwigBind inline PtrA<DArray2> da2Clone(const PtrA<DArray2> & a) { return aClone(a); }   ///< Clone 2D double array (without copying the values but with a new value field)         
// SwigBind inline PtrA<DArray3> da3Clone(const PtrA<DArray3> & a) { return aClone(a); }   ///< Clone 3D double array (without copying the values but with a new value field)         
// SwigBind inline PtrA<DArray4> da4Clone(const PtrA<DArray4> & a) { return aClone(a); }   ///< Clone 4D double array (without copying the values but with a new value field)         
// SwigBind inline PtrA<DArray5> da5Clone(const PtrA<DArray5> & a) { return aClone(a); }   ///< Clone 5D double array (without copying the values but with a new value field)         
// 										                                          
// SwigBind inline PtrA<IArray1> ia1Clone(const PtrA<IArray1> & a) { return aClone(a); }   ///< Clone 1D int array  (without copying the values but with a new value field)		 
// SwigBind inline PtrA<IArray2> ia2Clone(const PtrA<IArray2> & a) { return aClone(a); }   ///< Clone 2D int array  (without copying the values but with a new value field)		 
// SwigBind inline PtrA<IArray3> ia3Clone(const PtrA<IArray3> & a) { return aClone(a); }   ///< Clone 3D int array  (without copying the values but with a new value field)		 
// SwigBind inline PtrA<IArray4> ia4Clone(const PtrA<IArray4> & a) { return aClone(a); }   ///< Clone 4D int array  (without copying the values but with a new value field)		 
// SwigBind inline PtrA<IArray5> ia5Clone(const PtrA<IArray5> & a) { return aClone(a); }   ///< Clone 5D int array  (without copying the values but with a new value field)            
// 										                                          
// SwigBind inline PtrA<ZArray1> za1Clone(const PtrA<ZArray1> & a) { return aClone(a); }   ///< Clone 1D double_complex array   (without copying the values but with a new value field)
// SwigBind inline PtrA<ZArray2> za2Clone(const PtrA<ZArray2> & a) { return aClone(a); }   ///< Clone 2D double_complex array	  (without copying the values but with a new value field)
// SwigBind inline PtrA<ZArray3> za3Clone(const PtrA<ZArray3> & a) { return aClone(a); }   ///< Clone 3D double_complex array	  (without copying the values but with a new value field)
// SwigBind inline PtrA<ZArray4> za4Clone(const PtrA<ZArray4> & a) { return aClone(a); }   ///< Clone 4D double_complex array	  (without copying the values but with a new value field)
// SwigBind inline PtrA<ZArray5> za5Clone(const PtrA<ZArray5> & a) { return aClone(a); }   ///< Clone 5D double_complex array	  (without copying the values but with a new value field)


//////////////
// 11.2 Short cut  constructors

//
// Inline routiness so that we can just write 
// matsolv(Mat,..., da1(3,5,5)

inline PtrA<DArray1> da1(double d1                                 ){PtrA<DArray1> v=new DArray1(1); DA1(v,1)=d1;                                      return v;}                          ///< Short array constructor with value assignment
inline PtrA<DArray1> da1(double d1, double d2                      ){PtrA<DArray1> v=new DArray1(2); DA1(v,1)=d1;DA1(v,2)=d2;                          return v;}                          ///< Short array constructor with value assignment
inline PtrA<DArray1> da1(double d1, double d2, double d3           ){PtrA<DArray1> v=new DArray1(3); DA1(v,1)=d1;DA1(v,2)=d2;DA1(v,3)=d3;              return v;}                          ///< Short array constructor with value assignment
inline PtrA<DArray1> da1(double d1, double d2, double d3, double d4){PtrA<DArray1> v=new DArray1(4); DA1(v,1)=d1;DA1(v,2)=d2;DA1(v,3)=d3; DA1(v,4)=d4; return v;}                          ///< Short array constructor with value assignment
inline PtrA<DArray1> da1(double d1, double d2, double d3, double d4, double d5){Ptr<DArray1> v=new DArray1(5); DA1(v,1)=d1;DA1(v,2)=d2;DA1(v,3)=d3; DA1(v,4)=d4;  DA1(v,5)=d5;return v;}  ///< Short array constructor with value assignment

inline PtrA<IArray1> ia1(double d1                                 ){PtrA<IArray1> v=new IArray1(1); IA1(v,1)=d1;                                      return v;}                           ///< Short array constructor with value assignment
inline PtrA<IArray1> ia1(double d1, double d2                      ){PtrA<IArray1> v=new IArray1(2); IA1(v,1)=d1;IA1(v,2)=d2;                          return v;}				  ///< Short array constructor with value assignment
inline PtrA<IArray1> ia1(double d1, double d2, double d3           ){PtrA<IArray1> v=new IArray1(3); IA1(v,1)=d1;IA1(v,2)=d2;IA1(v,3)=d3;              return v;}				  ///< Short array constructor with value assignment
inline PtrA<IArray1> ia1(double d1, double d2, double d3, double d4){PtrA<IArray1> v=new IArray1(4); IA1(v,1)=d1;IA1(v,2)=d2;IA1(v,3)=d3; IA1(v,4)=d4; return v;}				  ///< Short array constructor with value assignment
inline PtrA<IArray1> ia1(double d1, double d2, double d3, double d4, double d5){PtrA<IArray1> v=new IArray1(5); IA1(v,1)=d1;IA1(v,2)=d2;IA1(v,3)=d3; IA1(v,4)=d4;  IA1(v,5)=d5;return v;}	  ///< Short array constructor with value assignment


/////////////////
// 11.3 Dimension information

// SwigBind inline int da1NAll(const PtrA<DArray1> & a) { return a->nall(); } ///< Number of elements in array
// SwigBind inline int da2NAll(const PtrA<DArray2> & a) { return a->nall(); } ///< Number of elements in array
// SwigBind inline int da3NAll(const PtrA<DArray3> & a) { return a->nall(); } ///< Number of elements in array
// SwigBind inline int da4NAll(const PtrA<DArray4> & a) { return a->nall(); } ///< Number of elements in array
// SwigBind inline int da5NAll(const PtrA<DArray5> & a) { return a->nall(); } ///< Number of elements in array

// SwigBind inline int ia1NAll(const PtrA<IArray1> & a) { return a->nall(); } ///< Number of elements in array
// SwigBind inline int ia2NAll(const PtrA<IArray2> & a) { return a->nall(); } ///< Number of elements in array
// SwigBind inline int ia3NAll(const PtrA<IArray3> & a) { return a->nall(); } ///< Number of elements in array
// SwigBind inline int ia4NAll(const PtrA<IArray4> & a) { return a->nall(); } ///< Number of elements in array
// SwigBind inline int ia5NAll(const PtrA<IArray5> & a) { return a->nall(); } ///< Number of elements in array
// 
// SwigBind inline int za1NAll(const PtrA<ZArray1> & a) { return a->nall(); } ///< Number of elements in array
// SwigBind inline int za2NAll(const PtrA<ZArray2> & a) { return a->nall(); } ///< Number of elements in array
// SwigBind inline int za3NAll(const PtrA<ZArray3> & a) { return a->nall(); } ///< Number of elements in array
// SwigBind inline int za4NAll(const PtrA<ZArray4> & a) { return a->nall(); } ///< Number of elements in array
// SwigBind inline int za5NAll(const PtrA<ZArray5> & a) { return a->nall(); } ///< Number of elements in array



//SwigBind inline int da1NCol(const PtrA<DArray1> & a) { return a->ncol(); } ///< Number of columns
//SwigBind inline int da2NCol(const PtrA<DArray2> & a) { return a->ncol(); } ///< Number of columns
//SwigBind inline int da3NCol(const PtrA<DArray3> & a) { return a->ncol(); } ///< Number of columns
//SwigBind inline int da4NCol(const PtrA<DArray4> & a) { return a->ncol(); } ///< Number of columns
//SwigBind inline int da5NCol(const PtrA<DArray5> & a) { return a->ncol(); } ///< Number of columns

// SwigBind inline int ia1NCol(const PtrA<IArray1> & a) { return a->ncol(); } ///< Number of columns
// SwigBind inline int ia2NCol(const PtrA<IArray2> & a) { return a->ncol(); } ///< Number of columns
// SwigBind inline int ia3NCol(const PtrA<IArray3> & a) { return a->ncol(); } ///< Number of columns
// SwigBind inline int ia4NCol(const PtrA<IArray4> & a) { return a->ncol(); } ///< Number of columns
// SwigBind inline int ia5NCol(const PtrA<IArray5> & a) { return a->ncol(); } ///< Number of columns
// 
// SwigBind inline int za1NCol(const PtrA<ZArray1> & a) { return a->ncol(); } ///< Number of columns
// SwigBind inline int za2NCol(const PtrA<ZArray2> & a) { return a->ncol(); } ///< Number of columns
// SwigBind inline int za3NCol(const PtrA<ZArray3> & a) { return a->ncol(); } ///< Number of columns
// SwigBind inline int za4NCol(const PtrA<ZArray4> & a) { return a->ncol(); } ///< Number of columns
// SwigBind inline int za5NCol(const PtrA<ZArray5> & a) { return a->ncol(); } ///< Number of columns

//SwigBind inline int da1NRow(const PtrA<DArray1> & a) { return a->nrow(); } ///< Number of rows
//SwigBind inline int da2NRow(const PtrA<DArray2> & a) { return a->nrow(); } ///< Number of rows
//SwigBind inline int da3NRow(const PtrA<DArray3> & a) { return a->nrow(); } ///< Number of rows
//SwigBind inline int da4NRow(const PtrA<DArray4> & a) { return a->nrow(); } ///< Number of rows
//SwigBind inline int da5NRow(const PtrA<DArray5> & a) { return a->nrow(); } ///< Number of rows
                   
// SwigBind inline int ia1NRow(const PtrA<IArray1> & a) { return a->nrow(); } ///< Number of rows
// SwigBind inline int ia2NRow(const PtrA<IArray2> & a) { return a->nrow(); } ///< Number of rows
// SwigBind inline int ia3NRow(const PtrA<IArray3> & a) { return a->nrow(); } ///< Number of rows
// SwigBind inline int ia4NRow(const PtrA<IArray4> & a) { return a->nrow(); } ///< Number of rows
// SwigBind inline int ia5NRow(const PtrA<IArray5> & a) { return a->nrow(); } ///< Number of rows
// 
// SwigBind inline int za1NRow(const PtrA<ZArray1> & a) { return a->nrow(); } ///< Number of rows
// SwigBind inline int za2NRow(const PtrA<ZArray2> & a) { return a->nrow(); } ///< Number of rows
// SwigBind inline int za3NRow(const PtrA<ZArray3> & a) { return a->nrow(); } ///< Number of rows
// SwigBind inline int za4NRow(const PtrA<ZArray4> & a) { return a->nrow(); } ///< Number of rows
// SwigBind inline int za5NRow(const PtrA<ZArray5> & a) { return a->nrow(); } ///< Number of rows


////////////////////
// 11.4 Load/store

// SwigBind inline double da1XDump(const PtrA<DArray1> & a, FILE *f){return da1XDump(aCast(a),f);}
// SwigBind inline double da2XDump(const PtrA<DArray2> & a, FILE *f){return da2XDump(aCast(a),f);}
// SwigBind inline double da3XDump(const PtrA<DArray3> & a, FILE *f){return da3XDump(aCast(a),f);}
// SwigBind inline double da4XDump(const PtrA<DArray4> & a, FILE *f){return da4XDump(aCast(a),f);}
// SwigBind inline double da5XDump(const PtrA<DArray5> & a, FILE *f){return da5XDump(aCast(a),f);}
// 
// SwigBind inline double ia1XDump(const PtrA<IArray1> & a, FILE *f){return ia1XDump(aCast(a),f);}
// SwigBind inline double ia2XDump(const PtrA<IArray2> & a, FILE *f){return ia2XDump(aCast(a),f);}
// SwigBind inline double ia3XDump(const PtrA<IArray3> & a, FILE *f){return ia3XDump(aCast(a),f);}
// SwigBind inline double ia4XDump(const PtrA<IArray4> & a, FILE *f){return ia4XDump(aCast(a),f);}
// SwigBind inline double ia5XDump(const PtrA<IArray5> & a, FILE *f){return ia5XDump(aCast(a),f);}
// 
// 
// SwigBind inline void da1XLoad(const PtrA<DArray1> & a, FILE *f,double pos){da1XLoad(aCast(a),f,pos);}
// SwigBind inline void da2XLoad(const PtrA<DArray2> & a, FILE *f,double pos){da2XLoad(aCast(a),f,pos);}
// SwigBind inline void da3XLoad(const PtrA<DArray3> & a, FILE *f,double pos){da3XLoad(aCast(a),f,pos);}
// SwigBind inline void da4XLoad(const PtrA<DArray4> & a, FILE *f,double pos){da4XLoad(aCast(a),f,pos);}
// SwigBind inline void da5XLoad(const PtrA<DArray5> & a, FILE *f,double pos){da5XLoad(aCast(a),f,pos);}
// 
// SwigBind inline void ia1XLoad(const PtrA<IArray1> & a, FILE *f,double pos){ia1XLoad(aCast(a),f,pos);} 
// SwigBind inline void ia2XLoad(const PtrA<IArray2> & a, FILE *f,double pos){ia2XLoad(aCast(a),f,pos);} 
// SwigBind inline void ia3XLoad(const PtrA<IArray3> & a, FILE *f,double pos){ia3XLoad(aCast(a),f,pos);} 
// SwigBind inline void ia4XLoad(const PtrA<IArray4> & a, FILE *f,double pos){ia4XLoad(aCast(a),f,pos);} 
// SwigBind inline void ia5XLoad(const PtrA<IArray5> & a, FILE *f,double pos){ia5XLoad(aCast(a),f,pos);} 
// 
// 
// SwigBind inline double da1XAppend(const PtrA<DArray1> & a,const char *fname){return da1XAppend(aCast(a),fname);}
// SwigBind inline double da2XAppend(const PtrA<DArray2> & a,const char *fname){return da2XAppend(aCast(a),fname);}
// SwigBind inline double da3XAppend(const PtrA<DArray3> & a,const char *fname){return da3XAppend(aCast(a),fname);}
// SwigBind inline double da4XAppend(const PtrA<DArray4> & a,const char *fname){return da4XAppend(aCast(a),fname);}
// SwigBind inline double da5XAppend(const PtrA<DArray5> & a,const char *fname){return da5XAppend(aCast(a),fname);}
// 
// SwigBind inline double ia1XAppend(const PtrA<IArray1> & a,const char *fname){return ia1XAppend(aCast(a),fname);}
// SwigBind inline double ia2XAppend(const PtrA<IArray2> & a,const char *fname){return ia2XAppend(aCast(a),fname);}
// SwigBind inline double ia3XAppend(const PtrA<IArray3> & a,const char *fname){return ia3XAppend(aCast(a),fname);}
// SwigBind inline double ia4XAppend(const PtrA<IArray4> & a,const char *fname){return ia4XAppend(aCast(a),fname);}
// SwigBind inline double ia5XAppend(const PtrA<IArray5> & a,const char *fname){return ia5XAppend(aCast(a),fname);}
// 
//   
// SwigBind inline void da1XRestore(const PtrA<DArray1> & a,const char *fname,double pos){da1XRestore(aCast(a),fname,pos);}
// SwigBind inline void da2XRestore(const PtrA<DArray2> & a,const char *fname,double pos){da2XRestore(aCast(a),fname,pos);}
// SwigBind inline void da3XRestore(const PtrA<DArray3> & a,const char *fname,double pos){da3XRestore(aCast(a),fname,pos);}
// SwigBind inline void da4XRestore(const PtrA<DArray4> & a,const char *fname,double pos){da4XRestore(aCast(a),fname,pos);}
// SwigBind inline void da5XRestore(const PtrA<DArray5> & a,const char *fname,double pos){da5XRestore(aCast(a),fname,pos);}

// SwigBind inline void ia1XRestore(const PtrA<IArray1> & a,const char *fname,double pos){ia1XRestore(aCast(a),fname,pos);} 
// SwigBind inline void ia2XRestore(const PtrA<IArray2> & a,const char *fname,double pos){ia2XRestore(aCast(a),fname,pos);} 
// SwigBind inline void ia3XRestore(const PtrA<IArray3> & a,const char *fname,double pos){ia3XRestore(aCast(a),fname,pos);} 
// SwigBind inline void ia4XRestore(const PtrA<IArray4> & a,const char *fname,double pos){ia4XRestore(aCast(a),fname,pos);} 
// SwigBind inline void ia5XRestore(const PtrA<IArray5> & a,const char *fname,double pos){ia5XRestore(aCast(a),fname,pos);} 

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// 12. ArrayView classes
// 

template <typename T>
class TArray2ColView
{
  PtrA< TArray2 <T> > a;
  const int icol;
 public:
  typedef T value_type;
 TArray2ColView(Ptr<DArray2> a, int icol): a(a),icol(icol) {};
  T &operator()(int irow) { return a(irow,icol);};
  T &operator()(int irow, int dummycol) { return a(irow,icol);}; // for PtrA<>
  ~TArray2ColView(){};
};

typedef TArray2ColView<double_complex> ZArray2ColView;
typedef TArray2ColView<double> DArray2ColView;
typedef TArray2ColView<int > IArray2ColView;


template <typename T>
class TArray2RowView
{
  PtrA< TArray2 <T> > a;
  const int irow;
 public:
  typedef T value_type;

 TArray2RowView(Ptr<DArray2> a, int irow): a(a),irow(irow) {};
  T &operator()(int icol) { return a(irow,icol);};
  T &operator()(int dummyrow, int icol) { return a(irow,icol);}; // for PtrA<>
  ~TArray2RowView(){};
};


typedef TArray2RowView<double_complex> ZArray2RowView;
typedef TArray2RowView<double> DArray2RowView;
typedef TArray2RowView<int > IArray2RowView;


template <typename T>
class TArray1StrideView
{
  PtrA< TArray1 <T> > a;
  const int stride;
  const int ofs;
 public:
  typedef T value_type;

 TArray1StrideView(Ptr<DArray1> a, int stride): a(a), stride(stride), ofs(1-stride) {};
  T &operator()(int i) { return a(ofs+stride*i);};
  T &operator()(int i, int dummycol) { return a(ofs+stride*i);}; // for PtrA<>
  ~TArray1StrideView(){};
};

typedef TArray1StrideView<double_complex> ZArray21trideView;
typedef TArray1StrideView<double> DArray1StrideView;
typedef TArray1StrideView<int > IArray1StrideView;


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// 13. BLAS wrappers

//SwigBind inline int vdim(PtrA<DArray1> x) {return x->nall(); }  ///< vector space dimension 
//SwigBind inline int vdim(PtrA<DArray2> x) {return x->nall(); }  ///< vector space dimension
//SwigBind inline int vdim(Ptr<ZArray1> x) {return x->nall(); }  ///< vector space dimension
//SwigBind inline int vdim(Ptr<ZArray2> x) {return x->nall(); }  ///< vector space dimension
//SwigBind inline int vdim(Ptr<IArray1> x) {return x->nall(); }  ///< vector space dimension
//SwigBind inline int vdim(Ptr<IArray2> x) {return x->nall(); }  ///< vector space dimension


//SwigBind inline void vscal(double a, PtrA<DArray1> x) { vscal(a,PtrCast<Vector>(x)); }///< vector routine,see vscal(double,Ptr<Vector>)
//SwigBind inline void vscal(double a, PtrA<DArray2> x) { vscal(a,PtrCast<Vector>(x)); }///< vector routine, see vscal(double,Ptr<Vector>)
//        inline void vscal(double a, PtrA<DArray3> x) { vscal(a,PtrCast<Vector>(x)); }///< vector routine, see vscal(double,Ptr<Vector>)
//        inline void vscal(double a, PtrA<DArray4> x) { vscal(a,PtrCast<Vector>(x)); }///< vector routine, see vscal(double,Ptr<Vector>)
//        inline void vscal(double a, PtrA<DArray5> x) { vscal(a,PtrCast<Vector>(x)); }///< vector routine, see vscal(double,Ptr<Vector>)
//
//
//SwigBind inline void vcopy(PtrA<DArray1> x, PtrA<DArray1> y) { vcopy(PtrCast<Vector>(x),PtrCast<Vector>(y)); }///< vector routine, see vcopy(Ptr<Vector>,Ptr<Vector>)
//SwigBind inline void vcopy(PtrA<DArray2> x, PtrA<DArray2> y) { vcopy(PtrCast<Vector>(x),PtrCast<Vector>(y)); }///< vector routine, see vcopy(Ptr<Vector>,Ptr<Vector>)
//        inline void vcopy(PtrA<DArray3> x, PtrA<DArray3> y) { vcopy(PtrCast<Vector>(x),PtrCast<Vector>(y)); }///< vector routine, see vcopy(Ptr<Vector>,Ptr<Vector>)
//        inline void vcopy(PtrA<DArray4> x, PtrA<DArray4> y) { vcopy(PtrCast<Vector>(x),PtrCast<Vector>(y)); }///< vector routine, see vcopy(Ptr<Vector>,Ptr<Vector>)
//        inline void vcopy(PtrA<DArray5> x, PtrA<DArray5> y) { vcopy(PtrCast<Vector>(x),PtrCast<Vector>(y)); }///< vector routine, see vcopy(Ptr<Vector>,Ptr<Vector>)
//
//
//SwigBind inline void vset(double a, PtrA<DArray1> x) { vset(a,PtrCast<Vector>(x)); }///< vector routine, see vset(double,Ptr<Vector>)    
//SwigBind inline void vset(double a, PtrA<DArray2> x) { vset(a,PtrCast<Vector>(x)); }///< vector routine, see vset(double,Ptr<Vector>)   
//        inline void vset(double a, PtrA<DArray3> x) { vset(a,PtrCast<Vector>(x)); }///< vector routine, see vset(double,Ptr<Vector>)   
//        inline void vset(double a, PtrA<DArray4> x) { vset(a,PtrCast<Vector>(x)); }///< vector routine, see vset(double,Ptr<Vector>)   
//        inline void vset(double a, PtrA<DArray5> x) { vset(a,PtrCast<Vector>(x)); }///< vector routine, see vset(double,Ptr<Vector>)   
//
//SwigBind inline void vaxpy(double a, PtrA<DArray1> x, PtrA<DArray1> y) { vaxpy(a,PtrCast<Vector>(x),PtrCast<Vector>(y)); } ///vector routine, see vaxpy(double,Ptr<Vector>,Ptr<Vector>)
//SwigBind inline void vaxpy(double a, PtrA<DArray2> x, PtrA<DArray2> y) { vaxpy(a,PtrCast<Vector>(x),PtrCast<Vector>(y)); } ///vector routine, see vaxpy(double,Ptr<Vector>,Ptr<Vector>)
//        inline void vaxpy(double a, PtrA<DArray3> x, PtrA<DArray3> y) { vaxpy(a,PtrCast<Vector>(x),PtrCast<Vector>(y)); } ///vector routine, see vaxpy(double,Ptr<Vector>,Ptr<Vector>)
//        inline void vaxpy(double a, PtrA<DArray4> x, PtrA<DArray4> y) { vaxpy(a,PtrCast<Vector>(x),PtrCast<Vector>(y)); } ///vector routine, see vaxpy(double,Ptr<Vector>,Ptr<Vector>)
//        inline void vaxpy(double a, PtrA<DArray5> x, PtrA<DArray5> y) { vaxpy(a,PtrCast<Vector>(x),PtrCast<Vector>(y)); } ///vector routine, see vaxpy(double,Ptr<Vector>,Ptr<Vector>)
//
//SwigBind inline double vamax(PtrA<DArray1> x) { return vamax(PtrCast<Vector>(x)); } ///vector routine, see vamax(Ptr<Vector>)
//SwigBind inline double vamax(PtrA<DArray2> x) { return vamax(PtrCast<Vector>(x)); } ///vector routine, see vamax(Ptr<Vector>)
//        inline double vamax(PtrA<DArray3> x) { return vamax(PtrCast<Vector>(x)); } ///vector routine, see vamax(Ptr<Vector>)
//        inline double vamax(PtrA<DArray4> x) { return vamax(PtrCast<Vector>(x)); } ///vector routine, see vamax(Ptr<Vector>)
//        inline double vamax(PtrA<DArray5> x) { return vamax(PtrCast<Vector>(x)); } ///vector routine, see vamax(Ptr<Vector>)
//
//SwigBind inline double vnm2(PtrA<DArray1> x) { return vnm2(PtrCast<Vector>(x)); } ///vector routine, see vnm2(double,Ptr<Vector>,Ptr<Vector>)
//SwigBind inline double vnm2(PtrA<DArray2> x) { return vnm2(PtrCast<Vector>(x)); } ///vector routine, see vnm2(double,Ptr<Vector>,Ptr<Vector>)
//        inline double vnm2(PtrA<DArray3> x) { return vnm2(PtrCast<Vector>(x)); } ///vector routine, see vnm2(double,Ptr<Vector>,Ptr<Vector>)
//        inline double vnm2(PtrA<DArray4> x) { return vnm2(PtrCast<Vector>(x)); } ///vector routine, see vnm2(double,Ptr<Vector>,Ptr<Vector>)
//        inline double vnm2(PtrA<DArray5> x) { return vnm2(PtrCast<Vector>(x)); } ///vector routine, see vnm2(double,Ptr<Vector>,Ptr<Vector>)

//SwigBind inline double vdot(PtrA<DArray1> x, PtrA<DArray1> y) { return vdot(PtrCast<Vector>(x),PtrCast<Vector>(y)).real(); }  ///vector routine, see vdot(Ptr<Vector>,Ptr<Vector>) 
//SwigBind inline double vdot(PtrA<DArray2> x, PtrA<DArray2> y) { return vdot(PtrCast<Vector>(x),PtrCast<Vector>(y)).real(); }  ///vector routine, see vdot(Ptr<Vector>,Ptr<Vector>) 
//        inline double vdot(PtrA<DArray3> x, PtrA<DArray3> y) { return vdot(PtrCast<Vector>(x),PtrCast<Vector>(y)).real(); }  ///vector routine, see vdot(Ptr<Vector>,Ptr<Vector>) 
//        inline double vdot(PtrA<DArray4> x, PtrA<DArray4> y) { return vdot(PtrCast<Vector>(x),PtrCast<Vector>(y)).real(); }  ///vector routine, see vdot(Ptr<Vector>,Ptr<Vector>) 
//        inline double vdot(PtrA<DArray5> x, PtrA<DArray5> y) { return vdot(PtrCast<Vector>(x),PtrCast<Vector>(y)).real(); }  ///vector routine, see vdot(Ptr<Vector>,Ptr<Vector>) 
//
//SwigBind inline PtrA<DArray1> vclone(PtrA<DArray1> x) { return PtrCast<DArray1>(vclone(PtrCast<Vector>(x))); }   ///vector routine, see vclone(Ptr<Vector>) 
//SwigBind inline PtrA<DArray2> vclone(PtrA<DArray2> x) { return PtrCast<DArray2>(vclone(PtrCast<Vector>(x))); }   ///vector routine, see vclone(Ptr<Vector>) 
//        inline PtrA<DArray3> vclone(PtrA<DArray3> x) { return PtrCast<DArray3>(vclone(PtrCast<Vector>(x))); }   ///vector routine, see vclone(Ptr<Vector>) 
//        inline PtrA<DArray4> vclone(PtrA<DArray4> x) { return PtrCast<DArray4>(vclone(PtrCast<Vector>(x))); }   ///vector routine, see vclone(Ptr<Vector>) 
//        inline PtrA<DArray5> vclone(PtrA<DArray5> x) { return PtrCast<DArray5>(vclone(PtrCast<Vector>(x))); }   ///vector routine, see vclone(Ptr<Vector>) 
//
//SwigBind inline void vlin1(PtrA<DArray1> x0, double a, PtrA<DArray1> x, PtrA<DArray1> y) { vlin1(PtrCast<Vector>(x0),a,PtrCast<Vector>(x),PtrCast<Vector>(y)); } /// vector routine, see vlin1(Ptr<Vector>,double,Ptr<Vector>,Ptr<Vector>)
//SwigBind inline void vlin1(PtrA<DArray2> x0, double a, PtrA<DArray2> x, PtrA<DArray2> y) { vlin1(PtrCast<Vector>(x0),a,PtrCast<Vector>(x),PtrCast<Vector>(y)); }/// vector routine, see vlin1(Ptr<Vector>,double,Ptr<Vector>,Ptr<Vector>)
//        inline void vlin1(PtrA<DArray3> x0, double a, PtrA<DArray3> x, PtrA<DArray3> y) { vlin1(PtrCast<Vector>(x0),a,PtrCast<Vector>(x),PtrCast<Vector>(y)); }/// vector routine, see vlin1(Ptr<Vector>,double,Ptr<Vector>,Ptr<Vector>)
//        inline void vlin1(PtrA<DArray4> x0, double a, PtrA<DArray4> x, PtrA<DArray4> y) { vlin1(PtrCast<Vector>(x0),a,PtrCast<Vector>(x),PtrCast<Vector>(y)); }/// vector routine, see vlin1(Ptr<Vector>,double,Ptr<Vector>,Ptr<Vector>)
//        inline void vlin1(PtrA<DArray5> x0, double a, PtrA<DArray5> x, PtrA<DArray5> y) { vlin1(PtrCast<Vector>(x0),a,PtrCast<Vector>(x),PtrCast<Vector>(y)); }/// vector routine, see vlin1(Ptr<Vector>,double,Ptr<Vector>,Ptr<Vector>)
//
//SwigBind inline void vlin2(PtrA<DArray1> x0,double a1,PtrA<DArray1> x1,double a2,PtrA<DArray1> x2,PtrA<DArray1> y) { vlin2(PtrCast<Vector>(x0),a1,PtrCast<Vector>(x1),a2,PtrCast<Vector>(x2),PtrCast<Vector>(y)); } /// vector routine, see vlin2(Ptr<Vector>,double,Ptr<Vector>,double,Ptr<Vector>,Ptr<Vector>)
//SwigBind inline void vlin2(PtrA<DArray2> x0,double a1,PtrA<DArray2> x1,double a2,PtrA<DArray2> x2,PtrA<DArray2> y) { vlin2(PtrCast<Vector>(x0),a1,PtrCast<Vector>(x1),a2,PtrCast<Vector>(x2),PtrCast<Vector>(y)); } /// vector routine, see vlin2(Ptr<Vector>,double,Ptr<Vector>,double,Ptr<Vector>,Ptr<Vector>)
//        inline void vlin2(PtrA<DArray3> x0,double a1,PtrA<DArray3> x1,double a2,PtrA<DArray3> x2,PtrA<DArray3> y) { vlin2(PtrCast<Vector>(x0),a1,PtrCast<Vector>(x1),a2,PtrCast<Vector>(x2),PtrCast<Vector>(y)); } /// vector routine, see vlin2(Ptr<Vector>,double,Ptr<Vector>,double,Ptr<Vector>,Ptr<Vector>)
//        inline void vlin2(PtrA<DArray4> x0,double a1,PtrA<DArray4> x1,double a2,PtrA<DArray4> x2,PtrA<DArray4> y) { vlin2(PtrCast<Vector>(x0),a1,PtrCast<Vector>(x1),a2,PtrCast<Vector>(x2),PtrCast<Vector>(y)); } /// vector routine, see vlin2(Ptr<Vector>,double,Ptr<Vector>,double,Ptr<Vector>,Ptr<Vector>)
//        inline void vlin2(PtrA<DArray5> x0,double a1,PtrA<DArray5> x1,double a2,PtrA<DArray5> x2,PtrA<DArray5> y) { vlin2(PtrCast<Vector>(x0),a1,PtrCast<Vector>(x1),a2,PtrCast<Vector>(x2),PtrCast<Vector>(y)); } /// vector routine, see vlin2(Ptr<Vector>,double,Ptr<Vector>,double,Ptr<Vector>,Ptr<Vector>)


SwigBind inline void da1scal(double a, PtrA<DArray1> x) { vscal(a,PtrCast<Vector>(x)); }///< vector routine,see vscal(double,Ptr<Vector>)
SwigBind inline void da2scal(double a, PtrA<DArray2> x) { vscal(a,PtrCast<Vector>(x)); }///< vector routine, see vscal(double,Ptr<Vector>)
SwigBind inline void da1copy(PtrA<DArray1> x, PtrA<DArray1> y) { vcopy(PtrCast<Vector>(x),PtrCast<Vector>(y)); }///< vector routine, see vcopy(Ptr<Vector>,Ptr<Vector>)
SwigBind inline void da2copy(PtrA<DArray2> x, PtrA<DArray2> y) { vcopy(PtrCast<Vector>(x),PtrCast<Vector>(y)); }///< vector routine, see vcopy(Ptr<Vector>,Ptr<Vector>)
SwigBind inline void da1set(double a, PtrA<DArray1> x) { vset(a,PtrCast<Vector>(x)); }///< vector routine, see vset(double,Ptr<Vector>)    
SwigBind inline void da2set(double a, PtrA<DArray2> x) { vset(a,PtrCast<Vector>(x)); }///< vector routine, see vset(double,Ptr<Vector>)   
SwigBind inline void da1axpy(double a, PtrA<DArray1> x, PtrA<DArray1> y) { vaxpy(a,PtrCast<Vector>(x),PtrCast<Vector>(y)); } ///vector routine, see vaxpy(double,Ptr<Vector>,Ptr<Vector>)
SwigBind inline void da2axpy(double a, PtrA<DArray2> x, PtrA<DArray2> y) { vaxpy(a,PtrCast<Vector>(x),PtrCast<Vector>(y)); } ///vector routine, see vaxpy(double,Ptr<Vector>,Ptr<Vector>)
SwigBind inline double da1amax(PtrA<DArray1> x) { return vamax(PtrCast<Vector>(x)); } ///vector routine, see vamax(Ptr<Vector>)
SwigBind inline double da2amax(PtrA<DArray2> x) { return vamax(PtrCast<Vector>(x)); } ///vector routine, see vamax(Ptr<Vector>)
SwigBind inline double da1nm2(PtrA<DArray1> x) { return vnm2(PtrCast<Vector>(x)); } ///vector routine, see vnm2(double,Ptr<Vector>,Ptr<Vector>)
SwigBind inline double da2nm2(PtrA<DArray2> x) { return vnm2(PtrCast<Vector>(x)); } ///vector routine, see vnm2(double,Ptr<Vector>,Ptr<Vector>)
//SwigBind inline double da1dot(PtrA<DArray1> x, PtrA<DArray1> y) { return vdot(PtrCast<Vector>(x),PtrCast<Vector>(y)).real(); }  ///vector routine, see vdot(Ptr<Vector>,Ptr<Vector>) 
//SwigBind inline double da2dot(PtrA<DArray2> x, PtrA<DArray2> y) { return vdot(PtrCast<Vector>(x),PtrCast<Vector>(y)).real(); }  ///vector routine, see vdot(Ptr<Vector>,Ptr<Vector>) 
SwigBind inline void da1lin1(PtrA<DArray1> x0, double a, PtrA<DArray1> x, PtrA<DArray1> y) { vlin1(PtrCast<Vector>(x0),a,PtrCast<Vector>(x),PtrCast<Vector>(y)); } /// vector routine, see vlin1(Ptr<Vector>,double,Ptr<Vector>,Ptr<Vector>)
SwigBind inline void da2lin1(PtrA<DArray2> x0, double a, PtrA<DArray2> x, PtrA<DArray2> y) { vlin1(PtrCast<Vector>(x0),a,PtrCast<Vector>(x),PtrCast<Vector>(y)); }/// vector routine, see vlin1(Ptr<Vector>,double,Ptr<Vector>,Ptr<Vector>)
SwigBind inline void da1lin2(PtrA<DArray1> x0,double a1,PtrA<DArray1> x1,double a2,PtrA<DArray1> x2,PtrA<DArray1> y) { vlin2(PtrCast<Vector>(x0),a1,PtrCast<Vector>(x1),a2,PtrCast<Vector>(x2),PtrCast<Vector>(y)); } /// vector routine, see vlin2(Ptr<Vector>,double,Ptr<Vector>,double,Ptr<Vector>,Ptr<Vector>)
SwigBind inline void da2lin2(PtrA<DArray2> x0,double a1,PtrA<DArray2> x1,double a2,PtrA<DArray2> x2,PtrA<DArray2> y) { vlin2(PtrCast<Vector>(x0),a1,PtrCast<Vector>(x1),a2,PtrCast<Vector>(x2),PtrCast<Vector>(y)); } /// vector routine, see vlin2(Ptr<Vector>,double,Ptr<Vector>,double,Ptr<Vector>,Ptr<Vector>)


SwigBind inline void ia1set(double a, PtrA<IArray1> x) { vset(a,PtrCast<Vector>(x)); }///< vector routine, see vset(double,Ptr<Vector>)    
SwigBind inline void ia2set(double a, PtrA<IArray2> x) { vset(a,PtrCast<Vector>(x)); }///< vector routine, see vset(double,Ptr<Vector>)   

//SwigBind inline int vdim(PtrA<DArray1> x) {return x->nall(); }  ///< vector space dimension 
//SwigBind inline int vdim(PtrA<DArray2> x) {return x->nall(); }  ///< vector space dimension
//SwigBind inline int vdim(PtrA<ZArray1> x) {return x->nall(); }  ///< vector space dimension
//SwigBind inline int vdim(PtrA<ZArray2> x) {return x->nall(); }  ///< vector space dimension
//SwigBind inline int vdim(PtrA<IArray1> x) {return x->nall(); }  ///< vector space dimension
//SwigBind inline int vdim(PtrA<IArray2> x) {return x->nall(); }  ///< vector space dimension


//SwigBind inline void vscal(double a, PtrA<DArray1> x) { vscal(a,PtrCast<Vector>(x)); }///< vector routine,see vscal(double,Ptr<Vector>)
//SwigBind inline void vscal(double a, PtrA<DArray2> x) { vscal(a,PtrCast<Vector>(x)); }///< vector routine, see vscal(double,Ptr<Vector>)
//        inline void vscal(double a, PtrA<DArray3> x) { vscal(a,PtrCast<Vector>(x)); }///< vector routine, see vscal(double,Ptr<Vector>)
//        inline void vscal(double a, PtrA<DArray4> x) { vscal(a,PtrCast<Vector>(x)); }///< vector routine, see vscal(double,Ptr<Vector>)
//        inline void vscal(double a, PtrA<DArray5> x) { vscal(a,PtrCast<Vector>(x)); }///< vector routine, see vscal(double,Ptr<Vector>)


//SwigBind inline void vcopy(PtrA<DArray1> x, PtrA<DArray1> y) { vcopy(PtrCast<Vector>(x),PtrCast<Vector>(y)); }///< vector routine, see vcopy(Ptr<Vector>,Ptr<Vector>)
//SwigBind inline void vcopy(PtrA<DArray2> x, PtrA<DArray2> y) { vcopy(PtrCast<Vector>(x),PtrCast<Vector>(y)); }///< vector routine, see vcopy(Ptr<Vector>,Ptr<Vector>)
//        inline void vcopy(PtrA<DArray3> x, PtrA<DArray3> y) { vcopy(PtrCast<Vector>(x),PtrCast<Vector>(y)); }///< vector routine, see vcopy(Ptr<Vector>,Ptr<Vector>)
//        inline void vcopy(PtrA<DArray4> x, PtrA<DArray4> y) { vcopy(PtrCast<Vector>(x),PtrCast<Vector>(y)); }///< vector routine, see vcopy(Ptr<Vector>,Ptr<Vector>)
//        inline void vcopy(PtrA<DArray5> x, PtrA<DArray5> y) { vcopy(PtrCast<Vector>(x),PtrCast<Vector>(y)); }///< vector routine, see vcopy(Ptr<Vector>,Ptr<Vector>)


//SwigBind inline void vset(double a, PtrA<DArray1> x) { vset(a,PtrCast<Vector>(x)); }///< vector routine, see vset(double,Ptr<Vector>)    
//SwigBind inline void vset(double a, PtrA<DArray2> x) { vset(a,PtrCast<Vector>(x)); }///< vector routine, see vset(double,Ptr<Vector>)   
//        inline void vset(double a, PtrA<DArray3> x) { vset(a,PtrCast<Vector>(x)); }///< vector routine, see vset(double,Ptr<Vector>)   
//        inline void vset(double a, PtrA<DArray4> x) { vset(a,PtrCast<Vector>(x)); }///< vector routine, see vset(double,Ptr<Vector>)   
//        inline void vset(double a, PtrA<DArray5> x) { vset(a,PtrCast<Vector>(x)); }///< vector routine, see vset(double,Ptr<Vector>)   
//
//SwigBind inline void vaxpy(double a, PtrA<DArray1> x, PtrA<DArray1> y) { vaxpy(a,PtrCast<Vector>(x),PtrCast<Vector>(y)); } ///vector routine, see vaxpy(double,Ptr<Vector>,Ptr<Vector>)
//SwigBind inline void vaxpy(double a, PtrA<DArray2> x, PtrA<DArray2> y) { vaxpy(a,PtrCast<Vector>(x),PtrCast<Vector>(y)); } ///vector routine, see vaxpy(double,Ptr<Vector>,Ptr<Vector>)
//        inline void vaxpy(double a, PtrA<DArray3> x, PtrA<DArray3> y) { vaxpy(a,PtrCast<Vector>(x),PtrCast<Vector>(y)); } ///vector routine, see vaxpy(double,Ptr<Vector>,Ptr<Vector>)
//        inline void vaxpy(double a, PtrA<DArray4> x, PtrA<DArray4> y) { vaxpy(a,PtrCast<Vector>(x),PtrCast<Vector>(y)); } ///vector routine, see vaxpy(double,Ptr<Vector>,Ptr<Vector>)
//        inline void vaxpy(double a, PtrA<DArray5> x, PtrA<DArray5> y) { vaxpy(a,PtrCast<Vector>(x),PtrCast<Vector>(y)); } ///vector routine, see vaxpy(double,Ptr<Vector>,Ptr<Vector>)
//
//SwigBind inline double vamax(PtrA<DArray1> x) { return vamax(PtrCast<Vector>(x)); } ///vector routine, see vamax(Ptr<Vector>)
//SwigBind inline double vamax(PtrA<DArray2> x) { return vamax(PtrCast<Vector>(x)); } ///vector routine, see vamax(Ptr<Vector>)
//        inline double vamax(PtrA<DArray3> x) { return vamax(PtrCast<Vector>(x)); } ///vector routine, see vamax(Ptr<Vector>)
//        inline double vamax(PtrA<DArray4> x) { return vamax(PtrCast<Vector>(x)); } ///vector routine, see vamax(Ptr<Vector>)
//        inline double vamax(PtrA<DArray5> x) { return vamax(PtrCast<Vector>(x)); } ///vector routine, see vamax(Ptr<Vector>)
//
//SwigBind inline double vnm2(PtrA<DArray1> x) { return vnm2(PtrCast<Vector>(x)); } ///vector routine, see vnm2(double,Ptr<Vector>,Ptr<Vector>)
//SwigBind inline double vnm2(PtrA<DArray2> x) { return vnm2(PtrCast<Vector>(x)); } ///vector routine, see vnm2(double,Ptr<Vector>,Ptr<Vector>)
//        inline double vnm2(PtrA<DArray3> x) { return vnm2(PtrCast<Vector>(x)); } ///vector routine, see vnm2(double,Ptr<Vector>,Ptr<Vector>)
//        inline double vnm2(PtrA<DArray4> x) { return vnm2(PtrCast<Vector>(x)); } ///vector routine, see vnm2(double,Ptr<Vector>,Ptr<Vector>)
//        inline double vnm2(PtrA<DArray5> x) { return vnm2(PtrCast<Vector>(x)); } ///vector routine, see vnm2(double,Ptr<Vector>,Ptr<Vector>)
//
//SwigBind inline double vdot(PtrA<DArray1> x, PtrA<DArray1> y) { return vdot(PtrCast<Vector>(x),PtrCast<Vector>(y)).real(); }  ///vector routine, see vdot(Ptr<Vector>,Ptr<Vector>) 
//SwigBind inline double vdot(PtrA<DArray2> x, PtrA<DArray2> y) { return vdot(PtrCast<Vector>(x),PtrCast<Vector>(y)).real(); }  ///vector routine, see vdot(Ptr<Vector>,Ptr<Vector>) 
//        inline double vdot(PtrA<DArray3> x, PtrA<DArray3> y) { return vdot(PtrCast<Vector>(x),PtrCast<Vector>(y)).real(); }  ///vector routine, see vdot(Ptr<Vector>,Ptr<Vector>) 
//        inline double vdot(PtrA<DArray4> x, PtrA<DArray4> y) { return vdot(PtrCast<Vector>(x),PtrCast<Vector>(y)).real(); }  ///vector routine, see vdot(Ptr<Vector>,Ptr<Vector>) 
//        inline double vdot(PtrA<DArray5> x, PtrA<DArray5> y) { return vdot(PtrCast<Vector>(x),PtrCast<Vector>(y)).real(); }  ///vector routine, see vdot(Ptr<Vector>,Ptr<Vector>) 
//
//SwigBind inline PtrA<DArray1> vclone(PtrA<DArray1> x) { return PtrCast<DArray1>(vclone(PtrCast<Vector>(x))); }   ///vector routine, see vclone(Ptr<Vector>) 
//SwigBind inline PtrA<DArray2> vclone(PtrA<DArray2> x) { return PtrCast<DArray2>(vclone(PtrCast<Vector>(x))); }   ///vector routine, see vclone(Ptr<Vector>) 
//        inline PtrA<DArray3> vclone(PtrA<DArray3> x) { return PtrCast<DArray3>(vclone(PtrCast<Vector>(x))); }   ///vector routine, see vclone(Ptr<Vector>) 
//        inline PtrA<DArray4> vclone(PtrA<DArray4> x) { return PtrCast<DArray4>(vclone(PtrCast<Vector>(x))); }   ///vector routine, see vclone(Ptr<Vector>) 
//        inline PtrA<DArray5> vclone(PtrA<DArray5> x) { return PtrCast<DArray5>(vclone(PtrCast<Vector>(x))); }   ///vector routine, see vclone(Ptr<Vector>) 
//
//SwigBind inline void vlin1(PtrA<DArray1> x0, double a, PtrA<DArray1> x, PtrA<DArray1> y) { vlin1(PtrCast<Vector>(x0),a,PtrCast<Vector>(x),PtrCast<Vector>(y)); } /// vector routine, see vlin1(Ptr<Vector>,double,Ptr<Vector>,Ptr<Vector>)
//SwigBind inline void vlin1(PtrA<DArray2> x0, double a, PtrA<DArray2> x, PtrA<DArray2> y) { vlin1(PtrCast<Vector>(x0),a,PtrCast<Vector>(x),PtrCast<Vector>(y)); }/// vector routine, see vlin1(Ptr<Vector>,double,Ptr<Vector>,Ptr<Vector>)
//        inline void vlin1(PtrA<DArray3> x0, double a, PtrA<DArray3> x, PtrA<DArray3> y) { vlin1(PtrCast<Vector>(x0),a,PtrCast<Vector>(x),PtrCast<Vector>(y)); }/// vector routine, see vlin1(Ptr<Vector>,double,Ptr<Vector>,Ptr<Vector>)
//        inline void vlin1(PtrA<DArray4> x0, double a, PtrA<DArray4> x, PtrA<DArray4> y) { vlin1(PtrCast<Vector>(x0),a,PtrCast<Vector>(x),PtrCast<Vector>(y)); }/// vector routine, see vlin1(Ptr<Vector>,double,Ptr<Vector>,Ptr<Vector>)
//        inline void vlin1(PtrA<DArray5> x0, double a, PtrA<DArray5> x, PtrA<DArray5> y) { vlin1(PtrCast<Vector>(x0),a,PtrCast<Vector>(x),PtrCast<Vector>(y)); }/// vector routine, see vlin1(Ptr<Vector>,double,Ptr<Vector>,Ptr<Vector>)

//SwigBind inline void vlin2(PtrA<DArray1> x0,double a1,PtrA<DArray1> x1,double a2,PtrA<DArray1> x2,PtrA<DArray1> y) { vlin2(PtrCast<Vector>(x0),a1,PtrCast<Vector>(x1),a2,PtrCast<Vector>(x2),PtrCast<Vector>(y)); } /// vector routine, see vlin2(Ptr<Vector>,double,Ptr<Vector>,double,Ptr<Vector>,Ptr<Vector>)
//SwigBind inline void vlin2(PtrA<DArray2> x0,double a1,PtrA<DArray2> x1,double a2,PtrA<DArray2> x2,PtrA<DArray2> y) { vlin2(PtrCast<Vector>(x0),a1,PtrCast<Vector>(x1),a2,PtrCast<Vector>(x2),PtrCast<Vector>(y)); } /// vector routine, see vlin2(Ptr<Vector>,double,Ptr<Vector>,double,Ptr<Vector>,Ptr<Vector>)
//        inline void vlin2(PtrA<DArray3> x0,double a1,PtrA<DArray3> x1,double a2,PtrA<DArray3> x2,PtrA<DArray3> y) { vlin2(PtrCast<Vector>(x0),a1,PtrCast<Vector>(x1),a2,PtrCast<Vector>(x2),PtrCast<Vector>(y)); } /// vector routine, see vlin2(Ptr<Vector>,double,Ptr<Vector>,double,Ptr<Vector>,Ptr<Vector>)
//        inline void vlin2(PtrA<DArray4> x0,double a1,PtrA<DArray4> x1,double a2,PtrA<DArray4> x2,PtrA<DArray4> y) { vlin2(PtrCast<Vector>(x0),a1,PtrCast<Vector>(x1),a2,PtrCast<Vector>(x2),PtrCast<Vector>(y)); } /// vector routine, see vlin2(Ptr<Vector>,double,Ptr<Vector>,double,Ptr<Vector>,Ptr<Vector>)
//        inline void vlin2(PtrA<DArray5> x0,double a1,PtrA<DArray5> x1,double a2,PtrA<DArray5> x2,PtrA<DArray5> y) { vlin2(PtrCast<Vector>(x0),a1,PtrCast<Vector>(x1),a2,PtrCast<Vector>(x2),PtrCast<Vector>(y)); } /// vector routine, see vlin2(Ptr<Vector>,double,Ptr<Vector>,double,Ptr<Vector>,Ptr<Vector>)


////////////////////////////////////////////////////
////////////////////////////////////////////////////
// 14. Dense linear algebra extension


struct LinAlgDenseExt : public Extension
{
  typedef bool (*func_type)(const TArray2<double>& a2, const TArray1<double>& v1, TArray1<double>& w1);
  
  func_type solve_func;
  func_type multi_func;
  
  LinAlgDenseExt() : solve_func(NULL), multi_func(NULL) {}
  
  virtual Ptr<Extension> clone() const { return new LinAlgDenseExt(*this); }
  virtual ~LinAlgDenseExt() {}
};

extern LinAlgDenseExt default_LinAlgDenseExt;

template<class T> inline
PtrA<TArray1<T> > TArray2<T>::operator*(const PtrA<TArray1<T> >& x)
{ 
  Ptr<LinAlgDenseExt> la=this->ext.template get_extension<LinAlgDenseExt>();
  
  if(!la || !la->multi_func) return 0;
  
  PtrA<TArray1<double> > y=new TArray1<T>(this->nrow());
  
  if(!la->multi_func(*this,x,y))
   return 0;
  
  return y;
}

template<class T> inline
PtrA<TArray1<T> > TArray1<T>::operator/(const PtrA<TArray2<T> >& A)
{
  Ptr<LinAlgDenseExt> la=A->ext.template get_extension<LinAlgDenseExt>();
  
  if(!la || !la->solve_func) return 0;
  
  PtrA<TArray1<double> > x=new TArray1<double>(A->ncol());
  
  if(!la->solve_func(A,*this,x))
   return 0;
  
  return x;
}





/*------------------------------------------------------------------*/

class DA_LU_decom
{
  const DArray2& A;
  const int dim;
  DArray1 scales;
  IArray1 I;
  DArray2 L;
  bool decom_ok;
  
  bool decom();
  
 public:
  
  DA_LU_decom(const DArray2& A);
  
  bool is_decom_ok() const { return decom_ok; }
  
  bool solve(const DArray1& y, DArray1& x);
};

SwigBind PtrA<DArray2> da2Inverse(PtrA<DArray2> A);

#endif






