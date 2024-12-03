:orphan:

ISCE's Library Interface 
==========================

ISCE's library interface is meant to mimic the C++ code structure.

The following classes are exposed to the Python level:

Core Datastructures
--------------------
These are data structures that are directly related to C++ classes in the isce::core namespace

* :doc:`DateTime <./core/DateTime>` 
* :doc:`TimeDelta <./core/TimeDelta>`
* :doc:`Ellipsoid <./core/Ellipsoid>`
* :doc:`Poly1d <./core/Poly1d>`
* :doc:`Poly2d <./core/Poly2d>`
* :doc:`LUT1d <./core/LUT1d>`
* :doc:`Orbit <./core/Orbit>`
* :doc:`ProjectionBase <./core/Projection>`

I/O Datastructures
------------------

* :doc:`Raster <./io/Raster>`

Product Datastructures
----------------------
* :doc:`RadarGridParameters <./product/RadarGridParameters>`

Image Datastructures
--------------------

* :doc:`ResampSlc <./image/ResampSlc>`

Geometry Datastructures
------------------------
* :doc:`Rdr2Geo <./geometry/Rdr2geo>`
* :doc:`Geo2Rdr <./geometry/Geo2rdr>`
* :doc:`DEMInterpolator <./geometry/DEMInterpolator>`

Geometry Functions
---------------------
* :doc:`getGeoPerimeter <./geometry/getGeoPerimeter>`
* :doc:`rdr2geo_point <./geometry/Rdr2geo_pt>`
* :doc:`geo2rdr_point <./geometry/Geo2rdr_pt>`

