#cython: language_level=3
#
#
#

from GeoGridParameters cimport GeoGridParameters

cdef class pyGeoGridParameters:
    """
    Cython wrapper for isce::product::GeoGridParameters.
    """
    cdef GeoGridParameters * c_geogrid
    cdef bool __owner

    #def __cinit__(self, startX=None, startY=None,
    #            spacingX=None, spacingY=None, 
    #            width=None, length=None, 
    #            epsg=None):

    #    self.c_geogrid = new GeoGridParameters(startX, startY,
    #                            spacingX, spacingY, width, length, epsg)
    def __cinit__(self):
        self.c_geogrid = new GeoGridParameters()
        self.__owner = True
        return

    def __dealloc__(self):
        if self.__owner:
            del self.c_geogrid
    
    @staticmethod
    cdef cbind(GeoGridParameters geoGrid):
        """
        Creates a new pyGeoGridParameters instance from a C++ GeoGridParameters instance.
        """
        new_grid = pyGeoGridParameters()
        del new_grid.c_geogrid
        new_grid.c_geogrid = new GeoGridParameters(geoGrid)
        new_grid.__owner = True
        return new_grid

    @property
    def startX(self):
        """
        Returns the starting X coordinate of the geo-grid
        """
        return self.c_geogrid.startX()

    @startX.setter
    def startX(self, val):
        self.c_geogrid.startX(val)

    @property
    def startY(self):
        """
        Returns the starting Y coordinate of the geo-grid
        """
        return self.c_geogrid.startY()

    @startY.setter
    def startY(self, val):
        self.c_geogrid.startY(val)

    @property
    def spacingX(self):
        """
        Returns the spacing X coordinate of the geo-grid
        """
        return self.c_geogrid.spacingX()

    @spacingX.setter
    def spacingX(self, val):
        self.c_geogrid.spacingX(val)

    @property
    def spacingY(self):
        """
        Returns the spacing Y coordinate of the geo-grid
        """
        return self.c_geogrid.spacingY()

    @spacingY.setter
    def spacingY(self, val):
        self.c_geogrid.spacingY(val)

    @property
    def width(self):
        return self.c_geogrid.width()

    @width.setter
    def width(self, val):
        self.c_geogrid.width(val)

    @property
    def length(self):
        return self.c_geogrid.length()

    @length.setter
    def length(self, val):
        self.c_geogrid.length(val)

    @property
    def epsg(self):
        return self.c_geogrid.epsg()

    @epsg.setter
    def epsg(self, val):
        self.c_geogrid.epsg(val) 

