:orphan:

.. title:: Raster

Raster
========

Raster provides a light wrapper for isce::io::Raster objects for the primary purpose
of passing h5py and GDAL-compatible rasters to lower-level ISCE routines. 
Provides basic querying of raster information.


Factory
----------

.. code-block:: python

   from isce3.io import Raster

   obj = Raster(**kwds)



Documentation
----------------

.. autoclass:: isce3.io.Raster
   :members:
   :inherited-members:
