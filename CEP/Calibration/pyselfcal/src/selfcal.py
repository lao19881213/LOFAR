#!/usr/bin/env python


# IMPORT general modules

import sys,os,commands,glob,time
import getopt

import pyrap.tables as pt
import numpy as np

# Import local modules (classes)
from lofar.selfcal import class_obsparMergedData
from lofar.selfcal import class_obsPreprocessing
from lofar.selfcal import class_selfcalparam
from lofar.selfcal import class_selfcalrun  



#############################
# check the Launch
#############################


def main(initparameterlist):

	try:
	      opts, args = getopt.getopt(sys.argv[1:], "hoonoV:", ["help", "obsDir=", "outputDir=", "nbCycle=", "outerFOVclean=", "VLSSSuse="])
	      
      
	except getopt.GetoptError as err:
	      print "Usage: selfcal.py --obsDir= --outputDir= --nbCycle= --outerFOVclean= --VLSSSuse="
	      print ""
	      sys.exit(2)
	      
	         	
	for par1, par2 in opts:
		
		if par1 in ("--help"):
			print ""
        		print "Usage: selfcal.py --obsDir= --outputDir= --nbCycle=  --outerFOVclean= --VLSSSuse=\n"
        		
        		print """
Selfcal parameter:
--obsDir: This is the observation directory which contains datas (or frequency merged data generated by mergeSB.py)
Datas must contains the same number of subbands, have the same frequency range and pointed the same target. Checks are done internally. Other point, obsDir must contains ONLY datas, so other files. In addition if the outputDir is in the obsDir tree, the code must crash, because obsDir will not contain ONLY DATA.
Full path must be provided.
        		
--outputDir:This is the Output directory where the selfcal process will be executed. If not exists, it will be created automatically. 
Full path must be provided.
        		
--nbCycle: This is the number of cycle that you want for the selfcal process. It must be between 5 and 20. The selfcal process will start at 15 times the best possible resolution. The gap (in resolution) between 2 cycle is linear.
        		
--outerFOVclean: yes or no => if this option is activated, a prepocessing cleaning will be apply before the selfcal process.
A first low resolution image will be generated (using core stations only and short baselines) to obtain a sky model. This preliminary image at 30'' pixel size, has a bigger fov than selfcal images (new fov=5/sin^2(average elevation) degree instead 5 degree).
An annulus (contains sources with angular distance >5deg from the target) and a center (contains sources with angular distance <5deg from the target)Skymodel will be created from the extracted Skymodel (from the preliminary image).
The annulus skymodel will be substracted from visibilities. This cleaning is an iterative process, until there are less than 10% of sources from the original annulus skymodel (typically 2-3 iterations are needed)
        		      		
--VLSSSuse: yes or no => you have the possibility to use for the first Skymodel for the selfcal process the VLSSS Skymodel (generated by gsm.py).
If the user do not want to use VLSSS Skymodel, the code will use the last generated center Skymodel if outerFOVclean parameter was yes.
If outerFOVclean was no, the code will create a preliminary image (see above the outerFOVclean parameter), and use the center Skymodel as an input instead the VLSSS Skymodel.  
        		
"""
					
        		
			print ""
        		sys.exit(2)
        
        	
		elif par1 in ("--obsDir"):
			initparameterlist[0]=par2
		elif par1 in ("--outputDir"):
			initparameterlist[1]=par2		
		elif par1 in ("--nbCycle"):
			initparameterlist[2]=par2
		elif par1 in ("--outerFOVclean"):
			initparameterlist[3]=par2	
		elif par1 in ("--VLSSSuse"):
			initparameterlist[4]=par2															
		else:
        		print("Option {} Unknown".format(par1))
        		sys.exit(2)


        		
        # Check parameters		
 	if initparameterlist[0] == "none" or initparameterlist[1] == "none" or initparameterlist[2] == "none" or initparameterlist[3] == "none" or initparameterlist[4] == "none":
		print ""
		print "MISSING Parameters"	
		print "Usage: selfcal.py --obsDir= --outputDir= --nbCycle=  --outerFOVclean= --VLSSSuse="
		print ""
		sys.exit(2)       	
 
 
 	if initparameterlist[3] != 'yes' and initparameterlist[3] != 'no': 
		print ''
		print 'outerFOVclean parameter must be equal to yes or no' 
 		print ""
		sys.exit(2) 
 

 	if initparameterlist[4] != 'yes' and initparameterlist[4] != 'no': 
		print ''
		print 'VLSSSuse parameter must be equal to yes or no' 
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
    initparameterlist=range(5)
    
    initparameterlist[0]	= "none"	# Observation Directory
    initparameterlist[1]	= "none"	# Output Directory
    initparameterlist[2]	= "none"	# Number Of cycle for the selfcal loop
    initparameterlist[3]	= "none"	# outerFOVclean parameter => yes or no
    initparameterlist[4]	= "none"	# VLSSuse parameter => yes or no    


    # Read and check parameters	
    initparameterlist = main(initparameterlist);

    obsDir			= initparameterlist[0]
    outputDir		= initparameterlist[1]
    nbCycle			= int(initparameterlist[2])
    nIteration		= 1000000    
    outerFOVclean	= initparameterlist[3]
    VLSSuse			= initparameterlist[4]
	

	#check the number of cycle value 

    if nbCycle <5:
		nbCycle = 5
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
		


    ### WARNINGS on the outputDir 
    if outerFOVclean == 'yes':
		preprocessDir	= '%sPreprocessDir/'%(outputDir)
		cmd="""mkdir %s"""%(preprocessDir)
		os.system(cmd)
		print ''
		print 'The PreProcess directory: %s\n has been created'%(preprocessDir)
		print ''

		preprocessImageDir	= '%sImage/'%(preprocessDir)
		cmd="""mkdir %s"""%(preprocessImageDir)
		os.system(cmd)
		print ''
		print 'The PreProcess Image directory: %s\n has been created'%(preprocessImageDir)
		print ''
	
		preprocessSkymodelDir	= '%sSkymodel/'%(preprocessDir)
		cmd="""mkdir %s"""%(preprocessSkymodelDir)
		os.system(cmd)
		print ''
		print 'The PreProcess Skymodel directory: %s\n has been created'%(preprocessSkymodelDir)
		print ''	
	
		preprocessBBSDir	= '%sBBS-Dir/'%(preprocessDir)
		cmd="""mkdir %s"""%(preprocessBBSDir)
		os.system(cmd)
		print ''
		print 'The PreProcess BBS directory: %s\n has been created'%(preprocessSkymodelDir)
		print ''
	
		
	
	
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
    obsPar_Obj																												= class_obsparMergedData.observationMergedDataParam(obsDir)
    listFiles,Files,NbFiles,nbChan,frequency,maxBaseline,integTimeOnechunk,observationIntegTime,UVmin,ra_target,dec_target	= obsPar_Obj.obsParamMergedDataFunc()    

    tstop=time.time()
    timeDuration=tstop-tstart	
    
    print ''
    print '#########################################################'
    print 'End Observation directory global parameters determination'
    print 'Time ellapsed: %s seconds'%(timeDuration)
    print '#########################################################\n'
    

 
    #######################################################################################################################
 
    
    initNofAnnulusSources	= 1000
    nb_annulus				= 1000 
    preprocessIndex			= 0
    

    if outerFOVclean =='yes':


			tstop=time.time()
			timeDuration=tstop-tstart	

    
			print ''
			print '###########################################################'
			print 'Start Observation Preprocessing'
			print 'Time ellapsed: %s seconds'%(timeDuration)	
			print '###########################################################\n'
			 
			# index for iteration cleaning
			i = 0
		  
			while (nb_annulus > 0.1*initNofAnnulusSources):
				
					# Observation Directory Parameter determination
					obsPreprocess_Obj									= class_obsPreprocessing.obsPreprocessing(obsDir,preprocessDir,preprocessImageDir,preprocessSkymodelDir,preprocessBBSDir,i,listFiles,Files,NbFiles,frequency,UVmin,nIteration,ra_target,dec_target,initNofAnnulusSources)

					print '###########################################################'
					print 'Start Imaging Preprocessing at step %s'%(i)
					print 'Time ellapsed: %s seconds'%(timeDuration)	
					print '###########################################################\n'


					obsPreprocess_Obj.obsPreprocessImagingFunc()    
					
					print '###########################################################'
					print 'End Imaging Preprocessing at step %s, and Start Source Extraction at step %s'%(i,i)
					print 'Time ellapsed: %s seconds'%(timeDuration)	
					print '###########################################################\n'					
					
					
					obsPreprocess_Obj.obsPreprocessSrcExtractionFunc()				
					
					initNofAnnulusSources,nb_annulus					= obsPreprocess_Obj.obsPreprocessAnnulusExtractionFunc()
					
					if nb_annulus > 0:
					
							print '###########################################################'
							print ''
							print 'At step %s: Init source annulus:%s and now we have in the annulus %s'%(i,initNofAnnulusSources,nb_annulus)
							print ''
							print '###########################################################'
							print ''
							print '###########################################################'
							print 'End Source Extraction at step %s and Start BBS annulus substraction at step %s and NDPPP'%(i,i)
							print 'Time ellapsed: %s seconds'%(timeDuration)	
							print '###########################################################\n'					
							
							obsPreprocess_Obj.obsPreprocessCreateBBSFunc()
							
							preprocessIndex	= i
							i = i+1

							print '###########################################################'
							print 'End Start BBS annulus substraction at step %s and NDPPP'%(i)
							print 'END OF STEP %s'%(i)
							print 'Time ellapsed: %s seconds'%(timeDuration)	
							print '###########################################################\n'					

					else:
						
							initNofAnnulusSources	= 1000
							nb_annulus				= 1
						


			tstop=time.time()
			timeDuration=tstop-tstart	
			
			print ''
			print '#########################################################'
			print 'End Observation Preprocessing'
			print 'Time ellapsed: %s seconds'%(timeDuration)
			print '#########################################################\n'
			
    else: 
			print ''
			print '#########################################################'
			print 'No preprocessing has been requested by the user, Start selfcal parameter determination'
			print '#########################################################'
			print ''
    
    


    #######################################################################################################################
 
 

 
    
    print ''
    print '##############################################'
    print 'Start Determination of Selfcal parameters'
    print '##############################################\n'  	
	

    # selfcal parameters determination
    selfCalParam_Obj																								= class_selfcalparam.selfCalParam(obsDir,outputDir,listFiles,Files,NbFiles,nbChan,frequency,maxBaseline,integTimeOnechunk,observationIntegTime,nbCycle,ra_target,dec_target,outerFOVclean,VLSSuse,preprocessIndex)
    ImagePathDir,pixsize,nbpixel,robust,UVmax,wmax,SkymodelPath,GSMSkymodel,RMS_BOX,BBSParset,thresh_isl,thresh_pix	= selfCalParam_Obj.selfCalParamFunc()
	
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
		selfCalRun_Obj	= class_selfcalrun.selfCalRun(i,obsDir,outputDir,nbCycle,listFiles,Files,NbFiles,BBSParset,SkymodelPath,GSMSkymodel,ImagePathDir,UVmin,UVmax,wmax,pixsize,nbpixel,robust,nIteration,RMS_BOX,thresh_isl,thresh_pix,outerFOVclean,VLSSuse,preprocessIndex)
		
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
    selfCalRun_Obj	= class_selfcalrun.selfCalRun(i,obsDir,outputDir,nbCycle,listFiles,Files,NbFiles,BBSParset,SkymodelPath,GSMSkymodel,ImagePathDir,UVmin,UVmax,wmax,pixsize,nbpixel,robust,nIteration,RMS_BOX,thresh_isl,thresh_pix,outerFOVclean,VLSSuse,preprocessIndex)
	
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
