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

/*
Local Variables: 
mode: c
mode: font-lock
End:
*/
