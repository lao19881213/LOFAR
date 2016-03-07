#!/usr/bin/env python
#coding: iso-8859-15
#
# Copyright (C) 2015
# ASTRON (Netherlands Institute for Radio Astronomy)
# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
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
"""
Daemon that listens to OTDB status changes to SCHEDULED, requests
the parset of such jobs, and starts them using SLURM and Docker.

The execution chain is as follows:

[SCHEDULED]          -> PipelineStarter schedules

                           runPipeline.sh <obsid> || pipelineAborted.sh <obsid>

                        using two SLURM jobs, guaranteeing that pipelineAborted.sh is
                        called in the following circumstances:

                          - runPipeline.sh exits with failure
                          - runPipeline.sh is killed by SLURM
                          - runPipeline.sh job is cancelled in the queue

                        State is set to [QUEUED].

(runPipeline.sh)     -> Calls
                          - state <- [ACTIVE]
                          - getParset
                          - (run pipeline)
                          - state <- [COMPLETING]
                          - (wrap up)
                          - state <- [FINISHED]

(pipelineAborted.sh) -> Calls
                          - state <- [ABORTED]
"""

from lofar.messaging import FromBus, ToBus, RPC, EventMessage
from lofar.parameterset import PyParameterValue
from lofar.sas.otdb.OTDBBusListener import OTDBBusListener
from lofar.common.util import waitForInterrupt
from lofar.messaging.RPC import RPC, RPCTimeoutException

import subprocess
import datetime
import os

import logging
logger = logging.getLogger(__name__)

def runCommand(cmdline, input=None):
  logger.info("Running '%s'", cmdline)

  # Start command
  proc = subprocess.Popen(
    cmdline,
    stdin=subprocess.PIPE if input else file("/dev/null"),
    stdout=subprocess.PIPE,
    shell=True,
    universal_newlines=True
    )

  # Feed input and wait for termination
  stdout, _ = proc.communicate(input)
  logger.debug(stdout)

  # Check exit status, bail on error
  if proc.returncode != 0:
    raise subprocess.CalledProcessError(proc.returncode, cmdline)

  # Return output
  return stdout.strip()

def runSlurmCommand(cmdline, headnode="head01.cep4"):
  cmdline = "ssh %s %s" % (headnode, cmdline)
  runCommand(cmdline)

""" Prefix that is common to all parset keys, depending on the exact source. """
PARSET_PREFIX="ObsSW."

class Parset(dict):
  def predecessors(self):
    """ Extract the list of predecessor obs IDs from the given parset. """

    key = PARSET_PREFIX + "Observation.Scheduler.predecessors"
    strlist = PyParameterValue(str(self[key]), True).getStringVector()

    # Key contains "Lxxxxx" values, we want to have "xxxxx" only
    result = [int(filter(str.isdigit,x)) for x in strlist]

    return result

  def dockerTag(self):
    # For now, return OUR tag
    return runCommand("docker-template", "${LOFAR_TAG}")

  def slurmJobName(self):
    return self[PARSET_PREFIX + "Observation.ObsID"]

def getSlurmJobInfo():
  stdout = runSlurmCommand("scontrol show job --oneliner")

  if stdout == "No jobs in the system":
    return {}

  jobs = {}
  for l in stdout.split("\n"):
    # One line is one job
    job_properties = {}

    # Information is in k=v pairs
    for i in l.split():
      k,v = i.split("=", 1)
      job_properties[k] = v

    if "JobName" not in job_properties:
      logger.warning("Could not find job name in line: %s" % (l,))
      continue

    name = job_properties["JobName"]
    if name in jobs:
      logger.warning("Duplicate job name: %s" % (name,))
    jobs[name] = job_properties

  return jobs

class PipelineStarter(OTDBBusListener):
  def __init__(self, otdb_busname=None, setStatus_busname=None, **kwargs):
    super(PipelineStarter, self).__init__(busname=otdb_busname, **kwargs)

    self.parset_rpc = RPC(service="TaskSpecification", busname=otdb_busname, ForwardExceptions=True)
    self.setStatus_busname = setStatus_busname if setStatus_busname else otdb_busname

  def _setStatus(self, obsid, status):
    try:
        with RPC("StatusUpdateCmd", busname=self.setStatus_busname, timeout=10, ForwardExceptions=True) as status_rpc:
            result, _ = status_rpc(OtdbID=obsid, NewStatus=status)
    except RPCTimeoutException, e:
        # We use a queue, so delivery is guaranteed. We don't care about the answer.
        pass

  def start_listening(self, **kwargs):
    self.parset_rpc.open()

    super(PipelineStarter, self).start_listening(**kwargs)

  def stop_listening(self, **kwargs):
    super(PipelineStarter, self).stop_listening(**kwargs)

    self.parset_rpc.close()

  @classmethod
  def _shouldHandle(self, parset):
    if parset[PARSET_PREFIX + "Observation.processType"] != "Pipeline":
      logger.info("Not processing tree: is not a pipeline")
      return False

    if parset[PARSET_PREFIX + "Observation.Cluster.ProcessingCluster.clusterName"] in ["", "CEP2"]:
      logger.info("Not processing tree: is a CEP2 pipeline")
      return False

    return True

  @classmethod
  def _slurmJobNames(self, parsets, allowed):
    names = []

    for obsid, p in parsets.iteritems():
      processType = p[PARSET_PREFIX + "Observation.processType"]
      jobName = p.slurmJobName()

      if jobName not in allowed:
        raise KeyError("No SLURM job for predecessor %d (JobName '%s')." % (obsid, jobName))

      names.append(p.slurmJobName())

    return names

  def _getPredecessorParsets(self, parset):
    obsIDs = parset.predecessors()

    logger.info("Obtaining predecessor parsets %s", obsIDs)

    preparsets = {}
    for obsid in obsIDs:
      preparsets[obsid] = Parset(self.parset_rpc( OtdbID=obsid, timeout=10 )[0])

    return preparsets

  def onObservationAborted(self, treeId, modificationTime):
    logger.info("***** STOP Tree ID %s *****", treeId)

    # Request the parset
    parset = Parset(self.parset_rpc( OtdbID=treeId, timeout=10 )[0])

    if not self._shouldHandle(parset):
      return

    # Cancel corresponding SLURM job, causing any successors
    # to be cancelled as well.
    stdout = self.runSlurmCommand("scancel --jobname %s" % (self._slurmJobName(parset),))

  """
    More statusses we want to abort on.
  """
  onObservationConflict = onObservationAborted
  onObservationHold     = onObservationAborted

  @classmethod
  def _minStartTime(self, preparsets):
    result = None

    for preparset in preparsets.values():
      processType = preparset[PARSET_PREFIX + "Observation.processType"]

      if processType == "Observation":
        # If we depend on an observation, start 1 minute after it
        obs_endtime = datetime.datetime.strptime(preparset[PARSET_PREFIX + "Observation.stopTime"], "%Y-%m-%d %H:%M:%S")
        min_starttime = obs_endtime + datetime.timedelta(0, 60, 0)

        result = max(result, min_starttime) if result else min_starttime

    return result

  def onObservationScheduled(self, treeId, modificationTime):
    logger.info("***** QUEUE Tree ID %s *****", treeId)

    # Request the parset
    parset = Parset(self.parset_rpc( OtdbID=treeId, timeout=10 )[0])

    if not self._shouldHandle(parset):
      return

    """
      Collect predecessor information.
    """

    # Collect the parsets of predecessors
    preparsets = self._getPredecessorParsets(parset)

    # Collect SLURM job information
    logger.info("Obtaining SLURM job list")
    slurm_jobs = getSlurmJobInfo()

    """
      Schedule "docker-runPipeline.sh", which will fetch the parset and run the pipeline within
      a SLURM job.
    """

    # Determine SLURM parameters
    sbatch_params = ["--job-name=%s" % (parset.slurmJobName(),),

                     # Only run job if all nodes are ready
                     "--wait-all-nodes=1",

                     # Enforce the dependencies, instead of creating lingering jobs
                     "--kill-on-invalid-dep=yes",

                     # Restart job if a node fails
                     "--requeue",

                     # Maximum run time for job (31 days)
                     "--time=31-0",
                    
                     # TODO: Compute nr nodes
                     "--nodes=50",
                    
                     # Define better places to write the output
                     os.path.expandvars("--error=$LOFARROOT/var/log/docker-startPython-%s.stderr" % (treeId,)),
                     os.path.expandvars("--output=$LOFARROOT/var/log/docker-startPython-%s.log" % (treeId,)),

                     ]

    min_starttime = self._minStartTime(preparsets)
    if min_starttime:
      sbatch_params.append("--begin=%s" % (min_starttime.strftime("%FT%T"),))

    predecessor_jobs = self._slurmJobNames(preparsets, slurm_jobs.keys())
    if predecessor_jobs:
      sbatch_params.append("--dependency=%s" % (",".join(("afterok:%s" % x for x in predecessor_jobs)),))

    # Schedule runPipeline.sh
    logger.info("Scheduling SLURM job for runPipeline.sh")
    slurm_job_id = runSlurmCommand(
      "sbatch %s bash -c '%s'" % (" ".join(sbatch_params), 

      # Supply script to run on-the-fly to reduce dependencies on
      # compute nodes.
      "docker run --rm lofar-pipeline:{tag}"
      " --net=host"
      " -v /data:/data"
      " -e LUSER=$UID"
      " -v $HOME/.ssh:/home/lofar/.ssh:ro"
      " -e SLURM_JOB_ID=$SLURM_JOB_ID"
      " runPipeline.sh {obsid}".format(
        obsid = treeId,
        tag = parset.dockerTag(),
      )
    ))
    logger.info("Scheduled SLURM job %s" % (slurm_job_id,))

    # Schedule pipelineAborted.sh
    logger.info("Scheduling SLURM job for pipelineAborted.sh")
    slurm_cancel_job_id = runSlurmCommand(
      "sbatch"
      " --job-name=%s-aborted"
      " --cpus-per=task=1 --ntasks=1"
      " --dependency=afternotok:%s"
      " --kill-on-invalid-dep=yes"
      " --requeue"
      " bash -c '%s'" % (parset.slurmJobName(), slurm_job_id,

      "docker run --rm lofar-pipeline:{tag}"
      " --net=host"
      " -e LUSER=$UID"
      " pipelineAborted.sh {obsid}".format(
        obsid = treeId,
        tag = parset.dockerTag(),
      )
    ))
    logger.info("Scheduled SLURM job %s" % (slurm_cancel_job_id,))

    # Set OTDB status to QUEUED
    logger.info("Setting status to QUEUED")
    self._setStatus(treeId, "queued")

    logger.info("Pipeline processed.")

