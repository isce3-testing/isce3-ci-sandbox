:orphan:

.. title:: Projection

Projection
===========

Projection is a simple data structure that provides coordinate transformation capability.
Projection systems are uniquely identified by EPSG codes. At present, only altitudes above 
the WGS84 Ellipsoid are supported by these data structures.


Factory
----------

.. code-block:: python

   from isce3.core import make_projection

   obj = make_projection(epsg)


Documentation
----------------

.. autoclass:: isce3.core.ProjectionBase
   :members:
   :inherited-members:
