/***************************************************************************
 *   Copyright (C) 2008 by A.R. Offringa   *
 *   offringa@astro.rug.nl   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <AOFlagger/remote/processcommander.h>

#include <unistd.h> //gethostname

#include <AOFlagger/remote/serverconnection.h>

#include <AOFlagger/quality/statisticscollection.h>

namespace aoRemote {

ProcessCommander::ProcessCommander(const ClusteredObservation &observation)
: _server(), _observation(observation)
{
	_collection = new StatisticsCollection();
	
	_server.SignalConnectionCreated().connect(sigc::mem_fun(*this, &ProcessCommander::onConnectionCreated));
}

ProcessCommander::~ProcessCommander()
{
	for(std::vector<RemoteProcess*>::iterator i=_processes.begin();i!=_processes.end();++i)
	{
		delete *i;
	}
	delete _collection;
}

void ProcessCommander::Run()
{
	makeNodeMap(_observation);
	const std::string thisHostName = GetHostName();
	
	//construct a process for each unique node name
	for(std::map<std::string, std::deque<ClusteredObservationItem> >::const_iterator i=_nodeMap.begin();i!=_nodeMap.end();++i)
	{
		_processes.push_back(new RemoteProcess(i->first, thisHostName));
	}
	_server.Run();
}

void ProcessCommander::makeNodeMap(const ClusteredObservation &observation)
{
	const std::vector<ClusteredObservationItem> &items = observation.GetItems();
	for(std::vector<ClusteredObservationItem>::const_iterator i=items.begin();i!=items.end();++i)
	{
		_nodeMap[i->HostName()].push_back(*i);
	}
}

std::string ProcessCommander::GetHostName()
{
	char name[HOST_NAME_MAX];
	if(gethostname(name, HOST_NAME_MAX) == 0)
	{
		return std::string(name);
	} else {
		throw std::runtime_error("Error retrieving hostname");
	}
}

void ProcessCommander::onConnectionCreated(ServerConnection &serverConnection, bool &acceptConnection)
{
	serverConnection.SignalAwaitingCommand().connect(sigc::mem_fun(*this, &ProcessCommander::onConnectionAwaitingCommand));
	serverConnection.SignalFinishReadQualityTables().connect(sigc::mem_fun(*this, &ProcessCommander::onConnectionFinishReadQualityTables));
	acceptConnection = true;
}

void ProcessCommander::onConnectionAwaitingCommand(ServerConnection &serverConnection)
{
	const std::string &hostname = serverConnection.Hostname();
	
	std::cout << "Connection " << hostname << " awaiting commands..." << std::endl;
	
	NodeMap::iterator iter = _nodeMap.find(hostname);
	if(iter == _nodeMap.end())
	{
		serverConnection.StopClient();
		if(_nodeMap.empty())
		{
			_server.Stop();
		}
	}
	else {
		std::deque<ClusteredObservationItem> &items = iter->second;
		if(items.empty())
		{
			serverConnection.StopClient();
			_nodeMap.erase(iter);
			if(_nodeMap.empty())
				_server.Stop();
		}
		else
		{
			const std::string msFilename = items.front().LocalPath();
			items.pop_front();
			if(items.empty())
				_nodeMap.erase(iter);
			StatisticsCollection *collection = new StatisticsCollection();
			serverConnection.ReadQualityTables(msFilename, *collection);
		}
	}
}

void ProcessCommander::onConnectionFinishReadQualityTables(class ServerConnection &serverConnection, StatisticsCollection &collection)
{
	if(collection.PolarizationCount() == 0)
		throw std::runtime_error("Client sent StatisticsCollection with 0 polarizations.");
	
	// If the collection is still empty, we need to set its polarization count
	if(_collection->PolarizationCount() == 0)
	{
		_collection->SetPolarizationCount(collection.PolarizationCount());
	}
	_collection->Add(collection);
}


}
