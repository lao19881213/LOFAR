#                                                        LOFAR IMAGING PIPELINE
#
#                                                       test_recipe node recipe
#                                                      Wouter Klijn 2015
#                                                                klijn@astron.nl
# ------------------------------------------------------------------------------

import os
import shutil
import sys
import time
import copy
import socket

from lofarpipe.support.lofarnode import LOFARnodeTCP

class test_recipe(LOFARnodeTCP):
    """
    """
    def run(self, argument1, argument2):
        """
        """
        self.logger.critical("#####We are in the test recipe and we are going good#####")

        f = open('/home/klijn/build/7629/gnu_debug/installed/raw_data_800.dat', 'r')

        data = f.read()
        data2 = copy.deepcopy(data)


        self.outputs["output"] = "OUPUT FROM TEST RECIPE"
        self.outputs["status"] = True
        self.outputs['data'] = data
        self.outputs['data2'] = data2

        print "Print to stdout"

        time.sleep(1)
        ## Time execution of this job
        return 0


if __name__ == "__main__":
    #   If invoked directly, parse command line arguments for logger information
    #                        and pass the rest to the run() method defined above
    # --------------------------------------------------------------------------
    jobid, jobhost, jobport = sys.argv[1:4]
    sys.exit(test_recipe(jobid, jobhost, jobport).run_with_stored_arguments())


    


