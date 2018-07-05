/* done by subst_insert_configh.sh */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "array.h"

// need plain "free" here
#undef free 


static void delete_array_double(void *mem, size_t sz)
{
  delete[] (double*)mem;
}
static void delete_array_int(void *mem, size_t sz)
{
  delete[] (int*)mem;
}

static void free_array_double(void *mem, size_t sz)
{
  free(mem);
}
static void free_array_int(void *mem, size_t sz)
{
  free(mem);
}


PtrA<DArray1> da1AttachXDelete(int n1, double *v, int offset)
{
  return new DArray1(n1,v,offset,delete_array_double);
}

PtrA<DArray2> da2AttachXDelete(int n1, int n2, double *v, int offset)
{
  return new DArray2(n1,n2,v,offset,delete_array_double);
}


PtrA<IArray1> ia1AttachXDelete(int n1,int *v, int offset)
{
  return new IArray1(n1,v,offset,delete_array_int);
}

PtrA<IArray2> ia2AttachXDelete(int n1, int n2, int *v, int offset)
{
  return new IArray2(n1,n2,v,offset,delete_array_int);
}




PtrA<DArray1> da1AttachXFree(int n1, double *v, int offset)
{
  return new DArray1(n1,v,offset,free_array_double);
}

PtrA<DArray2> da2AttachXFree(int n1, int n2, double *v, int offset)
{
  return new DArray2(n1,n2,v,offset,free_array_double);
}


PtrA<IArray1> ia1AttachXFree(int n1,int *v, int offset)
{
  return new IArray1(n1,v,offset,free_array_int);
}

PtrA<IArray2> ia2AttachXFree(int n1, int n2, int *v, int offset)
{
  return new IArray2(n1,n2,v,offset,free_array_int);
}


