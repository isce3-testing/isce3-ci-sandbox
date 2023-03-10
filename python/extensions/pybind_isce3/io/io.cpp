#include "io.h"
#include "decode_bfpq_lut.h"
#include "Raster.h"
#include "serialization.h"

#include "gdal/gdal.h"

void addsubmodule_io(py::module & m)
{
    py::module m_io = m.def_submodule("io");

    addsubmodule_gdal(m_io);

    py::class_<isce3::io::Raster> pyRaster(m_io, "Raster");

    addbinding(pyRaster);
    addbinding_decode_bfpq_lut(m_io);
    addbinding_serialization(m_io);
}
