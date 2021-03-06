# observation.py
#
# Copyright (C) 2016
# ASTRON (Netherlands Institute for Radio Astronomy)
# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
#
# This file is part of the LOFAR software suite.
# The LOFAR software suite is free software: you can redistribute it
# and/or modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation, either version 3 of the
# License, or (at your option) any later version.
#
# The LOFAR software suite is distributed in the hope that it will be
# useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
#
# $Id: observation.py 33534 2016-02-08 14:28:26Z schaap $

import logging
from math import ceil, floor
from base_resource_estimator import BaseResourceEstimator

logger = logging.getLogger(__name__)

COBALT = "Observation.ObservationControl.OnlineControl.Cobalt."

class ObservationResourceEstimator(BaseResourceEstimator):
    """ ResourceEstimator for LOFAR Observations
    """
    def __init__(self):
        logger.info("init ObservationResourceEstimator")
        super(ObservationResourceEstimator, self).__init__(name='observation')
        self.required_keys = ('Observation.sampleClock',
                              'Observation.startTime',
                              'Observation.stopTime',
                              'Observation.antennaSet',
                              COBALT + 'Correlator.nrChannelsPerSubband',
                              COBALT + 'Correlator.integrationTime',
                              COBALT + 'BeamFormer.flysEye',
                              COBALT + 'BeamFormer.CoherentStokes.timeIntegrationFactor',
                              COBALT + 'BeamFormer.IncoherentStokes.timeIntegrationFactor',
                              'Observation.VirtualInstrument.stationList',
                              'Observation.DataProducts.Output_CoherentStokes.enabled',
                              COBALT + 'BeamFormer.CoherentStokes.which',
                              'Observation.DataProducts.Output_IncoherentStokes.enabled',
                              COBALT + 'BeamFormer.IncoherentStokes.which'
                              )

    def _calculate(self, parset, input_files={}):
        """ Calculate the combined resources needed by the different observation types that
        can be in a single observation.
        reply is something along the lines of:
        {'bandwidth': {'total_size': 19021319494},
        'storage': {'total_size': 713299481024,
        'output_files': 
          {'uv': {'nr_of_uv_files': 481, 'uv_file_size': 1482951104},
          'saps': [{'sap_nr': 0, 'properties': {'nr_of_uv_files': 319}},
                   {'sap_nr': 1, 'properties': {'nr_of_uv_files': 81}}, 
                   {'sap_nr': 2, 'properties': {'nr_of_uv_files': 81}}
        ]}}}
        The base_resource_estimator adds an {'observation': } around this.
        """
        logger.info("start estimate '{}'".format(self.name))
        logger.info('parset: %s ' % parset)
        duration = self._getDuration(parset.getString('Observation.startTime'), parset.getString('Observation.stopTime'))
        
        result = {}
        output_files = {}
        correlated_size, correlated_bandwidth, output_files_uv, correlated_saps = self.correlated(parset, duration)
        coherentstokes_size, coherentstokes_bandwidth, output_files_cs, coherentstokes_saps = self.coherentstokes(parset, duration)
        incoherentstokes_size, incoherentstokes_bandwidth, output_files_is, incoherentstokes_saps = self.incoherentstokes(parset, duration)
        
        if output_files_uv:
            output_files['uv'] = output_files_uv
        if output_files_cs:
            output_files['cs'] = output_files_cs
        if output_files_is:
            output_files['is'] = output_files_is

        output_files['saps'] = []
        for sap_nr in xrange(parset.getInt('Observation.nrBeams')):
            sap = {'sap_nr': sap_nr, 'properties': {}}
            if sap_nr in correlated_saps:
                sap['properties'].update(correlated_saps[sap_nr])
            if sap_nr in coherentstokes_saps:
                sap['properties'].update(coherentstokes_saps[sap_nr])
            if sap_nr in incoherentstokes_saps:
                sap['properties'].update(incoherentstokes_saps[sap_nr])
            output_files['saps'].append(sap)

        total_data_size = correlated_size + coherentstokes_size + incoherentstokes_size
        result['storage'] = {'total_size': total_data_size, 'output_files': output_files}
        result['bandwidth'] = {'total_size': correlated_bandwidth + coherentstokes_bandwidth + incoherentstokes_bandwidth}
        return result

    def correlated(self, parset, duration):
        """ Estimate number of files, file size and bandwidth needed for correlated data
        """
        logger.info("calculating correlated datasize")
        size_of_header   = 512 #TODO More magic numbers (probably from Alwin). ScS needs to check these. They look ok though.
        size_of_overhead = 600000
        size_of_short    = 2
        size_of_complex  = 8
        nr_polarizations = 2
        channels_per_subband = parset.getInt(COBALT + 'Correlator.nrChannelsPerSubband', 64) #TODO should these have defaults?
        intergration_time = parset.getFloat(COBALT + 'Correlator.integrationTime', 1)
        nr_virtual_stations = self._virtual_stations(parset)

        integrated_seconds = floor(duration / intergration_time)
        nr_baselines = nr_virtual_stations * (nr_virtual_stations + 1.0) / 2.0 #TODO Why is this done in float?
        data_size = ceil((nr_baselines * channels_per_subband * nr_polarizations**2 * size_of_complex) / 512.0) * 512.0 #TODO What's the 512.0 magic numbers?
        n_sample_size = ceil((nr_baselines * channels_per_subband * size_of_short) / 512.0) * 512.0

        # sum of all subbands in all digital beams
        total_files = 0
        sap_files = {}
        
        for sap_nr in xrange(parset.getInt('Observation.nrBeams')):
            subbandList = parset.getStringVector('Observation.Beam[%d].subbandList' % sap_nr)
            nr_files = len(subbandList)
            total_files += nr_files
            sap_files[sap_nr]= {'nr_of_uv_files': nr_files}

        file_size = int((data_size + n_sample_size + size_of_header) * integrated_seconds + size_of_overhead)
        output_files = {'nr_of_uv_files': total_files, 'uv_file_size': file_size}
        logger.info("correlated_uv: {} files {} bytes each".format(total_files, file_size))

        total_data_size = int(ceil(file_size * total_files))  # bytes
        total_bandwidth = int(ceil((total_data_size * 8) / duration))  # bits/second
        return (total_data_size, total_bandwidth, output_files, sap_files)

    def coherentstokes(self, parset, duration):
        """  Estimate number of files, file size and bandwidth needed for coherent stokes
        """
        if not parset.getBool('Observation.DataProducts.Output_CoherentStokes.enabled'):
            return (0,0, {}, {})
            
        logger.info("calculate coherentstokes datasize")
        coherent_type = parset.getString(COBALT + 'BeamFormer.CoherentStokes.which')
        subbands_per_file = parset.getInt(COBALT + 'BeamFormer.CoherentStokes.subbandsPerFile', 512)
        samples_per_second = self._samples_per_second(parset)
        integration_factor = parset.getInt(COBALT + 'BeamFormer.CoherentStokes.timeIntegrationFactor')

        nr_coherent = 4 if coherent_type in ('XXYY', 'IQUV') else 1
        if coherent_type in ('XXYY',):
            size_per_subband = samples_per_second * 4.0 * duration
        else:
            size_per_subband = (samples_per_second * 4.0 * duration) / integration_factor

        total_nr_stokes = 0
        total_files     = 0
        max_nr_subbands = 0
        sap_files       = {}
        
        for sap_nr in xrange(parset.getInt('Observation.nrBeams')):
            logger.info("checking SAP {}".format(sap_nr))
            subbandList = parset.getStringVector('Observation.Beam[%d].subbandList' % sap_nr)
            nr_subbands = len(subbandList)
            max_nr_subbands = max(nr_subbands, max_nr_subbands)
            nr_files = 0
            total_nr_tabs = 0
            
            for tab_nr in xrange(parset.getInt('Observation.Beam[%d].nrTiedArrayBeams' % sap_nr)):
                logger.info("checking TAB {}".format(tab_nr))
                if parset.getBool("Observation.Beam[%d].TiedArrayBeam[%d].coherent" % (sap_nr, tab_nr)):
                    logger.info("adding coherentstokes size")
                    nr_stokes = nr_coherent #TODO what does min mean here?
                    total_nr_tabs += 1
                    total_nr_stokes += nr_stokes
                    nr_files += int(nr_stokes * ceil(nr_subbands / float(subbands_per_file)))

            nr_tab_rings = parset.getInt('Observation.Beam[%d].nrTabRings' % sap_nr)
            if nr_tab_rings > 0:
                logger.info("adding size for {} tab_rings".format(nr_tab_rings))
                nr_tabs = (3 * nr_tab_rings * (nr_tab_rings + 1) + 1)
                total_nr_tabs += nr_tabs
                nr_stokes = nr_tabs * nr_coherent
                total_nr_stokes += nr_stokes
                nr_files += int(nr_stokes * ceil(nr_subbands / float(subbands_per_file)))

            if parset.getBool(COBALT + 'BeamFormer.flysEye'):
                logger.info("adding flys eye data size")
                nr_tabs = self._virtual_stations(parset)
                total_nr_tabs += nr_tabs
                nr_stokes = nr_tabs * nr_coherent
                total_nr_stokes += nr_stokes
                nr_files += int(nr_stokes * ceil(nr_subbands / float(subbands_per_file)))

            sap_files[sap_nr]= {'nr_of_cs_files': nr_files, 'nr_of_tabs': total_nr_tabs}
            total_files += nr_files

        nr_subbands_per_file = min(subbands_per_file, max_nr_subbands)
        size_per_file = int(nr_subbands_per_file * size_per_subband)

        output_files = {'nr_of_cs_files': total_files, 'nr_of_cs_stokes': nr_coherent, 'cs_file_size': size_per_file}
        logger.info("coherentstokes: {} files {} bytes each".format(total_files, size_per_file))

        total_data_size = int(ceil(total_nr_stokes * max_nr_subbands * size_per_subband))
        total_bandwidth = int(ceil((total_data_size * 8) / duration)) # bits/second
        return (total_data_size, total_bandwidth, output_files, sap_files)

    def incoherentstokes(self, parset, duration):
        """  Estimate number of files, file size and bandwidth needed for incoherentstokes
        """
        if not parset.getBool('Observation.DataProducts.Output_IncoherentStokes.enabled'):
            return (0,0, {}, {})
            
        logger.info("calculate incoherentstokes data size")
        incoherent_type = parset.getString(COBALT + 'BeamFormer.IncoherentStokes.which')
        subbands_per_file = parset.getInt(COBALT + 'BeamFormer.CoherentStokes.subbandsPerFile', 512)
        samples_per_second = self._samples_per_second(parset)
        time_integration_factor = parset.getInt(COBALT + 'BeamFormer.IncoherentStokes.timeIntegrationFactor')
        channels_per_subband = parset.getInt(COBALT + 'Correlator.nrChannelsPerSubband', 64) #TODO should these have defaults?
        incoherent_channels_per_subband = parset.getInt(COBALT + 'BeamFormer.IncoherentStokes.nrChannelsPerSubband', 0)

        nr_incoherent = 4 if incoherent_type in ('IQUV',) else 1

        total_nr_stokes = 0
        total_files     = 0
        max_nr_subbands = 0
        sap_files       = {}

        for sap_nr in xrange(parset.getInt('Observation.nrBeams')):
            logger.info("checking SAP {}".format(sap_nr))
            subbandList = parset.getStringVector('Observation.Beam[%d].subbandList' % sap_nr)
            nr_subbands = len(subbandList)
            max_nr_subbands = max(nr_subbands, max_nr_subbands)
            nr_files = 0
            total_nr_tabs = parset.getInt('Observation.Beam[%d].nrTiedArrayBeams' % sap_nr)
            for tab_nr in xrange(total_nr_tabs):
                logger.info("checking TAB {}".format(tab_nr))
                if not parset.getBool("Observation.Beam[%d].TiedArrayBeam[%d].coherent" % (sap_nr, tab_nr)):
                    logger.info("adding incoherentstokes size")
                    total_nr_stokes += nr_incoherent
                    nr_files += int(nr_incoherent * ceil(nr_subbands / float(subbands_per_file)))

            sap_files[sap_nr]= {'nr_of_is_files': nr_files, 'nr_of_tabs': total_nr_tabs}
            total_files += nr_files

        if incoherent_channels_per_subband > 0:
            channel_integration_factor = channels_per_subband / incoherent_channels_per_subband
        else:
            channel_integration_factor = 1

        if total_files > 0:
            nr_subbands_per_file = min(subbands_per_file, max_nr_subbands)
            size_per_subband = int((samples_per_second * 4) / time_integration_factor / channel_integration_factor * duration)
            size_per_file = nr_subbands_per_file * size_per_subband

        output_files = {'nr_of_is_files': total_files, 'nr_of_is_stokes': nr_incoherent, 'is_file_size': int(size_per_file)}
        logger.info("incoherentstokes: {} files {} bytes each".format(total_files, size_per_file))

        total_data_size = int(ceil(total_nr_stokes * max_nr_subbands * size_per_subband))  # bytes
        total_bandwidth = int(ceil((total_data_size * 8) / duration))  # bits/sec
        return (total_data_size, total_bandwidth, output_files, sap_files)

    def _samples_per_second(self, parset):
        """ set samples per second
        """
        samples_160mhz = 155648
        samples_200mhz = 196608
        sample_clock = parset.getInt('Observation.sampleClock')
        samples = samples_160mhz if 160 == sample_clock else samples_200mhz
        logger.info("samples per second for {} MHz clock = {}".format(sample_clock, samples))
        return samples

    def _virtual_stations(self, parset):
        """ calculate virtualnumber of stations
        """
        stationList = parset.getStringVector('Observation.VirtualInstrument.stationList')
        nr_virtual_stations = 0
        if parset.getString('Observation.antennaSet') in ('HBA_DUAL', 'HBA_DUAL_INNER'):
            for station in stationList:
                if 'CS' in station:
                    nr_virtual_stations += 2
                else:
                    nr_virtual_stations += 1
        else:
            nr_virtual_stations = len(stationList)
        logger.info("number of virtual stations = {}".format(nr_virtual_stations))
        return nr_virtual_stations

