# -*- mode: python; mode: font-lock; py-indent-offset: 4; -*-
# $Id: setup.py 4 2006-09-07 15:54:00Z mok $

import os
from distutils.core import setup, Extension
from distutils.sysconfig import get_python_lib

incdir = os.path.join(get_python_lib(plat_specific=1), "numpy/core/include/numpy")

module1 = Extension('odbparsermodule',
                    sources = ["src/odbparsermodule.c",
                               "src/odb_io.c",
                               "src/odb_io_f.c",
                              ],
                    include_dirs=[incdir]
                    )

setup (name = 'OdbParser',
       version = '1.0',
       description = 'A PYthon module for reading O files',
       author = "Morten Kjeldgaard",
       author_email = "mok@bioxray.dk",
       ext_modules = [module1])


####
