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
#ifndef THRESHOLDTOOLS_H
#define THRESHOLDTOOLS_H

#include "../msio/image2d.h"
#include "../msio/mask2d.h"

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class ThresholdTools{
	public:
		static void MeanAndStdDev(Image2DCPtr image, Mask2DCPtr mask, num_t &mean, num_t &stddev);
		static num_t Mode(Image2DCPtr input, Mask2DCPtr mask);
		static num_t WinsorizedMode(Image2DCPtr image, Mask2DCPtr mask);
		static num_t WinsorizedMode(Image2DCPtr image);
		static void WinsorizedMeanAndStdDev(Image2DCPtr image, Mask2DCPtr mask, num_t &mean, num_t &variance);
		static void WinsorizedMeanAndStdDev(Image2DCPtr image, num_t &mean, num_t &variance);
		static num_t MinValue(Image2DCPtr image, Mask2DCPtr mask);
		static num_t MaxValue(Image2DCPtr image, Mask2DCPtr mask);
		static void SetFlaggedValuesToZero(Image2DPtr dest, Mask2DCPtr mask);
		static void CountMaskLengths(Mask2DCPtr mask, int *lengths, size_t lengthsSize);
		static void OneDimensionalConvolution(num_t *data, unsigned dataSize, const num_t *kernel, unsigned kernelSize);
		static void OneDimensionalGausConvolution(num_t *data, unsigned dataSize, num_t variance);
		static void FilterConnectedSamples(Mask2DPtr mask, size_t minConnectedSampleArea, bool eightConnected=true);
		static void FilterConnectedSample(Mask2DPtr mask, unsigned x, unsigned y, size_t minConnectedSampleArea, bool eightConnected=true);
		static void UnrollPhase(Image2DPtr image);
	private:
		ThresholdTools() { }
};

#endif
