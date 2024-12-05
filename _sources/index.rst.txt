.. isce documentation master file, created by
   sphinx-quickstart on Wed Jun  6 20:49:45 2018.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

Welcome to ISCE3's documentation!
=======================================

ISCE3's interface is built as two separate layers:

1. `Python interface </isce3-ci-sandbox/api/python>`_ for workflows and end users
2. `C++ library interface </isce3-ci-sandbox/api/cxx/annotated.html>`_ for implementing performance-critical algorithms

The Python layer is built on top of the underlying C++ library using `pybind11 <https://pybind11.readthedocs.io/>`_.

The C++ layer is subject to change, so if you need any functionality that currently only exists in the C++ code, it's recommended to add this functionality to the Python bindings rather than use it directly.


Library Interface
-----------------

:doc:`Library Interface <./library>`
