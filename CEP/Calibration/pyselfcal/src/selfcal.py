#!/usr/bin/env python


# IMPORT general modules

import sys,os,commands,glob,time
import getopt

import pyrap.tables as pt
import numpy as np

# Import local modules (classes)
from lofar.selfcal import class_obsparMergedData
from lofar.selfcal import class_selfcalparam
from lofar.selfcal import class_selfcalrun  



#############################
# check the Launch
#############################


def main(initparameterlist):

	try:
	      opts, args = getopt.getopt(sys.argv[1:], "hoonnf:", ["help", "obsDir=", "outputDir=", "nbCycle="])
	      
      
	except getopt.GetoptError as err:
	      print "Usage: selfcal.py --obsDir= --outputDir= --nbCycle="
	      print ""
	      sys.exit(2)
	      
	         	
	for par1, par2 in opts:
		
		if par1 in ("--help"):
			print ""
        		print "Usage: selfcal.py --obsDir= --outputDir= --nbCycle="
			print ""
        		sys.exit(2)
        
        	
		elif par1 in ("--obsDir"):
			initparameterlist[0]=par2
		elif par1 in ("--outputDir"):
			initparameterlist[1]=par2		
		elif par1 in ("--nbCycle"):
			initparameterlist[2]=par2								
		else:
        		print("Option {} Unknown".format(par1))
        		sys.exit(2)


        		
        # Check parameters		
 	if initparameterlist[0] == "none" or initparameterlist[1] == "none" or initparameterlist[2] == "none":
		print ""
		print "MISSING Parameters"	
        	print "Usage: selfcal.py --obsDir= --outputDir= --nbCycle="
		print ""
        	sys.exit(2)       	
 
 		
 	return initparameterlist 



    
##############################    
# Main Program
##############################
    


if __name__=='__main__':
  

    tstart=time.time()
  
    ######################    
    #Inputs
    ######################  
    initparameterlist=range(4)
    
    initparameterlist[0]	= "none"	# Observation Directory
    initparameterlist[1]	= "none"	# Output Directory
    initparameterlist[2]	= "none"	# Number Of cycle for the selfcal loop



    # Read and check parameters	
    initparameterlist = main(initparameterlist);

    obsDir	= initparameterlist[0]
    outputDir	= initparameterlist[1]
    nbCycle	= int(initparameterlist[2])
    nIteration	= 1000000


	#check the number of cycle value 

    if nbCycle <2:
		nbCycle = 2
		print ''
		print 'The number of cycle must be greater or equal to 5 and lower or equal to 20\n'
		print 'The value of the number of cycle has been automatiquely changed to %s.\n'%(nbCycle)
		print ''
		
    if nbCycle >20:
		nbCycle = 20		
		print ''
		print 'The number of cycle must be greater or equal to 5 and lower or equal to 20\n'
		print 'The value of the number of cycle has been automatiquely changed to %s.\n'%(nbCycle)
		print ''
    		
    
    ######################    
    #Warnings
    ######################
    
    
    ### WARNINGS on the obsDir 
    if obsDir[-1] != '/':
		obsDir = obsDir+'/'
	
    if os.path.isdir(obsDir) != True:
		print ""
		print "The observation directory do not exists ! Check it Please."
		print ""
		sys.exit(2)  
   
   
    ### WARNINGS on the outputDir 
    if outputDir[-1] != '/':
		outputDir = outputDir+'/'
	
    if os.path.isdir(outputDir) != True:
		cmd="""mkdir %s"""%(outputDir)
		os.system(cmd)
		print ""
		print """The output directory do not exists !\n%s has been created"""%(outputDir)
		print ""
		

	
    ######################   
    ## End of warnings
    ######################
   
   
   
   
    ######################   
    #Main code started Now !!!!
    ######################

    #######################################################################################################################
    
    print ''
    print '###########################################################'
    print 'Start Observation directory global parameters determination'
    print '###########################################################\n'
  
    # Observation Directory Parameter determination
    obsPar_Obj															= class_obsparMergedData.observationMergedDataParam(obsDir)
    listFiles,Files,NbFiles,nbChan,frequency,maxBaseline,integTimeOnechunk,observationIntegTime,UVmin,ra_target,dec_target	= obsPar_Obj.obsParamMergedDataFunc()    

    tstop=time.time()
    timeDuration=tstop-tstart	
    
    print ''
    print '#########################################################'
    print 'End Observation directory global parameters determination'
    print 'Time ellapsed: %s seconds'%(timeDuration)
    print '#########################################################\n'
    

 
    #######################################################################################################################
    
    print ''
    print '##############################################'
    print 'Start Determination of Selfcal parameters'
    print '##############################################\n'  	
	

    # selfcal parameters determination
    selfCalParam_Obj											= class_selfcalparam.selfCalParam(outputDir,listFiles,Files,NbFiles,nbChan,frequency,maxBaseline,integTimeOnechunk,observationIntegTime,nbCycle,ra_target,dec_target)
    ImagePathDir,pixsize,nbpixel,robust,UVmax,wmax,SkymodelPath,GSMSkymodel,RMS_BOX,BBSParset		= selfCalParam_Obj.selfCalParamFunc()
	
    tstop=time.time()
    timeDuration=tstop-tstart	
	
    print ''
    print '############################################'
    print 'End Determination of Selfcal parameters'
    print 'Time ellapsed: %s seconds'%(timeDuration)
    print '############################################\n'    
 

    
    #######################################################################################################################
    
    print ''
    print '##############################################'
    print 'Start Selfcal Computation'
    print '##############################################\n'  
        
    k		= range(nbCycle)
     
    
    for i in k:
		
		tstop=time.time()
		timeDuration=tstop-tstart		
		
		print ''
		print """##############################################"""
		print """Start Selfcal Cycle %s on %s"""%(i,nbCycle-1)
		print 'Time ellapsed: %s seconds'%(timeDuration)		
		print """##############################################\n"""
		 
		#####################
		# Selfcal computation
		selfCalRun_Obj	= class_selfcalrun.selfCalRun(i,obsDir,outputDir,nbCycle,listFiles,Files,NbFiles,BBSParset,SkymodelPath,GSMSkymodel,ImagePathDir,UVmin,UVmax,wmax,pixsize,nbpixel,robust,nIteration,RMS_BOX)
		
		#Run the BBS-cal on each Time chunks
		selfCalRun_Obj.selfCalRunFuncCalibBBSNDPPP()
		
		tstop=time.time()
		timeDuration=tstop-tstart		

		print ''
		print """##############################################"""
		print """BBS and NDPPP at Cycle %s on %s is finished"""%(i,nbCycle-1)
		print 'Time ellapsed: %s seconds'%(timeDuration)		
		print """##############################################\n"""		
		
		
		#Concatenate in time and imaging 
		selfCalRun_Obj.selfCalRunFuncImaging()

		tstop=time.time()
		timeDuration=tstop-tstart		

		print ''
		print """##############################################"""
		print """Image at Cycle %s on %s has been generated"""%(i,nbCycle-1)
		print 'Time ellapsed: %s seconds'%(timeDuration)		
		print """##############################################\n"""		
		
		#Extract the skymodel
		selfCalRun_Obj.selfCalRunFuncSrcExtraction()
		
		tstop=time.time()
		timeDuration=tstop-tstart	
		
		print ''
		print """##############################################"""
		print """End Selfcal Cycle %s on %s, model extraction is done"""%(i,nbCycle-1)
		print 'Time ellapsed: %s seconds'%(timeDuration)		
		print """##############################################\n"""		
			

    tstop=time.time()
    timeDuration=tstop-tstart
    
    print ''
    print '##############################################'
    print 'Start Final Image Computation'
    print 'Time ellapsed: %s seconds'%(timeDuration)
    print '##############################################'   
    print ''    
    
    i=nbCycle	
    selfCalRun_Obj	= class_selfcalrun.selfCalRun(i,obsDir,outputDir,nbCycle,listFiles,Files,NbFiles,BBSParset,SkymodelPath,GSMSkymodel,ImagePathDir,UVmin,UVmax,wmax,pixsize,nbpixel,robust,nIteration,RMS_BOX)
	
    #Run the BBS-cal on each Time chunks
    selfCalRun_Obj.selfCalRunFuncCalibBBSNDPPP()	
    
    tstop=time.time()
    timeDuration=tstop-tstart		
    
    print ''
    print """##############################################"""
    print """BBS and NDPPP at Final Cycle %s finished"""%(nbCycle)
    print 'Time ellapsed: %s seconds'%(timeDuration)		
    print """##############################################\n"""    
    
    #Concatenate in time and imaging 
    selfCalRun_Obj.selfCalRunFuncImaging()

    tstop=time.time()
    timeDuration=tstop-tstart	    
    
    print ''
    print """##############################################"""
    print """Image at Final Cycle %s has been generated"""%(nbCycle)
    print 'Time ellapsed: %s seconds'%(timeDuration)		
    print """##############################################\n"""	    
    
    #Extract the skymodel
    selfCalRun_Obj.selfCalRunFuncSrcExtraction()

    tstop=time.time()
    timeDuration=tstop-tstart	     
    
    print ''
    print """##############################################"""
    print """End Selfcal Final Cycle %s, model extraction is done"""%(nbCycle)
    print 'Time ellapsed: %s seconds'%(timeDuration)		
    print """##############################################\n"""	    
    
    
    print ''
    print '##############################################'
    print 'End of Final Image Computation'
    print '##############################################\n' 		
		

    print ''
    print '##############################################'
    print 'End Selfcal Computation'
    print '##############################################\n' 

    tstop=time.time()
    timeDuration=tstop-tstart
    
    print ''
    print '##############################################'
    print 'Self-Calibration is finished'
    print 'Time ellapsed: %s seconds'%(timeDuration)
    print '##############################################'   
    print ''
