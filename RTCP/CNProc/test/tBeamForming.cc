//#  tTiedArray: test Tied Array mode
//#
//#  Copyright (C) 2008
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <Common/lofar_complex.h>
#include <Common/Timer.h>

#include <CNProc/BeamFormerAsm.h>

#include <spi/UPC.h>

#include <cassert>


using namespace LOFAR;
using namespace LOFAR::RTCP;


#define NR_STATIONS	 36
#define NR_BEAMS	 103
#define NR_POLARIZATIONS 2
#define NR_CHANNELS	 16
#define NR_TIMES	 12288


static fcomplex samples[NR_CHANNELS][NR_STATIONS][NR_TIMES|2][NR_POLARIZATIONS] __attribute__ ((aligned(128)));
static fcomplex weights[NR_STATIONS][NR_BEAMS] __attribute__ ((aligned(128)));
static fcomplex sums[NR_CHANNELS][NR_BEAMS][NR_TIMES|2][NR_POLARIZATIONS] __attribute__ ((aligned(128)));


void init_data()
{
  for (unsigned count = 23, stat = 0; stat < NR_STATIONS; stat ++)
    for (unsigned chan = 0; chan < NR_CHANNELS; chan ++)
      for (unsigned time = 0; time < NR_TIMES; time ++)
	for (unsigned pol = 0; pol < NR_POLARIZATIONS; pol ++, count += 17, count &= 0xFF)
	  samples[chan][stat][time][pol] = makefcomplex(count, count + 9);

  for (unsigned count = 37, stat = 0; stat < NR_STATIONS; stat ++)
    for (unsigned beam = 1; beam < NR_BEAMS; beam ++, count += 51, count &= 0xFF)
      weights[stat][beam] = makefcomplex(count, 2 * count + 5);
}


void test_add2()
{
  NSTimer timer("add2", true);
  timer.start();

  for (unsigned chan = 0; chan < NR_CHANNELS; chan ++)
    _add_2_single_precision_vectors((float *) sums[chan],
				    (const float *) samples[chan][0],
				    (const float *) samples[chan][1],
				    NR_TIMES * NR_POLARIZATIONS * 2);
  timer.stop();

  for (unsigned chan = 0; chan < NR_CHANNELS; chan ++)
    for (unsigned time = 0; time < NR_TIMES; time ++)
      for (unsigned pol = 0; pol < NR_POLARIZATIONS; pol ++)
	assert(sums[chan][0][time][pol] == samples[chan][0][time][pol] +
					   samples[chan][1][time][pol]);
}


void test_add3()
{
  NSTimer timer("add3", true);
  timer.start();

  for (unsigned chan = 0; chan < NR_CHANNELS; chan ++)
    _add_3_single_precision_vectors((float *) sums[chan],
				    (const float *) samples[chan][0],
				    (const float *) samples[chan][1],
				    (const float *) samples[chan][2],
				    NR_TIMES * NR_POLARIZATIONS * 2);
  timer.stop();

  for (unsigned chan = 0; chan < NR_CHANNELS; chan ++)
    for (unsigned time = 0; time < NR_TIMES; time ++)
      for (unsigned pol = 0; pol < NR_POLARIZATIONS; pol ++)
	assert(sums[chan][0][time][pol] == samples[0][chan][time][pol] +
					   samples[1][chan][time][pol] +
					   samples[2][chan][time][pol]);
}


void test_add4()
{
  NSTimer timer("add4", true);
  timer.start();

  for (unsigned chan = 0; chan < NR_CHANNELS; chan ++)
    _add_4_single_precision_vectors((float *) sums[chan],
				    (const float *) samples[chan][0],
				    (const float *) samples[chan][1],
				    (const float *) samples[chan][2],
				    (const float *) samples[chan][3],
				    NR_TIMES * NR_POLARIZATIONS * 2);
  timer.stop();

  for (unsigned chan = 0; chan < NR_CHANNELS; chan ++)
    for (unsigned time = 0; time < NR_TIMES; time ++)
      for (unsigned pol = 0; pol < NR_POLARIZATIONS; pol ++)
	assert(sums[chan][0][time][pol] == samples[0][chan][time][pol] +
					   samples[1][chan][time][pol] +
					   samples[2][chan][time][pol] +
					   samples[3][chan][time][pol]);
}


void test_add6()
{
  memset(sums, 0, sizeof sums);

  NSTimer timer("add6", true);
  timer.start();

  for (unsigned chan = 0; chan < NR_CHANNELS; chan ++)
    _add_6_single_precision_vectors((float *) sums[chan],
				    (const float *) samples[chan][0],
				    (const float *) samples[chan][1],
				    (const float *) samples[chan][2],
				    (const float *) samples[chan][3],
				    (const float *) samples[chan][4],
				    (const float *) samples[chan][5],
				    NR_TIMES * NR_POLARIZATIONS * 2);
  timer.stop();

  for (unsigned chan = 0; chan < NR_CHANNELS; chan ++)
    for (unsigned time = 0; time < NR_TIMES; time ++)
      for (unsigned pol = 0; pol < NR_POLARIZATIONS; pol ++)
	assert(sums[chan][0][time][pol] == samples[0][chan][time][pol] +
					   samples[1][chan][time][pol] +
					   samples[2][chan][time][pol] +
					   samples[3][chan][time][pol] +
					   samples[4][chan][time][pol] +
					   samples[5][chan][time][pol]);
}


void check_beamformed_data(unsigned nr_stations)
{
  for (unsigned beam = 1; beam < NR_BEAMS; beam ++)
    for (unsigned chan = 0; chan < NR_CHANNELS; chan ++)
      for (unsigned time = 0; time < NR_TIMES; time = time == 100 ? NR_TIMES - 100 : time + 1)
	for (unsigned pol = 0; pol < NR_POLARIZATIONS; pol ++) {
	  fcomplex sum = makefcomplex(0, 0);

	  for (unsigned stat = 0; stat < nr_stations; stat ++)
	    sum += weights[stat][beam] * samples[chan][stat][time][pol];

	  if (sums[chan][beam][time][pol] != sum) {
	    std::cout << "sums[" << chan << "][" << beam << "][" << time << "][" << pol << "] = " << sums[chan][beam][time][pol] << ", sum = " << sum << std::endl;
	    abort();
	  }

	  //assert(sums[chan][beam][time][pol] == sum);
	}
}


void test_beamform_3stations_6beams()
{
  memset(sums, 0, sizeof sums);

  NSTimer timer("beamform_3st_6bm", true);
  timer.start();
  for (unsigned chan = 0; chan < NR_CHANNELS; chan ++)
    for (unsigned stat = 0; stat < NR_STATIONS; stat += 3)
      for (unsigned time = 0; time < NR_TIMES; time += 1024)
	for (unsigned beam = 1; beam < NR_BEAMS; beam += 6)
	  _beamform_3stations_6beams(
	    &sums[chan][beam][time][0],
	    (char *) &sums[0][1][0][0] - (char *) &sums[0][0][0][0],
	    &samples[chan][stat][time][0],
	    (char *) &samples[0][1][0][0] - (char *) &samples[0][0][0][0],
	    &weights[stat][beam],
	    (char *) &weights[1][0] - (char *) &weights[0][0],
	    1024,
	    stat == 0
	  );
  timer.stop();

  check_beamformed_data(NR_STATIONS / 3 * 3);
}


void test_beamform_3_beams(unsigned nr_stations_at_once)
{
  unsigned nr_stations = NR_STATIONS / nr_stations_at_once * nr_stations_at_once;

  std::cout << "beam forming " << nr_stations << " stations in groups of " << nr_stations_at_once << std::endl;
  NSTimer timer("beamform_3bm", true);
  timer.start();
  for (unsigned chan = 0; chan < NR_CHANNELS; chan ++)
    for (unsigned stat = 0; stat + nr_stations_at_once <= NR_STATIONS; stat += nr_stations_at_once)
      for (unsigned time = 0; time < NR_TIMES; time += 96)
	for (unsigned beam = 1; beam < NR_BEAMS; beam += 3)
	  _beamform_up_to_6_stations_and_3_beams(
	    &sums[chan][beam][time][0],
	    (char *) &sums[0][1][0][0] - (char *) &sums[0][0][0][0],
	    &samples[chan][stat][time][0],
	    (char *) &samples[0][1][0][0] - (char *) &samples[0][0][0][0],
	    &weights[stat][beam],
	    (char *) &weights[1][0] - (char *) &weights[0][0],
	    96,
	    stat == 0,
	    nr_stations_at_once
	  );
  timer.stop();

  check_beamformed_data(NR_STATIONS / nr_stations_at_once * nr_stations_at_once);
}


int main()
{
  BGP_UPC_Initialize();
  BGP_UPC_Initialize_Counter_Config(BGP_UPC_MODE_0, BGP_UPC_CFG_EDGE_DEFAULT);

  init_data();
  test_add2();
  test_add3();
  test_add4();
  test_add6();
  //test_beamform_3stations_6beams();

  for (unsigned nr_stations_at_once = 4; nr_stations_at_once <= 6; ++ nr_stations_at_once)
    test_beamform_3_beams(nr_stations_at_once);

#if 0
  BGP_UPC_Start(0);

  NSTimer timer("TAB", true);
  timer.start();

  timer.stop();

  BGP_UPC_Stop();
  BGP_UPC_Print_Counter_Values(BGP_UPC_READ_EXCLUSIVE);

  std::cout << "nr ops = " << ((unsigned long long) NR_CHANNELS * NR_TIMES * NR_STATIONS * (NR_BEAMS - 1) * NR_POLARIZATIONS * 8) << std::endl;
#endif

  return 0;
}
