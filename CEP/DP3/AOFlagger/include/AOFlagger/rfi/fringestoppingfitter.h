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
#ifndef FRINGESTOPPINGFITTER_H
#define FRINGESTOPPINGFITTER_H

#include <vector>

#include "image.h"

#include "../msio/samplerow.h"
#include "../msio/timefrequencymetadata.h"

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class FringeStoppingFitter : public SurfaceFitMethod {
	public:
		FringeStoppingFitter();
		virtual ~FringeStoppingFitter();

		virtual void SetMetaData(TimeFrequencyMetaDataCPtr metaData)
		{
			_metaData = metaData;
			_fieldInfo = &metaData->Field();
			_bandInfo = &metaData->Band();
			_antenna1Info = &metaData->Antenna1();
			_antenna2Info = &metaData->Antenna2();
			_observationTimes = &metaData->ObservationTimes();
		}
		virtual void Initialize(const TimeFrequencyData &input) throw()
		{
			_originalData=&input;
			_realBackground =
				Image2D::CreateZeroImagePtr(_originalData->ImageWidth(), _originalData->ImageHeight());
			_imaginaryBackground =
				Image2D::CreateZeroImagePtr(_originalData->ImageWidth(), _originalData->ImageHeight());
			_originalMask =
				input.GetSingleMask();
		}
		virtual unsigned int TaskCount()
		{
			return _fringeFit ? _originalData->ImageHeight() : _originalData->ImageWidth();
		}
		virtual void PerformFit(unsigned taskNumber);
		virtual void PerformFitOnOneChannel(unsigned y);
		void PerformFringeStop();
		virtual class TimeFrequencyData Background()
		{
			return TimeFrequencyData(SinglePolarisation, _realBackground, _imaginaryBackground);
		}
		virtual enum TimeFrequencyData::PhaseRepresentation PhaseRepresentation() const
		{
			return TimeFrequencyData::ComplexRepresentation;
		}
		void SetFringesToConsider(long double fringesToConsider)
		{
			_fringesToConsider = fringesToConsider;
		}
		void SetMaxWindowSize(size_t maxWindowSize)
		{
			_maxWindowSize = maxWindowSize;
		}
		void SetFitChannelsIndividually(bool fitChannelsIndividually)
		{
			_fitChannelsIndividually = fitChannelsIndividually;
		}
		void SetReturnFittedValue(bool returnFittedValue)
		{
			_returnFittedValue = returnFittedValue;
		}
		void SetReturnMeanValue(bool returnMeanValue)
		{
			_returnMeanValue = returnMeanValue;
		}
		void PerformRFIFit();
		void PerformRFIFitOnOneChannel(unsigned y);
		void PerformRFIFitOnOneChannel(unsigned y, unsigned windowSize);
		void PerformRFIFit(unsigned yStart, unsigned yEnd);
		void PerformRFIFit(unsigned yStart, unsigned yEnd, unsigned windowSize);
		num_t GetAmplitude(unsigned yStart, unsigned yEnd);
	private:
		num_t CalculateFitValue(const Image2D &image, size_t y);
		inline num_t CalculateMaskedAverage(const Image2D &image, size_t x, size_t yFrom, size_t yLength);
		inline num_t CalculateUnmaskedAverage(const Image2D &image, size_t x, size_t yFrom, size_t yLength);
		void CalculateFitValue(const Image2D &real, const Image2D &imaginary, size_t x, size_t yFrom, size_t yLength,num_t  &rValue, num_t &iValue);
		num_t GetFringeFrequency(size_t x, size_t y);

		void GetRFIValue(num_t &r, num_t &i, int x, int y, const class Baseline &baseline, num_t rfiPhase, num_t rfiStrength);
		num_t GetRFIFitError(SampleRowCPtr real, SampleRowCPtr imaginary, int xStart, int xEnd, int y, num_t rfiPhase, num_t rfiStrength);
		num_t GetRowVariance(SampleRowCPtr real, SampleRowCPtr imaginary, int xStart, int xEnd);
//		long double MinimizeRFIPhase(SampleRowCPtr real, SampleRowCPtr imaginary, int xStart, int xEnd, int y);
//		long double MinimizeRFIStrength(SampleRowCPtr real, SampleRowCPtr imaginary, int xStart, int xEnd, int y, long double phase);
		void MinimizeRFIFitError(num_t &phase, num_t &amplitude, SampleRowCPtr real, SampleRowCPtr imaginary, unsigned xStart, unsigned xEnd, unsigned y) const throw();
		
		void PerformRFIFitOnOneRow(SampleRowCPtr real, SampleRowCPtr imaginary, unsigned y);
		void PerformRFIFitOnOneRow(SampleRowCPtr real, SampleRowCPtr imaginary, unsigned y, unsigned windowSize);

		Mask2DCPtr _originalMask;
		const class TimeFrequencyData *_originalData;

		Image2DPtr _realBackground, _imaginaryBackground;

		TimeFrequencyMetaDataCPtr _metaData;
		const class FieldInfo *_fieldInfo;
		const class BandInfo *_bandInfo;
		const class AntennaInfo *_antenna1Info, *_antenna2Info;
		const std::vector<double> *_observationTimes;
		num_t _fringesToConsider;
		size_t _minWindowSize, _maxWindowSize;
		bool _fitChannelsIndividually;
		bool _returnFittedValue, _returnMeanValue;
		bool _fringeFit;
};

#endif
