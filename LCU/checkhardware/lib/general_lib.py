r"""
general script 
"""

from subprocess import (Popen, PIPE)
import traceback
import time
import os
import sys
import logging

general_version = '0913'
logger = logging.getLogger()

def writeMessage(msg):
    sendCmd('wall', msg)
    return

# Return date string in the following format YYYYMMDD
def getShortDateStr(tm=time.gmtime()):
    return (time.strftime("%Y%m%d", tm))

# Return time string in the following format HH:MM:SS
def getDateStr(tm=time.gmtime()):
    return (time.strftime("%d-%m-%Y", tm))    
    
# Return time string in the following format HH:MM:SS
def getTimeStr(tm=time.gmtime()):
    return (time.strftime("%H:%M:%S", tm))
# Return time string in the following format HH:MM:SS
def getDateTimeStr(tm=time.gmtime()):
    return (time.strftime("%d-%m-%YT%H:%M:%S", tm))    

# Run cmd with args and return response 
def sendCmd(cmd='', args=''):
    if cmd != '':
        try:
            args = args.replace(' =','=').replace('= ','=')
            cmdList = [cmd] + args.split()
            #print cmdList
            cmdline = Popen(cmdList, stdout=PIPE, stderr=PIPE )
            (so, se) = cmdline.communicate()
            if len(so) != 0:
                return (so)
            else:
                return ('Error, %s' % se)
        except:
            logger.error('Caught %s', str(sys.exc_info()[0]))
            logger.error(str(sys.exc_info()[1]))
            logger.error('TRACEBACK:\n%s', traceback.format_exc())
            return ('Exception Error')
            
    return ('')

# Get Host name
def getHostName():
    try:
        host = sendCmd('hostname', '-s')
        if host == 'Exception Error':
            host = 'Unknown'
    except:
        host = 'Unknown'
    return (host.strip())
    
# file logger
class cLogger:
    def __init__(self, logdir, filename, screenPrefix=''):
        self.fullFilename = os.path.join(logdir,filename)
        self.logfile = open(self.fullFilename, 'w')
        self.prefix = screenPrefix
        self.starttime = time.time()

    def __del__(self):
        self.logfile.close()
    
    def getFullFileName(self):
        return (self.fullFilename)
    
    def resetStartTime(self, screen=False):
        self.starttime = time.time()
        self.info("Start time %s" %(time.strftime("%H:%M:%S", time.gmtime(self.starttime))), screen=screen)
        
    def printBusyTime(self, screen=False):
        self.info("Time from start %s" %(time.strftime("%H:%M:%S", (time.gmtime(time.time() - self.starttime)))), screen=screen)
    
    def printTimeNow(self, screen=False):
        self.info("Time %s" %(time.strftime("%H:%M:%S", time.gmtime(time.time()))), screen=screen)
                
    def info(self, msg, noEnd=False, screen=False):
        if len(msg) != 0:
            if screen:
                print self.prefix+' '+msg
            if noEnd == False:
                msg += '\n'
            self.logfile.write(msg)
            self.logfile.flush()

class cTestLogger(cLogger):
    def __init__(self, logdir):
        filename = getHostName() + "_StationTest" + '.csv'
        cLogger.__init__(self, logdir, filename)
             
    def addLine(self, info):
        cLogger.info(self, info)

class cStationLogger(cLogger):
    def __init__(self, logdir, filetime=time.gmtime()):
        filename = "stationtest_" + getHostName() + '.log'        
        cLogger.__init__(self, logdir, filename)
        cLogger.info(self, "StID  >: %s" %(getHostName()))
        cLogger.info(self, "Lgfl  >: %s" %(os.path.join(logdir,filename)))
        testdate = time.strftime("%a, %d %b %Y %H:%M:%S", filetime)
        cLogger.info(self, "Time  >: %s" %(testdate))
    
    def addLine(self, info):
        cLogger.info(self, info) 


class cPVSSLogger(cLogger):
    def __init__(self, logdir):
        filename = getHostName() + "_StationTest_PVSS" + '.log'
        cLogger.__init__(self, logdir, filename)
        #cLogger.info(self, "# PVSS input file")
             
    def addLine(self, info):
        cLogger.info(self, info)
