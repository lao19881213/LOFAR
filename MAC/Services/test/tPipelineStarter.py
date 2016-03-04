#!/usr/bin/env python

# Be able to find service python file
import sys, os
sys.path.insert(0, "{srcdir}/../src".format(**os.environ))

import lofar.mac.PipelineStarter as module
import subprocess

import unittest

import logging
logging.basicConfig(stream=sys.stdout, level=logging.DEBUG)

def setUpModule():
  pass

def tearDownModule():
  pass

class TestRunCommand(unittest.TestCase):
  def test_basic(self):
    """ Test whether we can run a trivial command. """
    module.runCommand("true")

  def test_invalid_command(self):
    """ Test whether an invalid command produces an error. """
    with self.assertRaises(subprocess.CalledProcessError):
      output = module.runCommand(".")

  def test_shell(self):
    """ Test whether the command is parsed by a shell. """
    module.runCommand("true --version")

  def test_output(self):
    """ Test whether we catch the command output correctly. """
    output = module.runCommand("echo yes")
    self.assertEqual(output, "yes")

  def test_input(self):
    """ Test whether we can provide input. """
    output = module.runCommand("cat -", "yes")
    self.assertEqual(output, "yes")

class TestSlurmJobInfo(unittest.TestCase):
  def test_no_jobs(self):
    """ Test 'scontrol show job' output if there are no jobs. """
    module.runSlurmCommand = lambda _: """No jobs in the system"""
    self.assertEqual(module.getSlurmJobInfo(), {})

  def test_one_job(self):
    """ Test 'scontrol show job' output for a single job. """
    module.runSlurmCommand = lambda _: """JobId=119 JobName=foo UserId=mol(7261) GroupId=mol(7261) Priority=4294901736 Nice=0 Account=(null) QOS=(null) JobState=RUNNING Reason=None Dependency=(null) Requeue=1 Restarts=0 BatchFlag=0 Reboot=0 ExitCode=0:0 RunTime=00:00:07 TimeLimit=UNLIMITED TimeMin=N/A SubmitTime=2016-03-04T12:05:52 EligibleTime=2016-03-04T12:05:52 StartTime=2016-03-04T12:05:52 EndTime=Unknown PreemptTime=None SuspendTime=None SecsPreSuspend=0 Partition=cpu AllocNode:Sid=thead01:7040 ReqNodeList=(null) ExcNodeList=(null) NodeList=tcpu[01-02] BatchHost=tcpu01 NumNodes=2 NumCPUs=2 CPUs/Task=1 ReqB:S:C:T=0:0:*:* TRES=cpu=2,mem=6000,node=2 Socks/Node=* NtasksPerN:B:S:C=0:0:*:* CoreSpec=* MinCPUsNode=1 MinMemoryNode=3000M MinTmpDiskNode=0 Features=(null) Gres=(null) Reservation=(null) Shared=OK Contiguous=0 Licenses=(null) Network=(null) Command=(null) WorkDir=/home/mol Power= SICP=0"""

    jobs = module.getSlurmJobInfo()
    self.assertEqual(jobs["foo"]["JobName"], "foo")
    self.assertEqual(jobs["foo"]["JobId"], "119")

  def test_two_jobs(self):
    """ Test 'scontrol show job' output for multiple jobs. """
    module.runSlurmCommand = lambda _: """JobId=120 JobName=foo UserId=mol(7261) GroupId=mol(7261) Priority=4294901735 Nice=0 Account=(null) QOS=(null) JobState=RUNNING Reason=None Dependency=(null) Requeue=1 Restarts=0 BatchFlag=0 Reboot=0 ExitCode=0:0 RunTime=00:00:17 TimeLimit=UNLIMITED TimeMin=N/A SubmitTime=2016-03-04T12:09:53 EligibleTime=2016-03-04T12:09:53 StartTime=2016-03-04T12:09:53 EndTime=Unknown PreemptTime=None SuspendTime=None SecsPreSuspend=0 Partition=cpu AllocNode:Sid=thead01:7250 ReqNodeList=(null) ExcNodeList=(null) NodeList=tcpu[01-02] BatchHost=tcpu01 NumNodes=2 NumCPUs=2 CPUs/Task=1 ReqB:S:C:T=0:0:*:* TRES=cpu=2,mem=6000,node=2 Socks/Node=* NtasksPerN:B:S:C=0:0:*:* CoreSpec=* MinCPUsNode=1 MinMemoryNode=3000M MinTmpDiskNode=0 Features=(null) Gres=(null) Reservation=(null) Shared=OK Contiguous=0 Licenses=(null) Network=(null) Command=(null) WorkDir=/home/mol Power= SICP=0
    JobId=121 JobName=bar UserId=mol(7261) GroupId=mol(7261) Priority=4294901734 Nice=0 Account=(null) QOS=(null) JobState=PENDING Reason=Resources Dependency=(null) Requeue=1 Restarts=0 BatchFlag=0 Reboot=0 ExitCode=0:0 RunTime=00:00:00 TimeLimit=UNLIMITED TimeMin=N/A SubmitTime=2016-03-04T12:09:59 EligibleTime=2016-03-04T12:09:59 StartTime=2017-03-04T12:09:53 EndTime=Unknown PreemptTime=None SuspendTime=None SecsPreSuspend=0 Partition=cpu AllocNode:Sid=thead01:7250 ReqNodeList=(null) ExcNodeList=(null) NodeList=(null) NumNodes=2-2 NumCPUs=2 CPUs/Task=1 ReqB:S:C:T=0:0:*:* TRES=cpu=2,node=2 Socks/Node=* NtasksPerN:B:S:C=0:0:*:* CoreSpec=* MinCPUsNode=1 MinMemoryNode=0 MinTmpDiskNode=0 Features=(null) Gres=(null) Reservation=(null) Shared=OK Contiguous=0 Licenses=(null) Network=(null) Command=(null) WorkDir=/home/mol Power= SICP=0"""

    jobs = module.getSlurmJobInfo()
    self.assertEqual(jobs["foo"]["JobName"], "foo")
    self.assertEqual(jobs["foo"]["JobId"], "120")

    self.assertEqual(jobs["bar"]["JobName"], "bar")
    self.assertEqual(jobs["bar"]["JobId"], "121")

class TestPipelineStarter(unittest.TestCase):
  def test_shouldHandle(self):
    """ Test whether we filter the right OTDB trees. """

    trials = [ { "type": "Observation", "cluster": "CEP2", "shouldHandle": False },
               { "type": "Observation", "cluster": "CEP4", "shouldHandle": False },
               { "type": "Observation", "cluster": "foo",  "shouldHandle": False },
               { "type": "Observation", "cluster": "",     "shouldHandle": False },
               { "type": "Pipeline",    "cluster": "CEP2", "shouldHandle": False },
               { "type": "Pipeline",    "cluster": "CEP4", "shouldHandle": True },
               { "type": "Pipeline",    "cluster": "foo",  "shouldHandle": True },
               { "type": "Pipeline",    "cluster": "",     "shouldHandle": False },
             ]

    for t in trials:
      parset = { "ObsSW.Observation.processType": t["type"],
                 "ObsSW.Observation.Cluster.ProcessingCluster.clusterName": t["cluster"] }
      self.assertEqual(module.PipelineStarter._shouldHandle(parset), t["shouldHandle"])

def main(argv):
  unittest.main(verbosity=2)

if __name__ == "__main__":
  # run all tests
  import sys
  main(sys.argv[1:])

