/**
   \file array.h
   \brief Array API.
   \author T.Streckenbach


*/

#ifndef ARRAYPRINT_H
#define ARRAYPRINT_H

#include <stdio.h>
#include <string.h>

#include "stdcxx/ptr.h"
#include "stdcxx/mem.h"
#include "stdcxx/memacs.h"

#include "array/array.h"


/*------------------------------------------------------------------*/

struct ArrayFormat
{
 char flinesep [50];
 char fcomment [50];
 char findexbeg[50];
 char findex   [50];
 char findexsep[50];
 char findexend[50];
 char fdatasep [50];  
 int  fwrap;
 
 int ilen;
 
 ArrayFormat()
 {
#if defined _WIN32 && !defined MINGW
	strcpy_s(flinesep ,"--");
    strcpy_s(fcomment ,"# ");
    strcpy_s(findexbeg,"[ ");
    strcpy_s(findex   ,"%5d");
    strcpy_s(findexsep,", ");
    strcpy_s(findexend," ] ");
    strcpy_s(fdatasep ,"  ");
#else
    strcpy(flinesep ,"--");
    //    strcpy(fcomment ,"# ");
    strcpy(fcomment ,"");
    //    strcpy(findexbeg,"[ ");
    strcpy(findexbeg,"(");
    strcpy(findex   ,"%5d");
    strcpy(findexsep,", ");
    //    strcpy(findexend," ] ");
    strcpy(findexend,") ");
    strcpy(fdatasep ," ");
#endif
    fwrap=1;
    
    char s[256];
#if defined _WIN32 && !defined MINGW
	sprintf_s(s,findex,123);
#else
    sprintf(s,findex,123);
#endif
    ilen=strlen(s);
 }
 virtual ~ArrayFormat() { }
 
 virtual void data_print(int idx) = 0 ;
 virtual int  data_length() = 0 ;
 
 
 int index_length() const { return ilen; }
};


/*------------------------------------------------------------------*/

class ArrayFormatD : public ArrayFormat
{
   char fdata_[50];
   int  dlen;
   const double * const vd;
   
   public:
   
   ArrayFormatD(const double * const da_);
   virtual void data_print(int idx);
   virtual int  data_length();
};

class ArrayFormatZ : public ArrayFormat
{
   char fdata_[50];
   int  dlen;
   const double_complex * const vd;
   
   public:
   
   ArrayFormatZ(const double_complex * const da_);
   virtual void data_print(int idx);
   virtual int  data_length();
};

class ArrayFormatI : public ArrayFormat
{
   char fdata_[50];
   int  dlen;
   const int * const vd;
   
   public:
   
   ArrayFormatI(const int * const da_);
   virtual void data_print(int idx);
   virtual int  data_length();
};

class ArrayFormatB : public ArrayFormat
{
   char fdata_[50];
   int  dlen;
   const char * const vd;
   
   public:
   
   ArrayFormatB(const char * const da_);
   virtual void data_print(int idx);
   virtual int  data_length();
};

template<typename T> void generic_print_block( const TArray<T> * a, const Ptr<ArrayFormat> & af, const char*name, int numcol, const int low[], const int high[] );
extern bool static_array_print_test_raw;
extern bool static_array_print_test_val;


#endif




