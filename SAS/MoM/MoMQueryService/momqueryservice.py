#!/usr/bin/python
# $Id$

'''
Simple Service listening on momqueryservice.GetProjectDetails
which gives the project details for each requested mom object id

Example usage:
service side: just run this service somewhere where it can access the momdb and
a qpid broker.
Make sure the bus exists: qpid-config add exchange topic <busname>

client side: do a RPC call to the <busname>.GetProjectDetails with a
comma seperated string of mom2object id's as argument.
You get a dict of mom2id to project-details-dict back.

with RPC(busname, 'GetProjectDetails') as getProjectDetails:
    res, status = getProjectDetails(ids_string)

'''
from os import stat
import sys
import logging
import time
from optparse import OptionParser
from mysql import connector
from mysql.connector.errors import OperationalError
from lofar.messaging import Service
from lofar.messaging.Service import MessageHandlerInterface
from lofar.common.util import waitForInterrupt
from lofar.mom.momqueryservice.config import DEFAULT_MOMQUERY_BUSNAME, DEFAULT_MOMQUERY_SERVICENAME
from lofar.common import dbcredentials

logger=logging.getLogger(__file__)

def _idsFromString(id_string):
    if not isinstance(id_string, basestring):
        raise ValueError('Expected a string, got a ' + str(type(id_string)))

    # parse text: it should contain a list of ints
    # filter out everything else to prevent sql injection
    ids = [int(y) for y in [x.strip() for x in id_string.split(',')] if y.isdigit()]
    return ids

def _isListOfInts(items):
    if not items:
        return False

    if not isinstance(items, list):
        return False

    for x in items:
        if not isinstance(x, int):
            return False

    return True

class MoMDatabaseWrapper:
    '''handler class for details query in mom db'''
    def __init__(self, dbcreds):
        self.dbcreds = dbcreds
        self.conn = None

    def _connect(self):
        if self.conn:
            self.conn.close()

        connect_options = self.dbcreds.mysql_connect_options()
        connect_options['connection_timeout'] = 5
        try:
            logger.info("Connecting to %s" % self.dbcreds.stringWithHiddenPassword())
            self.conn = connector.connect(**connect_options)
            logger.debug("Connected to %s" % self.dbcreds.stringWithHiddenPassword())
        except Exception as e:
            logger.error(str(e))
            self.conn = None

    def _executeQuery(self, query):
        # try to execute query on flaky lofar mysql connection
        # max of 3 tries, on succes return result
        # use new connection for every query,
        # because on the flaky lofar network a connection may appear functional but returns improper results.
        for i in range(3):
            try:
                self._connect()
                cursor = self.conn.cursor(dictionary=True)
                cursor.execute(query)
                return cursor.fetchall()
            except (OperationalError, AttributeError) as e:
                logger.error(str(e))

    def getProjectDetails(self, mom_ids):
        ''' get the project details (project_mom2id, project_name,
        project_description, object_mom2id, object_name, object_description,
        object_type, object_group_id) for given mom object mom_ids
        :param mixed mom_ids comma seperated string of mom2object id's, or list of ints
        :rtype list of dict's key value pairs with the project details
        '''
        if not mom_ids:
            return {}

        if _isListOfInts(mom_ids):
            ids = mom_ids
        else:
            ids = _idsFromString(mom_ids)

        if not ids:
            raise ValueError("Could not find proper ids in: " + mom_ids)

        ids_str = ','.join([str(id) for id in ids])

        logger.info("Query for mom id%s: %s" %
                    ('\'s' if len(ids) > 1 else '', ids_str))

        # TODO: make a view for this query in momdb!
        query = '''SELECT project.mom2id as project_mom2id, project.id as project_mom2objectid, project.name as project_name, project.description as project_description,
        object.mom2id as object_mom2id, object.id as object_mom2objectid, object.name as object_name, object.description as object_description, object.mom2objecttype as object_type, object.group_id as object_group_id
        FROM mom2object as object
        left join mom2object as project on project.id = object.ownerprojectid
        where object.mom2id in (%s)
        order by project_mom2id
        ''' % (ids_str,)
        rows = self._executeQuery(query)

        logger.info("Found %d results for mom id(s): %s" %
                    (len(rows), ids_str))

        result = {}
        for row in rows:
            object_mom2id = row['object_mom2id']
            result[str(object_mom2id)] = dict(row)

        logger.info(result)

        return result

    def getProjects(self):
        ''' get the list of all projects with columns (project_mom2id, project_name,
        project_description, status_name, status_id, last_user_id,
        last_user_name, statustime)
        :rtype list of dict's key value pairs with all projects
        '''
        # TODO: make a view for this query in momdb!
        query = '''SELECT project.mom2id as mom2id, project.name as name, project.description as description,
                statustype.code as status_name,  statustype.id as status_id,
        status.userid as last_user_id, status.name as last_user_name, status.statustime as statustime
        FROM mom2object as project
        left join mom2objectstatus as status on project.currentstatusid = status.id
        left join status as statustype on status.statusid=statustype.id
        where project.mom2objecttype='PROJECT'
        order by mom2id;
        '''
        result = self._executeQuery(query)

        logger.info("Found %d projects" % (len(result), ))

        return result


class ProjectDetailsQueryHandler(MessageHandlerInterface):
    '''handler class for details query in mom db
    :param MoMDatabaseWrapper momdb inject database access via wrapper
    '''
    def __init__(self, **kwargs):
        super(ProjectDetailsQueryHandler, self).__init__(**kwargs)
        self.dbcreds = kwargs.pop("dbcreds", None)

        self.service2MethodMap = {
            'GetProjects': self.getProjects,
            'GetProjectDetails': self.getProjectDetails
            }

    def prepare_loop(self):
        self.momdb = MoMDatabaseWrapper(self.dbcreds)

    def getProjectDetails(self, mom_ids):
        if not mom_ids:
            return {}
        
        ids = _idsFromString(mom_ids)
        if not _isListOfInts(ids):
            raise ValueError("%s is not a proper list of ints" % str(mom_ids))
        return self.momdb.getProjectDetails(ids)

    def getProjects(self):
        return self.momdb.getProjects()

def createService(busname=DEFAULT_MOMQUERY_BUSNAME,
                  servicename=DEFAULT_MOMQUERY_SERVICENAME,
                  dbcreds=None,
                  handler=None,
                  broker=None):
    '''create the GetProjectDetails on given busname
    :param string busname: name of the bus on which this service listens
    :param string servicename: name of the service
    :param Credentials dbcreds: Credentials for the MoM database.
    :param ProjectDetailsQueryHandler handler: ProjectDetailsQueryHandler class Type, or mock like type
    :rtype: lofar.messaging.Service'''

    if not handler:
        handler = ProjectDetailsQueryHandler

    return Service(servicename,
                   handler,
                   busname=busname,
                   numthreads=2,
                   use_service_methods=True,
                   verbose=False,
                   broker=broker,
                   handler_args={'dbcreds' : dbcreds})


def main():
    '''Starts the momqueryservice.GetProjectDetails service'''

    # Check the invocation arguments
    parser = OptionParser("%prog [options]",
                          description='runs the momqueryservice')
    parser.add_option('-q', '--broker', dest='broker', type='string', default=None, help='Address of the qpid broker, default: localhost')
    parser.add_option("-b", "--busname", dest="busname", type="string", default=DEFAULT_MOMQUERY_BUSNAME, help="Name of the bus exchange on the qpid broker, [default: %default]")
    parser.add_option("-s", "--servicename", dest="servicename", type="string", default=DEFAULT_MOMQUERY_SERVICENAME, help="Name for this service, [default: %default]")
    parser.add_option_group(dbcredentials.options_group(parser))
    parser.set_defaults(dbcredentials="MoM")
    (options, args) = parser.parse_args()

    dbcreds = dbcredentials.parse_options(options)

    # start the service and listen.
    with createService(busname=options.busname,
                       servicename=options.servicename,
                       broker=options.broker,
                       dbcreds=dbcreds):
        waitForInterrupt()

if __name__ == '__main__':
    logging.basicConfig(format='%(asctime)s %(levelname)s %(message)s', level=logging.INFO)
    main()
