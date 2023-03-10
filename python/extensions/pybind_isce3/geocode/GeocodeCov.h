#pragma once

#include <isce3/geocode/GeocodeCov.h>
#include <pybind11/pybind11.h>

template<typename T>
void addbinding(pybind11::class_<isce3::geocode::Geocode<T>>&);
void addbinding(pybind11::enum_<isce3::geocode::geocodeOutputMode> &);
