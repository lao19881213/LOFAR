#**************************************************************** 
#* Reads a formatted cluster description containing
#* CPU, GPU and station mapping and converts it to a parset file
#* ready for consumption by cobalt
#*
inputfile = 'cobalt-station-mapping-interfaces.txt'
outputfile = 'station_map.parset'

f = open(inputfile, 'r')
cobalt_nodes = {}  # Will contain the parsed node information
for idx, line in enumerate(f):
  #----------------------------------------------------------------------------
  # sanitize the input
  # skip the first 10 lines for they are header
  if idx < 10:
    continue
  # Skip the empty lines
  if len(line) < 2:
    continue
  #split in whitelines
  entries = line.split()
  
  # skip if the second entry does not start with CD or RS
  if ( not (entries[1].startswith("CS") or entries[1].startswith("RS")) ):
    continue
    
  # -----------------------------------------------------------------------
  # convert to a proper parset fragment
  # split the cobalt node information
  cid_pair = entries[3].split(".")
  # cobalt nodenode id - 1 times 2
  cid = (int(cid_pair[0][3:]) - 1)* 2 
  
  cid_eth = int(cid_pair[1][3:])
 
  # parse the ethernet name
  if (cid_eth is 4 or cid_eth is 5):
    cid_cpu = 1
  else:
    cid_cpu = 0
  cid += cid_cpu
   #else do not increase for cpu number
  
  # sanitiz  the station name
  if entries[1].startswith("RS"):
      entries[1] = entries[1] + "HBA"
      
  # If we already saw the cid
  if (cid in cobalt_nodes):
    node = cobalt_nodes[cid] 
    # Only append the station name
    node["stations"].append(entries[1])
    
  # else parse the lines to dicts  
  else:
    node = {'cid': '',
            'host': '',
            'cpu': '',
            'gpus': [],
            'stations': []}
    node['cid'] = cid
    node['host'] = "cbm" + entries[3].split('.')[0][3:]  # remove cbt and prepend cbm
    node['cpu'] = cid_cpu
    if (node['cpu'] is 0):
      node['gpus'] = [0,1]
    else:
      node['gpus'] = [2,3]
    node['stations'].append(entries[1])
    
  # The configuration file only mentions HBA/HBA0/HBA1, but
  # we want all antenna sets to be mentioned in the parset.
  if (entries[1].endswith("HBA")): # RSxxxHBA
    stationname = entries[1][0:5]
    node['stations'].append(stationname + "LBA")

  if (entries[1].endswith("HBA0")): # CSxxxHBA0
    stationname = entries[1][0:5]
    node['stations'].append(stationname + "LBA")
    node['stations'].append(stationname + "HBA")
  
    # add the newly create node
    cobalt_nodes[cid] = node

# -----------------------------------------------------------------
# print to parset format
prefix = "Cobalt.Hardware."  
with open(outputfile,'w') as f:
  # number of nodes
  f.write(prefix + "nrNodes=" + str(len(cobalt_nodes)) + "\n")
  
  # actual content
  for key, value in cobalt_nodes.items():
    node_prefix = "Node[" + str(value["cid"]) +"]."
       
    f.write( prefix + node_prefix + "host=" + str(value["host"]) + "\n")
    f.write( prefix + node_prefix + "cpu=" + str(value["cpu"]) + "\n")
    f.write( prefix + node_prefix + "gpus=" + str(value["gpus"]) + "\n")
    f.write( prefix + node_prefix + "stations=" + str(value["stations"]) + "\n")
  
  
  
  