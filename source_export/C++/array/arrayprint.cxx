/* done by subst_insert_configh.sh */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif



#include "arrayprint.h"
#include "stdsys/dprintf.h"

#ifndef MAX
#define MAX(x,y) ((x)>(y) ? (x):(y))
#endif
#ifndef MIN
#define MIN(x,y) ((x)<(y) ? (x):(y))
#endif

using namespace std;


/*------------------------------------------------------------------*/
bool static_array_print_test_raw=false;
bool static_array_print_test_val=false;

ArrayFormatD::ArrayFormatD(const double * const da_)
  : vd(da_)
{
  strcpy(fdata_,"%+11e");

  char s[256];
  sprintf(s,fdata_,123.4);
  dlen = strlen(s);
}

void ArrayFormatD::data_print(int idx)
{
  printf(fdata_,vd[idx]);
}

int  ArrayFormatD::data_length()
{
  return dlen;
}

/*------------------------------------------------------------------*/

ArrayFormatZ::ArrayFormatZ(const double_complex * const da_)
  : vd(da_)
{
  strcpy(fdata_,"(%+8e,%+8e)");

  char s[256];
  sprintf(s,fdata_,123.4);
  dlen = strlen(s);
}

void ArrayFormatZ::data_print(int idx)
{
  printf(fdata_,vd[idx].real(),vd[idx].imag());
}

int  ArrayFormatZ::data_length()
{
  return dlen;
}

/*------------------------------------------------------------------*/


ArrayFormatI::ArrayFormatI(const int * const da_)
  : vd(da_)
{
  strcpy(fdata_,"%11d");

  char s[256];
  sprintf(s,fdata_,123);
  dlen = strlen(s);
}

void ArrayFormatI::data_print(int idx)
{
  printf(fdata_,vd[idx]);
}

int  ArrayFormatI::data_length()
{
  return dlen;
}

/*------------------------------------------------------------------*/


ArrayFormatB::ArrayFormatB(const char * const da_)
  : vd(da_)
{
  strcpy(fdata_,"%3d");

  char s[256];
  sprintf(s,fdata_,123);
  dlen = strlen(s);
}

void ArrayFormatB::data_print(int idx)
{
  printf(fdata_,vd[idx]);
}

int  ArrayFormatB::data_length()
{
  return dlen;
}




template<typename T>
void generic_print_block( const TArray<T> * a, const Ptr<ArrayFormat> & af, const char*name, int numcol, const int low[], const int high[] )
{
  const void * obj_addr = a;
  void * buf_addr = a->v;
  int size = a->memsize();
  const MemAccess2 & acs = a->acs;
  const cTypePerformer<T> & tp = a->tp;

  assert(1<=acs.dim && acs.dim<=2);

  int acs_n[] = { -1,acs.n1,acs.n2,acs.n3,acs.n4,acs.n5 };
  int acs_m[] = { -1,acs.m1,acs.m2,acs.m3,acs.m4,acs.m5 };


  int i[6];
  int lline;

  /* determine the line length */
  lline = 0;
  lline += af->data_length()*numcol;
  lline += af->index_length()*acs.dim;
  lline += strlen(af->fdatasep)*(numcol-1);
  lline += strlen(af->findexsep)*(acs.dim-1)+strlen(af->findexbeg)+strlen(af->findexend);
  lline -= strlen(af->fcomment);
  if (lline<40) lline=40;

  /* print array name and some info */

  printf(af->fcomment); for(int j=1;j<=lline;j+=strlen(af->flinesep)) printf(af->flinesep);
  printf("\n");
  if (acs.dim==1)
    printf("%s%sArray%1d(%d): %s\n",af->fcomment,tp.vtype_str, acs.dim, acs.n1, name);
  else if (acs.dim==2)
    printf("%s%sArray%1d(%d,%d): %s \n",af->fcomment,tp.vtype_str, acs.dim, acs.n1, acs.n2, name);
  printf("%sptr=%p  val=%p mem=%5.2f%s\n",af->fcomment,obj_addr,buf_addr,
	 (size<1024.0) ? size: (size<1048576.0) ? size/1024.0 : size/1048576.0,
	 (size<1024.0) ? "B" : (size<1048576.0) ? "KB"        : "MB"
	 );
//printf("%s  aclass= %p\n",af->fcomment,aclass);
//  printf("%s  aclass= ...\n",af->fcomment);

  printf(af->fcomment); for(int j=1;j<=lline;j+=strlen(af->flinesep)) printf(af->flinesep);

  /* print the data *********************************************************/

  int k=acs.offset;
  int lo[6],hi[6];
  for(int d=1; d<=acs.dim; d++) {
    lo[d]=low[d];
    hi[d]=high[d];
    i[d]=lo[d]=MAX(1,lo[d]), hi[d]=MIN(acs_n[d],hi[d]), k+=acs_m[d]*i[d];
  }

  int icol=1;
  while(i[1]<=hi[1])
  {
    if(icol==1){
      printf("\n");
      if(static_array_print_test_raw)
      {
        printf("PDELIB-TEST-RAW: ");
      } 
      else  if(static_array_print_test_val)
      {
        printf("PDELIB-TEST-VAL: ");
      } 
      else
      {
        printf(af->findexbeg);
        for(int d=1; d< acs.dim; d++) { printf(af->findex,i[d]); printf(af->findexsep);}
        printf(af->findex,i[acs.dim]);
        printf(af->findexend);
      }
    }

    // printf(af->fdata(),val()[k]);
    af->data_print(k);

    if(icol<numcol) { printf(af->fdatasep); icol++; }
    else            { icol=1; }

    i[acs.dim]++; // j+=acs_m[acs.dim];
    k++;
    for(int d=acs.dim; d>1; d--)
      if(i[d]>hi[d])
	{
	  i[d]=lo[d];
	  i[d-1]++;
	  k+=acs_m[d-1]-(hi[d]-lo[d]+1)*acs_m[d];
	  if((hi[d]>lo[d])&& af->fwrap) icol=1;
	}
  }
  printf("\n");
  printf(af->fcomment); for(int d=1;d<=lline;d+=strlen(af->flinesep)) printf(af->flinesep);
  printf("\n");
}



template void generic_print_block<double>( const TArray<double> * a, const Ptr<ArrayFormat> & af, const char*name, int numcol, const int low[], const int high[] );
template void generic_print_block<double_complex>( const TArray<double_complex> * a, const Ptr<ArrayFormat> & af, const char*name, int numcol, const int low[], const int high[] );
template void generic_print_block<int>( const TArray<int> * a, const Ptr<ArrayFormat> & af, const char*name, int numcol, const int low[], const int high[] );
template void generic_print_block<char>( const TArray<char> * a, const Ptr<ArrayFormat> & af, const char*name, int numcol, const int low[], const int high[] );

