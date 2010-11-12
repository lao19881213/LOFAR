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
#ifndef RFI_TIME_CONVOLUTION_ACTION
#define RFI_TIME_CONVOLUTION_ACTION

#include <AOFlagger/rfi/strategy/action.h>
#include <AOFlagger/rfi/strategy/actionblock.h>
#include <AOFlagger/rfi/strategy/artifactset.h>

#include <AOFlagger/imaging/uvimager.h>

#include <AOFlagger/rfi/sinusfitter.h>
#include <AOFlagger/rfi/thresholdtools.h>
#include <AOFlagger/rfi/uvprojection.h>

#include <AOFlagger/util/ffttools.h>

namespace rfiStrategy {

	class TimeConvolutionAction : public Action
	{
		public:
			enum Operation { SincOperation, ProjectedSincOperation, ProjectedFTOperation, ExtrapolatedSincOperation, IterativeExtrapolatedSincOperation };
			
			TimeConvolutionAction() : Action(), _operation(IterativeExtrapolatedSincOperation), _sincSize(32.0), _directionRad(M_PI*(-86.7/180.0)), _etaParameter(0.2), _autoAngle(true), _iterations(1)
			{
			}
			virtual std::string Description()
			{
				switch(_operation)
				{
					case SincOperation:
						return "Time sinc convolution";
						break;
					case ProjectedSincOperation:
						return "Projected sinc convolution";
						break;
					case ProjectedFTOperation:
						return "Projected Fourier transform";
						break;
					case ExtrapolatedSincOperation:
						return "Projected extrapolated sinc";
						break;
					case IterativeExtrapolatedSincOperation:
						return "Iterative projected extrapolated sinc";
						break;
					default:
						return "?";
						break;
				}
			}
			virtual ActionType Type() const { return TimeConvolutionActionType; }
			virtual void Perform(ArtifactSet &artifacts, class ProgressListener &listener)
			{
				Image2DCPtr newImage;
				TimeFrequencyData newRevisedData;
				if(_autoAngle)
				{
					_directionRad = FindStrongestSourceAngle(artifacts.ContaminatedData(), artifacts.MetaData());
				}
				switch(_operation)
				{
					case SincOperation:
						newImage = PerformSincOperation(artifacts);
						break;
					case ProjectedSincOperation:
						newImage = PerformProjectedSincOperation(artifacts, listener);
						break;
					case ProjectedFTOperation:
					case ExtrapolatedSincOperation:
					case IterativeExtrapolatedSincOperation:
					{
						TimeFrequencyData data = artifacts.ContaminatedData();
						TimeFrequencyData *realData = data.CreateTFData(TimeFrequencyData::RealPart);
						TimeFrequencyData *imagData = data.CreateTFData(TimeFrequencyData::ImaginaryPart);
						Image2DPtr real = Image2D::CreateCopy(realData->GetSingleImage());
						Image2DPtr imaginary = Image2D::CreateCopy(imagData->GetSingleImage());
						PerformExtrapolatedSincOperation(artifacts, real, imaginary, listener);
						newRevisedData = TimeFrequencyData(data.Polarisation(), real, imaginary);
					}
					break;
				}
				
				if(_operation == SincOperation || _operation == ProjectedSincOperation)
				{
					newRevisedData = TimeFrequencyData(artifacts.ContaminatedData().PhaseRepresentation(), artifacts.ContaminatedData().Polarisation(), newImage);
				}

				newRevisedData.SetMask(artifacts.RevisedData());

				TimeFrequencyData *contaminatedData =
					TimeFrequencyData::CreateTFDataFromDiff(artifacts.ContaminatedData(), newRevisedData);
				contaminatedData->SetMask(artifacts.ContaminatedData());
				artifacts.SetRevisedData(newRevisedData);
				artifacts.SetContaminatedData(*contaminatedData);
				delete contaminatedData;
			}
			
			enum Operation Operation() const { return _operation; }
			void SetOperation(enum Operation operation) { _operation = operation; }

			num_t DirectionRad() const { return _directionRad; }
			void SetDirectionRad(num_t directionRad) { _directionRad = directionRad; }
			
			num_t SincScale() const { return _sincSize; }
			void SetSincScale(num_t sincSize) { _sincSize = sincSize; }

			num_t EtaParameter() const { return _etaParameter; }
			void SetEtaParameter(num_t etaParameter) { _etaParameter = etaParameter; }

			unsigned Iterations() const { return _iterations; }
			void SetIterations(unsigned iterations) { _iterations = iterations; }
private:
			Image2DPtr PerformSincOperation(ArtifactSet &artifacts) const
			{
				num_t sincScale = ActualSincScaleInSamples(artifacts);
				TimeFrequencyData data = artifacts.ContaminatedData();
				Image2DCPtr image = data.GetSingleImage();
				num_t *row = new num_t[image->Width()*3];
				Image2DPtr newImage = Image2D::CreateEmptyImagePtr(image->Width(), image->Height());
				unsigned width = image->Width();
				num_t sign;
				if(IsImaginary(data))
					sign = -1.0;
				else
					sign = 1.0;
				for(unsigned y=0;y<image->Height();++y)
				{
					for(unsigned x=0;x<width;++x) {
						row[x] = sign * image->Value(x, y);
						row[x+width] = image->Value(x, y);
						row[x+2*width] = sign * image->Value(x, y);
					}
					ThresholdTools::OneDimensionalSincConvolution(row, width*3, sincScale);
					for(unsigned x=0;x<width;++x)
						newImage->SetValue(x, y, row[x+width]);
				}
				delete[] row;
				
				return newImage;
			}
			
			Image2DPtr PerformProjectedSincOperation(ArtifactSet &artifacts, class ProgressListener &listener) const
			{
				num_t sincScale = ActualSincScaleInLambda(artifacts);
				TimeFrequencyData data = artifacts.ContaminatedData();
				Image2DCPtr image = data.GetSingleImage();
				Image2DPtr newImage = Image2D::CreateEmptyImagePtr(image->Width(), image->Height());
				TimeFrequencyMetaDataCPtr metaData = artifacts.MetaData();

				bool isImaginary = IsImaginary(data);
				const size_t width = image->Width();
					
				for(size_t y=0;y<image->Height();++y)
				{
					listener.OnProgress(y, image->Height());
					
					numl_t
						*rowValues = new numl_t[width],
						*rowUPositions = new numl_t[width],
						*rowVPositions = new numl_t[width];
					bool
						*rowSignsNegated = new bool[width];

					UVProjection::Project(image, metaData, y, rowValues, rowUPositions, rowVPositions, rowSignsNegated, _directionRad, isImaginary);

					// Perform the convolution
					for(size_t t=0;t<width;++t)
					{
						const numl_t pos = rowUPositions[t];

						numl_t valueSum = 0.0;
						numl_t weightSum = 0.0;
						
						for(size_t x=0;x<width;++x)
						{
							// if this is exactly a point on the u axis, this point is ignored
							// (it would have infinite weight)
							const UVW &uvw = metaData->UVW()[x];
							if(uvw.v != 0.0) 
							{
								//const numl_t weight = fabsnl(uvw.u / uvw.v);
								const numl_t dist = (rowUPositions[x] - pos) / sincScale;
								if(dist!=0.0)
								{
									const numl_t sincValue = sinnl(dist) / dist;
									valueSum += sincValue * rowValues[x];// * weight;
									weightSum += sincValue;
								}
								else
								{
									valueSum += rowValues[x];// * weight;
									weightSum += 1.0;
								}
							}
						}
						const numl_t currentSign = rowSignsNegated[t] ? (-1.0) : 1.0;
						newImage->SetValue(t, y, (num_t) (valueSum * currentSign / weightSum));
					}
					
					delete[] rowValues;
					delete[] rowUPositions;
					delete[] rowVPositions;
					delete[] rowSignsNegated;
				}
				listener.OnProgress(image->Height(), image->Height());
				
				return newImage;
			}
			
			void PerformExtrapolatedSincOperation(ArtifactSet &artifacts, Image2DPtr real, Image2DPtr imaginary, class ProgressListener &listener) const
			{
				TimeFrequencyMetaDataCPtr metaData = artifacts.MetaData();

				const size_t width = real->Width();
				const size_t fourierWidth = width * 2;
					
				numl_t
					*rowRValues = new numl_t[width],
					*rowIValues = new numl_t[width],
					*rowUPositions = new numl_t[width],
					*rowVPositions = new numl_t[width],
					*fourierValuesReal = new numl_t[fourierWidth],
					*fourierValuesImag = new numl_t[fourierWidth];
				bool
					*rowSignsNegated = new bool[width];

				for(size_t y=0;y<real->Height();++y)
				{
					listener.OnProgress(y, real->Height());
					
					UVProjection::Project(real, metaData, y, rowRValues, rowUPositions, rowVPositions, rowSignsNegated, _directionRad, false);
					UVProjection::Project(imaginary, metaData, y, rowIValues, rowUPositions, rowVPositions, rowSignsNegated, _directionRad, true);

					numl_t maxDist = maxUVDistance(metaData->UVW());

					// Find the point closest to v=0
					numl_t vDist = fabsnl(rowVPositions[0]);
					size_t vZeroPos = 0;
					for(unsigned i=1;i<width;++i)
					{
						if(fabsnl(rowVPositions[i]) < vDist)
						{
							vDist = fabsnl(rowVPositions[i]);
							vZeroPos = i;
						}
					}
					std::cout << "U is max at t=" << vZeroPos << " (u=+-" << vDist << ")" << std::endl;

					size_t
						rangeStart = (size_t) roundn(_etaParameter * (num_t) width / 2.0),
						rangeEnd = width - rangeStart;

					// Precalculate sinus values for FT
					numl_t
						**fSinTable = new numl_t*[fourierWidth],
						**fCosTable = new numl_t*[fourierWidth];
					for(size_t xF=0;xF<fourierWidth;++xF)
					{
						fSinTable[xF] = new numl_t[rangeEnd - rangeStart];
						fCosTable[xF] = new numl_t[rangeEnd - rangeStart];
						const numl_t
							fourierPos = (2.0 * (numl_t) xF / fourierWidth - 1.0),
							fourierFactor = -fourierPos * 2.0 * M_PInl * maxDist / width;
						for(size_t tIndex=rangeStart;tIndex<rangeEnd;++tIndex)
						{
							size_t t = (tIndex + width - vZeroPos) % width;
							const numl_t posU = rowUPositions[t];
							fCosTable[xF][tIndex-rangeStart] = cosnl(fourierFactor * posU);
							fSinTable[xF][tIndex-rangeStart] = sinnl(fourierFactor * posU);
						}
					}

					for(unsigned iteration=0;iteration<_iterations;++iteration)
					{
						// Perform a 1d Fourier transform, ignoring eta part of the data
						for(size_t xF=0;xF<fourierWidth;++xF)
						{
							numl_t
								realVal = 0.0,
								imagVal = 0.0,
								weightSum = 0.0;
	
							// compute F(xF) = \int f(x) * exp( -2 * \pi * i * x * xF )
							for(size_t tIndex=rangeStart;tIndex<rangeEnd;++tIndex)
							{
								size_t t = (tIndex + width - vZeroPos) % width;
								if(rowUPositions[t] != 0.0)
								{
									const numl_t weight = 1.0;//fabsnl(rowVPositions[t]/posU);
									const numl_t weightSqrt = 1.0;//sqrtnl(weight);
									realVal += (rowRValues[t] * fCosTable[xF][tIndex-rangeStart] -
									            rowIValues[t] * fSinTable[xF][tIndex-rangeStart]) * weightSqrt;
									imagVal += (rowIValues[t] * fCosTable[xF][tIndex-rangeStart] +
									            rowRValues[t] * fSinTable[xF][tIndex-rangeStart]) * weightSqrt;
									weightSum += weight;
								}
							}
							fourierValuesReal[xF] = realVal / weightSum;
							fourierValuesImag[xF] = imagVal / weightSum;
							if(_operation == ProjectedFTOperation)
							{
								real->SetValue(xF/2, y, (num_t) realVal / weightSum);
								imaginary->SetValue(xF/2, y, (num_t) imagVal / weightSum);
							}
						}
					
						numl_t sincScale = ActualSincScaleInLambda(artifacts);
						numl_t clippingFrequency = 2.0*M_PInl/sincScale;
						long fourierClippingIndex = (long) ((numl_t) fourierWidth * clippingFrequency * maxDist / (2.0 * (numl_t) width));
						if(fourierClippingIndex*2 > (long) width) fourierClippingIndex = width/2;
						if(fourierClippingIndex < 0) fourierClippingIndex = 0;
						size_t
							startXf = fourierWidth/2 - fourierClippingIndex,
							endXf = fourierWidth/2 + fourierClippingIndex;
						if(_operation == ExtrapolatedSincOperation)
						{
							std::cout << "Inv FT, using 0-" << startXf << " and " << endXf << "-" << fourierWidth << std::endl;
							
							for(size_t t=0;t<width;++t)
							{
								const numl_t
									posU = rowUPositions[t],
									posV = rowVPositions[t],
									posFactor = posU * 2.0 * M_PInl * maxDist / width;
								numl_t
									realVal = 0.0,
									imagVal = 0.0;
								bool residual = false;
		
								if(posV != 0.0)
								{
									const numl_t weightSum = 1.0; //(endXf - startXf); // * fabsnl(posV / posU);
									// compute f(x) = \int F(xF) * exp( 2 * \pi * i * x * xF )
									size_t xF, loopEnd;
									if(residual)
									{
										loopEnd = startXf;
										xF = 0;
									}
									else
									{
										loopEnd = endXf;
										xF = startXf;
									}
									while(xF < loopEnd)
									{
										const numl_t
											fourierPosL = (2.0 * (numl_t) xF / fourierWidth - 1.0),
											fourierRealL = fourierValuesReal[xF],
											fourierImagL = fourierValuesImag[xF];
			
										const numl_t
											cosValL = cosnl(fourierPosL * posFactor),
											sinValL = sinnl(fourierPosL * posFactor);
	
										realVal += fourierRealL * cosValL - fourierImagL * sinValL;
										imagVal += fourierImagL * cosValL + fourierRealL * sinValL;
	
										if(residual)
										{
											const numl_t
												fourierPosR = (2.0 * (numl_t) (endXf + xF) / fourierWidth - 1.0),
												fourierRealR = fourierValuesReal[endXf + xF],
												fourierImagR = fourierValuesImag[endXf + xF];
	
											const numl_t
												cosValR = cosnl(fourierPosR * posFactor),
												sinValR = sinnl(fourierPosR * posFactor);
	
											realVal += fourierRealR * cosValR - fourierImagR * sinValR;
											imagVal += fourierImagR * cosValR + fourierRealR * sinValR;
										}
										++xF;
									}
									real->SetValue(t, y, -realVal/weightSum);
									if(rowSignsNegated[t])
										imaginary->SetValue(t, y, imagVal/weightSum);
									else
										imaginary->SetValue(t, y, -imagVal/weightSum);
								}
							}
						} else if(_operation == IterativeExtrapolatedSincOperation) {
							size_t xFRemoval = 0;
							double xFValue = fourierValuesReal[0]*fourierValuesReal[0] + fourierValuesImag[0]*fourierValuesImag[0];
							for(size_t xF=0;xF<startXf;++xF)
							{
								numl_t val = fourierValuesReal[xF]*fourierValuesReal[xF] + fourierValuesImag[xF]*fourierValuesImag[xF];
								if(val > xFValue)
								{
									xFRemoval = xF;
									xFValue = val;
								}
								val = fourierValuesReal[xF+endXf]*fourierValuesReal[xF+endXf] + fourierValuesImag[xF+endXf]*fourierValuesImag[xF+endXf];
								if(val > xFValue)
								{
									xFRemoval = xF+endXf;
									xFValue = val;
								}
							}
							std::cout << "Strongest frequency at xF=" << xFRemoval << ", amp^2=" << xFValue << std::endl;
							for(size_t t=0;t<width;++t)
							{
								const numl_t
									posU = rowUPositions[t],
									posFactor = posU * 2.0 * M_PInl * maxDist / width,
									weightSum = 1.0;
									
								const numl_t
									fourierPos = (2.0 * (numl_t) xFRemoval / fourierWidth - 1.0),
									fourierReal = fourierValuesReal[xFRemoval],
									fourierImag = fourierValuesImag[xFRemoval];
	
								const numl_t
									cosValL = cosnl(fourierPos * posFactor),
									sinValL = sinnl(fourierPos * posFactor);
	
								numl_t realVal = (fourierReal * cosValL - fourierImag * sinValL) * 0.75 / weightSum;
								numl_t imagVal = (fourierImag * cosValL + fourierReal * sinValL) * 0.75 / weightSum;
								
								rowRValues[t] -= realVal;
								rowIValues[t] -= imagVal;
								real->SetValue(t, y, rowRValues[t]);
								if(rowSignsNegated[t])
									imaginary->SetValue(t, y, -rowIValues[t]);
								else
									imaginary->SetValue(t, y, rowIValues[t]);
							}
						}
					}
					for(size_t xF=0;xF<fourierWidth;++xF)
					{
						delete[] fSinTable[xF];
						delete[] fCosTable[xF];
					}
					delete[] fSinTable;
					delete[] fCosTable;
				}
				listener.OnProgress(real->Height(), real->Height());

				delete[] rowRValues;
				delete[] rowIValues;
				delete[] rowUPositions;
				delete[] rowVPositions;
				delete[] rowSignsNegated;
				delete[] fourierValuesReal;
				delete[] fourierValuesImag;
			}

			numl_t maxUVDistance(const std::vector<UVW> &uvw) const
			{
				numl_t maxDist = 0.0;
				for(std::vector<UVW>::const_iterator i=uvw.begin();i!=uvw.end();++i)
				{
					numl_t dist = i->u * i->u + i->v * i->v;
					if(dist > maxDist) maxDist = dist;
				}
				return sqrtnl(maxDist);
			}
			
			numl_t ActualSincScaleInSamples(ArtifactSet &artifacts) const
			{
				return artifacts.ContaminatedData().ImageWidth() * _sincSize / maxUVDistance(artifacts.MetaData()->UVW());
			}

			numl_t ActualSincScaleInLambda(ArtifactSet &) const
			{
				return _sincSize;
			}
			
			numl_t ActualSincScaleAsRaDecDist() const
			{
				return 2.0*M_PInl/_sincSize;
			}
			
			numl_t FindStrongestSourceAngle(TimeFrequencyData &data, TimeFrequencyMetaDataCPtr metaData)
			{
				UVImager imager(2048, 2048);
				imager.Image(data, metaData);
				imager.PerformFFT();
				Image2DPtr image(FFTTools::CreateAbsoluteImage(imager.FTReal(), imager.FTImaginary()));
				long maxX = 0, maxY = 0;
				num_t maxValue = image->Value(maxX, maxY);
				for(unsigned y=0;y<image->Height();++y)
				{
					for(unsigned x=0;x<image->Width();++x)
					{
						if(image->Value(x, y) > maxValue)
						{
							maxValue = image->Value(x, y);
							maxX = x;
							maxY = y;
						}
					}
				}
				maxX = maxX*2-image->Width();
				maxY = image->Height() - maxY*2;
				numl_t angle = SinusFitter::Phase((numl_t) maxX, (numl_t) maxY);
 				std::cout << "Angle: " << angle/M_PInl*180.0 << ",maxX=" << maxX << ",maxY=" << maxY << "\n";
				return angle;
				/*
				image = FFTTools::AngularTransform(image);
				unsigned pixelDist = (unsigned) (ActualSincScaleAsRaDecDist()*image->Height()/2.0);
				std::cout << "Ignoring " << (image->Height()/2-pixelDist) << "-" << (image->Height()/2+pixelDist) << std::endl;
				numl_t highestSum = -1e10;
				size_t highestIndex = 0;
				for(size_t x=0;x<image->Width();++x)
				{
					numl_t sum = 0.0;
					for(size_t y=0;y<pixelDist;++y)
					{
						sum += image->Value(x, y);
						sum += image->Value(x, image->Height() - y - 1);
					}
					if(sum > highestSum)
					{
						highestSum = sum;
						highestIndex = x;
					}
				}
				numl_t angle = (numl_t) highestIndex * M_PInl / image->Width();
				std::cout << "Angle: " << angle/M_PInl*180.0;
				return angle;*/
			}

			bool IsImaginary(const TimeFrequencyData &data) const
			{
				if(data.PhaseRepresentation() == TimeFrequencyData::RealPart)
					return false;
				else if(data.PhaseRepresentation() == TimeFrequencyData::ImaginaryPart)
					return true;
				else
					throw BadUsageException("Data is not real or imaginary");
			}
			
			enum Operation _operation;
			num_t _sincSize, _directionRad, _etaParameter;
			bool _autoAngle;
			unsigned _iterations;
	};

} // namespace

#endif // RFI_TIME_CONVOLUTION_ACTION
