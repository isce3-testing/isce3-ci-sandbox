// -*- C++ -*-
// -*- coding: utf-8 -*-
//
// Author: Bryan V. Riel
// Copyright 2017-2018

#include <chrono>
#include "Geo2rdr.h"
#include "utilities.h"
#include "gpuGeo2rdr.h"

// pull in some isce namespaces
using isce3::core::Ellipsoid;
using isce3::core::Orbit;
using isce3::core::LUT1d;
using isce3::core::DateTime;
using isce3::product::RadarGridParameters;
using isce3::io::Raster;

// Run geo2rdr with no offsets; internal creation of offset rasters
/** @param[in] topoRaster outputs of topo -i.e, pixel-by-pixel x,y,h as bands
  * @param[in] outdir directory to write outputs to
  * @param[in] azshift Number of lines to shift by in azimuth
  * @param[in] rgshift Number of pixels to shift by in range
  *
  * This is the main geo2rdr driver. The pixel-by-pixel output filenames are fixed for now
  * <ul>
  * <li>azimuth.off - Azimuth offset to be applied to product to align with topoRaster
  * <li>range.off - Range offset to be applied to product to align with topoRaster
*/
void isce3::cuda::geometry::Geo2rdr::
geo2rdr(isce3::io::Raster & topoRaster,
        const std::string & outdir,
        double azshift, double rgshift) {

    // Cache the size of the DEM images
    const size_t demWidth = topoRaster.width();
    const size_t demLength = topoRaster.length();

    // Create output rasters
    Raster rgoffRaster = Raster(outdir + "/range.off", demWidth, demLength, 1,
        GDT_Float64, "ISCE");
    Raster azoffRaster = Raster(outdir + "/azimuth.off", demWidth, demLength, 1,
        GDT_Float64, "ISCE");

    // Call main geo2rdr with offsets set to zero
    geo2rdr(topoRaster, rgoffRaster, azoffRaster, azshift, rgshift);
}

// Run geo2rdr with externally created offset rasters
/** @param[in] topoRaster outputs of topo - i.e, pixel-by-pixel x,y,h as bands
  * @param[in] outdir directory to write outputs to
  * @param[in] rgoffRaster range offset output
  * @param[in] azoffRaster azimuth offset output
  * @param[in] azshift Number of lines to shift by in azimuth
  * @param[in] rgshift Number of pixels to shift by in range */
void isce3::cuda::geometry::Geo2rdr::
geo2rdr(isce3::io::Raster & topoRaster,
        isce3::io::Raster & rgoffRaster,
        isce3::io::Raster & azoffRaster,
        double azshift, double rgshift) {

    // Create reusable pyre::journal channels
    pyre::journal::warning_t warning("isce.cuda.geometry.Geo2rdr");
    pyre::journal::info_t info("isce.cuda.geometry.Geo2rdr");

    // Cache the size of the DEM images
    const size_t demWidth = topoRaster.width();
    const size_t demLength = topoRaster.length();

    // Cache EPSG code for topo results
    const int topoEPSG = topoRaster.getEPSG();

    // Cache ISCE objects (use public interface of parent isce3::geometry::Geo2rdr class)
    const Ellipsoid & ellipsoid = this->ellipsoid();
    const Orbit & orbit = this->orbit();
    const LUT1d<double> doppler = isce3::core::avgLUT2dToLUT1d<double>(this->doppler());
    const RadarGridParameters & radarGrid = this->radarGridParameters();

    // Cache sensing start in seconds since reference epoch
    double t0 = radarGrid.sensingStart();
    // Adjust for const azimuth shift
    t0 -= azshift / radarGrid.prf();

    // Cache starting range
    double r0 = radarGrid.startingRange();
    // Adjust for constant range shift
    r0 -= rgshift * radarGrid.rangePixelSpacing();

    // Compute azimuth time extents
    double dtaz = 1.0 / radarGrid.prf();
    const double tend = t0 + ((radarGrid.length() - 1) * dtaz);
    const double tmid = 0.5 * (t0 + tend);

    // Compute range extents
    const double dmrg = radarGrid.rangePixelSpacing();
    const double rngend = r0 + ((radarGrid.width() - 1) * dmrg);

    // Print out extents
    _printExtents(info, t0, tend, dtaz, r0, rngend, dmrg, demWidth, demLength);

    // Interpolate orbit to middle of the scene as a test
    _checkOrbitInterpolation(tmid);

    // Compute number of DEM blocks needed to process image
    size_t nBlocks = demLength / linesPerBlock();
    if ((demLength % linesPerBlock()) != 0)
        nBlocks += 1;

    // Cache near, mid, far ranges for diagnostics on Doppler
    const double nearRange = radarGrid.startingRange();
    const double farRange = radarGrid.endingRange();
    const double midRange = radarGrid.midRange();

    // Loop over blocks
    unsigned int totalconv = 0;
    for (size_t block = 0; block < nBlocks; ++block) {

        // Get block extents
        size_t lineStart, blockLength;
        lineStart = block * linesPerBlock();
        if (block == (nBlocks - 1)) {
            blockLength = demLength - lineStart;
        } else {
            blockLength = linesPerBlock();
        }
        size_t blockSize = blockLength * demWidth;

        // Diagnostics
        info << "Processing block: " << block << " " << pyre::journal::newline
             << "  - line start: " << lineStart << pyre::journal::newline
             << "  - line end  : " << lineStart + blockLength << pyre::journal::newline
             << "  - dopplers near mid far: "
             << doppler.eval(nearRange) << " "
             << doppler.eval(midRange) << " "
             << doppler.eval(farRange) << " "
             << pyre::journal::endl;

        // Valarrays to hold input block from topo rasters
        std::valarray<double> x(blockSize), y(blockSize), hgt(blockSize);
        // Valarrays to hold block of geo2rdr results
        std::valarray<double> rgoff(blockSize), azoff(blockSize);

        // Read block of topo data
        topoRaster.getBlock(x, 0, lineStart, demWidth, blockLength, 1);
        topoRaster.getBlock(y, 0, lineStart, demWidth, blockLength, 2);
        topoRaster.getBlock(hgt, 0, lineStart, demWidth, blockLength,3);

        // Process block on GPU
        isce3::cuda::geometry::runGPUGeo2rdr(
            ellipsoid, orbit, doppler, x, y, hgt, azoff, rgoff, topoEPSG,
            lineStart, demWidth, t0, r0,
            radarGrid.length(), radarGrid.width(), radarGrid.prf(),
            radarGrid.rangePixelSpacing(), radarGrid.wavelength(),
            radarGrid.lookSide(), this->threshold(), this->numiter(), totalconv
        );

        // Write block of data
        rgoffRaster.setBlock(rgoff, 0, lineStart, demWidth, blockLength);
        azoffRaster.setBlock(azoff, 0, lineStart, demWidth, blockLength);

    } // end for loop blocks in DEM image

    // Print out convergence statistics
    info << "Total convergence: " << totalconv << " out of "
         << (demWidth * demLength) << pyre::journal::endl;

}

// Print extents and image sizes
void isce3::cuda::geometry::Geo2rdr::
_printExtents(pyre::journal::info_t & info, double t0, double tend, double dtaz,
              double r0, double rngend, double dmrg, size_t demWidth, size_t demLength) {
    info
        << pyre::journal::newline
        << "Starting acquisition time: " << t0 << pyre::journal::newline
        << "Stop acquisition time: " << tend << pyre::journal::newline
        << "Azimuth line spacing in seconds: " << dtaz << pyre::journal::newline
        << "Slant range spacing in meters:" << dmrg << pyre::journal::newline
        << "Near range (m): " << r0 << pyre::journal::newline
        << "Far range (m): " << rngend << pyre::journal::newline
        << "Radar image length: " << this->radarGridParameters().length() << pyre::journal::newline
        << "Radar image width: " << this->radarGridParameters().width() << pyre::journal::newline
        << "Geocoded lines: " << demLength << pyre::journal::newline
        << "Geocoded samples: " << demWidth << pyre::journal::newline;
}

// Check we can interpolate orbit to middle of DEM
void isce3::cuda::geometry::Geo2rdr::
_checkOrbitInterpolation(double aztime) {
    isce3::core::cartesian_t satxyz, satvel;
    this->orbit().interpolate(&satxyz, &satvel, aztime);
}
