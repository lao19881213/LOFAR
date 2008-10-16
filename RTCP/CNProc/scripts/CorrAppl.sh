#!/bin/bash
#
# CorrAppl: a start/stop/status script for swlevel
#
# Copyright (C) 2007
# ASTRON (Netherlands Foundation for Research in Astronomy)
# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
# Syntax: CorrAppl start|stop|status
#
# $Id$
#

#
# SyntaxError msg
#
SyntaxError()
{
	Msg=$1

	[ -z "${Msg}" ] || echo "ERROR: ${Msg}"
	echo ""
	echo "Syntax: $(basename $0) start | stop | status"
	echo ""
	exit 1
}

#
# Start the program when it exists
#
start_prog()
{
	# put here your code to start your program
	echo 'start_prog()'
}

#
# Stop the program when it is running
#
stop_prog()
{
	# put here your code to stop your program
	killall ApplController
	ps -ef | grep -v grep | grep -v ACDaemon[^\ ] | grep ACDaemon 2>&1 >/dev/null
	if [ $? -ne 0 ]; then
	  if [ -f ../etc/ACD.admin ]; then 	
	    rm ../etc/ACD.admin
	  fi
	fi  
	echo 'Freeing CorrAppl (5 minutes)'
	ssh $USER@bglsn /opt/lofar/bin/stopBGL.py 
}

#
# show status of program
#
# arg1 = levelnr
#
status_prog()
{
	levelnr=$1

	# put here code to figure out the status of your program and
	# fill the variables prog and pid with the right information

	# e.g.
	prog=CorrAppl

	# this line should be left in, it shows the status in the right format
	#echo ${levelnr} ${prog} ${pid} | awk '{ printf "%s : %-25s %s\n", $1, $2, $3 }'
	echo ${levelnr} ${prog} `ssh $USER@bglsn /opt/lofar/bin/stopBGL.py --status=true` | awk '{ printf "%s : %-25s %s\n", $1, $2, $3 }'
}

#
# MAIN
#

# when no argument is given show syntax error.
if [ -z "$1" ]; then
	SyntaxError
fi

# first power down to this level
case $1 in
	start)	start_prog
			;;
	stop)	stop_prog
			;;
	status)	status_prog $2
			;;
	*)		SyntaxError
			;;
esac
