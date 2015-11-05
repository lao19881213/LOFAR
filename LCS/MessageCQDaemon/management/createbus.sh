#!/bin/bash -x
# -v
# description:
# 
# createbus <busname> <hub> <spoke> [<spoke> ..]
#   create a bus topology with name busname over the hub and spokes
#
# deletebus <busname> <hub> <spoke> [<spoke> ..]
#   delete the bus topology with name busname over the hub and spokes
#
# addlocalbus <busname> [nodename]
#   create the bus components on localhost or on node [nodename]
#
# dellocalbus <busname> [nodename]
#   delete the bus components on localhost or on node [nodename]

#
# TODO:
#   - add proper error handling
#   - add option for uni-directional spoke connections
#   - produce logging for automated tools
#   - refactor 'add' vs 'create'


function addlocalbus {
    host=`hostname`
    if (( $# >= 1 ))
    then
        busname=$1
        if (( $# > 1 ))
        then
            port=$2
            host=$3
        fi
        
        qpid-config -b $host:$port add exchange topic $busname".deadletterproxy" --durable
        qpid-config -b $host:$port add exchange direct $busname --durable --alternate-exchange=$busname".deadletterproxy"

        qpid-config -b $host:$port add queue $busname".deadletter" --durable
        qpid-config -b $host:$port bind $busname".deadletterproxy" $busname".deadletter" '#' --durable
    else
        echo "Usage: $FUNCNAME <busname> [nodename]"
    fi
}
 
function dellocalbus {
    host=`hostname`
    if (( $# >= 1 ))
    then
        busname=$1
        if (( $# > 1 ))
        then
            port=$2
            host=$3
        fi
  

        qpid-config -b $host:$port del exchange $busname 
        qpid-config -b $host:$port del exchange $busname".deadletterproxy"
        qpid-config -b $host:$port del queue $busname".deadletter" --force
    else
        echo "Usage: $FUNCNAME <busname> [nodename]"
    fi
}



function connectbus {
    if (( $# == 3 ))
    then
        busname=$1
        firstnode=$2  # Hubnode
        secondnode=$3 # Spoke node

        qpid-route dynamic add $firstnode $secondnode $busname --durable --ack=1  
        qpid-route dynamic add $secondnode $firstnode $busname --durable --ack=1
    else
        echo "Usage $FUNCNAME <busname> <firstnode> <secondnode>"
    fi
}
function disconnectbus {
    if (( $# == 3 ))
    then
        busname=$1
        firstnode=$2
        secondnode=$3

        qpid-route dynamic del $firstnode $secondnode $busname 
        qpid-route dynamic del $secondnode $firstnode $busname 
    else
        echo "Usage $FUNCNAME <busname> <firstnode> <secondnode>"
    fi
}

function createbus {
    if (( "$#" >=4 ))
    then
        busname=$1
        port=$2
        hubname=$3    
       
        addlocalbus $busname $port $hubname
        shift 3
        for i in "$@"
        do
            addlocalbus $busname $port $i
            connectbus $busname $hubname:$port $i:$port
        done
    else
        echo "usage: $FUNCNAME <busname> <hubnode> <port> <spokenode> [<spokenode>..]"
    fi
}
 
function deletebus {
    if (( "$#" >=4 ))
    then
        busname=$1
        port=$2
        hubname=$3        
        shift 3
        for i in "$@"
        do
            disconnectbus $busname $hubname:$port $i:$port
            dellocalbus $busname $port $i
        done
        dellocalbus $busname $port  $hubname 
    else
        echo "usage: $FUNCNAME <busname> <hubnode> <port> <spokenode> [<spokenode>..]"
    fi
}

# ----------------------------------------------------------------------------------

function usage {
    echo "'createbus.sh' creates or deletes a dynamic routing bus between two nodes." 
    echo "   createbus.sh <add|del> <busname> <port> <hubnode> <spkokenode> [<spokenode>..] [ignore]" 
    echo ""
    echo "If only one hostname is given the second hostname is presumed to be the local node." 
    echo "to ensure one-time delivery of messages avoid loops in the bus topology." 
    echo "If ignore is provided errors will be ignored, this could be used to clean partly created"
    echo "bus structures/setups"
}

if (( $# < 2  ))
# validate the number of arguments
then
    usage
    exit 1
fi

operation="$1"
busname="$2"
port="$3"



# make an array of the node names
declare -a nodenames
declare -i numnodes
numnodes=0

# We need the option to ignore errors: it can be messy
# check if "ignore" is mentioned on the command line

# skip both the name of the process and the required operation
shift 3

ignore="False"
for tmp in $@
do 
    if [ "$tmp" == "ignore" ]
    then
        ignore="True"
    else
        nodenames[$numnodes]="$tmp"
        numnodes=$((numnodes + 1))
    fi
done

if [ "$ignore" == "True" ]
then
    # do not exit on ignore
    echo "ignoring errors"
else
    set -e   
fi    

# allow for zero nodes and use hostname as nodename.
if (( numnodes == 0 )); then
    nodenames[$numnodes]="localhost"
    numnodes=$((numnodes + 1))
fi

# depending on the first argument call the del or add  bus function
if [ "$operation" == "del" ]; then
    if (( numnodes == 1 )); then    
        dellocalbus $busname $port ${nodenames[*]}
    else
        deletebus $busname $port ${nodenames[*]}
    fi
elif [ "$operation" == "add" ]; then
    if (( numnodes == 1 )); then
        addlocalbus  $busname $port ${nodenames[*]}
    else
        createbus $busname $port ${nodenames[*]}
    fi
else
    usage
    exit 1
fi    

