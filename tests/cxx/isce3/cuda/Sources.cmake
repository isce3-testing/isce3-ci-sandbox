set(CUDA_TESTFILES
container/cuda-radargeometry.cu
core/ellipsoid/gpuEllipsoid.cu
core/compute_capability.cpp
core/cuda-interp1d.cu
core/device.cpp
core/interpolator/gpuInterpolator.cpp
core/lut/gpuLUT1d.cpp
core/lut/gpuLUT2d.cpp
core/orbit/cuda-orbit.cu
core/poly/gpuPoly2d.cpp
core/projections/gpuCEA.cpp
core/projections/gpuGeocent.cpp
core/projections/gpuPolar.cpp
core/projections/gpuUTM.cpp
core/sinc2dinterpolator/gpuSinc2dInterpolator.cpp
core/stream/event.cu
core/stream/stream.cu
except/checkCudaErrors.cu
fft/cuda-fft.cu
fft/cuda-fftplan.cu
geocode/MaskedMinMax.cu
geometry/geo2rdr/gpuGeo2rdr.cpp
geometry/geometry/gpuGeometry.cpp
geometry/topo/gpuTopo.cpp
image/resampslc/gpuResampSlc.cpp
io/datastream/datastream.cu
signal/gpuCrossMul.cpp
signal/gpuFilter.cpp
signal/gpuLooks.cpp
signal/gpuSignal.cpp
)
