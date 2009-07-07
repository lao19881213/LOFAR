""" script for testing the HBA elements
Andre 21 April 2009
The reading and plotting part of SST data was done by
Gijs, 16 april 2009
Usage:
first argument: subband number within sst data
second argument: number of RCUs to test
e.g.

python hbaelementtest.py 155 96

"""
# INIT

import array
import operator
import os
import time
import sys
import time

# Read directory with the files to processs	
def open_dir(dirname) :
	files = filter(os.path.isfile, os.listdir('.'))
	#files.sort(key=lambda x: os.path.getmtime(x))
 	return files

def rm_files(dir_name,file) :
        cmdstr = 'rm ' + file
	os.popen(cmdstr)
	return

def rec_stat(dirname,num_rcu) :
	os.popen("rspctl --statistics --duration=10 --integration=10 --select=0:" + str(num_rcu-1) + " 2>/dev/null")
        return

# Open file for processsing
def open_file(files, file_nr) :
        # check if file is data file, no junk
	if files[file_nr][-3:] == 'dat':
		file_name = files[file_nr]
		fileinfo = os.stat(file_name)
		size = int(fileinfo.st_size)
		f=open(file_name,'rb')
		max_frames = size/(512*8)
		frames_to_process=max_frames
    		rcu_nr = int(files[file_nr][-6:-4])
		#print 'File nr ' + str(file_nr) + ' RCU nr ' + str(rcu_nr) + '  ' + files[file_nr][-6:-4]
	else :
		frames_to_process=0
		f=open(files[file_nr],'rb')
		rcu_nr = 0
	return f, frames_to_process, rcu_nr	

# Read single frame from file	
def read_frame(f):
	sst_data = array.array('d')	  	
	sst_data.fromfile(f,512)
	sst_data = sst_data.tolist()
	return sst_data

def capture_data(dir_name,num_rcu,hba_elements,ctrl_word,sleeptime,subband_nr,element):
	meet_data=range(0, num_rcu)
        rm_files(dir_name,'*')
        ctrl_string='='
        for ind in range(hba_elements) :
               if ind == element:
			ctrl_string=ctrl_string + '128,'
	       else:	
			ctrl_string=ctrl_string + '2,'
	strlength=len(ctrl_string)
        ctrl_string=ctrl_string[0:strlength-1]
	cmd_str='rspctl --hbadelay' + ctrl_string + ' 2>/dev/null'
        os.popen(cmd_str)
	time.sleep(sleeptime)
        print 'Capture HBA element ' + str(element+1) + ' data'
        rec_stat(dir_name,num_rcu)
        # get list of all files in dir_name
 	files = open_dir(dir_name)
        
        # start processing the element measurements
	for file_cnt in range(len(files)) :
		f, frames_to_process, rcu_nr  = open_file(files, file_cnt)
                if frames_to_process > 0 : 
		   	sst_data = read_frame(f)
                   	sst_subband = sst_data[subband_nr]
		   	meet_data[rcu_nr] = sst_subband
		f.close
	return meet_data

# Main loop
def main() :
	sub_time=[]
	sub_file=[]
	dir_name = './hbadatatest/' #Work directory will be cleaned
        if not(os.path.exists(dir_name)):
	    os.mkdir(dir_name)
  	rmfile = '*.log'
	hba_elements=2
        factor=1000
        sleeptime=1
 	ctrl_string='='
	# read in arguments
        if len(sys.argv) < 2 :
	        subband_nr=155
        else :
		subband_nr = int(sys.argv[1])
        print ' Dir name is ' + dir_name
        if len(sys.argv) < 3 :
	        num_rcu=96
        else :
		num_rcu = int(sys.argv[2])
        print ' Number of RCUs is ' + str(num_rcu)
        # init log file
        f_log1 = file('HBA_elements1.log', 'w')
        f_log1.write(' ************ \n \n LOG File for HBA element test (ctrl word 128) \n \n *************** \n \n')
        f_log1fac = file('HBA_factors1.log', 'w')
        f_log2 = file('HBA_elements2.log', 'w')
        f_log2.write(' ************ \n \n LOG File for HBA element test (ctrl word 253) \n \n *************** \n \n')
        f_log2fac = file('HBA_factors2.log', 'w')
	# initialize data arrays
	ref_data=range(0, num_rcu)
	os.chdir(dir_name)
	#os.popen("rspctl --clock=200")
	#print 'Clock is set to 200 MHz'
        #time.sleep(10)
        #---------------------------------------------
	# capture reference data (all HBA elements off)
        rm_files(dir_name,'*')
        os.popen("rspctl --rcumode=5 2>/dev/null")
        os.popen("rspctl --enable 2>/dev/null")
        for ind in range(hba_elements) :
		ctrl_string=ctrl_string + '2,'
	strlength=len(ctrl_string)
        ctrl_string=ctrl_string[0:strlength-1]
	cmd_str='rspctl --hbadelay' + ctrl_string + ' 2>/dev/null'
        os.popen(cmd_str)
        time.sleep(sleeptime)
        print 'Capture reference data'
        rec_stat(dir_name,num_rcu)
	#rm_files(dir_name,rmfile)
        # get list of all files in dir_name
 	files = open_dir(dir_name)
        # start processing the reference measurement
	for file_cnt in range(len(files)) :
		f, frames_to_process, rcu_nr  = open_file(files, file_cnt)
                if frames_to_process > 0 : 
		   sst_data = read_frame(f)
                   sst_subband = sst_data[subband_nr]
		   ref_data[rcu_nr] = sst_subband
		   #if rcu_nr==0:
		   #	print ' waarde is ' + str(sst_subband)
		f.close
        #---------------------------------------------
        # capture hba element data for all elements

        for element in range(hba_elements) :
                #First capture when the control word is 128 of one of the elements
                ctrl_word=128
                meet_data1=capture_data(dir_name,num_rcu,hba_elements,ctrl_word,sleeptime,subband_nr,element)
                ctrl_word=253
                meet_data2=capture_data(dir_name,num_rcu,hba_elements,ctrl_word,sleeptime,subband_nr,element)

                #Second capture when the control word is 253 of one of the elements


		for rcuind in range(num_rcu) :
                        f_log1fac.write(str(element+1) + ' ' + str(rcuind) + ' ' + str(round(meet_data1[rcuind]/ref_data[rcuind])) + '\n')  
			if meet_data1[rcuind] < factor*ref_data[rcuind] :        
				if rcuind == 0 :
					tilenumb=0
				else:
					tilenumb=int(rcuind/2)
                                f_log1.write('Element ' + str(element+1) + ', Tile ' + str(tilenumb) + ' in RCU: ' + str(rcuind)+ ' factor: ' + str(round(meet_data1[rcuind]/ref_data[rcuind])) + '\n')  
                        f_log2fac.write(str(element+1) + ' ' + str(rcuind) + ' ' + str(round(meet_data2[rcuind]/ref_data[rcuind])) + '\n')  
			if meet_data2[rcuind] < factor*ref_data[rcuind] :        
				if rcuind == 0 :
					tilenumb=0
				else:
					tilenumb=int(rcuind/2)
                                f_log2.write('Element ' + str(element+1) + ', Tile ' + str(tilenumb) + ' in RCU: ' + str(rcuind)+ ' factor: ' + str(round(meet_data2[rcuind]/ref_data[rcuind])) + '\n')  

        f_log1.close
	f_log1fac.close
        f_log2.close
	f_log2fac.close

main()
