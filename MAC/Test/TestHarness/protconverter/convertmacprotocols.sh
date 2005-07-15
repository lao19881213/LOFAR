#!/bin/sh

#
#
if [ "$1" == "" ]; then
	echo "usage: convertmacprotocols <base search dir>"
	exit -1
fi

#
# Get list of prot files
#
files=`/usr/bin/find $1 -name *.prot`

#
# for all .prot files
#
for f in $files; do

	#
	# check protocol type
	#
	protNr=0
	proto=`grep F_APL_PROTOCOL $f`
	if [ "$proto" != "" ] ; then
		echo "$f": APL protocol
                protNr=10
	else
        	proto=`grep F_GCF_PROTOCOL $f`
		if [ "$proto" != "" ] ; then
	                echo "$f": GCF protocol
			protNr=3
		fi
	fi
	if [ "$protNr" != "0" ] ; then
		
		#
		# convert the protocol
		#
		fbase=`basename "$f" .prot`
		fname=`dirname "$f"`"/$fbase"
		echo "converting $f..."
		./converter $f $protNr
		autogen -T TSE-protocol.tpl $fname.tmp

		# concatenate it with previous converted prot files
		cat $fbase.tseprot >> MACall.tseprot

		# cleanup
		rm -f $fname.tmp
		rm -f $fbase.tseprot
		echo " "
	fi
done
