#!/usr/bin/python
# Copyright (C) 2012-2013  ASTRON (Netherlands Institute for Radio Astronomy)
# P.O. Box 2, 7990 AA Dwingeloo, The Netherlands
#
# This file is part of the LOFAR software suite.
# The LOFAR software suite is free software: you can redistribute it and/or
# modify it under the terms of the GNU General Public License as published
# by the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# The LOFAR software suite is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
#
# $Id$
import sys
import os
import socket
import ConfigParser

import lofarpipe.daemons.pipelineSCQDaemonImp as PipelineSCQDaemonImp

if __name__ == "__main__":
    # Read the config file, fail of not supplied
    if len(sys.argv) != 2:
        print "Usage: pipelineSCQDaemon config.cfg"
        exit(-1)

    config_path = sys.argv[1]
    if not os.path.isfile(config_path):
        print "supplied config file {0} does not exist".format(config_path)
        sys.exit(-1)
    config = ConfigParser.ConfigParser()
    config.read(config_path)

    # create or get parameters 
    
   
    busname = config.get(               "DEFAULT", "busname")
    hostname = socket.gethostname()
    broker=  "{0}:{1}".format(hostname, config.get("DEFAULT", "broker_port"))

    slaveCommandQueueNameTemplate = config.get( "slave_cqdaemon",
                             "command_queue_template").format(hostname)
    deadLetterQueueName = config.get(   "DEFAULT", "deadletter")
    deadletterfile = config.get(        "slave_cqdaemon", "deadletter_log_path")
    logfile = config.get(               "slave_cqdaemon", "logfile")
    loop_interval = config.getfloat(    "slave_cqdaemon", "loop_interval")
    max_repost =  config.getfloat(      "slave_cqdaemon", "max_repost")

    daemon = PipelineSCQDaemonImp.PipelineSCQDaemonImp(
                broker, 
                busname, 
                slaveCommandQueueNameTemplate, 
                deadLetterQueueName, 
                deadletterfile,
                logfile,
                loop_interval=loop_interval,
                max_repost=max_repost)

    daemon.run()

    
