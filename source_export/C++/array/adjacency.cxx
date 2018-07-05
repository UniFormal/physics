/* done by subst_insert_configh.sh */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdlib.h>
#include "adjacency.h"
#include "stdsys/memory.h"



/* we need this to allow to retrieve indices @ EOLoop not used */
#define FILL 0

void adCreateIA(Ptr<Adjacency>  p , int nia)
{
  p->ia=ia1Create(nia+FILL);
}
void adCreateJA(Ptr<Adjacency>  p, int nja)
{
  p->ja=ia1Create(nja+FILL);
}

Ptr<Adjacency>  adCreateEmpty(void)
{
  Ptr<Adjacency> a = new Adjacency();
  a->rows_sorted=0;
  return a;
}

Ptr<Adjacency>  adCreateF(int n, int m,int rowsize)
{
  Ptr<Adjacency>  p=adCreateEmpty();
  p->n=n;
  p->m=m;
  p->dja=rowsize;
  p->nja=p->n*p->dja;
  adCreateJA(p,p->nja);
  p->ia=NULL;
  return p;
}

Ptr<Adjacency>  adCreateV(int n, int m,int nja)
{
  Ptr<Adjacency>  p=adCreateEmpty();
  p->n=n;
  p->m=m;
  p->dja=0;
  p->nja=nja;
  adCreateIA(p,p->n+1);
  adCreateJA(p,p->nja);
  return p;
}


Ptr<Adjacency>  adCreateFA2(PtrA<IArray2>  a, int m)
{
  Ptr<Adjacency>  p=adCreateEmpty();
  p->n=ia2NRow(a);
  p->m=m;
  p->dja=ia2NCol(a);
  p->ja=ia2All(a);
  p->nja=ia1NAll(p->ja);
  return p;
}

Ptr<Adjacency>  adCreateFA1(PtrA<IArray1>  a, int m)
{
  Ptr<Adjacency>  p=adCreateEmpty();
  p->n=ia1NAll(a);
  p->m=m;
  p->dja=1;
  p->ja=a;
  p->nja=ia1NAll(p->ja);
  return p;
}

Ptr<Adjacency>  adCreateIAJA(PtrA<IArray1> ia, PtrA<IArray1> ja, int m)
{
  Ptr<Adjacency>  p=adCreateEmpty();
  p->n=ia1NAll(ia)-1;
  p->m=m;
  p->dja=0;
  p->ia=ia;
  p->ja=ja;
  p->nja=ia1NAll(p->ja);
  return p;
}

Ptr<Adjacency> adCreate(int n, int m)
{
  Ptr<Adjacency>  p=adCreateEmpty();
  p->n=n;
  p->m=m;
  p->nja=0;
  return p;
}

void adBeginAssembly(Ptr<Adjacency> ad)
{
  int i;
  assert(ad->n);
  assert(ad->m);
  assert(!(ad->ia));
  assert(!(ad->ja));
  assert(ad->nja==0);

  ad->nja=ad->n+1;
  ad->ia=ia1Create(2*ad->n);
  ad->ja=ia1Create(2*ad->n);
  for(i=1;i<=ad->n;i++)
    {
      ad->ia->v[i]= -i;
      ad->ja->v[i]= i;
    }
}

void adSetIJ(const Ptr<Adjacency>& ad, int i, int j)
{
  int k,k0=0; 
  for (k=i; k>0; k0=k, k=IA1(ad->ia,k))
    if (IA1(ad->ja,k)==j) {k0=0; break;}
 if (k0) 
   {
     ia1Insert(ad->ja,ad->nja,j);
     ia1Insert(ad->ia,ad->nja,-i);
     ia1Insert(ad->ia,k0,ad->nja);
     ad->nja++;
   }
}

void adEndAssembly(Ptr<Adjacency> ad)
{
  int i,j,k;

  PtrA<IArray1> ia=ia1Create(ad->n+1);
  PtrA<IArray1> ja=ia1Create(ad->nja-1);
  
  /* Move from extended CRS format to CRS format */
  j=1;
  for(i=1;i<=ad->n;i++)
    {
      IA1(ia,i)=j;
      for (k=i; k>0; k=IA1(ad->ia,k),j++)
	{
	  IA1(ja,j)=IA1(ad->ja,k);
	}
    }
  IA1(ia,i)=j;
  assert(j==ad->nja);


  ad->nja=ad->nja-1;
  ad->ia=ia;
  ad->ja=ja;

}


void adPrint(Ptr<Adjacency> ad)
{
  int i,j;
  int *row;
#define SEP "----------------------------------------------------------------------------\n"
  printf(SEP);
  printf("ad= %p ia=%p ja=%p\n",ad.ptr(),ad->ia.ptr(),ad->ja.ptr());
  printf("n=%d m=%d nja=%d dja=%d\n",
	 ad->n,ad->m,ad->nja,ad->dja);
  for (i=1;i<=ad->n;i++)
    {
      printf("%4d:",i);
      row=adGetRow(ad,i);
      for (j=1;j<=adGetRowLength(ad,i);j++)
	printf(" %d",row[j]);
      printf("\n");
    }
  printf(SEP);
}

/* TODO: test_strip_ia: test if we can replace ia by dja... */
Ptr<Adjacency>  adTranspose(Ptr<Adjacency>  p)
{
  Ptr<Adjacency>  p_t=adCreateEmpty();
  int /* *row, */ *tia,*tja;
//  int rowlength;
  p_t->n=p->m;
  p_t->m=p->n;
  p_t->dja=0;
  p_t->nja=p->nja;
  /*
   * here, we know all sizes a priori.
   * so we would like to allocate things 
   * and to assemble inline.
   *
   */
  adCreateIA(p_t,p_t->n+1);
  /* we need this to be initialized 
     further, we need ia[n+1] for proper looping
  */
  adCreateJA(p_t,p_t->nja);
  /* we have the same number of edges
   *	in transposed matrix.
   */
  /*
   * 1st pass: calculate new row sizes, store them
   * in p_t->ia
   */
  tia=ia1Val(p_t->ia);
  tja=ia1Val(p_t->ja);

  {
    int i,j;
    int *cols,ncols;
  //for(i=1;i<=p_t->n+1;i++) tia[i]=0;
    p_t->ia->set0();
    
    if (p->dja)
      for(i=1;i<=p->n;i++)
	{
	  cols=adGetRowF(p,i);
	  for (j=1;j<=p->dja;j++)
	    tia[cols[j]]++;
	}
    else
      {
	for(i=1;i<=p->n;i++)
	  {
	    cols=adGetRowV(p,i); 
	    ncols=adGetRowLengthV(p,i);
	    for (j=1;j<=ncols;j++)
	      tia[cols[j]]++;
	  }
      }
  }
  
  /*
   * 2nd pass: calculate initial assembly
   * addresses in p->ja by summing up deltas
   * store them a the ends of  the rows:
   * in p_t->ja[tia[i+1]-1].
   * it will be overwritten when the last element of
   * the row is assembled.
   * We get automatically increasing  column indices.
   *
   */
  {
    int i;
    int delta, save;
    delta=tia[1];
    tia[1]=1;
    for(i=2;i<=p_t->n;i++)
      {
	save=tia[i];
	tia[i]=tia[i-1]+delta; /* set ia value */
	if (delta) tja[tia[i]-1]=tia[i-1]; 
	     /* set assembly address (only if delta !=0)*/
	delta=save;
      }
    tia[i]=tia[i-1]+delta;
    if (delta) tja[tia[i]-1]=tia[i-1];
  }
  /*
   * 3rd pass: assemble new rows
   */
  {
    int i,j;
    int *cols,ncols;
    int asm_idx,asm_loc;
    if (p->dja)
      for(i=1;i<=p->n;i++)
	{
	  cols=adGetRowF(p,i); 
	  ncols=adGetRowLengthF(p,i);
	  for (j=1;j<=ncols;j++)
	    {
	      asm_idx=tia[cols[j]+1]-1;
	      asm_loc=tja[asm_idx];
	      tja[asm_idx]++; /* one superfluous add @EOR*/
	      tja[asm_loc]=i;
	    }
	}
      else for(i=1;i<=p->n;i++)
	{
	  cols=adGetRowV(p,i); 
	  ncols=adGetRowLengthV(p,i);
	  for (j=1;j<=ncols;j++)
	    {
	      asm_idx=tia[cols[j]+1]-1;
	      asm_loc=tja[asm_idx];
	      tja[asm_idx]++; /* one superfluous add @EOR*/
	      tja[asm_loc]=i;
	    }
	}
  }

  ia1Check(p_t->ja);
  ia1Check(p_t->ia);
  return p_t;
}

 
Ptr<Adjacency>  adMultiply(Ptr<Adjacency>  pa, Ptr<Adjacency>  pb)
{
  Ptr<Adjacency>  pc=adCreateEmpty();
//  int *row;
//  int rowlength;
  int *cia;
  pc->n=pa->n;
  pc->m=pb->m;
  pc->dja=0;
  adCreateIA(pc,pc->n+1);
  cia=ia1Val(pc->ia);

  /*
   *   1st pass: trial assembly of
   *   rows  of pc in xccols to calculate new row sizes and pc->ia->val;
   *
   *   2nd pass: assemble rows.
   */
  {
    int xccols[1011];/*TODO A->max_row*B->max_row*/
    int ipass;
    for (ipass=1;ipass<=2;ipass++)
      {
	int i,j,k;
	int ii,jj,kk;
	int nacols,nbcols;
	int *acols,*bcols,*ccols;
	int iccols,found;
	if (ipass==1) cia[1]=1;
	if (ipass==2)
	  { 
	    pc->nja=cia[pc->n+1]-1;
	    adCreateJA(pc,cia[pc->n+1]-1);
	  }
	for (i=1;i<=pa->n;i++)
	  {   
	    acols=adGetRow(pa,i);
	    nacols= adGetRowLength(pa,i);
	    if (ipass==1)
	      {
		ccols=&xccols[0];
	      }
	    else
	      {
		ccols=adGetRow(pc,i);
	      }
	    iccols=0;
	    for (ii=1;ii<=nacols;ii++)
	      { 
		k=acols[ii];
		bcols=adGetRow(pb,k);
		nbcols= adGetRowLength(pb,k);
		for (kk=1;kk<=nbcols;kk++)
		  {
		    j=bcols[kk];
		    found=0;
		    for (jj=1;jj<=iccols;jj++)
		      {
			if (ccols[jj]==j) {found=1;break;}
		      }
		    if (!found) ccols[++iccols]=j;
		}
	      }
	    if (ipass==1) cia[i+1]=cia[i]+iccols;
	  }
      }
  }
  
  ia1Check(pc->ja);
  ia1Check(pc->ia);
  return pc;
}

/* Create only upper triang part of product */
Ptr<Adjacency>  adUMultiply(Ptr<Adjacency>  pa, Ptr<Adjacency>  pb)
{
  Ptr<Adjacency>  pc=adCreateEmpty();
//  int *row;
//  int rowlength;
  int *cia;
  pc->n=pa->n;
  pc->m=pb->m;
  pc->dja=0;
  adCreateIA(pc,pc->n+1);
  cia=ia1Val(pc->ia);
  
//for(i=1;i<=pc->n+1;i++) cia[i]=0;
  pc->ia->set0();

  /*
   *   1st pass: trial assembly of
   *   rows  of pc in xccols to calculate new row sizes and pc->ia->val;
   *
   *   2nd pass: assemble rows.
   */
  {
    int xccols[1011];/*TODO A->max_row*B->max_row*/
    int ipass;
    for (ipass=1;ipass<=2;ipass++)
      {
	int i,j,k;
	int ii,jj,kk;
	int nacols,nbcols;
	int *acols,*bcols,*ccols;
	int iccols,found;
	if (ipass==1) cia[1]=1;
	if (ipass==2)
	  { 
	    pc->nja=cia[pc->n+1];
	    adCreateJA(pc,cia[pc->n+1]-1);
	  }
	for (i=1;i<=pa->n;i++)
	  {   
	    acols=adGetRow(pa,i);
	    nacols= adGetRowLength(pa,i);
	    if (ipass==1)
	      {
		ccols=&xccols[0];
	      }
	    else
	      {
		ccols=adGetRowV(pc,i);
	      }
	    iccols=0;
	    for (ii=1;ii<=nacols;ii++)
	      { 
		k=acols[ii];
		bcols=adGetRow(pb,k);
		nbcols= adGetRowLength(pb,k);
		for (kk=1;kk<=nbcols;kk++)
		{
		  j=bcols[kk];
		  if (j>i)
		    {
		      found=0;
		      for (jj=1;jj<=iccols;jj++)
			{
			  if (ccols[jj]==j) {found=1;break;}
			}
		      if (!found) ccols[++iccols]=j;
		    }
		}
	      }
	    if (ipass==1) cia[i+1]=cia[i]+iccols;
	  }
      }
  }
  ia1Check(pc->ja);
  ia1Check(pc->ia);
  return pc;
}



Ptr<Adjacency>  adPnMultiply(Ptr<Adjacency>  pa, Ptr<Adjacency>  pb, int np)
{
  /*
     Speziell angepasste Version fuer netze:
     Anzahl der klist-Eintraege muss = dimSpace sein damit
     eine flaechenbasierte cell-cell  adjazenz existiert.
     laesst sich lokal billig ausrechnen.
  */

  Ptr<Adjacency>  pc=adCreateEmpty();
  int /* *row, */ *cia;
//  int rowlength;
  pc->n=pa->n;
  pc->m=pb->m;
  pc->dja=0;
  adCreateIA(pc,pc->n+1);
  cia=ia1Val(pc->ia);


  /*
   *   1st pass: trial assembly of
   *   rows  of pc in xccols to calculate new row sizes and pc->ia->val;
   *
   *   2nd pass: assemble rows.
   */
  {
    int xccols[1011];/* TODO A->max_row*B->max_row*/ 
    int nconn[1011]; /* TODO A->max_row*B->max_row*/ 
    int ipass;
    for (ipass=1;ipass<=2;ipass++)
      {
	int i,j,k;
	int ii,jj,kk;
	int nacols,nbcols;
	int *acols,*bcols,*ccols;
	int iccols,found;
	if (ipass==1) cia[1]=1;
	if (ipass==2)
	  {
	    pc->nja=cia[pc->n+1]-1;
	    adCreateJA(pc,pc->nja);
	  }
	for (i=1;i<=pa->n;i++)
	  {   
	    acols=adGetRow(pa,i);
	    nacols= adGetRowLength(pa,i);
	    if (ipass==1)
	      {
		ccols=&xccols[0];
	      }
	    else
	      {
		ccols=&xccols[0];
	      }
	    iccols=0;
	    for (ii=1;ii<=nacols;ii++)
	      { 
		k=acols[ii];
		bcols=adGetRow(pb,k);
		nbcols= adGetRowLength(pb,k);
		for (kk=1;kk<=nbcols;kk++)
		  {
		    j=bcols[kk];
		    found=0;
		    for (jj=1;jj<=iccols;jj++)
		      {
			if (ccols[jj]==j) {found=1; nconn[jj]++; break;}
		      }
		    if (!found) {iccols++; nconn[iccols]=1,ccols[iccols]=j;}
		  }
	      }
	    {
	      int npn,ipn;
	      if (ipass==1) 
		{
		  npn=0;
		  for (ipn=1;ipn<=iccols;ipn++)
		    if (nconn[ipn]==np) npn++;
		  cia[i+1]=cia[i]+npn;
		}
	      else
		{
		  int jpn;
		  ccols=adGetRow(pc,i);
		  for (jpn=ipn=1;ipn<=iccols;ipn++)
		    if (nconn[ipn]==np) ccols[jpn++]=xccols[ipn];
		}
	      
	    }
	  }
      }
  }
  ia1Check(pc->ja);
  ia1Check(pc->ia);
  return pc;
}

/* TODO: fixed row length code */
Ptr<Adjacency>  adBlowup(Ptr<Adjacency> p, int nblock)
{
  Ptr<Adjacency> pb;
  int i,j,ib,jb,iblock;
  int *row,*xrow,*bia;
  if (nblock==1) return p;
  pb=adCreateEmpty();
  assert(p->n==p->m);
  pb->n=nblock*p->n;
  pb->m=nblock*p->m;
  pb->nja=nblock*nblock*p->nja;
  adCreateIA(pb,pb->n+1);
  bia=ia1Val(pb->ia);
  adCreateJA(pb,pb->nja);
  bia[1]=1;
  for (i=1;i<=p->n;i++)
    for (ib=1;ib<=nblock;ib++)
    {
      iblock=nblock*(i-1)+ib;
      bia[1+iblock]=
	bia[iblock]+nblock*(adGetRowLength(p,i));
      xrow=adGetRowV(pb,iblock);
      row=adGetRow(p,i);
      for (j=1;j<=adGetRowLength(p,i);j++)
	for(jb=1;jb<=nblock;jb++)
	  xrow[(j-1)*nblock+jb]=nblock*(row[j]-1)+jb;
    }
  if (p->rows_sorted) pb->rows_sorted=1;
  return pb;
}


static int  compare_int (const void *pi1, const void *pi2)
{
  int i1=*(int*)pi1,i2=*(int*)pi2;
  if (i1<i2) return -1;
  else if (i1==i2) return 0;
  return 1;
}

void adSortRows(Ptr<Adjacency>  p)
{
  int *ia=ia1Val(p->ia);
  int *ja=ia1Val(p->ja);
  int i;
  if (p->rows_sorted) return;
  for (i=1;i<=p->n;i++)
    qsort(&ja[ia[i]],ia[i+1]-ia[i],sizeof(int),compare_int);
  p->rows_sorted=1;
}

PtrA<IArray1>  adCreateDiagonalIndex(Ptr<Adjacency>  p)
{
  int *ia=ia1Val(p->ia);
  int *ja=ia1Val(p->ja);
  int *idia;
  int i,j;
  
  PtrA<IArray1>  xidia=ia1Create(p->n);
  idia=ia1Val(xidia);
  for (i=1;i<=p->n;i++)
    for (j=ia[i];j<ia[i+1];j++)
      if (ja[j]==i) 
	{
	  idia[i]=j;
	  break;
	}
  return xidia;
}

void adPrintPat(Ptr<Adjacency>  p)
{
  char *line;
  int i,j;
  int *cols,ncols;
  line=(char*)malloc((p->m+2)*sizeof(char));
  for (j=0;j<=p->m+1;j++) line[j]='-';line[j]='\0';
  printf("%s\n",line);
  for(i=1;i<=p->n;i++)
  {
    cols=adGetRow(p,i); 
    ncols=adGetRowLength(p,i);
    for (j=1;j<=p->m+1;j++) line[j]=' ';line[j]='\0';
    line[0]=line[p->m+1]='|';
    for (j=1;j<=ncols;j++) line[cols[j]]='*';
    printf("%s\n",line);
  }
  for (j=0;j<=p->m+1;j++) line[j]='-';line[j]='\0';
  printf("%s\n",line);
  free(line);
}

/* Create Edge pattern from vertex-vertex adjacency*/
Ptr<Adjacency>  adCreateEdges(Ptr<Adjacency>  p)
{


  /* 
     First count no of edges. -- we need to skip diagonal
     entries. We could assume that in p they are always there,
     but for security reasons, we allow any possibility of input.
     */
  Ptr<Adjacency>  pc=adCreateEmpty();
  int i,j;
  int ne=0;
  int *cols,*ccols,ncols;
  for (i=1;i<=p->n;i++)
    {
      cols=adGetRow(p,i);
      ncols=adGetRowLength(p,i);
      for (j=1;j<=ncols;j++)
	if (cols[j]>i) ne++;
    }

  /*Create new fixed size pattern. */
  pc=adCreateF(ne,p->n,2);
  ne=0;
  for (i=1;i<=p->n;i++)
    {
      cols=adGetRow(p,i);
      ncols=adGetRowLength(p,i);
      for (j=1;j<=ncols;j++)
	if (cols[j]>i) 
	  {
	    ne++;
	    ccols=adGetRowF(pc,ne);
	    ccols[1]=i;
	    ccols[2]=cols[j];
	  }
    }
  ia1Check(pc->ja);
  return pc;
}

/* Create Face-cell pattern from cell-cell adjacency*/
Ptr<Adjacency>  adCreateFaces(Ptr<Adjacency>  p, int face_per_cell)
{

 
  /* 
     First count no of faces. -- we need to skip diagonal
     entries and use ordered faces. For the time being, we assum that
     each cell has the same nunber of faces. If not, celltype would be involved.
     We assume that each cell has the same number of adjacent faces.
     */
  Ptr<Adjacency>  pc=adCreateEmpty();
  int i,j,*cia;
  int nf=0,jf;
  int nja=0,ija;
  int *cols,*ccols,ncols;
  for (i=1;i<=p->n;i++)
    {
      cols=adGetRow(p,i);
      ncols=adGetRowLength(p,i);
      for (j=1;j<=ncols;j++)
	if (cols[j]>i) nf++, nja+=2;
      
      /* add no of "open" boundary faces*/
      nf+=(face_per_cell-ncols);
      nja+=(face_per_cell-ncols);
    }


  pc->n=nf;    /* num of faces */
  pc->m=p->n;  /* num of Cells*/
  pc->dja=0; 
  pc->nja=nja;

  adCreateIA(pc,pc->n+1); 
  cia=ia1Val(pc->ia);
  adCreateJA(pc,pc->nja);
//for (i=1;i<=p->n;i++) cia[i]=0;
  pc->ia->set0();
 
  jf=1;
  ija=1;
  cia[1]=1;
  for (i=1;i<=p->n;i++)
    {
      cols=adGetRow(p,i);
      ncols=adGetRowLength(p,i);
      for (j=1;j<=ncols;j++)
	if (cols[j]>i) 
	  {
	    ija+=2;
	    cia[jf+1]=ija;
	    ccols=adGetRowV(pc,jf);
	    ccols[1]=i;
	    ccols[2]=cols[j];
	    jf++;
	  }
      for (j=1;j<=face_per_cell-ncols;j++)
	{
	  ija++;
	  cia[jf+1]=ija;
	  ccols=adGetRowV(pc,jf);
	  ccols[1]=i;
	  jf++;
	}
	  
    }
  ia1Check(pc->ja);
  ia1Check(pc->ia);
  return pc;
}

Ptr<Adjacency> adCloneCopy(Ptr<Adjacency> adjIn)
{
  if (!adjIn) {return 0;}
  Ptr<Adjacency> adjOut=adCreateEmpty();
  //copy the attributes
  adjOut->n=adjIn->n;
  adjOut->m=adjIn->m;
  adjOut->nja=adjIn->nja;
  adjOut->dja=adjIn->dja;
  
  adjOut->ia=aCloneCopy(adjIn->ia);
  adjOut->ja=aCloneCopy(adjIn->ja);
  
  adjOut->rows_sorted=adjIn->rows_sorted;
  
  return adjOut;
}

PtrA<IArray2> adDesparsify(Ptr<Adjacency> ad)
{
  if (ad->dja)
    return ia2Format(ad->ja,ad->n,ad->dja);
  else
    {
      IArray1& ia=ad->ia;
      IArray1& ja=ad->ja;
      
      int maxrow=0;
      for (int i=1;i<=ad->n;i++)
	{
	  int lrow=ia(i+1)-ia(i);
	  if (lrow>maxrow) maxrow=lrow;
	}
      PtrA<IArray2> FullMat=new IArray2(ad->n,maxrow);
      IArray2 &mat=FullMat;
      for (int i=1;i<=ad->n;i++)
	{
	  int icol=1;
	  for (int j=ia(i);j<ia(i+1);j++,icol++)
	    mat(i,icol)=ja(j);
	  for (;icol<=maxrow;icol++)
	    mat(i,icol)=0;
	}
      return FullMat;
    }
}
