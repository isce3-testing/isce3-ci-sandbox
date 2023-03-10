#include "product.h"

#include "RadarGridParameters.h"
#include "GeoGridParameters.h"
#include "SubSwaths.h"
#include "Swath.h"
#include "Grid.h"
#include "GeoGridProductClass.h"

namespace py = pybind11;

void addsubmodule_product(py::module & m)
{
    py::module m_product = m.def_submodule("product");

    // forward declare bound classes
    py::class_<isce3::product::GeoGridParameters> pyGeoGridParameters(
        m_product, "GeoGridParameters");
    py::class_<isce3::product::RadarGridParameters> pyRadarGridParameters(
        m_product, "RadarGridParameters");
    py::class_<isce3::product::SubSwaths> pySubSwaths(m_product, "SubSwaths");
    py::class_<isce3::product::Swath> pySwath(m_product, "Swath");
    py::class_<isce3::product::Grid> pyGrid(m_product, "Grid");
    py::class_<isce3::product::GeoGridProduct> pyGeoGridProduct(
        m_product, "GeoGridProduct");

    // add bindings
    addbinding(pyGeoGridParameters);
    addbinding(pyRadarGridParameters);
    addbinding(pySubSwaths);
    addbinding(pySwath);
    addbinding(pyGrid);
    addbinding(pyGeoGridProduct);
    addbinding_bbox_to_geogrid(m_product);
}
