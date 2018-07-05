/**
   \file array_block.h
   \brief Block structure extension for array.
   \author T.Streckenbach

 \section array_blockIntro Associate BlockInfo with array.
 
*/
#ifndef ARRAY_BLOCK_H
#define ARRAY_BLOCK_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "block.h"


/**
 Global logical block structure of an array used for BlockRect.
*/
struct ArrayBlockInfo : public BlockInfoBase
{
  BlockInfo H;
  
  ArrayBlockInfo() {}
  
  ArrayBlockInfo(int nblocks, int ndof_per_block)
  {
    set_block_H(nblocks,ndof_per_block);
  }
  
  void set_block_H(int nblocks, int ndof_per_block)
  {
    H.set_block(nblocks,ndof_per_block);
  }
  
  virtual Ptr<Extension> clone() const;
  virtual ~ArrayBlockInfo() {}
};


Ptr<ArrayBlockInfo> ArrayBlock(const DArray1& a);

Ptr<ArrayBlockInfo> ArrayBlock(const IArray1& a);

Ptr<ArrayBlockInfo> ArrayBlockForce(const DArray1& a);

Ptr<ArrayBlockInfo> ArrayBlockForce(const IArray1& a);

Ptr<ArrayBlockInfo> ArrayBlockForceAdd(DArray1& a);

Ptr<ArrayBlockInfo> ArrayBlockForceAdd(IArray1& a);




#endif
