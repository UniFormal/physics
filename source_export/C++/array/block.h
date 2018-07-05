/**
   \file block.h
   \brief Basic block structure.
   \author T.Streckenbach

 \section blockIntro Basic block structure ..
 
*/
#ifndef BLOCK_H
#define BLOCK_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "stdsys/swigbind.h"
#include "stdcxx/ptrx.h"

#include "array.h"


/**
 Global logical block structure.
*/
struct BlockInfo
{
  IArray1 block_dof0;
  
  BlockInfo() : block_dof0(1) { block_dof0(1)=0; }
  
  int NBlock() const { return block_dof0.nall()-1; }
  
  int NDOF(int block0, int nblock) const { return block_dof0(block0+nblock)-block_dof0(block0); }
  
  int NDOF(int block) const { return NDOF(block,1); }
  
  int TotalNumberOfDOF() const { return block_dof0(NBlock()+1); }
  
  void set_block(int nblocks, int ndof_per_block)
  {
    resize_block(nblocks);
    for(int b=1;b<=nblocks+1;++b)
     block_dof0(b)=(b-1)*ndof_per_block;
  }
  
  void resize_block(int nblocks)
  {
    if(NBlock()!=nblocks)
    {
      size_t nn[]={0, (size_t)(nblocks+1)};
      block_dof0.resize(nn);
    }
    
    block_dof0.set0();
    block_dof0(1)=-12346;
  }
  
  void copy(const BlockInfo& o)
  {
    resize_block(o.NBlock());
    block_dof0.copy_v(o.block_dof0);
  }
};


void set_block_from_table_of_ndof(BlockInfo& BI, const PtrA<IArray1>& Op_ndof_vector);


struct BlockInfoBase : public Extension
{
  virtual Ptr<Extension> clone() const { assert(0); }
  virtual ~BlockInfoBase() {}
};









/**
 Block access convention.
 
 Maps a matrix (block) access to memory (vector) access.
 
 \param nblocks matrix width
 \param block matrix column
 \param dof matrix row
*/
inline int DOFInBlockMatrix_(int nblocks, int block, int dof)
{
  return nblocks * (dof-1) + block;
}

inline int NOfBlockMatrix(int nblocks, int block, int n)
{
  return (n-block) / nblocks + 1;
}




SwigBindBegin


/**
 Block access convention.
 
 Foundation structute for simultanous assembling of numerous equations
 in one system.
 
 The members W,H define the dimension of the hole block and the
 values i0,i1,j0,j1 are a selection of a block part.
*/
class BlockRect
{
  bool transposed;
  
 public:
  
  BlockInfo H, W; ///< Dimension of the block.
  int h_,w_; ///< Dimension of the selected block part.
  int i0     ///< Row index of block part
    , j0;    ///< Column index of block part

  
  IArray1 block_i0_h;
  IArray1 block_j0_w;
  
  void print() const
  {
    printf("BlockRect.transposed: %i\n",int(transposed));
    printf("BlockRect.h         : %i\n",h_);
    printf("BlockRect.w         : %i\n",w_);
    printf("BlockRect.i0        : %i\n",i0);
    printf("BlockRect.H         : \n");
    H.block_dof0.print();
    printf("BlockRect.W         : \n");
    W.block_dof0.print();
    printf("BlockRect.block_i0_h: \n");
    block_i0_h.print();
    printf("BlockRect.block_j0_w: \n");
    block_j0_w.print();
  }
  
  void resize_hw()
  {
    size_t nn[]={0, 0};
    if(block_i0_h.nall()!=h_) { nn[1]=h_; block_i0_h.resize(nn); } block_i0_h.set0(); block_i0_h(1)=-12346;
    if(block_j0_w.nall()!=w_) { nn[1]=w_; block_j0_w.resize(nn); } block_j0_w.set0(); block_j0_w(1)=-12346;
  }
  
  /**
   Empty block of size 0.
  */
  BlockRect()
  : block_i0_h(0)
  , block_j0_w(0)
  {
    i0=1;
    j0=1;
    w_=h_=0;
    transposed=false;
  }
  
  /**
   Default, block part is the hole block.
  */
  BlockRect(const BlockInfo& BH, const BlockInfo& BW)
  : block_i0_h(0)
  , block_j0_w(0)
  {
    set_HW(BH,BW);
    set_all();
    transposed=false;
  }
  
  BlockRect(const BlockRect& o)
  : block_i0_h(0)
  , block_j0_w(0)
  {
    transposed=o.transposed;
    H.copy(o.H);
    W.copy(o.W);
    h_=o.h_;
    w_=o.w_;
    i0=o.i0;
    j0=o.j0;
    
    resize_hw();
    
    block_i0_h.copy_v(o.block_i0_h);
    block_j0_w.copy_v(o.block_j0_w);
  }
  
  /**
   Valid if the block dimesions are not zero.
  */
  bool is_valid() const { return (w_>0 && h_>0); }
  
  bool empty() const { return w()<=0 || h()<=0; }
  
  bool all_selected() const { return (w()==W.NBlock() && h()==H.NBlock()); }
  
  void set_all()
  {
    i0=1;
    j0=1;
    h_=H.NBlock();
    w_=W.NBlock();
    resize_hw();
  }
  
  void set_H(const BlockInfo& B)
  {
    H.copy(B);
    W.set_block(1,0);
  }
  
  int H_NDOF_i0(int d) const { return H.NDOF(i0+d-1); }
  int W_NDOF_j0(int d) const { return W.NDOF(j0+d-1); }
  
  void set_HW(const BlockInfo& B)
  {
    H.copy(B);
    W.copy(B);
  }
  
  void set_HW(const BlockInfo& BH, const BlockInfo& BW)
  {
    H.copy(BH);
    W.copy(BW);
  }
  
  void set_hw(int hw_)
  {
    h_=hw_;
    w_=hw_;
    resize_hw();
  }
  
  void set_hw(int h, int w)
  {
    h_=h;
    w_=w;
    resize_hw();
  }
  
  void set_w(int ww)
  {
    h_=1;
    w_=ww;
    resize_hw();
  }
  
  void set_h(int hh)
  {
    h_=hh;
    w_=1;
    resize_hw();
  }
  
  void set_ij(int i, int j)
  {
    i0 = i;
    j0 = j;
  }
  
  void set_i(int i)
  {
    i0 = i;
  }
  
  void set_j(int j)
  {
    j0 = j;
  }
  
  int h() const { return h_; }
  
  int w() const { return w_; }
  
  void transpose_hw(bool b)
  {
    if(b==transposed) return;
    transposed=b;
    
    int swap=h_;
    h_=w_;
    w_=swap;
    
    resize_hw();
  }
  
  #if 0
  int dof_h(int block, int i) const { return DOFInBlockMatrix_(h_,block,i); }
  int dof_w(int block, int j) const { return DOFInBlockMatrix_(w_,block,j); }
  
  int dof_H(int block, int i) const { return DOFInBlockMatrix_(H.NBlock(),block+i0-1,i); }
  int dof_W(int block, int j) const { return DOFInBlockMatrix_(W.NBlock(),block+j0-1,j); }
  
  void set_block_i0_h(int ndof_h) { }
  void set_block_j0_w(int ndof_w) { }
  void set_block_i0j0_hw(int ndof_hw) { }
  
  #else
  int dof_h(int block, int i) const { set_block_assert1(); return block_i0_h(block)+i; }
  int dof_w(int block, int j) const { set_block_assert2(); return block_j0_w(block)+j; }
  
  int dof_H(int block, int i) const { set_block_assert3(); return H.block_dof0(block+i0-1)+i; }
  int dof_W(int block, int j) const { set_block_assert4(); return W.block_dof0(block+j0-1)+j; }
  // '+i0-1' and '+j0-1' may also go to set_block_i0_H() and set_block_j0_W(), if it is
  // ensured that i0,j0 will not be changed after.
  
  void set_block_assert1() const
  {
    #if PDELIB_VARIANT_DBG
    int BlockRect_set_block_not_done=0;
    if(!block_i0_h.nall()) BlockRect_set_block_not_done=11;
    assert(!BlockRect_set_block_not_done);
    
    if(0>block_i0_h(1)) BlockRect_set_block_not_done=21;
    assert(!BlockRect_set_block_not_done);
    
    if(block_i0_h.nall()>1 && 0==block_i0_h(block_i0_h.nall())) BlockRect_set_block_not_done=1;
    assert(!BlockRect_set_block_not_done);
    #endif
  }
  void set_block_assert2() const
  {
    #if PDELIB_VARIANT_DBG
    int BlockRect_set_block_not_done=0;
    if(!block_j0_w.nall()) BlockRect_set_block_not_done=12;
    assert(!BlockRect_set_block_not_done);
    
    if(0>block_j0_w(1)) BlockRect_set_block_not_done=22;
    assert(!BlockRect_set_block_not_done);
    
    if(block_j0_w.nall()>1 && 0==block_j0_w(block_j0_w.nall())) BlockRect_set_block_not_done=2;
    assert(!BlockRect_set_block_not_done);
    #endif
  }
  void set_block_assert3() const
  {
    #if PDELIB_VARIANT_DBG
    int BlockRect_set_block_not_done=0;
    if(!H.NBlock()) BlockRect_set_block_not_done=13;
    assert(!BlockRect_set_block_not_done);
    
    if(0>H.block_dof0(1)) BlockRect_set_block_not_done=23;
    assert(!BlockRect_set_block_not_done);
    
    if(H.NBlock()>1 && 0==H.block_dof0(H.NBlock())) BlockRect_set_block_not_done=3;
    assert(!BlockRect_set_block_not_done);
    #endif
  }
  void set_block_assert4() const
  {
    #if PDELIB_VARIANT_DBG
    int BlockRect_set_block_not_done=0;
    if(!W.NBlock()) BlockRect_set_block_not_done=14;
    assert(!BlockRect_set_block_not_done);
    
    if(0>W.block_dof0(1)) BlockRect_set_block_not_done=24;
    assert(!BlockRect_set_block_not_done);
    
    if(W.NBlock()>1 && 0==W.block_dof0(W.NBlock())) BlockRect_set_block_not_done=4;
    assert(!BlockRect_set_block_not_done);
    #endif
  }
  
  void set_block_i0_h(int ndof_h) { for(int b=1;b<=h_;++b) block_i0_h(b)=(b-1)*ndof_h; }
  void set_block_j0_w(int ndof_w) { for(int b=1;b<=w_;++b) block_j0_w(b)=(b-1)*ndof_w; }
  void set_block_i0j0_hw(int ndof_hw)
  {
    set_block_i0_h(ndof_hw);
    set_block_j0_w(ndof_hw);
  }
  
  #endif
};
SwigBindEnd
SwigBindPtr(BlockRect)

SwigBind inline Ptr<BlockRect> feBlock_hi(int h, int i) { Ptr<BlockRect> blk=new BlockRect(); blk->set_h( h); blk->set_i (i  ); return blk; }
SwigBind inline Ptr<BlockRect> feBlock_i (int i       ) { Ptr<BlockRect> blk=new BlockRect(); blk->set_h( 1); blk->set_i (i  ); return blk; }
SwigBind inline Ptr<BlockRect> feBlock_ij(int i, int j) { Ptr<BlockRect> blk=new BlockRect(); blk->set_hw(1); blk->set_ij(i,j); return blk; }




















#endif

