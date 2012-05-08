#!/bin/bash

# Shell script which provides a crude single-subband progressmeter for bbs-reducer
#
# File:         sbprogress.sh
# Author:       Sven Duscha (duscha at astron.nl)
# Date:         2012-05-04
# Last change:  2012-05-07
#
# Parameters:
# -u update time    update interval
# -p process id     watch particular process id
# -k db key         monitor progress of db-based calibrate
# -l logfile        if an alternative log should be watched
# -n port           open network listening for gds file-based central display
# -a                display all user's bbs-reducers progress
# -s                silent mode, only return percentage progress
# -v                turn verbose messages on
# -d                turn debug messages on

# Set default values:
interval=30          # update every 30 seconds
network=0            # don't wait for network connection
key=""               # don't look for session key in db
logfile=""           # default log file to log for
port=32000           # default port if listening for network connection
a=0                  # look for all user's processes
silent=0             # silent mode default off
verbosity=0          # verbose mode default off
debug=0              # debug mode


# Help: display usage info
function usage()
{
echo "usage: sbprogress.sh <options>"
echo "Due to file permission restrictions, you can only watch your on processes through -p <pid>"
echo "Note: The remaining time is only accurate, if the average over a couple of chunks"
echo "      has been calculated.\n"
echo ""
echo "Options:"
echo "-u <update time>   update interval in seconds"
echo "-p <pid>           watch particular process id, can use \`pgrep <procname>\` if its unique"
echo "-k <key>           monitor progress of db-based calibrate [NOT SUPPORTED YET]"
echo "-l logfile         if an alternative log should be watched, instead of -p <pid>"
echo "-n port            open network listening for gds file-based central display [NOT SUPPORTED YET]"
echo "-a                 display all user's bbs-reducers progress [NOT SUPPORTED YET]"
echo "-s                 silent mode, only return percentage progress (e.g. for cexec)"
echo "-v                 turn verbose messages on"
echo "-d                 turn debug messages on"
echo "-h                 display this help info"
}

# Need to set time LOCAL to "C" to correctly interprete date and time stamps
LC_ALL=C

# Parse command line arguments
while getopts "u:p:k:l:n:asvdh" opt
do
  case ${opt} in
    u) 
      interval=${OPTARG}
      ;;
    p)
      pid=${OPTARG}
      ;;
    k)
      key=${OPTARG}
      ;;    
    l)
      logfile=${OPTARG}
      ;;        
    n)
      network=1
      port=${OPTARG}      # set listening port
      ;;        
    a)
      all=1
      ;;            
    s)
      silent=1            # silent mode, only return status
      ;;
    v)
      verbosity=1
      ;;            
    d)
      debug=1
      ;;            
    h)
      usage
      exit 0
      ;;
    :)
      usage
      exit 1
      ;;
    \?)
      echo "Invalid option: -${OPTARG}" >&2
      ;;
  esac
done

# Check which system we are on
system=`uname`


# Display parameters used
if [ ${verbosity} -eq 1 ]
then
  echo "Using the following options:"
  echo "system              = ${system}"
  echo "pid                 = ${pid}"
  echo "update interval     = ${interval}"
  echo "logfile             = ${logfile}"
  echo "network connection  = ${network}"
  echo "port                = ${network}"
  echo "key                 = ${key}"
  echo "verbosity           = ${verbosity}"
  echo "silent              = ${silent}"
  echo "debug               = ${debug}"
fi

# TODO:
# if no -p option was given, catch all bbs-reducers under running as this ${USER}
# pids=`pgrep bbs-reducer -u ${USER}`

# Check command parameters
#echo "pid = ${pid}"
if ! [[ "${pid}" =~ ^[0-9]+$ ]]
then
  exit 1
fi
# From
#[[ ${pid} =~ "^[0-9]+$" ]] && echo "pid exists" || echo "pid doesn't exist" && exit 1

if [ "${logfile}" == "" ]   # check if we have already been given an over-ride logfile
then 
  if [ -z "${pid}" ]    # Check if process is running
  then
    exit 1
  fi
  # If no logfile was given on the command line, look for it
  if [ "${system}" == "Darwin" ]
  then
    logfile=`/usr/sbin/lsof | grep  ${pid} | grep '.log$' | gawk '{print $9}'`
  else
    logfile=`ls -l  /proc/${pid}/fd | grep .log | gawk '{print $10}'`
  fi
fi

#echo "logfile = ${logfile}"   # DEBUG

# Check if logfile exists and has size>0 
if [ ! -e "${logfile}" ]
then
  echo "${logfile} for ${pid} doesn't exist. Exiting"
  exit 1
fi
if [ ${verbosity} -eq 1 ]
then
  echo "Found logfile for ${pid}: ${logfile}"
fi

# TODO
# Network connection
# [ -S FILE ]	True if FILE exists and is a socket.


# Get start and end time of MS from log
# Time          : 2011/10/30/02:30:00 - 2011/10/30/11:50:01
starttime=`head -n 30 ${logfile} | grep -i "Time          : "| gawk '{print $3}'`
endtime=`head -n 30 ${logfile} | grep -i "Time          : "| gawk '{print $5}'`

# Get process start time from creation time of logfile
#if [ "${system}" == "Darwin" ]
#then
#  creationtime=$(stat -r ${logfile} | gawk '{print $12}')
#else
#  creationtime=$(stat -c %y%h ${logfile} | gawk '{print $1 " "$2}' | date -d - +%s)
#fi

#echo "starttime = ${starttime}"   # DEBUG
#echo "endtime = ${endtime}"   # DEBUG

# Convert starttime and endtime to seconds
if [ "${system}" == "Darwin" ]
then
#  starttimes=`date -jf '%Y/%m/%d/%H:%M:%S' "$(echo ${starttime} | sed 's,/, ,3')" +%s`
#  endtimes=`date -jf '%Y/%m/%d/%H:%M:%S' "$(echo ${starttime} | sed 's,/, ,3')" +%s`
  starttimes=`date -jf '%Y/%m/%d/%H:%M:%S' ${starttime} +%s`
  endtimes=`date -jf '%Y/%m/%d/%H:%M:%S' ${endtime} +%s`
else
  starttimes=`date -d "$(echo ${starttime} | sed 's,/, ,3')" +%s`
  endtimes=`date -d "$(echo ${endtime} | sed 's,/, ,3')" +%s`
fi

#echo "starttimes = ${starttimes}"   # DEBUG
#echo "endtimes = ${endtimes}"   # DEBUG

# Loop while process is alive
pid=$(ps -p ${pid} | gawk 'NR < 2 { next };{print $1}')
while [ ! -z "${pid}" ]
do
  # tail the provided BBS log and grep for "Time: "
  timeline=`tail -n 30 ${logfile} | grep "Time:"`
  stepline=`tail -n 30 ${logfile} | grep "Step:"`

  if [ ${debug} -eq 1 ]; then
    echo "timeline = ${timeline}"   # DEBUG
    echo "stepline = ${stepline}"   # DEBUG
  fi

  if [ -z "${timeline}" ]     # Retry with longer "tail"
  then
    timeline=`tail -n 100 ${logfile} | grep "Time:"`
  fi
  if [ -z "${stepline}" ]     # Retry with longer "tail"  
  then
    stepline=`tail -n 100 ${logfile} | grep "Step:"`
  fi
  if [ -z "${timeline}" ]
  then
    sleep 1
    continue
  fi


  # Do we need this for chunk statistics?
  startchunk=`echo ${timeline} | gawk '{print $2}'`   # catch the start time of the chunk
  endchunk=`echo ${timeline} | gawk '{print $4}'`    # catch the end time of the chunk
  step=`echo ${stepline} | gawk '{print $2}'`

  # Convert to seconds
  if [ "${system}" == "Darwin" ]
  then
    startchunks=`date -jf '%Y/%m/%d/%H:%M:%S' ${startchunk} +%s`
    endchunks=`date -jf '%Y/%m/%d/%H:%M:%S' ${endchunk} +%s`
    #now=$(date -j +%s)
  else
    startchunks=`date -d "$(echo ${startchunk} | sed 's,/, ,3')" +%s`
    endchunks=`date -d "$(echo ${endchunk} | sed 's,/, ,3')" +%s`
    #now=$(date +%s)
  fi

  # Get time elapsed from process manager 
  ncolon=$(ps -o etime ${pid} | gawk 'NR < 2 { next };{print $1}' | grep -c ':')
  
  #echo "$(ps -o etime ${pid} | gawk 'NR < 2 { next };{print $1}')"    # DEBUG
  ncolon="$(ps -o etime ${pid} | gawk 'NR < 2 { next };{print $1}' | grep -o ":" | wc -l | sed s/\ //g)"
  
  if [ ${ncolon} -eq 1 ]  # if we have only mm:ss
  then  
    #hours=`ps -o etime ${pid} | gawk 'NR < 2 { next };{print $1}' | gawk 'BEGIN{FS=":"}; {print $1}'`  
    mins=`ps -o etime ${pid} | gawk 'NR < 2 { next };{print $1}' | gawk 'BEGIN{FS=":"}; {print $1}'`
    secs=`ps -o etime ${pid} | gawk 'NR < 2 { next };{print $1}' | gawk 'BEGIN{FS=":"}; {print $2}'`
    timeelapsed=`echo "scale=10; ${mins}*60+${secs}" | bc`  # elapsed wall clock time
  else                    # otherwise hh:mm:ss
    hours=`ps -o etime ${pid} | gawk 'NR < 2 { next };{print $1}' | gawk 'BEGIN{FS=":"}; {print $1}'`  
    mins=`ps -o etime ${pid} | gawk 'NR < 2 { next };{print $1}' | gawk 'BEGIN{FS=":"}; {print $2}'`
    secs=`ps -o etime ${pid} | gawk 'NR < 2 { next };{print $1}' | gawk 'BEGIN{FS=":"}; {print $3}'`
    timeelapsed=`echo "scale=10; ${hours}*3600+${mins}*60+${secs}" | bc`  # elapsed wall clock time
  fi

  if [ ${debug} -eq 1 ]; then
    echo "hours = ${hours}" # DEBUG
    echo "mins = ${mins}"   # DEBUG
    echo "secs = ${secs}"   # DEBUG  
  fi

  # Get time that has passed (timeelapsed is only CPU time, now use wallclocktime)  
  # wallclocktime=$(echo "${now} - ${creationtime}" | bc)
  #timeelapsed=$(ps -oetime -p ${pid} | gawk 'NR < 2 { next };{print $1}')


  if [ ${debug} -eq 1 ]; then
    #echo "creationtime  = ${creationtime}"  # DEBUG
    #echo "wallclocktime = ${wallclocktime}"   # DEBUG
    #echo "now           = ${now}"           # DEBUG
    echo "timeelapsed   = ${timeelapsed}"   # DEBUG
  fi

  # calculate chunks done
  nchunks=`echo "(${endtimes} - ${starttimes})/(${endchunks} - ${startchunks})" | bc`

  if [ ${debug} -eq 1 ]; then
    echo "nchunks = ${nchunks}"                   # DEBUG
  fi

  # calculate chunks done
  chunk=`echo "(${endchunks}-${starttimes})/(${endchunks} - ${startchunks})"   | bc`
  remainingchunks=`echo "${nchunks}-${chunk}" | bc`
  percentage=`echo "scale=0; (100*${chunk})/${nchunks}" | bc`

  if [ ${debug} -eq 1 ]; then  
    echo "chunk = ${chunk}"                       # DEBUG
    echo "remaining chunks = ${remainingchunks}"  # DEBUG
    echo "percentage = ${percentage}"
  fi

  if [ "! -z ${nchunks}" -a   "! -z ${chunk}" -a "! -z ${percentage}" ]
  then
    # estimate remaining time
    remainingtime=`echo "scale=2; (${timeelapsed}/${chunk})*(${remainingchunks})" | bc`  
#    remainingtime=`echo "scale=2; (${wallclocktime}/${chunk})*(${remainingchunks})" | bc`  
    
    if [ ${debug} -eq 1 ]; then
      echo "remainingtime = ${remainingtime}"  # DEBUG
    fi
  
    # get terminal width and determine variables for progress bar
    rim=31            # rim for information around progress bar
    if [ ${silent} -eq 1 ]
    then
      width=`tput cols -T xterm-color`
    else
        width=`tput cols`
    fi
    nblocks=`echo "scale=0;  (${width}-${rim})*${percentage}*0.01" | bc`
    nblocks=`printf %0.f ${nblocks}`      # round to integer
    pblocks=`for((i=1;i<=${nblocks};i++));do printf "%s" "#";done;`
    nspaces=`echo "scale=0; ${width}-${rim}-${nblocks}-${#step}" | bc`
    nspaces=`printf %0.f ${nspaces}`      # round to integer
    pspaces=`for((i=1;i<=${nspaces};i++));do printf "%s" " ";done`
 
    if [ `echo "${remainingtime} > 3600.0" | bc` -eq 1 ]
    then 
      remainingtime=`echo "scale=1; ${remainingtime}/3600.0" | bc`
      echo -ne "Chunk ${chunk}/${nchunks} [${pblocks} ${step}${pspaces} ${percentage}%] ${remainingtime} h\r"   
    elif [ `echo "${remainingtime} > 60.0" | bc` -eq 1 ]
    then
      remainingtime=`echo "scale=1; ${remainingtime}/60.0" | bc`
      echo -ne "Chunk ${chunk}/${nchunks} [${pblocks} ${step}${pspaces} ${percentage}%] ${remainingtime}  min\r"   
    else
      echo -ne "Chunk ${chunk}/${nchunks} [${pblocks} ${step}${pspaces} ${percentage}%] ${remainingtime}  sec\r"   
    fi
  fi
  
  if [ ${silent} -eq 1 ]      # in silent mode only return one progress information
  then
    echo ""   # put a return
    break
  fi
  # Exit when complete
  if [ ${chunk} -eq ${nchunks} ]
  then
    break
  else  
    sleep ${interval}         # wait for update period
    #pid=$(ps -p ${pid} | gawk 'NR < 2 { next };{print $1}')  # get pid if process is still alive
  fi
done

exit 0
