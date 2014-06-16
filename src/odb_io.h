/* $Id: odb_io.h 7 2006-09-09 13:33:21Z mok $ 
   Copyright (C) Morten Kjeldgaard 2001-2006.
   License: GPL
*/

#define SWAP 1
#define NOSWAP 0

#ifdef __i386__
#  define DOSWAP SWAP
#else
#  define DOSWAP NOSWAP
#endif

/* Declaration of binary read functions */
int read_param (int fd, char *par, char *partyp, int *size, int swap);
int read_text (int fd, char *text, int size, int swap);
int read_c6 (int fd, char *cstore, int size, int swap);
int read_int4 (int fd, int *istore, int size, int swap);
int read_float4 (int fd, float *rstore, int size, int swap);

/* Declaration of formatted read functions */
int read_param_f (FILE *fp, char *par, char *partyp, int *size, char *fmt);
int read_int4_f (FILE *fp, int *array, int size);
int read_float4_f (FILE *fp, float *array, int size);
int read_c6_f (FILE *fp, char *array, int size, char *fmt);
int read_text_f (FILE *fp, char *array, int nrec, int size);

/*
Local Variables: 
mode: c
mode: font-lock
End:
*/
