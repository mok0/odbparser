## OdbParser -- a Python extension module to read O files ##

Odbparser is a module for the Python programmer who would like to
extract information from O files.

The use of the odbparser module is probably best illustrated by an
example.  First, we import the module, and open up an O data
file. This file could be both binary and formatted, and it could
contain any number of datablocks.  For this example, we choose a
binary file as written by O when it exits:

```python
>>> import odbparser
>>> db = odbparser.get("binary.o")
```

Now, after reading the file, db is an ordinary Python dictionary, the
keys are names of datablocks, and the content of each entry is
whatever the type of that data is. For example, let us look at a few
well-known O datablocks:

```python
>>> type (db[".gs_real"])
<type 'numpy.ndarray'>
>>> print db[".gs_real"]
[  3.18265676e-01   1.38070002e+01   3.88569984e+01   2.27169991e+01
   6.26805648e-02  -5.41465655e-02   3.07297319e-01   0.00000000e+00
  -1.21590950e-01  -2.92898685e-01  -2.68076919e-02   0.00000000e+00
   2.87366927e-01  -1.12122096e-01  -7.83707127e-02   0.00000000e+00
  -2.66888547e+00   1.46758423e+01  -1.42084014e+00   1.00000000e+00
   9.20000002e-02   9.81999993e-01   0.00000000e+00   8.99999976e-01
   0.00000000e+00   0.00000000e+00   0.00000000e+00   1.12185276e+00]
```

We see that the type of the .gs_real array is a
[**numpy**](http://numpy.scipy.org/) array, and we can print the array to
see that it contains 28 floating point numbers with all kinds of
viewing information used by O. We can extract the orientation matrix:

```python
>>> gsreal = db['.gs_real']
>>> mat = gsreal[4:20]
>>> mat44 = mat.reshape(4,4)
>>> print mat44
[[  0.06268056  -0.05414657   0.30729732   0.        ]
 [ -0.12159095  -0.29289868  -0.02680769   0.        ]
 [  0.28736693  -0.1121221   -0.07837071   0.        ]
 [ -2.66888547  14.67584229  -1.42084014   1.        ]]
```

Now `mat` contains the OpenGL 4x4 orientation matrix in column major
format.  For most uses, the orientation part (upper 3x3) needs to be
transposed:

```python
>>> mat33 = mat44[0:3,0:3]
>>> ori = mat33.transpose()
>>> print ori
[[ 0.06268056 -0.12159095  0.28736693]
 [-0.05414657 -0.29289868 -0.1121221 ]
 [ 0.30729732 -0.02680769 -0.07837071]]
```

For the translation, we just need to extract that from gsreal:

```python
>>> trans = gsreal[16:19]
>>> print trans
[ -2.66888547  14.67584229  -1.42084014]
```

Of course there are other fun things one can do. List all datablocks
is one:

```python
>>> for k in db.keys():
...   print (k, type(k))
...
.object_menu <type 'tuple'>
@on_rama_pick <type 'tuple'>
.menu_major_name <type 'tuple'>
.graph_y <type 'tuple'>
.graph_x <type 'tuple'>
.sam_integer <type 'numpy.ndarray'>
.map_real <type 'numpy.ndarray'>
```

... and so on. Lets try something a bit more interesting. We have a
molecule named ALPHA in the database. Let us load the coordinates,
atom names and so on from the molecule into some variables and print
the information out:

```python
>>> xyz = db["alpha_atom_xyz"].reshape(-1,3)  # give us a 3xn array
>>> atnam = db["alpha_atom_name"]
>>> natoms = len(atnam)
>>> b  = db["alpha_atom_b"]
>>> wt = db["alpha_atom_wt"]
>>> for i in range(0,natoms): 
...    print ("ATOM", i, atnam[i], xyz[i], occ[i], b[i])
...
ATOM 0 N [  4.98199987  14.45300007  31.3390007 ] 1.0 0.0
ATOM 1 CA [  3.6329999   15.08199978  31.40999985] 1.0 0.0
ATOM 2 C [  3.17600012  15.45100021  30.01399994] 1.0 0.0
ATOM 3 O [  1.99600005  15.31999969  29.62999916] 1.0 0.0
```

... and so on, you get the idea. Of course this is only half-way
towards PDB output. We need the residue names and types, and we need
to know where each residue starts and ends in the list of atoms. The
last bit of information can be found in the _residue_pointers
datablock.  We also need to format the line correctly, which is
straighforward Python.

```python
>>> resnam= db["alpha_residue_name"]
>>> restyp = db["alpha_residue_type"]
>>> rp = db["alpha_residue_pointers"]
```

The rest is left as an exercise for the reader.

### Download and installation ###

To compile odbparser move into the directory and go:

```python
python setup.py build
python setup.py install --user
```
The above installs the module in the user space (usually in
the `~/.local` tree).  Of course you need to be root to
install into `/usr/lib/`.

### License ###
<a name="license"></a>
The source files of odbparser are distributed under the GNU Public License.
