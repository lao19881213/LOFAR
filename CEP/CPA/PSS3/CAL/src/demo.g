include 'table.g'
include 'meqcalibrater.g'
include 'parmtable.g'
include 'gsm.g'
include 'imgannotator.g'
include 'mkimg.g'

#
# Demo function showing the predict functionality and creating an image of it.
#
predict := function(fname='michiel.demo', ant=4*[0:20])
{
    local mc := meqcalibrater(spaste(fname,'.MS'), 
                              fname, fname, ant=ant);
    if (is_fail(mc)) {
        print "meqcalibratertest(): could not instantiate meqcalibrater";
        fail;
    }
    
    mc.settimeinterval(3600); # calibrate per 1 hour
    mc.clearsolvableparms();
    
    mc.resetiterator()
    while (mc.nextinterval())
    {
        d := mc.getsolvedomain();
        print 'solvedomain = ', d;
        
        mc.predict('MODEL_DATA');
    }

    mc.done();

    mssel := '';
    if (len(ant) > 0) {
      ant +:=1;               # msselect adds 1 to ANTENNA1,2
      mssel := spaste('all([ANTENNA1,ANTENNA2] in ',substitutevar(ant), ')');
    }
    print mssel
    mkimg(spaste(fname, '.MS'), spaste(fname, '.img'), msselect=mssel);

    return T;
}

solve := function(fname='michiel.demo', ant=4*[0:20], niter=1)
{
    annotator := imgannotator(spaste(fname, '.img'), 'raster');
    
    mc := meqcalibrater(spaste(fname,'.MS'), fname, fname, ant=ant);

    if (is_fail(mc)) {
        print "meqcalibratertest(): could not instantiate meqcalibrater";
        fail;
    }

    mc.select('', 5, 5);

    # Plot initial position
    parms := mc.getparms("GSM.*.RA GSM.*.DEC GSM.*.I");
    print parms
    print len(parms)
    nrpos := len(parms) / 3;
    if (nrpos > 0) {
        for (i in [1:nrpos]) {
            ra      := parms[spaste('GSM.',i,'.RA')].value[1];
            dec     := parms[spaste('GSM.',i,'.DEC')].value[1];
            stokesI := parms[spaste('GSM.',i,'.I')].value[1];
            print 'src = ', i, ' ra = ', ra, ' dec = ', dec, ' I = ', stokesI;

	    if (is_fail(annotator)) fail;
            annotator.add_marker(i, real(ra), real(dec), i==nrpos);
        }
    }

    mc.settimeinterval(3600); # calibrate per 3600 seconds
    mc.clearsolvableparms();
    mc.setsolvableparms("GSM.*.I");
    #mc.setsolvableparms("GSM.*.RA GSM.*.DEC");
    mc.setsolvableparms("GSM.*.DEC GSM.*.RA");
    #mc.setsolvableparms("Leakage.{11,22}.*");

    solverec := [=];

    mc.resetiterator()
    while (mc.nextinterval())
    {
        d := mc.getsolvedomain();
        print 'solvedomain = ', d;
        
        for (i in [1:niter]) {
            print "iteration", i;
            srec := mc.solve('MODEL_DATA');
	    solverec[spaste("iter",i)] := srec;
            
            parms := mc.getparms("GSM.*.RA GSM.*.DEC GSM.*.I");
            nrpos := len(parms) / 3;
            if (nrpos > 0) {
                for (j in [1:nrpos]) {
                    ra  := parms[spaste('GSM.',j,'.RA')].value[1];
                    dec := parms[spaste('GSM.',j,'.DEC')].value[1];
                    stokesI := parms[spaste('GSM.',j,'.I')].value[1];
                    print 'src = ', j, ' ra = ', ra, ' dec = ', dec, ' I = ', stokesI;
                    
                    if (is_fail(annotator)) fail;
                    annotator.add_marker(j, real(ra), real(dec), j==nrpos);
                }
            }
        }
        print mc.getstatistics()
        }
    
    mc.done();

    return solverec;    
#    return ref annotator;
}

solvepos := function(fname='michiel.demo', ant=4*[0:20], niter=1)
{
    annotator := imgannotator(spaste(fname, '.img'), 'raster');
	
    mc := meqcalibrater(spaste(fname,'.MS'), fname, fname, ant=ant);
    if (is_fail(mc)) {
        print "meqcalibratertest(): could not instantiate meqcalibrater";
        fail;
    }
    
    # Plot initial position
    parms := mc.getparms("GSM.*.RA GSM.*.DEC");
    nrpos := len(parms) / 2;
    if (nrpos > 0) {
        for (i in [1:nrpos]) {
            ra  := parms[spaste('GSM.',i,'.RA')].value[1];
            dec := parms[spaste('GSM.',i,'.DEC')].value[1];
            print 'src = ', i, ' ra = ', ra, ' dec = ', dec;

	    if (is_fail(annotator)) fail;
            annotator.add_marker(i, real(ra), real(dec), i==nrpos);
        }
    }

    mc.settimeinterval(3600); # calibrate per 3600 seconds (1 timeslot = 2 sec)
    for (i in [1:niter]) {
        print "iteration", i
        mc.clearsolvableparms();
        print 'Solving for RA ...';
        mc.setsolvableparms("GSM.*.RA");
        
        mc.resetiterator();
        while (mc.nextinterval())
        {
            mc.solve('MODEL_DATA');
        }
        
        mc.clearsolvableparms();
        print 'Solving for DEC ...';
        mc.setsolvableparms("GSM.*.DEC");
        
        mc.resetiterator();
        while (mc.nextinterval())
        {
            mc.solve('MODEL_DATA');
            
            parms := mc.getparms("GSM.*.RA GSM.*.DEC");
            nrpos := len(parms) / 2;
            if (nrpos > 0) {
                for (j in [1:nrpos]) {
                    ra  := parms[spaste('GSM.',j,'.RA')].value[1];
                    dec := parms[spaste('GSM.',j,'.DEC')].value[1];
                    print 'src =', j, ' ra =', ra, ' dec =', dec;
                    
                    if (is_fail(annotator)) fail;
                    annotator.add_marker(j, real(ra), real(dec), j==nrpos);
                }
            }
        }
    }
    
    mc.done();
    
    return ref annotator;
}

solvegain := function(fname='michiel.demo', ant=4*[0:20], niter=1)
{
    mc := meqcalibrater(spaste(fname,'.MS'), fname, fname, ant=ant);
    if (is_fail(mc)) {
	print "meqcalibratertest(): could not instantiate meqcalibrater"
	fail
    }

    mc.select('', 0, 0);

    mc.settimeinterval(3600); # calibrate per 1 hour
    mc.clearsolvableparms();

    mc.setsolvableparms("gain.*");

#			"gain.22.ST_{0,1,2,3,4,5,6,7,8,9,10}");

#    parms := mc.getparmnames("*");
#    print 'ALL PARMS = ', parms;

    mc.resetiterator()
    while (mc.nextinterval())
    {
	d := mc.getsolvedomain();
	print 'solvedomain = ', d;

	for (i in [1:niter])
	{
	    print "iteration", i
	    mc.solve('MODEL_DATA');

	    parms := mc.getparms("gain.11.*");
	    print 'GAIN SOLUTION = ';
	    for (k in [1:5])
	    {
		print parms[k].value[1];
	    }

	    parms := mc.getparms("gain.22.*");
	    print 'GAIN SOLUTION = ';
	    for (k in [1:5])
	    {
		print parms[k].value[1];
	    }
        }
	print mc.getstatistics()
    }

    parms := mc.getparms("gain.11.*");
    print 'SOLUTION 11: ', parms;
    parms := mc.getparms("gain.22.*");
    print 'SOLUTION 22: ', parms;

    mc.done();
}

initparms := function(fname='michiel.demo')
{
  pt := parmtable(spaste(fname,'.MEP'), T);
  if (is_fail(pt)) fail;
  pt.putinit ('frot', 0);
  pt.putinit ('drot', 0);
  pt.putinit ('dell', 0);
  pt.putinit ('gain.11', 1);
  pt.putinit ('gain.22', 0);
  pt.done();
}

setparms := function(fname='michiel.demo')
{
  pt := parmtable(spaste(fname,'.MEP'), T);
  if (is_fail(pt)) fail;
  pt.putinit ('frot', 0);
  pt.putinit ('drot', 0);
  pt.putinit ('dell', 0);
  pt.putinit ('gain.11', 1);
  pt.putinit ('gain.22', 0);
  pt.done();
}

initgsm := function(fname='michiel.demo')
{
  tg := gsm(spaste(fname,'.GSM'), T);
  if (is_fail(tg)) fail;
  tg.addpointsource ('src0', [0,1e20], [0,1e20],
		     2.734, 0.45379, 1, 0, 0, 0);
  tg.addpointsource ('src1', [0,1e20], [0,1e20],
		     2.73402, 0.45369, 0.5, 0, 0, 0);
  tg.addpointsource ('src2', [0,1e20], [0,1e20],
		     2.73398, 0.45375, 0.3, 0, 0, 0);
  tg.done()
}

setgsm := function(fname='michiel.demo')
{
  tg := gsm(spaste(fname,'.GSM'), T);
  if (is_fail(tg)) fail;
  tg.addpointsource ('src0', [0,1e20], [0,1e20],
		     2.734030, 0.453785, 1, 0, 0, 0);
  tg.addpointsource ('src1', [0,1e20], [0,1e20],
		     2.734025, 0.4536875, 1, 0, 0, 0);
  tg.addpointsource ('src2', [0,1e20], [0,1e20],
		     2.73399, 0.4537525, 1, 0, 0, 0);
  tg.done();
}

initchan := function(fname='michiel.demo')
{
  t := table(spaste(fname, '.MS/SPECTRAL_WINDOW'),readonly=F)
  if (is_fail(t)) fail;
  t.putcell ('CHAN_WIDTH', 1, array(1e7,1));
  t.close()
}

setchan := function(fname='michiel.demo')
{
  t := table(spaste(fname, '.MS/SPECTRAL_WINDOW'),readonly=F)
  if (is_fail(t)) fail;
  t.putcell ('CHAN_WIDTH', 1, array(1e6,1));
  t.close()
}

init := function(fname='michiel.demo')
{
	initparms(fname=fname);
	initgsm(fname=fname);
	predict(fname=fname);
}

demo := function(solver=0,niter=10,fname='michiel.demo')
{
	global annotator;

	setparms(fname);
	setgsm(fname);
	if (0 == solver)
	{
		annotator:=solve(fname=fname, niter=niter);
	}
	else
	{
		annotator:=solvepos(fname=fname, niter=niter);
	}
}

demogain := function(niter=10,fname='michiel.demo')
{
	setparms(fname);
	initgsm(fname);

	solvegain(niter=niter,fname=fname);
}


