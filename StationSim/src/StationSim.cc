//# StationSim.cc
//#
//#  Copyright (C) 2002
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
//#

// This is the main program for the Station prototype simulation using the 
// StationSim simulation environment.
// 


#include <Common/Debug.h>
#include <Common/lofar_iostream.h>
#include <BaseSim/Step.h>
#include <BaseSim/WH_Empty.h>
#include <BaseSim/ParamBlock.h>
#include <BaseSim/Simul.h>
#include TRANSPORTERINCLUDE
#ifdef HAVE_CORBA
# include <BaseSim/Corba/TH_Corba.h>
# include <BaseSim/Corba/BS_Corba.h>
#endif
#include <BaseSim/Simul2XML.h>
#include <StationSim/StationSim.h>

// Beamformer includes
#include <StationSim/WH_BeamFormer.h>
#include <StationSim/WH_STA.h>
#include <StationSim/WH_WeightDetermination.h>
#include <StationSim/WH_Projection.h>

#include <StationSim/DataGenConfig.h>
#include <StationSim/WH_AddSignals.h>
#include <StationSim/WH_CreateSource.h>
#include <StationSim/WH_Modulate.h>
#include <StationSim/WH_PhaseShift.h>
#include <StationSim/WH_ReadSignal.h>
#include <StationSim/WH_BandSep.h>
#include <StationSim/WH_DataReader.h>


StationSim::StationSim()
{}


void StationSim::define (const ParamBlock& params)
{
  char suffix[8];
  char suffix2[8];
  char suffix3[8];

#ifdef HAVE_CORBA
  // Start Orb Environment
  AssertStr (BS_Corba::init(),
	     "Could not initialise CORBA environment");
#endif

  int rank = TRANSPORTER::getCurrentRank ();
  unsigned int size = TRANSPORTER::getNumberOfNodes();
  cout << "StationSim Processor " << rank << " of " 
       << size << "  operational." << flush << endl;
  //if (rank == 1) sleep(30);

  // Get the various controls from the ParamBlock.
  const int nrcu                  = params.getInt    ("nrcu", 2);
  const int nsubband              = params.getInt    ("nsubband", 16);
  const int order                 = params.getInt    ("order", 12);
  const int nchannel              = params.getInt    ("nchannel", 16);
  const int nselsubband           = params.getInt    ("nselsubband", 4);
  const string selFileName        = params.getString ("selfilename", "");
  const string coeffFileNameSub   = params.getString ("coefffilenamesub", "");
  const string coeffFileNameChan  = params.getString ("coefffilenamechan", "");
  const string datagenFileName    = params.getString ("datagenfilename", "");
  const string bfDipoleFile       = params.getString ("bffilename","");
  const int fifolength            = params.getInt    ("bf_fifolength", 512);
  const int buflength             = params.getInt    ("bf_bufferlength", 256);
  const int modulationWindowSize  = params.getInt    ("modwindowsize", 32);
  const int nfft_phaseshift       = params.getInt    ("nfftphaseshift", 128);
  const int aweRate               = params.getInt    ("awerate", 1);
  const int nbeam                 = params.getInt    ("nbeam", 1);
  const int maxNtarget            = params.getInt    ("maxntarget", 1);
  const int maxNrfi               = params.getInt    ("maxnrfi", 1);
  const int STArate               = params.getInt    ("starate", 100);
  const int WDrate                = params.getInt    ("wdrate", 10000);
  const int delayMod              = modulationWindowSize - 1;
  const int delayPhase            = nfft_phaseshift - 1;
  const int delaySubFilt          = nrcu * (nsubband - 1);
  const int delayBeamForm         = 1;

  // Read in the configuration for the sources
  DataGenerator* DG_Config = new DataGenerator (datagenFileName);

  // Check
  AssertStr (nrcu == DG_Config->itsArray->size(), "The array configfile doesn't match the simulator input!");

  // Create the overall simul object.
  WH_Empty wh;
  Simul simul(wh, "StationSim");
  setSimul (simul);

  // Create the workholders
  for (int i = 0; i < DG_Config->itsNumberOfSources; ++i) {
    // Create for every signal a read_signal step/workholder
    // The signals are named as follows: signal_ + source number + _ + signal number (e.g. signal_1_3 )
    for (int j = 0; j < DG_Config->itsSources[i]->itsNumberOfSignals; ++j) {
      // create suffix for name
      sprintf (suffix, "%d_%d", i, j);

      // create the read signal steps
      Step signal (WH_ReadSignal ("",
								  1,
								  DG_Config->itsSources[i]->itsSignals[j]->itsInputFileName),
				   string ("signal_") + suffix, 
				   false);
      
      simul.addStep (signal);
      
      // Create the modulation steps
      Step modulate (WH_Modulate (1,
								  1,
								  DG_Config->itsSources[i]->itsSignals[j]->itsModulationType,
								  DG_Config->itsSources[i]->itsSignals[j]->itsCarrierFreq,
								  DG_Config->itsSources[i]->itsSignals[j]->itsSamplingFreq,
								  DG_Config->itsSources[i]->itsSignals[j]->itsOpt,
								  DG_Config->itsSources[i]->itsSignals[j]->itsAmplitude, 
								  modulationWindowSize),
					 string ("modulate_") + suffix, 
					 false);
      
      modulate.getOutData (0).setWriteDelay (delayMod);
      simul.addStep (modulate);
    }

	// Create the source creator steps
    sprintf (suffix, "%d", i);
    Step create_source (WH_CreateSource ("",
										 DG_Config->itsSources[i]->itsNumberOfSignals, 
										 1),
						string ("create_source_") + suffix, 
						false);
    
    for (int j = 0; j < DG_Config->itsSources[i]->itsNumberOfSignals; ++j) {
      create_source.getInData (j).setReadDelay (delayMod);
	}
    create_source.getOutData (0).setWriteDelay (delayMod);
    simul.addStep (create_source);

	// Create the phase shifters
    Step phase_shift (WH_PhaseShift (1,
									 1,
									 nrcu,
									 DG_Config,
									 nfft_phaseshift,
									 i,
									 modulationWindowSize),
					  string ("phase_shift_") + suffix, 
					  false);
    
    phase_shift.getInData (0).setReadDelay (delayMod);
    phase_shift.getOutData (0).setWriteDelay (delayMod + delayPhase);
    simul.addStep (phase_shift);
  }

  // Create the step that adds the signals 
  Step add_signals (WH_AddSignals ("",
								   DG_Config->itsNumberOfSources,
								   nrcu, 
								   nrcu), 
					"add_signals", 
					false); 
  for (int i = 0; i < DG_Config->itsNumberOfSources; ++i) {
    add_signals.getInData (i).setReadDelay (delayMod + delayPhase);
  }
  for (int i = 0; i < nrcu; ++i) {
    add_signals.getOutData (i).setWriteDelay (delayMod + delayPhase);
  }
  simul.addStep (add_signals);

//   // Create the NoEMI data readers
//   for (int i = 0; i < nrcu; ++i) { 
// 	sprintf (suffix, "%d", i);
	  
// 	Step data_reader (WH_DataReader(suffix,
// 									1,
// 									"/dop32_0/alex/files_data/ajb_data/f113ch1.dat"),
// 					  string ("data_reader_") + suffix,
// 					  false);
// 	simul.addStep (data_reader);	
//   }

  // Create the subband filterbank
  int nout = 2;
  for (int i = 0; i < nrcu; ++i) {
	sprintf (suffix, "%d", i);
	  
	Step subband_filter (WH_BandSep(suffix,	nsubband, coeffFileNameSub, nout),
			     string ("subband_filter_") + suffix,
			     false);
	subband_filter.getInData (0).setReadDelay (delayMod + delayPhase);
	for (int j = 0; j < nsubband * nout; ++j) {
	  subband_filter.getOutData (j).setWriteDelay (delaySubFilt);
	}
	subband_filter.setOutRate(nsubband);
	simul.addStep (subband_filter);	
  }

  // Create the beam forming components
  for (int i = 0; i < nsubband; ++i) {
    sprintf (suffix, "%d", i);

	// The beamformer object    
	Step beam (WH_BeamFormer(suffix, nrcu + 1, 1, nrcu, nbeam, maxNtarget, maxNrfi, 
							 STArate), 
			   string("beam_former_") + suffix, false);
    for (int j = 0; j < nrcu; ++j) { 
      beam.getInData (j).setReadDelay (delaySubFilt); 
    }
    for (int j = 0; j < 1; ++j) { 
	  beam.getOutData (j).setWriteDelay (delaySubFilt + delayBeamForm);
	}   
	beam.getInData (nrcu).setReadDelay (delaySubFilt + delayBeamForm);
	beam.setRate(nsubband);
	if (STArate <= WDrate) {
	  beam.setInRate (STArate, nrcu);
	} else {
	  beam.setInRate (WDrate, nrcu);
	}
    simul.addStep (beam);

	
	// The Space Time Analysis object
    Step sta (WH_STA(suffix, nrcu, 2, nrcu, maxNrfi, buflength), 
			  string ("sta_") + suffix, false);

    for (int j = 0; j < nrcu; ++j) {
      sta.getInData (j).setReadDelay (delaySubFilt);
    }
    for (int j = 0; j < 2; ++j) {                // the 1 + 1 is for the #rfi connection
	  sta.getOutData (j).setWriteDelay (delaySubFilt);
	}
	sta.setRate(nsubband);
	sta.setOutRate(nsubband + STArate);
    simul.addStep (sta);

	
	// the Weight Determination Object
    Step weight_det (WH_WeightDetermination(suffix, 0, 1, nrcu, bfDipoleFile, 0.3, 0.3), 
					 string("weight_det_") + suffix, false);   
        
    weight_det.getOutData (0).setWriteDelay (delaySubFilt);
    weight_det.setRate(nsubband + WDrate);
    simul.addStep(weight_det);

// 	// Deterministic nulls should have the same rate as STA !!!!!!!
//     // add a deterministic null
// 	Step det_null (WH_WeightDetermination(suffix, 0, 1, nrcu, bfDipoleFile, 2.583, 0.8238),
// 				   string("det_null_") + suffix, false);

//     det_null.getOutData (0).setWriteDelay (delaySubFilt);
//     det_null.setRate(nsubband + STArate);
//     simul.addStep(det_null);


//     // add a another deterministic null
// 	Step det_null_2 (WH_WeightDetermination(suffix, 0, 1, nrcu, bfDipoleFile, -2.9709, 0.7128),
// 					 string("det_null2_") + suffix, false);

//     det_null_2.getOutData (0).setWriteDelay (delaySubFilt);
//     det_null_2.setRate(nsubband + STArate);
//     simul.addStep(det_null_2);

	
	// the projection object
	int noutProj = 3;
    Step projection (WH_Projection(suffix, noutProj, 1, nrcu, maxNrfi), 
					 string("projection_") + suffix, false);

    for (int j = 0; j < noutProj; ++j) {    // One input + two det null + mdl + eigvectors
      projection.getInData (j).setReadDelay (delaySubFilt);
    }
    for (int j = 0; j < 1; ++j) {                
	  projection.getOutData (j).setWriteDelay (delaySubFilt);
	}
    projection.setRate(nsubband);
	for (int i = 0; i < noutProj - 2; ++i) {
	  projection.setInRate (nsubband + WDrate);
	}
	for (int i = noutProj - 2; i < noutProj; ++i) {
	  projection.setInRate(nsubband + STArate, i);
	}
	if (STArate <= WDrate) {
	  projection.setOutRate (STArate);
	} else {
	  projection.setOutRate (WDrate);
	}
    simul.addStep(projection); 
  }

 
  // Connect the steps.
  for (int i = 0; i < DG_Config->itsNumberOfSources; ++i) {
    sprintf (suffix2, "%d", i);

    // Connect the signals to the modulators
    for (int j = 0; j < DG_Config->itsSources[i]->itsNumberOfSignals; ++j) {
      sprintf (suffix, "%d_%d", i, j);
      sprintf (suffix3, "%d", j);
      simul.connect (string ("signal_") + suffix + string (".out_0"),
					 string ("modulate_") + suffix + string (".in_0"));
    }

    // Connect the modulators to a create_source
    for (int j = 0; j < DG_Config->itsSources[i]->itsNumberOfSignals; ++j) {
      sprintf (suffix, "%d_%d", i, j);
      sprintf (suffix3, "%d", j);
      simul.connect (string ("modulate_") + suffix + string (".out_0"),
					 string ("create_source_") + suffix2 +
					 string (".in_") + suffix3);
    }

    // Connect the create_sources to the phase_shifters
    simul.connect (string ("create_source_") + suffix2 + string (".out_0"),
				   string ("phase_shift_") + suffix2 + string (".in_0"));
  }

  // Connect the phaseshifters to the add signals
  for (int i = 0; i < DG_Config->itsNumberOfSources; ++i) {
    sprintf (suffix, "%d", i);
    simul.connect (string ("phase_shift_") + suffix + string (".out_0"),
				   string ("add_signals.in_") + suffix);
  }
  
  // Connect the add_signals (rcu signals generated by the DG) to the 
  // subband filterbank  
  for (int i = 0; i < nrcu; ++i) { 
	sprintf (suffix, "%d", i);
	simul.connect (string ("add_signals") + string (".out_") +
				   suffix, string ("subband_filter_") + suffix + string (".in"));
  }

  // Connect the Beamformer objects
  for (int s = 0; s < nsubband; ++s) {
    sprintf(suffix2, "%d", s);
    for (int r = 0; r < nrcu; ++r) {
      sprintf(suffix, "%d", r);

      // connect the subband filterbank to the Beamformer
      simul.connect (string ("subband_filter_") + suffix + string (".out0_") + suffix2,
					 string ("beam_former_") + suffix2 + string(".in_") + suffix);

      // connect the subband filterbank to STA
      simul.connect (string ("subband_filter_") + suffix + string (".out1_") + suffix2,
					 string ("sta_") + suffix2 + string (".in_") + suffix);
    }

    // connect Projection to Beamformer
    simul.connect (string ("projection_") + suffix2 + string (".out_0"),
				   string ("beam_former_") + suffix2 + string (".weights"));

    // connect the STA to Projection
	simul.connect (string ("sta_") + suffix2 + string (".out_0"),
				   string ("projection_") + suffix2 + string (".in_rfi"));
	simul.connect (string ("sta_") + suffix2 + string (".out_mdl"), 
				   string ("projection_") + suffix2 + string (".in_mdl"));
  
    // connect the Weight determinator to Projection
    simul.connect (string ("weight_det_") + suffix2 + string (".out_0"),
				   string ("projection_") + suffix2 + string (".in_0"));

// 	// connect the deterministic null to the projection
// 	simul.connect (string ("det_null_") + suffix2 + string (".out_0"),
// 				   string ("projection_") + suffix2 + string(".in_1"));

// 	// connect the deterministic null to the projection
// 	simul.connect (string ("det_null2_") + suffix2 + string (".out_0"),
// 				   string ("projection_") + suffix2 + string(".in_2"));

  }

//   // Connect the data_reader to the subband filterbank  

//   for (int i = 0; i < nrcu; ++i) { 
// 	sprintf (suffix, "%d", i);
// 	simul.connect (string ("data_reader_") + suffix + string (".out_0"), 
// 				   string ("subband_filter_") + suffix + string (".in"));
//   }


  // end of dataprocessor definition
  simul.checkConnections();

  //itsSimul->setLocalComm();

  //////////////////////////////////////////////////////////////////////
  //
  // Finished configuration definition
  // 
  //////////////////////////////////////////////////////////////////////
  cout << "Ready with definition of configuration on node " << rank << endl;  

  Simul2XML xml(simul);
  xml.write("out.xml");
}



void StationSim::run (int nsteps)
{
  if (nsteps < 0) {
    nsteps = 10;
  }

  int rank = TRANSPORTER::getCurrentRank ();
  
  if (rank == 0) cout << endl <<  "Start Process" << endl;    
  for (int i = 1; i <= nsteps; i++) {
    if ((i%1 == 0) && (rank == 0)) { // print a dot after every 1 process steps
      cout << "." << flush;
    }
    getSimul().process ();
  }
  cout << endl << "END OF SIMUL on node " << rank << endl;
  return;
}

void StationSim::dump() const {
  cout << "Dump StationSim" << endl;
  getSimul().dump();
}


void StationSim::quit()
{}



