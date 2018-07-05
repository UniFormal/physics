
#include "array_block.h"




Ptr<Extension> ArrayBlockInfo::clone() const
{
  ArrayBlockInfo* ab=new ArrayBlockInfo();
  ab->H.resize_block(H.NBlock());
  ab->H.block_dof0.copy_v(H.block_dof0);
  return ab;
}



Ptr<ArrayBlockInfo> ArrayBlock(const DArray1& a)
{ ErrorCallStack ecs("ArrayBlock(a)");
  if(!a.ext.get_extension<ArrayBlockInfo>()) { ErrorMsg("femlib","missing ArrayBlockInfo a.block (param 1)"); return 0; }
  Ptr<ArrayBlockInfo> ab=a.ext.get_extension<ArrayBlockInfo>();
  return ab;
}

Ptr<ArrayBlockInfo> ArrayBlock(const IArray1& a)
{ ErrorCallStack ecs("ArrayBlock(a)");
  if(!a.ext.get_extension<ArrayBlockInfo>()) { ErrorMsg("femlib","missing ArrayBlockInfo a.block (param 1)"); return 0; }
  Ptr<ArrayBlockInfo> ab=a.ext.get_extension<ArrayBlockInfo>();
  return ab;
}

Ptr<ArrayBlockInfo> ArrayBlockForce(const DArray1& a)
{
  Ptr<ArrayBlockInfo> V_block=a.ext.get_extension<ArrayBlockInfo>();
  if(!V_block)
      V_block=new ArrayBlockInfo(1,a.nall());
  return V_block;
}

Ptr<ArrayBlockInfo> ArrayBlockForce(const IArray1& a)
{
  Ptr<ArrayBlockInfo> V_block=a.ext.get_extension<ArrayBlockInfo>();
  if(!V_block)
      V_block=new ArrayBlockInfo(1,a.nall());
  return V_block;
}

Ptr<ArrayBlockInfo> ArrayBlockForceAdd(DArray1& a)
{
  Ptr<ArrayBlockInfo> V_block=a.ext.get_extension<ArrayBlockInfo>();
  if(!V_block)
      V_block=new ArrayBlockInfo(1,a.nall());
  a.ext.add_extension(PtrCast<Extension>(V_block));
  return V_block;
}

Ptr<ArrayBlockInfo> ArrayBlockForceAdd(IArray1& a)
{
  Ptr<ArrayBlockInfo> V_block=a.ext.get_extension<ArrayBlockInfo>();
  if(!V_block)
      V_block=new ArrayBlockInfo(1,a.nall());
  a.ext.add_extension(PtrCast<Extension>(V_block));
  return V_block;
}

  