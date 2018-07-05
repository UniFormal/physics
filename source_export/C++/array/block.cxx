#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


#include "block.h"


void set_block_from_table_of_ndof(BlockInfo& BI, const PtrA<IArray1>& Op_ndof_vector)
{
  const int nblock=Op_ndof_vector->nall();
  
  BI.resize_block(nblock);
  
  BI.block_dof0(1)=0;
  for(int b=1;b<=nblock;++b)
  {
    int ndofs=(*Op_ndof_vector)(b);
    
    BI.block_dof0(b+1)=BI.block_dof0(b)+ndofs;
  }
}



