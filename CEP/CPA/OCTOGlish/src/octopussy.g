pragma include once
include 'note.g'
include 'debug_methods.g'

octopussy := function (wpclass="",server="./octoglish",
          options="",autoexit=T,suspend=F,verbose=1) 
{
  self := [=];
  public := [=];

  self.opClient := F;
  self.opClient::Died := T;
  self.state := 0;
  self.started := F;
  # this should be consistent with PRI_NORMAL in OCTOPUSSY/Message.h, and
  # is used as the baseline priority value. The priority parameters
  # used below are added to this value
  self.priority_normal := 256;
  
  define_debug_methods(self,public,verbose);

  const self.makeclient := function (server,wpclass="",options="",suspend=F) 
  {
    wider self;
    self.dprint(1,"starting client(",server,",",options,")");
    self.opClient := client(server,options,suspend=suspend);
    if( !is_agent(self.opClient) ) 
      fail paste('server',server,'could not be started');
    self.dprint(1,"connected");
    self.opClient::Died := F;
    # set up fail/exit handler
    whenever self.opClient->["fail done"] do 
    {
      self.opClient::Died := T;
      self.dprint(1,'octoglish: remote client has terminated with event ',$name,$value);
    }
    return T;
  }
  
  const self.makemsg := function (id,rec=F,priority=0,datablock=F,blockset=F)
  {
    wider self;
    data := [=];
    if( !is_boolean(rec) )
    {
      if( !is_boolean(blockset) )
        fail 'unable to send record and blockset together';
      data := rec;
      data::payload := "DataRecord";
    }
    else if( !is_boolean(blockset) )
    {
      # NB: check for blocktype
      data := blockset;
      data::payload := blockset::blocktype;
    }
    if( !is_boolean(datablock) )
    {
      data::datablock := datablock;
    }
    data::id := id;
    data::priority := self.priority_normal+priority;
    data::state := self.state;
    return data;
  }
  
  const self.getscope := function (scope)
  {
    sc := to_lower(scope);
    if( sc == "local" || sc == "process" )
      return 0;
    else if( sc == "host" )
      return 1;
    else if( sc == "global" )
      return 2;
    fail paste('illegal scope',scope);
  }

  
# Public functions
#------------------------------------------------------------------------------
  const public.init := function (wpclass="",server="",options="",suspend=F) 
  {
    wider self;
    if( is_boolean(self.opClient) || self.opClient::Died ) 
    {
      self.makeclient(server,wpclass=wpclass,options=options,suspend=suspend);
    }
    return T;
  }

  const public.done := function () 
  {
    self.opClient->terminate();
  }
  
  const public.subscribe := function (ids,scope="global")
  {
    # set the scope parameter
    wider self;
    sc := self.getscope(scope);
    if( is_fail(sc) )
      fail sc;
    for( id in ids )
    {
     self.dprint(2,"subscribing: ",id,sc);
      # send event
      if( !self.opClient->subscribe([id=id,scope=sc]) )
        fail 'subscribe() failed';
    }
    return T;
  }

  const public.unsubscribe := function (id)
  {
    wider self;
    if( self.opClient->unsubscribe([id=id]) )
      return T;
    else
      fail 'unsubscribe() failed';
  }
  
  const public.start := function ()
  {
    wider self;
    if( self.started  )
      fail 'octopussy already started';
    if( self.opClient->start([=]) )
    {
      self.started := T;
      return T;
    }
    else
      fail 'start() failed';
  }

  const public.log := function (msg,type="normal",level=1)
  {
    wider self;
    # check that we're started
    if( !self.started )
      fail 'octopussy not started';
    # set the type
    tp := to_lower(type);
    if( tp == "normal" )
      tp := "LogNormal";
    else if( tp == "warn" || tp == "warning" )
      tp := "LogWarning";
    else if( tp == "error" )
      tp := "LogError";
    else if( tp == "debug" )
      tp := "LogDebug";
    else if( tp == "fatal" )
      tp := "LogFatal";
    else
      fail paste('unknown log message type: ',type);
    # send the event
    if( self.opClient->log([msg=msg,level=level,type=tp]) )
      return T;
    else
      fail 'log() failed';
  }
  
  const public.send := function (id,dest,rec=F,priority=0,datablock=F,blockset=F)
  {
    wider self;
    # check that we're started
    if( !self.started )
      fail 'octopussy not started';
    rec := self.makemsg(id,rec,priority,datablock,blockset);
    rec::to := dest;
    # send the event
    if( self.opClient->send(rec) )
      return T;
    else
      fail 'send() failed';
  }

  const public.publish := function (id,rec=F,scope="global",priority=0,datablock=F,blockset=F)
  {
    wider self;
    # check that we're started
    if( !self.started )
      fail 'octopussy not started';
    # set the scope
    self.dprint(3,"publish: ",id,scope);
    sc := self.getscope(scope);
    if( is_fail(sc) )
      return sc;
    # create message record
    rec := self.makemsg(id,rec,priority,datablock,blockset);
    rec::scope := sc;
    self.dprint(3,"publishing: ",rec::);
    # send the event
    if( self.opClient->publish(rec) )
      return T;
    else
      fail 'send() failed';
  }

  const public.receive := function (ref value)
  {
    wider self;
    # check that we're started
    if( !self.started )
      fail 'octopussy not started';
    # wait for message
    await self.opClient->*;
    val value := $value;
    self.dprint(3,"got event: ",$name);
    return $name;
  }
  
  const public.setdebug := function(context,level)
  {
    wider self;
    # check that we're started
    if( !self.started )
      fail 'octopussy not started';
    if( self.opClient->debug([context=context,level=level]) )
      return T;
    else
      fail 'setdebug failed'; 
  }
  
  const public.state := function ()
  {
    wider self;
    return self.state;
  }
  
  const public.setstate := function (newstate)
  {
    wider self;
    wider public;
    if( self.state != newstate )
    {
      self.state := newstate;
      return public.publish("WorkProcess.State");
    }
    return T;
  }
  
  const public.agentref := function ()
  {
    wider self;
    return ref self.opClient;
  }
  
  const public.connected := function ()
  {
    wider self;
    return !self.opClient::Died;
  }
  
  const public.started := function ()
  {
    wider self;
    return self.started;
  }
  
  res := public.init(wpclass,server=server,options=options,suspend=suspend);
  if( is_fail(res) )
    return res;
  
  if( autoexit )
    whenever self.opClient->exit do 
    {
      self.dprint(0,"Got 'exit' event, auto-exit enabled, exiting");
      exit 1;
    }
  
  return public;
}

test_octopussy := function (server="./test_glish",options="")
{
  oct := octopussy(server=server,options=options);
  if( is_fail(oct) || !oct.connected() )
    fail 'unable to connect to server';
  oct.subscribe("Pong");
  # create a ping message
  run := T;
  count := 0;
  while( run && count<5 )
  {
    rec := [=];
    rec.Timestamp := 0;
    rec.Invert := T;
    rec.Data := random(10);
    rec.Data_B := [];
    rec.Count := count;
    count +:= 1;
    res := oct.publish("Ping",rec);
    if( is_fail(res) )
      print "publish failed",res;
    msg := oct.receive();
    if( is_fail(msg) )
    {
      print "Receive failed: ",msg;
      run := F;
    }
    else
    {
      print "Received: ",msg;
    }
  }
}

# makes a HIID object from a string
hiid := function(...)
{
  ret := paste(...,sep=".");
  ret::is_hiid := T;
  return ret;
}



#test_octopussy();
#exit 0;
