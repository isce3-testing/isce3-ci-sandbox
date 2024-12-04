:orphan:

.. title:: Geometry Tutorial

Radar Geometry Tutorial
=======================

This tutorial is organized in 4 sections.

1. :ref:`usingpyorbit`
2. :ref:`usingradargrid`
3. :ref:`forwardmap`
4. :ref:`invmap`

.. _usingpyorbit:

Working with Orbits
--------------------

Orbit data structure is at the heart of geometric manipulation from the python level in ISCE.
This is just a uniformly sampled, time-tagged collection of state vectors in 
Earth Centered Earth Fixed (ECEF) coordinates. In this example, we will walk through 
an example of constructing a Orbit object with a list of state vectors. 
The input state vectors are provided in a simple 7-column text file as shown below:

.. literalinclude:: orbit_arc.txt

.. code-block:: python

    def loadOrbit(infilename):
        from isce3.core import StateVector, Orbit, DateTime
        
        #List of state vectors
        svs = []

        #Open file with state vectors for reading
        with open(infilename, 'r') as fid:
        
            #For each line in file
            for line in fid:
                vals = line.strip().split()

                #Create dateTime object.
                tstamp = DateTime(vals[0])

                pos = [float(x) for x in vals[1:4]]
                vel = [float(x) for x in vals[4:7]]


                svs.append( StateVector(datetime=tstamp,
                                        position=pos,
                                        velocity=vel))

        return Orbit(svs)

.. note::
    In this example, we demonstrated creation of Orbit objects with simple text files. The same approach can be used 
    to generate Orbits from database queries or Sentinel-1/ NISAR XML files.


.. _usingradargrid:

Working with RadarGridParameters
----------------------------------

RadarGridParameters is the basic minimal data structure used to represent the limits
of a radar image in azimuth time and slant range coordinates. This data structure is 
relevant for all NISAR L1 and L2 radar geometry products. It is inherently assumed 
that the imagery is laid out on a uniform grid in both azimuth time and slant range.

A simple RadarGridParameters object can be created as shown below:

.. code-block:: python

    from isce3.product import RadarGridParameters
    from isce3.core import DateTime, LookSide


    grid = RadarGridParameters(
        lookside = LookSide.Left,

        # Imaging wavelength
        wavelength = 0.06,

        # Slant range extent
        starting_range = 8e5,
        range_pixel_spacing = 10.,
        width = 1000,

        # Along track extent
        ref_epoch = DateTime("2023-01-03T14:21:55.125"),
        sensing_start = 0.,  # Seconds since ref_epoch
        prf = 1000.,
        length = 1500,
    )

.. note::
    This example goes into gory detail of setting up a basic radar grid at the lowest level. 
    In future, higher level python classes will include a getRadarGrid() method that should 
    return a populated grid structure with data from HDF5 products.

.. _forwardmap:

Forward mapping example - determining bounding boxes
----------------------------------------------------

In this example, we will demonstrate the forward mapping algorithm by using it to determine approximate bounding
boxes on the ground.

.. code-block:: python

    from isce3.core import DateTime, LookSide, make_projection
    from isce3.product import RadarGridParameters
    from isce3.geometry import get_geo_perimeter_wkt

    #See above for implementation details
    arc = loadOrbit('orbit_arc.txt')

    #Create radar grid, but sync referenceEpoch for fast computation
    grid = RadarGridParameters(
        lookside = LookSide.Left,
        wavelength = 0.06,
        starting_range = 8e5,
        range_pixel_spacing = 10.,
        width = 1000,
        ref_epoch = arc.reference_epoch,
        sensing_start = (DateTime("2023-01-03T14:21:55.125") - arc.reference_epoch).total_seconds()
        prf = 1000.,
        length = 1500,
    )

    assert(grid.ref_epoch == arc.reference_epoch)


    # Use perimeter functionality
    epsg = make_projection(4326)
    box = get_geo_perimeter_wkt(grid, arc, epsg, points_per_edge=5)

    # box is a WKT string
    print(box)

.. note::
    We could also have implemented the perimeter estimation by looping over points on the edge of 
    the swath and using isce3.geometry.rdr2geo function with appropriate inputs

.. _invmap:

Inverse mapping example - locating known targets
----------------------------------------------------

In this example, we will demonstrate the inverse mapping algorithm by using it to determine the location of a known
target in a radar image. For this example, we will use the coordinates of the estimated perimeter above and reuse
the orbit data structure

.. code-block:: python

   from isce3.core import LUT2d
   from isce3.geometry import geo2rdr
   import json
   import numpy as np

   ##Get points from Geojson
   targets = json.loads(box)['coordinates']

   #Set up zero doppler
   doppler = LUT2d()

   #Get ellipsoid spec
   elp = epsg.ellipsoid() 

   for targ in targets:
      #Convert from degrees to radians
      llh = [np.radians(targ[0]), np.radians(targ[1]), targ[2]]

      #Estimate target position
      taz, rng = geo2rdr(llh, elp, arc, doppler,
                         grid.wavelength, grid.lookside)

      #Line, pixel number
      print('Target at: ', *targ)
      print('Estimated line number: ', (taz - grid.sensing_start) * grid.prf)
      print('Estimated pixel number: ',(rng - grid.starting_range) / grid.range_pixel_spacing)


.. note:: The threshold parameter to geo2rdr determines the accuracy of the inversion. For precise
    location, use threshold on order of 1.0e-6. Default threshold at Python level is on order of cm, 
    which is generally good enough for bounding box estimates. 
