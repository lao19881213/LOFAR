#!/usr/bin/env python

import time
from optparse import OptionParser

from CS1_Hosts import *
from CS1_Parset import CS1_Parset
from CS1_RSPCtl import RSPCtl
from CS1_Sections import *

def doObservation(obsID, parset):
    if False:
        rsp = RSPCtl(CS10LCU)
        rsp.initBoards(parset.getClockString())
        rsp.selectSubbands(parset.getFirstSubband())
        rsp.selectRCUs([0, 1, 2, 8, 9])
        rsp.setWG([2], 60e6)
    

    sections = [\
        #DelayCompensationSection(parset, listfen),\
        InputSection(parset, liifen),\
        BGLProcSection(parset, bglfen3, 'R000_B00', '/bglhome2/lofarsystem'),\
        StorageSection(parset, listfen)\
        #Flagger(parset, listfen)\
        ]


    noRuns = int(parset['Observation.StopTime']) - int(parset['Observation.StartTime'])
    #print int(parset['Observation.StopTime']), int(parset['Observation.StartTime']), noRuns

    parset.writeToFile('/log/' + parset.getMSName().split('/')[-1] + '.parset')
    for section in sections:
        print ('Starting ' + section.package)
        runlog = '/log/' + obsID + '.' + section.getName() + '.runlog'
        section.run(runlog, noRuns)
        print 

    for section in sections:
        print ('Waiting for ' + section.package + ' to finish ...')
        ret = section.isRunSuccess()
        print (ret)
        print 

if __name__ == '__main__':

    parser = OptionParser()

    # do not use the callback actions of the OptionParser, because we must make sure we read the parset before adding any variables
    parser.add_option('--parset'         , dest='parset'         , default='CS1.parset', type='string', help='name of the parameterset [%default]')
    parser.add_option('--clock'          , dest='clock'          , default='160MHz'    , type='string', help='clock frequency (either 160MHz or 200MHz) [%default]')
    parser.add_option('--subbands'       , dest='subbands'       , default='60MHz,8'   , type='string', help='freq of first subband and number of subbands to use [%default]')
    parser.add_option('--stations'       , dest='stations'       , default='12'        , type='int'   , help='number of stations (or microstations [%default]')
    parser.add_option('--runtime'        , dest='runtime'        , default='600'       , type='int'   , help='length of measurement in seconds [%default]')
    parser.add_option('--starttime'     , dest='starttime', default=int(time.time() + 100), type='int', help='start of measurement in UTC seconds [now + 100s]')
    parser.add_option('--integrationtime', dest='integrationtime', default='60'        , type='int'   , help='length of integration interval in seconds [%default]')
    parser.add_option('--msname'         , dest='msname'                               , type='string', help='name of the measurement set')

    # parse the options
    (options, args) = parser.parse_args()

    # create the parset
    parset = CS1_Parset()    

    # todo: set following variables
    parset['General.SubbandsPerPset'] = 8
    parset['BGLProc.NodesPerPset'] = 8
    parset['BGLProc.PsetsPerCell'] = 1

    parset.readFromFile(options.parset)
    parset.setClock(options.clock)
    parset.setNStations(options.stations)
    parset.setIntegrationTime(options.integrationtime)
    if options.msname:
        parset.setMSName(options.msname)

    # read the subbands
    parts = options.subbands.split(',')
    nsb = int(parts[-1])
    first = ','.join(parts[:-1])
    parset.setSubbands(first, nsb)

    # read the runtime (optional start in utc and the length of the measurement)
    parset.setInterval(options.starttime, options.runtime)

    # if the msname wasn't given, read the next number from the file
    runningNumberFile = '/log/nextMSNumber'

    if not 'Storage.MSName' in parset:
        try:
            inf = open(runningNumberFile, 'r')
            measurementnumber = int(inf.readline())
            inf.close()
            MSName = '/data/' + str(measurementnumber) + '.MS'
            outf = open(runningNumberFile, 'w')
            outf.write(str(measurementnumber + 1) + '\n')
            outf.close()
        except:
            MSName = '/data/Test.MS'
        parset['Storage.MSName'] = MSName
            

    obsID = parset['Storage.MSName'].strip('.MS').split('/')[-1]



    # start the observation
    doObservation(obsID, parset)
