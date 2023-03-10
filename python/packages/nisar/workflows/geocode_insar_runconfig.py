import os

import journal
import nisar.workflows.helpers as helpers
from nisar.workflows.runconfig import RunConfig


class GeocodeInsarRunConfig(RunConfig):
    def __init__(self, args):
        # all insar submodules share a commmon `insar` schema
        super().__init__(args, 'insar')

        if self.args.run_config_path is not None:
            self.load_geocode_yaml_to_dict()
            self.geocode_common_arg_load()
            self.yaml_check()

    def yaml_check(self):
        '''
        Check GUNW specifics from YAML
        '''
        error_channel = journal.error('GeocodeInsarRunConfig.yaml_check')

        if 'runw_path' not in self.cfg['processing']['geocode']:
            err_str = "'runw_path' file path under `geocode' required for standalone execution with YAML"
            error_channel.log(err_str)
            raise ValueError(err_str)

        # Check if runw path is a directory or a file
        runw_path = self.cfg['processing']['geocode']['runw_path']
        if not os.path.isfile(runw_path):
            err_str = f"{runw_path} is invalid; needs to be a file"
            error_channel.log(err_str)
            raise ValueError(err_str)

        # Check if required polarizations/frequency are in provided HDF5 file
        freq_pols = self.cfg['processing']['input_subset']['list_of_frequencies']
        helpers.check_hdf5_freq_pols(runw_path, freq_pols)

        # create empty dict if geocode_datasets not in geocode
        for datasets in ['gunw_datasets', 'goff_datasets']:
            if datasets not in self.cfg['processing']['geocode']:
                self.cfg['processing']['geocode'][datasets] = {}

        # Initialize GUNW and GOFF names
        gunw_datasets = ['connected_components', 'coherence_magnitude',
                         'ionosphere_phase_screen', 
                         'ionosphere_phase_screen_uncertainty',
                         'unwrapped_phase', 'along_track_offset',
                         'slant_range_offset', 'layover_shadow_mask']
        goff_datasets = ['along_track_offset', 'snr',
                         'along_track_offset_variance',
                         'correlation_surface_peak',
                         'cross_offset_variance',
                         'slant_range_offset',
                         'slant_range_offset_variance']
        # insert both geocode datasets in dict keyed on datasets name
        geocode_datasets = {'gunw_datasets': gunw_datasets,
                            'goff_datasets': goff_datasets}
        for dataset_group in geocode_datasets:
            for dataset in geocode_datasets[dataset_group]:
                if dataset not in self.cfg['processing']['geocode'][
                   dataset_group]:
                   self.cfg['processing']['geocode'][dataset_group][
                            dataset] = True

        # multilooks valid?
        az_looks = self.cfg['processing']['crossmul']['azimuth_looks']
        if az_looks > 1 and az_looks % 2 == 0:
            err_str = f"azimuth looks = {az_looks} not an odd integer."
            error_channel.log(err_str)
            raise ValueError(err_str)

        rg_looks = self.cfg['processing']['crossmul']['range_looks']
        if rg_looks > 1 and rg_looks % 2 == 0:
            err_str = f"range looks = {rg_looks} not an odd integer."
            error_channel.log(err_str)
            raise ValueError(err_str)

