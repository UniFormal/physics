#include "partitioning.h"
#include "stdsys/memory.h"


Partitioning::Partitioning(int nd, Ptr<IArray1> cp, Ptr<IArray1> pd)
  :ndof(nd),
   npart(ia1NAll(pd)-1),
   ncol(ia1NAll(cp)-1),
   _color_partition(cp), 
   _partition_dof(pd)
{
};


Partitioning::Partitioning(int nd, Ptr<IArray1> pd, bool same_color)
  :ndof(nd),
   npart(ia1NAll(pd)-1),
   ncol(same_color?1:(ia1NAll(pd)-1)),
   _partition_dof(pd)
{
  _color_partition=new IArray1(ncol+1);
  if (same_color)
    {
      IA1(_color_partition,1)=1;
      IA1(_color_partition,2)=npart;
    }
  else
    {
      IA1(_color_partition,1)=1;
      for (int i=1; i<=npart; i++)
	IA1(_color_partition,i+1)=i+1;
    }
};

Partitioning::Partitioning(int nd)
  :ndof(nd),
   npart(1),
   ncol(1)
{
  _color_partition=new IArray1(2);
  IA1(_color_partition,1)=1;
  IA1(_color_partition,2)=npart+1;

  _partition_dof=new IArray1(2);
  IA1(_partition_dof,1)=1;
  IA1(_partition_dof,2)=ndof+1;
};

/// Create eqidistant  partitioning (all partitions of approx one size)
Partitioning::Partitioning(int nd, int np, bool same_color)
  :ndof(nd),
   npart(np),
   ncol(same_color?1:np)
{
  _color_partition=new IArray1(ncol+1);
  if (same_color)
    {
      IA1(_color_partition,1)=1;
      IA1(_color_partition,2)=npart;
    }
  else
    {
      IA1(_color_partition,1)=1;
      for (int i=1; i<=npart; i++)
	IA1(_color_partition,i+1)=i+1;
    }
  

  _partition_dof=new IArray1(npart+1);
  int *data=ia1Val(_partition_dof);
  /* First write sizes into partition data array*/
  /* Create equal sizes */
  int nx=ndof/npart;
  for(int i=1;i<=npart;i++)
    data[i+1]=nx;


  /* Normally, nx is positive, so we need to 
     increase the first partition sizes */
  nx=nd-nx*npart;
  for (int i=1;i<=nx;i++)
    data[i+1]++;
  
  /* Calculate actual table from sizes */
  data[1]=1;
  for (int i=1;i<=npart;i++)
    data[i+1]=data[i+1]+data[i];
}

// Blow up partitioning according to block size
Partitioning::Partitioning(Ptr<Partitioning> p, int bs)
  :ndof(p->ndof*bs),
   npart(p->npart),
   ncol(p->ncol),
   _color_partition(p->_color_partition)
{
  
  if (bs==1) 
    {
      _partition_dof=p->_partition_dof;
      return;
    }
  _partition_dof=new IArray1(npart+1);
  int *bdata=ia1Val(_partition_dof);
  int *data=ia1Val(p->_partition_dof);
  for (int i=1;i<=p->npart+1;i++)
    bdata[i]=bs*(data[i]-1)+1;


}



Ptr<Partitioning> Partitioning::clone_copy()
{
  return new Partitioning(ndof,aCloneCopy(_color_partition),aCloneCopy(_partition_dof));
}

