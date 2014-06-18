/*
   Python module to read O data files, binary or formatted.
   Morten Kjeldgaard, 03-Jan-2001.
   Copyright (C) Morten Kjeldgaard 2001-2006, 2014.
   Licence: GPL.
*/

#include <Python.h>             /* Python header files */
#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#include <arrayobject.h>
#include <fcntl.h>
#include "odb_io.h"

/*
  binfil -- return 1 if fnam is a binary O file, else 0. An O binary
  file normally has the byte pattern [0 0 0 036 .] in the first 5
  bytes of the file. */

static int binfil(char *fnam)
{
  int fd, n;
  char buf[8];

  fd = open(fnam, O_RDONLY);
  if (fd > -1) {
    n = read (fd, buf, 8);
    close(fd);
    if ((buf[0]&buf[1]&buf[2]) == 0 && buf[3] == 30 && buf[4] == '.')
      return 2;
    if ((buf[0]&buf[1]&buf[2]) == 0 && buf[3] == 30)
      /* The only binary O files that do not have a '.' in the fifth byte
	 are the dgnl data files. */
      return 1;
  }
   return 0;
}

/*
  Read a binary O database. The data is returned in a Python
  dictionary, with datablock names as keys. Real and integer data are
  stored in numpy arrays.  Type 'C' datablocks are in O character
  strings of length 6. These are returned as a tuple of strings. Type
  'T' datavblocks are returned as a tuple of strings.  Trailing spaces
  are stripped from both type 'C' and 'T' datablocks.
 */
static PyObject *readbinary (char *fnam)
{
  int fd;
  char par[26], typ, *s;
  int errcod, siz;
  npy_intp dims[] = {0};
  void *vector, *data;
  PyObject *pydict, *pykey, *pytup, *pystr;

  fd = open(fnam, O_RDONLY);
  if (fd < 0) {
    return NULL;
  }

  memset (par, 0, 26);
  pydict = PyDict_New();

  while (1) {

    errcod = read_param(fd, par, &typ, &siz, DOSWAP);
    if (errcod < 0 || siz == 0)
      break;
    //printf ("errcod=%d, param=%s type=%c size=%d\n", errcod, par, typ, siz);

    /* strip spaces off end of datablock name */
    s = &par[25];
    while (*s <= 32 && s > par)
      *s-- = '\0';

    pykey = PyUnicode_FromString(par);

    switch(typ) {

    case 'I':
      data = calloc(siz, sizeof(int));
      read_int4 (fd, data, siz, DOSWAP);
      dims[0] = siz;
      vector = (void *)PyArray_SimpleNewFromData(1, dims, NPY_INT, data);
      if (!vector) {
	close(fd);
	PyErr_SetString(PyExc_RuntimeError, "Failed to create integer array");
	Py_DECREF(pykey);	// decref latest key
	return pydict;		// return what we've got so far
      }
      PyDict_SetItem (pydict, pykey, (PyObject *)vector); // add to dictionary
      break;

    case 'R':
      data = calloc(siz, sizeof(float));
      read_float4 (fd, data, siz, DOSWAP);
      dims[0] = siz;
      vector = (void *)PyArray_SimpleNewFromData(1, dims, NPY_FLOAT, data);
      if (!vector) {
	close(fd);
	PyErr_SetString(PyExc_RuntimeError, "Failed to create real array");
	Py_DECREF(pykey);	// decref latest key
	return pydict;	// return what we've got so far
      }
      PyDict_SetItem (pydict, pykey, (PyObject *)vector); // add to dictionary
      break;

    case 'C':
      {
	register int i;
	char buf[7], *ch;
	buf[6] = 0;
	s = calloc (siz,6*sizeof(char));
	read_c6 (fd, s, siz, DOSWAP);
	pytup = PyTuple_New (siz);
	for (i=0; i < siz; i++) {
	  memcpy(buf, &s[6*i], 6);

	  /* strip spaces off end */
	  ch = &buf[6];
	  while (*ch <= 32 && ch > buf)
	    *ch-- = '\0';
	  /*
	     In principle we should use:
	     pystr = PyBytes_FromString(buf);
	     but it appears there are non-ascii bytes in some of
	     the character blocks in the distributed O data files.
	  */
	  pystr = PyBytes_FromString(buf);
	  if (PyTuple_SetItem (pytup, i, pystr) != 0)
	    fprintf (stderr, "tuple insert error");
	}
	free(s);
	PyDict_SetItem (pydict, pykey, pytup); // add to dictionary
      }
      break;

    case 'T':
      {
	register int i,j;
	char *ch, *t;
	int nrec=0;

	s = calloc(siz,sizeof(char));
	t = calloc(siz,sizeof(char)); // get a string that's big enough
	read_text (fd, s, siz, DOSWAP);

	for (i=0; i<siz; i++) { // count the number of records
	  if (s[i] == '\r')
	    nrec++;
	}
	pytup = PyTuple_New (nrec);

	nrec = 0;
	for (i=0,j=0; i<siz; i++) { // extract the individual strings into 't'
	  t[j] = s[i];

	  if (t[j] == '\r') {
	    ch = &t[j];
	    while (*ch <= 32 && ch > t) // strip spaces off end
	      *ch-- = '\0';

	    pystr = PyUnicode_FromString(t); // create python string
	    //pystr = PyBytes_FromString(t); // create python string
	    if (PyTuple_SetItem (pytup, nrec++, pystr) != 0) // add it to the tuple
	      fprintf (stderr, "tuple insert error");
	    j=0;
	  } else {
	    j++;
	  }
	}
	free(t);
	free(s);
	PyDict_SetItem (pydict, pykey, pytup); // add tuple to dictionary
      }

      break;

    } // end switch (typ)
  }
  close(fd);
  return pydict;
}

/*
   Read a formatted O datablock file. The current algorithm for
   reading formatted type 'C' datablocks requires that there are
   characters other than spaces in every element. This is in fact not
   always the case, for example the .major_menu datablock in menu.o
   has empty elements, which are filled with spaces by the Fortran
   FORMAT statement. It would require a lot of programming to deal
   with this issue, and it is frankly not important enough.
*/
static PyObject *readformatted (char *fnam)
{
  FILE *fp;
  char par[26], typ, fmt[64];
  int errcod, siz;
  npy_intp dims[] = {0};
  void *vector, *data;
  PyObject *pydict, *pykey, *pytup, *pystr;

  fp = fopen(fnam, "r");
  if (!fp) {
    return NULL;
  }

  pydict = PyDict_New();

  errcod = read_param_f(fp, par, &typ, &siz, fmt);
  while (!errcod) {

    pykey = PyUnicode_FromString(par);

    switch (toupper(typ)) {

    case 'I':
      data = calloc(siz, sizeof(int));
      read_int4_f (fp, data, siz);
      dims[0] = siz;
      vector = (void *)PyArray_SimpleNewFromData(1, dims, NPY_INT, data);
      if (!vector) {
	fclose(fp);
	PyErr_SetString(PyExc_RuntimeError, "Failed to create integer array");
	Py_DECREF(pykey);	// decref latest key
	return pydict;		// return what we've got so far
      }
      PyDict_SetItem (pydict, pykey, (PyObject *)vector); // add to dictionary

      break;

    case 'R':
      data = calloc(siz, sizeof(float));
      read_float4_f (fp, data, siz);
      dims[0] = siz;
      vector = (void *)PyArray_SimpleNewFromData(1, dims, NPY_FLOAT, data);
      if (!vector) {
	fclose(fp);
	PyErr_SetString(PyExc_RuntimeError, "Failed to create real array");
	Py_DECREF(pykey);	// decref latest key
	return pydict;	// return what we've got so far
      }
      PyDict_SetItem (pydict, pykey, (PyObject *)vector); // add to dictionary

      break;

    case 'C':
      {
	register int i;
	char buf[7], *ch, *s;
	buf[6] = 0;
	s = calloc (siz,6*sizeof(char));
	read_c6_f (fp, s, siz, fmt);
	pytup = PyTuple_New (siz);
	for (i=0; i < siz; i++) {
	  memcpy(buf, &s[6*i], 6);

	  /* strip spaces off end */
	  ch = &buf[6];
	  while (*ch <= 32 && ch > buf)
	    *ch-- = '\0';

	  pystr = PyUnicode_FromString(buf);
	  if (PyTuple_SetItem (pytup, i, pystr) != 0)
	    fprintf (stderr, "tuple insert error");
	}
	free(s);
	PyDict_SetItem (pydict, pykey, pytup); // add to dictionary
      }
      break;

    case 'T':
      {
	register int i,j;
	char *ch, *s, *t;
	int nrec, reclen;

	reclen = strtol(fmt, NULL, 10);
	nrec = siz;
	s = calloc(nrec*reclen,sizeof(char));
	t = calloc(reclen,sizeof(char)); // get a string that's big enough
	read_text_f (fp, s, nrec, reclen);

	pytup = PyTuple_New (nrec);

	for (i=0, j=0; i<nrec; i++) { // extract the individual strings into 't'
	  memcpy (t, s+j, reclen);
	  ch = &t[reclen-1];
	  while (*ch <= 32 && ch > t) // strip spaces off end
	    *ch-- = '\0';
	  pystr = PyUnicode_FromString(t); // create python string
	  if (PyTuple_SetItem (pytup, i, pystr) != 0) // add it to the tuple
	    fprintf (stderr, "tuple insert error");
	  j += reclen;
	}
	free(t);
	free(s);
	PyDict_SetItem (pydict, pykey, pytup); // add tuple to dictionary
      }

      break;
    } // end switch
    errcod = read_param_f(fp, par, &typ, &siz, fmt);
  }
  fclose(fp);
  return pydict;
}


/* 1. Functions available in odbparser module */

static PyObject *get (PyObject *self, PyObject *args)
{
  char *fnam;
  PyObject *pydict;

  if (!PyArg_ParseTuple(args, "s" , &fnam ))
    return NULL;

  /* Do the actual reading. The two subroutines readbinary and readformatted
     both return a Python dictionary. */

  if (binfil(fnam)) {
    //fprintf (stderr, "Reading binary O file\n");
    pydict = readbinary(fnam);
  }  else {
    //fprintf (stderr, "Reading formatted O file\n");
    pydict = readformatted(fnam);
  }
  return pydict;
}


/* 2. Doc strings */

static char odbparser_module__doc__[] =
"Parse O binary and formatted files";

static char odbparser_get__doc__[] =
"get(filename) -- return dictionary of O datablocks";


/* 3. Method table mapping names to wrappers */

static PyMethodDef odbparser_methods[] = {
  {"get", (PyCFunction)get,   METH_VARARGS, odbparser_get__doc__ },
  {NULL, (PyCFunction)NULL, 0, NULL} /* sentinel */
};


/* 4. Module initialization function */

static struct PyModuleDef moduledef = {
  PyModuleDef_HEAD_INIT,
  "odbparser",             /* name of module */
  odbparser_module__doc__, /* module documentation */
  -1,                      /* m_size */
  odbparser_methods,       /* methods */
  NULL,                    /* m_reload */
  NULL,                    /* m_traverse */
  NULL,                    /* m_clear */
  NULL                     /* m_free */
};


static PyObject *ErrorObject;

PyMODINIT_FUNC PyInit_odbparser(void) {
  PyObject *m, *d;

  /* Create the module and add the functions */
  m = PyModule_Create(&moduledef);

  /* Add some symbolic constants to the module */
  d = PyModule_GetDict(m);
  ErrorObject = PyUnicode_FromString("odbparser.error");
  PyDict_SetItemString(d, "error", ErrorObject);

  import_array();

  /* Check for errors */
  if (PyErr_Occurred())
    Py_FatalError("can't initialize module odbparser");

  return m;
}


/*
  Local Variables:
  mode: font-lock
  End:
*/
