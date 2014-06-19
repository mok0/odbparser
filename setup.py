import os
from distutils.core import setup, Extension
from distutils.sysconfig import get_python_lib

incdir = os.path.join(get_python_lib(plat_specific=1),
                      "numpy/core/include/numpy")

odbparser = Extension('odbparser',
                    sources=["src/odb_io.c",
                             "src/odb_io_f.c",
                             "src/odbparsermodule.c",
                             ],
                    include_dirs=[incdir])

setup(name='odbparser',
      version='2.0',
      description='Python module for reading O files',
      author="Morten Kjeldgaard",
      author_email="mok@bioxray.dk",
      ext_modules=[odbparser])
####
