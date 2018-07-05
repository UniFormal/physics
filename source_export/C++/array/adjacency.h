/**
   \file array/adjacency.h

   \brief pdelib2 adjacency structure

   \author Juergen Fuhrmann
   

*/


#ifndef ADJACENCY_H
#define ADJACENCY_H


#include "array.h"

/**
   \brief Adjacency pattern data structure.

   We distinguish two types of  adjacency patterns: one with fixed row
   size, and one with variable rowsize. In the case of fixed row size,
   we can omit the ia field, speeding up access and saving memory.
   
   Moreover, this case allows to cast the value array to a 2D array.

   All of the manipualation routines  take as arguments
   adjacencies with both fixed and variable row
   lengths.

*/

struct Adjacency
{
  int n;       /**< Number of rows of adjacency matrix */
  int m;       /**< Number of columns of adjacency matrix */ 
  int nja;     /**< length of ja field = number of non-zero elements
                    of adjacency matrix*/
  int dja;     /**< if >0, denote fixed row length*/

  PtrA<IArray1> ia;    /**< row pointer (NULL if dja>0) */
  PtrA<IArray1> ja;    /**< column index */

  int rows_sorted; /**< rows have been sorted */
  
  Adjacency() : dja(0) {}
}; 


/**
   \brief Create empty pattern 
*/

Ptr<Adjacency>  adCreateEmpty(void); 

/**
   \brief Create pattern with fixed rowsize containing no data 
*/
Ptr<Adjacency>  adCreateF(int n, int m,int rowsize);


/** 
    \brief Create pattern with variable rowsize containing no data 
 */
Ptr<Adjacency>  adCreateV(int n, int m,int nja);

/**
   \brief Create pattern with fixed rowsize containing data of \p a.
   
  Each row of the two-dimensional array a becomes a row
  of the adjacency pattern. 
*/
Ptr<Adjacency>  adCreateFA2(PtrA<IArray2>  a,int m);


/**
   \brief Create pattern with fixed rowsize containing data of  \p a.
   
  Each element of the  array a becomes a row of length 1
  of the adjacency pattern. 
*/
Ptr<Adjacency>  adCreateFA1(PtrA<IArray1>  a,int m);

/**
   \brief Create adjacency pattern from \p ia \p ja pair.
*/
Ptr<Adjacency>  adCreateIAJA(PtrA<IArray1> ia, PtrA<IArray1> ja, int m);

/**
   \brief Create adjacency pattern with empty CRS arrays
*/
Ptr<Adjacency> adCreate(int n, int m);

/**
   \brief Begin assembly of adjacency
   
   Internally create interim arrays for assembly
*/
void adBeginAssembly(Ptr<Adjacency> adj);


/**
   \brief Set entry IJ in Adjaceny
*/

void adSetIJ(const Ptr<Adjacency>& adj, int i, int j);
/**
   \brief End assembly of adjacency
   Set up proper CRS ia/ja structure, remove interim arrays
*/
void adEndAssembly(Ptr<Adjacency> adj);


/**
   \brief Create ia field of adjacency
*/
void adCreateIA(Ptr<Adjacency>  p , int nia);

/**
   \brief Create ja field of adjacency
*/
void adCreateJA(Ptr<Adjacency>  p, int nja);


/**
   \brief Print out adjacency data. 
*/
void adPrint(Ptr<Adjacency> adj);

/**
   \brief Print \p m x \p n pattern matrix.
*/
void adPrintPat(Ptr<Adjacency> adj);


/**
   \brief Transpose  pattern. 

   \return Variable row length pattern.
*/
Ptr<Adjacency>  adTranspose(Ptr<Adjacency>  p);

/**
   \brief Create pattern of product.

    Generally, the product  of two adjacency matrices, unlike its factors,
    may have other integer entries than just 0 and 1.
    
    This routine creates the pattern matrix of the product,
    whose entries are one where the product is non-zero.
   \return Variable row length pattern.
*/
Ptr<Adjacency>  adMultiply(Ptr<Adjacency>  pa, Ptr<Adjacency>  pb);

/**
   \brief Create upper tri part of product.
   \return Variable row length pattern.
*/
Ptr<Adjacency>  adUMultiply(Ptr<Adjacency>  pa, Ptr<Adjacency>  pb);

/** \brief Create n-pattern of product.
    
    Generally, the product  of two adjacency matrices, unlike its factors,
    may have other integer entries than just 0 and 1.

    This routine creates the n-pattern matrix of the product,
    whose entries are one where the entry of the product is \p n,
    and zero otherwise.
    
   \return Variable row length pattern.
*/
Ptr<Adjacency>  adPnMultiply(Ptr<Adjacency>  pa, Ptr<Adjacency>  pb, int n);


/**
   \brief Create blowup pattern of adjacency.
   By the blowup patternm we understand the 
   adjacency pattern of a matrix  containing nonzero  
   \p nblock x\p nblock matrices  in each place where the
   pattern \p is non-zero.
*/
Ptr<Adjacency>  adBlowup(Ptr<Adjacency> p, int nblock);

/** 
    \brief Create index of diagonal elements .
*/
PtrA<IArray1>  adCreateDiagonalIndex(Ptr<Adjacency>  p);

/** 
    \brief Sort rows in direction of growing indices 
*/
void adSortRows(Ptr<Adjacency>  p);

/** \brief Return integer pointer to the \p ia field data.
*/
inline int*  adIAVal(Ptr<Adjacency> a) { return ia1Val((a)->ia);};

/** \brief Return integer pointer to the \p ja field data.
*/
inline int* adJAVal(Ptr<Adjacency> a) {return ia1Val((a)->ja);};

/** \brief Return \p i-th entry of ia field 
*/
inline int & adIA(Ptr<Adjacency> a, int i) { return IA1((a)->ia,i);}

/** \brief Return \p i-th entry of ja field 
*/
inline int & adJA(Ptr<Adjacency> a, int i) { return IA1((a)->ja,i);}

/** \brief Return number of rows of adjacency matrix 
*/
inline int adN(Ptr<Adjacency> a) { return a->n;};

/** \brief Return number of columns of adjacency matrix 
*/
inline int adM(Ptr<Adjacency> a) { return a->m;}


/**\brief  Return row length in case of fixed row length pattern
*/
inline int adGetRowLengthF(Ptr<Adjacency> pat, int ir) { return (pat->dja);}

/**\brief  Return pointer to row in case of fixed row length pattern
*/
inline int* adGetRowF(Ptr<Adjacency> pat, int ir)  {return      (&pat->ja->v[((ir)-1)*pat->dja]);}

/**\brief  Return row length in case of fixed variable length pattern
*/
inline int adGetRowLengthV(Ptr<Adjacency> pat, int ir) {return (pat->ia->v[(ir)+1]-pat->ia->v[(ir)]);};

/**\brief  Return pointer to row in case of variable row length pattern
*/
inline int *adGetRowV(Ptr<Adjacency> pat, int ir)     {return   (&pat->ja->v[pat->ia->v[(ir)]-1]);};


/**\brief  Return row length in case of any pattern
*/
inline int   adGetRowLength(Ptr<Adjacency> p, int i) { return (p->dja?adGetRowLengthF(p,i):adGetRowLengthV(p,i));}

/**
   \brief  Return pointer to row in case of any type pattern
*/
inline int* adGetRow(Ptr<Adjacency> p, int i) {return  (p->dja?adGetRowF(p,i):adGetRowV(p,i));}

/**
   \brief Entry reference in case of any type pattern
 */
inline int& adEntry(Ptr<Adjacency> ad, int i, int j)   {return adGetRow(ad,i)[j];}

/**
   \brief Entry reference in case of fixed row length pattern
 */
inline int& adEntryF(Ptr<Adjacency> ad, int i, int j) {return adGetRowF(ad,i)[j];}

/**
   \brief Entry reference in case of variable row length pattern
 */
inline int& adEntryV(Ptr<Adjacency> ad, int i, int j) {return adGetRowF(ad,i)[j];}


/**
   \brief Clone copy constructor
 */
Ptr<Adjacency> adCloneCopy(Ptr<Adjacency> adjIn);

/**
   \brief Cast adjacency to 2D array of size nrows * maxrowlength.
   Missing entries are filled by zero.
*/
   
PtrA<IArray2> adDesparsify(Ptr<Adjacency> ad);

#endif
