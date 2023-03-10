import logging
from isce3.io import decode_bfpq_lut
from nisar.types import complex32
import numpy as np

# TODO some CSV logger
log = logging.getLogger("Raw")

class DataDecoder(object):
    """Handle the various data types floating around for raw data, currently
    complex32, complex64, and lookup table.  Indexing operatations always return
    data converted to complex64.
    """
    def __getitem__(self, key):
        return self.decoder(key)

    def _decode_lut(self, key):
        z = self.dataset[key]
        assert self.table is not None
        # Only have 2D version in C++, fall back to Python otherwise
        if z.ndim == 2:
            return decode_bfpq_lut(self.table, z)
        else:
            return self.table[z['r']] + 1j * self.table[z['i']]

    def _decode_complex32(self, key):
        with self.dataset.astype(np.complex64):
            z = self.dataset[key]
        return z

    def __init__(self, h5dataset):
        self.table = None
        self.decoder = lambda key: self.dataset[key]
        self.dataset = h5dataset
        self.shape = self.dataset.shape
        self.ndim = self.dataset.ndim        
        self.dtype = np.dtype('c8')
        self.dtype_storage = self.dataset.dtype
        group = h5dataset.parent
        if "BFPQLUT" in group:
            assert group["BFPQLUT"].dtype == np.float32
            self.table = np.asarray(group["BFPQLUT"])
            self.decoder = self._decode_lut
            log.info("Decoding raw data with lookup table.")
        elif h5dataset.dtype == complex32:
            self.decoder = self._decode_complex32
            log.info("Decoding raw data from float16 encoding.")
        else:
            assert h5dataset.dtype == np.complex64
            log.info("Decoding raw data not required")
