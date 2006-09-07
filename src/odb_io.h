/* $Id$ */

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
