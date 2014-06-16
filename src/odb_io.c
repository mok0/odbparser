/*
   Routines to read from binary O files.
   Morten Kjeldgaard, 03-Jan-2001.
   Copyright (C) Morten Kjeldgaard 2001-2014.
   Licence: GPL.
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include "odb_io.h"

/*
  Swap bytes in n 4-byte words
*/
static void
swap4 (char *buffer, int n)
{
  register int i;
  char j;

  for (i=0; i < n*4; i+=4) {
    j = buffer[i];
    buffer[i] = buffer[i+3];
    buffer[i+3] = j;
    j = buffer[i+1];
    buffer[i+1] = buffer[i+2];
    buffer[i+2] =j; 
  }
}

/*
  Read the parameter (datablock) from a binary O file.
*/
int read_param (int fd, char *par, char *partyp, int *size, int swap)
{
  int n;
  long rl1, rl2;

  n = read (fd, &rl1, 4);
  if (n == 0) return -1;
  if (swap) swap4 ((char *)&rl1, 1);
  n = read (fd, par, 25);
  // convert datablock name to lower case
  for (n=0; n<25; n++)
    par[n] = tolower(par[n]);
  n = read (fd, partyp, 1);
  n = read (fd, size, 4);
  if (swap) swap4 ((char *)size, 1);
  n = read (fd, &rl2, 4);
  if (swap) swap4 ((char *)&rl2, 1);
  
  if (rl1 != rl2) {
    fprintf (stderr, "Error reading parameter header (%d %ld %ld)\n", 
	     *size, rl1, rl2);
    return -2;
  }
  return 0;
}

/*
  Read a text datablock from the binary file
*/
int read_text (int fd, char *text, int size, int swap)
{
  int n;
  long rl1, rl2;

  n = read (fd, &rl1, 4);  
  if (n == 0) return -1;
  if (swap) swap4 ((char *)&rl1, 1);
  n = read (fd, text, rl1);
  n = read (fd, &rl2, 4);  
  if (swap) swap4 ((char *)&rl2, 1);

  if (rl1 != rl2) {
    fprintf (stderr, "Error read text block\n");
    return -2;
  }
  if (size != rl2)
    fprintf (stderr, "read_text: Expected %d, got %ld elements\n", size, rl2);

  return 0;
}

/* 
   Read 'size' C6 variables from the binary fortran file.
*/
int read_c6 (int fd, char *cstore, int size, int swap)
{
  int n;
  long rl1, rl2;

  n = read (fd, &rl1, 4);  
  if (n == 0) return -1;
  if (swap) swap4 ((char *)&rl1, 1);
  n = read (fd, cstore, rl1);
  n = read (fd, &rl2, 4);  
  if (swap) swap4 ((char *)&rl2, 1);

  if (rl1 != rl2) {
    fprintf (stderr, "Error read character block\n");
    return -2;
  }
  if (6*size != rl2) 
    fprintf (stderr, "read_c6: Expected %d, got %ld elements\n", 6*size, rl2);

  return 0;
}

/*
  Read 'size' integers from the binary fortran file.  Swap bytes if
  necessary, file is always in big-endian order.  
*/
int read_int4 (int fd, int *istore, int size, int swap)
{
  int n;
  long rl1, rl2;

  n = read (fd, &rl1, 4);  
  if (n == 0) return -1;
  if (swap) swap4 ((char *)&rl1, 1);
  n = read (fd, istore, rl1);
  if (swap) swap4 ((char *)istore, size);
  n = read (fd, &rl2, 4);  
  if (swap) swap4 ((char *)&rl2, 1);

  if (rl1 != rl2) {
    fprintf (stderr, "Error read int block\n");
    return -2;
  }
  if (4*size != rl2) 
    fprintf (stderr, "read_int4: Expected %d, got %ld elements\n", 4*size, rl2);

  return 0;
}

/* 
   Read 'size' floats from the binary fortran file.  Swap bytes if
   necessary, file is always in big-endian order. 
*/
int read_float4 (int fd, float *rstore, int size, int swap)
{
  int n;
  long rl1, rl2;

  n = read (fd, &rl1, 4);  
  if (n == 0) return -1;
  if (swap) swap4 ((char *)&rl1, 1);
  n = read (fd, rstore, rl1);
  if (swap) swap4 ((char *)rstore, size);
  n = read (fd, &rl2, 4);  
  if (swap) swap4 ((char *)&rl2, 1);

  if (rl1 != rl2) {
    fprintf (stderr, "Error read float block\n");
    return -2;
  }
  if (4*size != rl2)
    fprintf (stderr, "read_float4: Expected %d, got %ld elements\n", 4*size, rl2);

  return 0;
}

/*
Local Variables: 
mode: c
mode: font-lock
End:
*/

