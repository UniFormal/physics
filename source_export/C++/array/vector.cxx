#include "vector.h"
#include "string.h"

int vassert(Ptr<Vector> v1, Ptr<Vector> v2)
{
  assert(!strcmp(v1->vtype,v2->vtype));
  assert(v1->vlib==v2->vlib);
  assert(v1->vdim()==v2->vdim());
  return 1;
}

int vdim(Ptr<Vector> v) { return v->vdim(); }

char *vtype(Ptr<Vector> v) { return v->vtype;}


Vector::Vector()
  : vlib(NULL)
{
  vtype[0]='\0';
}

Vector::~Vector()
{
}


