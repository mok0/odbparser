/* $Id: odb_io.h,v 1.1 2001/01/15 14:13:11 mok Exp $ */

#define SWAP 1
#define NOSWAP 0

#ifdef __i386__
#  define DOSWAP SWAP
#else
#  define DOSWAP NOSWAP
#endif

/*
Local Variables: 
mode: c
mode: font-lock
End:
*/
