
#ifndef ARRAY_HANDLER_H
#define ARRAY_HANDLER_H

  int da1LineHandlerF(void* a, int iline, int argc, const char *argv[]);
  int da2LineHandlerF(void* a, int iline, int argc, const char *argv[]);
                            
  int ia1LineHandlerF(void* a, int iline, int argc, const char *argv[]);
  int ia2LineHandlerF(void* a, int iline, int argc, const char *argv[]);
                            
  int da1LineHandlerV(void* a, int iline, int argc, const char *argv[]);
  int da2LineHandlerV(void* a, int irow, int argc, const char *argv[]);
                            
  int ia1LineHandlerV(void* a, int iline, int argc, const char *argv[]);
  int ia2LineHandlerV(void* a, int irow, int argc, const char *argv[]);
  
  
  int da1XDRHandler(void* a, FILE *f);
  int da2XDRHandler(void* a, FILE *f);
  int da3XDRHandler(void* a, FILE *f);
  int da4XDRHandler(void* a, FILE *f);
  int da5XDRHandler(void* a, FILE *f);
                          
  int ia1XDRHandler(void* a, FILE *f);
  int ia2XDRHandler(void* a, FILE *f);
  int ia3XDRHandler(void* a, FILE *f);
  int ia4XDRHandler(void* a, FILE *f);
  int ia5XDRHandler(void* a, FILE *f);
  
#endif





