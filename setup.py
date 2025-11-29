from setuptools import setup, Extension
import os
# mpi4py provides the necessary configuration
# Make sure mpi4py is installed (it's in pyproject.toml build-system requires)
from mpi4py import rc
#rc.update('libs', False) # Do not use mpi4py's libraries, just its build info
from mpi4py import get_include, get_config

# Get MPI configuration
conf = get_config()
mpi_include_dirs = ["/usr/lib/x86_64-linux-gnu/openmpi/include"]
mpi_library_dirs = [conf['library_dirs']] if 'library_dirs' in conf else []
mpi_libraries = [conf['libraries']] if 'libraries' in conf else ['mpi'] # default to 'mpi' if not specified
mpi_runtime_dirs = [conf['runtime_library_dirs']] if 'runtime_library_dirs' in conf else mpi_library_dirs

# Define your C extension
mpi_extension = Extension(
    'py_doubly_linked_list.doubly_linked_list', # Name of the final module
    sources=['src/doubly_linked_list.c'], # Path to your source file(s)
    include_dirs=mpi_include_dirs,
    library_dirs=mpi_library_dirs,
    runtime_library_dirs=mpi_runtime_dirs,
    libraries=mpi_libraries,
    extra_compile_args=['-std=c99', '-fPIC']
)

setup(
    ext_modules=[mpi_extension],
    packages=['src/py_doubly_linked_list'], # If your code is in a package structure
    # The rest of the setup configuration can be in pyproject.toml
)
