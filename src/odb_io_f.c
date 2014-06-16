/* $Id: odb_io_f.c 7 2006-09-09 13:33:21Z mok $ 

   Routines to read from formatted O files.
   Morten Kjeldgaard, 03-Jan-2001.
   Copyright (C) Morten Kjeldgaard 2001-2006.
   Licence: GPL.
*/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define MAXWORD 100

/* Get a word from a stream.  Maximum word size is MAXWORD
 * characters. If a word reaches it's maximum limit, we choose not to
 * flush the rest of the word. Returns NULL on EOF.  */
static char * getword(FILE *fp)
{
  static char word[MAXWORD + 1];
  char *p = word;
  int c;

  while ((c = fgetc(fp)) != EOF && isspace(c))
    ;                           /* Skip over word separators */
  if (c == EOF)
    return(NULL);
  *p++ = c;
  while ((c = fgetc(fp)) != EOF && !isspace(c) && p != &(word[MAXWORD])) {
    *p++ = c;                   /* Quit when a word separator is encountered
                                 * or we reach maximum word length
                                 */
  }
  *p = '\0';                    /* Mustn't forget that word terminator */
  return ((c == EOF) ? NULL : word);
}



/*
  Read the header of a formatted O file. Reads past any comments and
  blank lines 
*/
int read_param_f (FILE *fp, char *par, char *partyp, int *size, char *fmt)
{
  char buf[256], *ch, *stat;

    // Get the first non-comment line.
  while (1) {
    if (!fgets(buf, 256, fp)) {
      return -1;
    }
    ch = strtok(buf," ");
    if (ch && ch[0] == '!')
      continue;
    if (ch && ch[0] == '\n')
      continue;
    break;
  }

  // Decode datablock name and convert to lower case
  memcpy (par, ch, 26); 
  ch = par;
  while (*ch) {
    *ch = tolower(*ch);
    ch++;
  }

  // decode datablock type (single character == I, R, C, T)
  ch = strtok(NULL, " ");
  if (!ch)
    return 1;
  *partyp = *ch;

  // decode datablock size (in elements)
  ch = strtok(NULL, " ");
  if (!ch)
    return 2;
  *size = (int)strtol(ch, &stat, 10); 
  if (*stat) {
    fprintf (stderr, "non-digits in datablock size\n");
    return 3; 
  }

  // Finally, decode the format. 
  ch = strtok(NULL, " \012");
  if (!fmt)
    return 4;
  memcpy (fmt, ch, 64); 

  return 0;
}

/*
  Read 'size' integers from the file 
*/

int read_int4_f (FILE *fp, int *array, int size)
{
  register int i;
  char *ch, *stat;

  for (i=0; i<size; i++) {
    ch = getword(fp);
    array[i] = (int)strtol(ch, &stat, 10); 
    if (*stat) {
      fprintf (stderr, "non-digits in datablock\n");

      return 1; 
    }
  }
  return 0;
}

/*
  Read 'size' floats from the file 
*/
int read_float4_f (FILE *fp, float *array, int size)
{
  register int i;
  char *ch, *stat;

  for (i=0; i<size; i++) {
    ch = getword(fp);
    array[i] = (float)strtod(ch, &stat);
    if (*stat) {
      fprintf (stderr, "non-digits in datablock\n");
      return 1; 
    }
  }
  return 0;
}

/*
  A simple parser for the fortran formats that the type C datablocks
  are stored in. Generate a string describing the format, so the
  program knows what to expect in the file. The function knows that 
  type C variables are 6 bytes always. Examples:

  (2a)         -> 123456123456
  (1x,5a)      -> _123456123456123456123456123456
  (5(1x,a6))   -> _123456_123456_123456_123456_123456
  (1x,2(2x,a)) -> ___123456__123456
  (aaaaa)      -> 123456123456123456123456123456

*/
#define RSIZ 256

static char *parse_format(char *fmt)
{
  register int i;
  char *result, *f, *r, *t;
  int mult = 0;

  //printf ("allocating %d bytes\n",RSIZ);
  result = malloc(RSIZ);
  memset (result, 0, RSIZ);
  
  f = fmt;
  r = result;
  
  while (*f) {
    switch (*f) {
    case '(':
      t = parse_format(++f);
      if (mult == 0) mult = 1;
      for (i=0; i < mult; i++)
	strncat(result, t, RSIZ);
      free(t);
      //printf ("freed %d bytes\n",RSIZ);
      //printf ("size of result: %d\n",strlen(result));
      return result;
      break;
    case 'x':
    case 'X':
      if (mult == 0) mult = 1;
      for (i=0; i < mult; i++)
	*r++ = '_';
      mult = 0;
      break;
    case 'a':
    case 'A':
      if (mult == 0) mult = 1;
      for (i=0; i < mult; i++) {
	*r++ = '0'+1;
	*r++ = '0'+2;
	*r++ = '0'+3;
	*r++ = '0'+4;
	*r++ = '0'+5;
	*r++ = '0'+6;
      }
      mult = 0;
      break;
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      mult = mult*10+(*f)-'0';
      //printf ("mult = %d\n", mult);
      break;
    case ')':
      //printf ("result:%s\n", result);
      return result;
    case ',':
      mult = 0;
      break;
    }
    f++;
  }
  fprintf (stderr, "shouldn't happen\n");
  return NULL;
}

/*
  Read 'size' C6 variables from the file 
*/
int read_c6_f (FILE *fp, char *array, int size, char *fmt)
{
  register int i;
  char c, *a, *t, *s;
  int eol, inword;

  t = parse_format(fmt);
  s = t;
  a = array;
  
  i = 0;
  eol = 0;
  while (i < size) {
    //fprintf (stderr, "%d ", i);

    if (!*s) { // if end of format string, rewind it.
      s = t;   // and reset everything
      eol = 0;
      while (fgetc(fp) != '\n')
	;
      //fprintf (stderr, "rewind\n");
    }	      

    inword = 1;
    while (inword) {
      if (!eol)
	c = fgetc(fp);
      if (c == '\n')
	eol = 1;
      switch (*s++) {
      case '_':
	break;
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
	if (eol)
	  //*a++ = ' ';
	  *a++ = '#';
	else
	  *a++ = c;
	break;
      case '6':
	if (eol)
	  //*a++ = ' ';
	  *a++ = '6';
	else
	  *a++ = c;
	i++;
	inword = 0;
	break;
      }
    }  // end get one word
  }
  free(t);
  return 0;
}

/*
  Read a text datablock from the formatted file
*/
int read_text_f (FILE *fp, char *array, int nrec, int size)
{
  char buf[256];
  register int i;
  int j, n;

  for (i=0, j=0; i<nrec; i++) {
    fgets (buf, 256, fp);
    n = strlen(buf);
    strncpy (array+j, buf, size);
    j += size;
  }
  return j;
}

/*
Local Variables: 
mode: c
mode: font-lock
End:
*/

