from distutils.core import setup, Extension

module1 = Extension('odbparsermodule',
                    sources = ["src/odbparsermodule.c",
                               "src/odb_io.c",
                               "src/odb_io_f.c",
                               "src/odb_io.h"],
                    )

setup (name = 'OdbParser',
       version = '1.0',
       description = 'A PYthon module for reading O files',
       author = "Morten Kjeldgaard",
       author_email = "mok@bioxray.dk",
       ext_modules = [module1])

