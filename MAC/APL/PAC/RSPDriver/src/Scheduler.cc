//#  Scheduler.cc: implementation of the Scheduler class
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

#include "Scheduler.h"
#include "SyncAction.h"

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>

using namespace RSP;
using namespace LOFAR;
using namespace RSP_Protocol;

#define SCHEDULING_DELAY 2

Scheduler::Scheduler()
{
}

Scheduler::~Scheduler()
{
  /* clear the various queues */
  while (!m_later_queue.empty())
  {
    Command* c = m_later_queue.top();
    delete c;
    m_later_queue.pop();
  }
  while (!m_now_queue.empty())
  {
    Command* c = m_now_queue.top();
    delete c;
    m_now_queue.pop();
  }
  while (!m_periodic_queue.empty())
  {
    Command* c = m_periodic_queue.top();
    delete c;
    m_periodic_queue.pop();
  }
  while (!m_done_queue.empty())
  {
    Command* c = m_done_queue.top();
    delete c;
    m_done_queue.pop();
  }

  for (map< GCFPortInterface*, vector<SyncAction*> >::iterator port = m_syncactions.begin();
       port != m_syncactions.end();
       port++)
  {
    for (vector<SyncAction*>::iterator sa = (*port).second.begin();
	 sa != (*port).second.end();
	 sa++)
    {
      delete (*sa);
    }
  }
}

GCFEvent::TResult Scheduler::run(GCFEvent& event, GCFPortInterface& /*port*/)
{
  const GCFTimerEvent* timeout = static_cast<const GCFTimerEvent*>(&event);
  
  if (F_TIMER == event.signal)
  {
    LOG_INFO("Scheduler::run");

    setCurrentTime(timeout->sec, 0);

    scheduleCommands();
    processCommands();

    initiateSync(event); // matched by completeSync
  }
  else
  {
    LOG_ERROR("received invalid event != F_TIMER");
  }

  return GCFEvent::HANDLED;
}

GCFEvent::TResult Scheduler::dispatch(GCFEvent& event, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::NOT_HANDLED;
  GCFTimerEvent timer;
  GCFEvent* current_event = &event;
  bool sync_completed = true;
  
  /**
   * Dispatch the event to the first SyncAction that
   * has not yet reached its 'final' state.
   */
  for (vector<SyncAction*>::iterator sa = m_syncactions[&port].begin();
       sa != m_syncactions[&port].end();
       sa++)
  {
    if (!(*sa)->hasCompleted())
    {
      // stil busy
      sync_completed = false;

      status = (*sa)->dispatch(*current_event, (*sa)->getBoardPort());

      //
      // if the syncaction has not yet been completed, break the loop
      // it will receive another event to continue
      //
      if (!(*sa)->hasCompleted()) break;
      else current_event = &timer; 
    }
  }

  if (sync_completed) m_sync_completed[&port] = true;

  if (syncHasCompleted())
  {
    completeSync();
  }

  return status;
}

bool Scheduler::syncHasCompleted()
{
  bool result = true;
  
  for (map< GCFPortInterface*, bool >::iterator it = m_sync_completed.begin();
       it != m_sync_completed.end();
       it++)
  {
    if (!(*it).second) result = false;
  }

  return result;
}

void Scheduler::addSyncAction(SyncAction* action)
{
  m_syncactions[&(action->getBoardPort())].push_back(action);
  m_sync_completed[&(action->getBoardPort())] = false;
}

Timestamp Scheduler::enter(Command* command)
{
  m_later_queue.push(command);
  
  Timestamp t = command->getTimestamp();

  /* determine at which time the command can actually be carried out */
  if (t.sec() < m_current_time.sec() + SCHEDULING_DELAY)
  {
    if (t.sec() > 0)
    {
      LOG_WARN(formatString("command missed deadline by %d seconds",
			    m_current_time.sec() - t.sec()));
    }
    
    t = m_current_time + SCHEDULING_DELAY;
  }

  return t;
}

void Scheduler::setCurrentTime(long sec, long usec)
{
  /* adjust the current time */
  struct timeval tv;
  tv.tv_sec  = sec;
  tv.tv_usec = usec;
  m_current_time.set(tv);
}

Timestamp Scheduler::getCurrentTime() const
{
  return m_current_time;
}

void Scheduler::scheduleCommands()
{
  /* move appropriate client commands to the now queue */
  while (!m_later_queue.empty())
  {
    Command* command = m_later_queue.top();

    if (m_current_time > command->getTimestamp() + 1)
    {
      /* discard old commands, the're too late! */
      LOG_WARN("discarding late command");

      m_later_queue.pop();
      delete command;
    }
    else if (m_current_time == command->getTimestamp() + 1)
    {
      m_now_queue.push(command);
      m_later_queue.pop();
    }
    else break;
  }

#if 0
  /* copy period commands to the now queue */
  while (!m_period_queue.empty())
  {
    Command * command = m_period_queue.top();

    struct timeval now;
    m_current_time.get(&now);
    if (0 == (now.tv_sec + 1 % command->getPeriod()))
    {
      /* copy the command and push on the now queue */
    }
  }
#endif
}

void Scheduler::processCommands()
{
  while (!m_now_queue.empty())
  {
    Command* command = m_now_queue.top();

    /* let the command apply its changes to the cache */
    command->apply(Cache::getInstance().getBack());

    /* move from the now queue to the done queue */
    m_now_queue.pop();
    m_done_queue.push(command);
  }
}

void Scheduler::initiateSync(GCFEvent& event)
{
  /**
   * Send the first syncaction for each board the timer
   * event to set of the data communication to each board.
   */
  for (map< GCFPortInterface*, vector<SyncAction*> >::iterator port = m_syncactions.begin();
       port != m_syncactions.end();
       port++)
  {
    // reset completed flag
    m_sync_completed[(*port).first] = false;

    for (vector<SyncAction*>::iterator sa = (*port).second.begin();
	 sa != (*port).second.end();
	 sa++)
    {
      (*sa)->setCompleted(false);
    }
    
    // send F_TIMER event to first syncactions for each board
    if (!(*port).second.empty())
    {
      (*port).second[0]->dispatch(event, (*port).second[0]->getBoardPort());
    }
  }
}

void Scheduler::completeSync()
{
  Cache::getInstance().swapBuffers();
  completeCommands();
}

void Scheduler::completeCommands()
{
  while (!m_done_queue.empty())
  {
    Command* command = m_done_queue.top();

    command->complete(Cache::getInstance().getFront());

    m_done_queue.pop();
    delete command;
  }
}

