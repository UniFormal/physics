/**
   \file partitioning.h
   \brief Array Partitioning for Parallel Processing.
   \author	J.Fuhrmann
 */

#ifndef PARTITIONING_H
#define PARTITIONING_H
#include "array.h"
/**
   \brief  Partition Data type.

*/

/// Partioning describes array partitioning conforming
/// to parallel grid calculations

#define pFirstPart(p,icol) (p->_color_partition->v[icol])
#define pLastPart(p,icol) (p->_color_partition->v[icol+1]-1)
class Partitioning
{
 public:  
  /// Size of partitioned index range.
  const int ndof;  

  /// Number of partitions.
  const int npart; 

  /// Number of colors of partition graph.
  /// It is assumed that matrix assembly can be run in parallel on
  /// partitions with the same color.
  const int ncol;  
  
  /// First color of  partitioning
  inline int firstcolor() {return  1; };
  
  /// Last color of partitioning
  inline int lastcolor() {return ncol;};
  
  /// First partition of given color
  inline int firstpart(const int icol) const { return _color_partition->v[icol];};

  /// Last partition of given color
  inline int lastpart(const int icol) const { return  _color_partition->v[icol+1]-1;};

  /// First dof of partition
  inline int firstdof(int ipart) { return IA1(_partition_dof,ipart);};

  /// Last dof of partition
  inline int lastdof(int ipart) { return IA1(_partition_dof,ipart+1)-1;};

  /// Create colored partitioning from grid/space information
  Partitioning(int nd, Ptr<IArray1> cp, Ptr<IArray1> pd);
  
  /// Create partitioning with trivial coloring (by default, all partions get the same color)
  Partitioning(int nd, Ptr<IArray1> pd, bool same_color=true);
  
  /// Create trivial partitioning (all nodes in one partition)
  Partitioning(int nd);
  
  /// Create equidistant  partitioning (all partitions of approx one size)
  Partitioning(int nd, int np, bool same_color=true);
  
  /// Blow up partitioning according to block size
  Partitioning(Ptr<Partitioning> p0, int bs);


  /// return color partition array
  inline  Ptr<IArray1> color_partition(void) {return _color_partition;}

  /// return partition_dof array
  inline  Ptr<IArray1> partition_dof(void) {return _partition_dof;}

  /// CloneCopy
  Ptr<Partitioning> clone_copy();

  /// Coloring of partition graph, for layout see access routines below.
  PtrA<IArray1> _color_partition;
  
  /// List of partitions of dof range, for layout see access routines below.
  PtrA<IArray1> _partition_dof;


};




#endif



