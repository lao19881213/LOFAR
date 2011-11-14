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
#ifndef STATISTICS_COLLECTION_H
#define STATISTICS_COLLECTION_H

#include <stdint.h>

#include <AOFlagger/util/serializable.h>

#include "baselinestatisticsmap.h"
#include "defaultstatistics.h"
#include "qualitytablesformatter.h"
#include "statisticalvalue.h"

class StatisticsCollection : public Serializable
{
	public:
		StatisticsCollection() : _polarizationCount(0)
		{
		}
		
		explicit StatisticsCollection(unsigned polarizationCount) : _polarizationCount(polarizationCount)
		{
		}
		
		StatisticsCollection(const StatisticsCollection &source) :
			_timeStatistics(source._timeStatistics),
			_frequencyStatistics(source._frequencyStatistics),
			_baselineStatistics(source._baselineStatistics),
			_polarizationCount(source._polarizationCount)
		{
		}
		
		void Clear()
		{
			_timeStatistics.clear();
			_frequencyStatistics.clear();
			_baselineStatistics.clear();
		}
		
		void InitializeBand(unsigned band, const double *frequencies, unsigned channelCount)
		{
			std::vector<DefaultStatistics *> pointers;
			for(unsigned i=0;i<channelCount;++i)
			{
				pointers.push_back(&getFrequencyStatistic(frequencies[i]));
			}
			_bands.insert(std::pair<unsigned, std::vector<DefaultStatistics *> >(band, pointers));
			double centralFrequency = (frequencies[0] + frequencies[channelCount-1]) / 2.0;
			_centralFrequencies.insert(std::pair<unsigned, double>(band, centralFrequency));
		}
		
		void Add(unsigned antenna1, unsigned antenna2, double time, unsigned band, int polarization, const std::vector<std::complex<float> > &samples, const bool *isRFI)
		{
			if(samples.empty()) return;
			
			const double centralFrequency = _centralFrequencies.find(band)->second;
			
			addTimeAndBaseline(antenna1, antenna2, time, centralFrequency, polarization, samples, isRFI, false);
			if(antenna1 != antenna2)
				addFrequency(band, polarization, samples, isRFI, false, false);
			
			std::vector<std::complex<float> > diffSamples;
			
			for(std::vector<std::complex<float> >::const_iterator i=samples.begin();i+1!=samples.end();++i)
			{
				diffSamples.push_back((*(i+1) - *i) * 0.5f);
			}
			bool *diffRFIFlags = new bool[samples.size()];
			for(unsigned i=0;i<samples.size()-1;++i)
			{
				diffRFIFlags[i] = isRFI[i] | isRFI[i+1];
			}
			addTimeAndBaseline(antenna1, antenna2, time, centralFrequency, polarization, diffSamples, diffRFIFlags, true);
			if(antenna1 != antenna2)
			{
				addFrequency(band, polarization, diffSamples, diffRFIFlags, true, false);
				addFrequency(band, polarization, diffSamples, diffRFIFlags, true, true);
			}
			delete[] diffRFIFlags;
		}
		
		void Save(QualityTablesFormatter &qualityData) const
		{
			saveTime(qualityData);
			saveFrequency(qualityData);
			saveBaseline(qualityData);
		}
		
		void Load(QualityTablesFormatter &qualityData)
		{
			loadTime<false>(qualityData);
			loadFrequency<false>(qualityData);
			loadBaseline<false>(qualityData);
		}
		
		void Add(QualityTablesFormatter &qualityData)
		{
			loadTime<true>(qualityData);
			loadFrequency<true>(qualityData);
			loadBaseline<true>(qualityData);
		}
		
		void Add(const StatisticsCollection &collection)
		{
			addTime(collection);
			addFrequency(collection);
			addBaseline(collection);
		}
		
		void GetGlobalTimeStatistics(DefaultStatistics &statistics)
		{
			statistics = getGlobalStatistics(_timeStatistics);
		}
		
		void GetGlobalFrequencyStatistics(DefaultStatistics &statistics)
		{
			statistics = getGlobalStatistics(_frequencyStatistics);
		}
		
		void GetGlobalAutoBaselineStatistics(DefaultStatistics &statistics)
		{
			statistics = getGlobalBaselineStatistics<true>();
		}
		
		void GetGlobalCrossBaselineStatistics(DefaultStatistics &statistics)
		{
			statistics = getGlobalBaselineStatistics<false>();
		}
		
		const BaselineStatisticsMap &BaselineStatistics() const
		{
			if(_baselineStatistics.size() == 1)
				return _baselineStatistics.begin()->second;
			else
				throw std::runtime_error("Requesting single band single baseline statistics in statistics collection with multiple bands");
		}
		
		const std::map<double, DefaultStatistics> &TimeStatistics() const
		{
			if(_timeStatistics.size() == 1)
				return _timeStatistics.begin()->second;
			else
				throw std::runtime_error("Requesting single band single timestep statistics in statistics collection with multiple bands");
		}
		
		const std::map<double, DefaultStatistics> &FrequencyStatistics() const
		{
			return _frequencyStatistics;
		}
		
		unsigned PolarizationCount() const
		{
			return _polarizationCount;
		}
		
		void SetPolarizationCount(unsigned newCount)
		{
			_polarizationCount = newCount;
		}
		
		virtual void Serialize(std::ostream &stream) const
		{
			SerializeToUInt64(stream, _polarizationCount);
			serializeTime(stream);
			serializeFrequency(stream);
			serializeBaselines(stream);
		}
		
		virtual void Unserialize(std::istream &stream)
		{
			_polarizationCount = UnserializeUInt64(stream);
			unserializeTime(stream);
			unserializeFrequency(stream);
			unserializeBaselines(stream);
		}
	private:
		struct StatisticSaver
		{
			QualityTablesFormatter::StatisticDimension dimension;
			double time;
			double frequency;
			unsigned antenna1;
			unsigned antenna2;
			QualityTablesFormatter *qualityData;
			
			void Save(StatisticalValue &value, unsigned kindIndex)
			{
				value.SetKindIndex(kindIndex);
				switch(dimension)
				{
					case QualityTablesFormatter::TimeDimension:
						qualityData->StoreTimeValue(time, frequency, value);
						break;
					case QualityTablesFormatter::FrequencyDimension:
						qualityData->StoreFrequencyValue(frequency, value);
						break;
					case QualityTablesFormatter::BaselineDimension:
						qualityData->StoreBaselineValue(antenna1, antenna2, frequency, value);
						break;
					case QualityTablesFormatter::BaselineTimeDimension:
						qualityData->StoreBaselineTimeValue(antenna1, antenna2, time, frequency, value);
						break;
				}
			}
		};
		
		struct Indices
		{
			unsigned kindRFIRatio;
			unsigned kindCount;
			unsigned kindMean;
			unsigned kindSumP2;
			unsigned kindDCount;
			unsigned kindDMean;
			unsigned kindDSumP2;
			
			void fill(QualityTablesFormatter &qd)
			{
				kindRFIRatio = qd.StoreOrQueryKindIndex(QualityTablesFormatter::RFIRatioStatistic),
				kindCount = qd.StoreOrQueryKindIndex(QualityTablesFormatter::CountStatistic),
				kindMean = qd.StoreOrQueryKindIndex(QualityTablesFormatter::MeanStatistic),
				kindSumP2 = qd.StoreOrQueryKindIndex(QualityTablesFormatter::SumP2Statistic),
				kindDCount = qd.StoreOrQueryKindIndex(QualityTablesFormatter::DCountStatistic),
				kindDMean = qd.StoreOrQueryKindIndex(QualityTablesFormatter::DMeanStatistic),
				kindDSumP2 = qd.StoreOrQueryKindIndex(QualityTablesFormatter::DSumP2Statistic);
			}
		};
		
		StatisticsCollection & operator=(const StatisticsCollection &source) // don't allow assignment
		{
			return *this;
		}

		void addTimeAndBaseline(unsigned antenna1, unsigned antenna2, double time, double centralFrequency, int polarization, const std::vector<std::complex<float> > samples, const bool *isRFI, bool isDiff)
		{
			unsigned long rfiCount = 0;
			unsigned long count = 0;
			long double sum_R = 0.0, sum_I = 0.0;
			long double sumP2_R = 0.0, sumP2_I = 0.0;
			for(unsigned i=0;i<samples.size();++i)
			{
				if(std::isfinite(samples[i].real()) && std::isfinite(samples[i].imag()))
				{
					if(isRFI[i])
					{
						++rfiCount;
					} else {
						const long double rVal = samples[i].real();
						const long double iVal = samples[i].imag();
						++count;
						sum_R += rVal;
						sum_I += iVal;
						sumP2_R += rVal*rVal;
						sumP2_I += iVal*iVal;
					}
				}
			}
			
			if(antenna1 != antenna2)
			{
				DefaultStatistics &timeStat = getTimeStatistic(time, centralFrequency);
				addToStatistic(timeStat, polarization, count, sum_R, sum_I, sumP2_R, sumP2_I, rfiCount, isDiff);
			}
			DefaultStatistics &baselineStat = getBaselineStatistic(antenna1, antenna2, centralFrequency);
			addToStatistic(baselineStat, polarization, count, sum_R, sum_I, sumP2_R, sumP2_I, rfiCount, isDiff);
		}
		
		void addToStatistic(DefaultStatistics &statistic, unsigned polarization, unsigned long count, long double sum_R, long double sum_I, long double sumP2_R, long double sumP2_I, unsigned long rfiCount, bool isDiff)
		{
			if(isDiff)
			{
				statistic.dCount[polarization] += count;
				statistic.dSum[polarization] += std::complex<long double>(sum_R, sum_I);
				statistic.dSumP2[polarization] += std::complex<long double>(sumP2_R, sumP2_I);
			} else {
				statistic.count[polarization] += count;
				statistic.sum[polarization] += std::complex<long double>(sum_R, sum_I);
				statistic.sumP2[polarization] += std::complex<long double>(sumP2_R, sumP2_I);
				statistic.rfiCount[polarization] += rfiCount;
			}
		}
		
		void addFrequency(unsigned band, int polarization, const std::vector<std::complex<float> > samples, const bool *isRFI, bool isDiff, bool shiftOneUp)
		{
			std::vector<DefaultStatistics *> &bandStats = _bands.find(band)->second;
			const unsigned fAdd = shiftOneUp ? 1 : 0;
			for(unsigned f=0;f<samples.size();++f)
			{
				if(std::isfinite(samples[f].real()) && std::isfinite(samples[f].imag()))
				{
					DefaultStatistics &freqStat = *bandStats[f + fAdd];
					if(isRFI[f])
					{
						addToStatistic(freqStat, polarization, 0, 0.0, 0.0, 0.0, 0.0, 1, isDiff);
					} else {
						const long double r = samples[f].real(), i = samples[f].imag();
						addToStatistic(freqStat, polarization, 1, r, i, r*r, i*i, 0, isDiff);
					}
				}
			}
		}
		
		void initializeEmptyStatistics(QualityTablesFormatter &qualityData, QualityTablesFormatter::StatisticDimension dimension) const
		{
			qualityData.InitializeEmptyStatistic(dimension, QualityTablesFormatter::RFIRatioStatistic);
			qualityData.InitializeEmptyStatistic(dimension, QualityTablesFormatter::CountStatistic);
			qualityData.InitializeEmptyStatistic(dimension, QualityTablesFormatter::MeanStatistic);
			qualityData.InitializeEmptyStatistic(dimension, QualityTablesFormatter::SumP2Statistic);
			qualityData.InitializeEmptyStatistic(dimension, QualityTablesFormatter::DCountStatistic);
			qualityData.InitializeEmptyStatistic(dimension, QualityTablesFormatter::DMeanStatistic);
			qualityData.InitializeEmptyStatistic(dimension, QualityTablesFormatter::DSumP2Statistic);
		}
		
		void saveEachStatistic(StatisticSaver &saver, const DefaultStatistics &stat, const Indices &indices) const
		{
			StatisticalValue value(_polarizationCount);
			
			for(unsigned p=0;p<_polarizationCount;++p)
				value.SetValue(p, std::complex<float>((double) stat.rfiCount[p] / (double) (stat.rfiCount[p] + stat.count[p]), 0.0f));
			saver.Save(value, indices.kindRFIRatio);
			
			for(unsigned p=0;p<_polarizationCount;++p)
				value.SetValue(p, std::complex<float>(stat.count[p], 0.0f));
			saver.Save(value, indices.kindCount);

			for(unsigned p=0;p<_polarizationCount;++p)
				value.SetValue(p, stat.Mean<float>(p));
			saver.Save(value, indices.kindMean);

			for(unsigned p=0;p<_polarizationCount;++p)
				value.SetValue(p, stat.SumP2<float>(p));
			saver.Save(value, indices.kindSumP2);
			
			for(unsigned p=0;p<_polarizationCount;++p)
				value.SetValue(p, std::complex<float>(stat.dCount[p], 0.0f));
			saver.Save(value, indices.kindDCount);
			
			for(unsigned p=0;p<_polarizationCount;++p)
				value.SetValue(p, stat.DMean<float>(p));
			saver.Save(value, indices.kindDMean);

			for(unsigned p=0;p<_polarizationCount;++p)
				value.SetValue(p, stat.DSumP2<float>(p));
			saver.Save(value, indices.kindDSumP2);
		}
		
		void saveTime(QualityTablesFormatter &qd) const
		{
			initializeEmptyStatistics(qd, QualityTablesFormatter::TimeDimension);
			
			Indices indices;
			indices.fill(qd);
				
			StatisticSaver saver;
			saver.dimension = QualityTablesFormatter::TimeDimension;
			saver.qualityData = &qd;
			
			for(std::map<double, DoubleStatMap>::const_iterator j=_timeStatistics.begin();j!=_timeStatistics.end();++j)
			{
				saver.frequency = j->first;
				const DoubleStatMap &map = j->second;
				
				for(DoubleStatMap::const_iterator i=map.begin();i!=map.end();++i)
				{
					saver.time = i->first;
					const DefaultStatistics &stat = i->second;
					
					saveEachStatistic(saver, stat, indices);
				}
			}
		}
		
		void saveFrequency(QualityTablesFormatter &qd) const
		{
			initializeEmptyStatistics(qd, QualityTablesFormatter::FrequencyDimension);
			
			Indices indices;
			indices.fill(qd);
				
			StatisticSaver saver;
			saver.dimension = QualityTablesFormatter::FrequencyDimension;
			saver.qualityData = &qd;
			
			for(DoubleStatMap::const_iterator i=_frequencyStatistics.begin();i!=_frequencyStatistics.end();++i)
			{
				saver.frequency = i->first;
				const DefaultStatistics &stat = i->second;
				
				saveEachStatistic(saver, stat, indices);
			}
		}
		
		void saveBaseline(QualityTablesFormatter &qd) const
		{
			initializeEmptyStatistics(qd, QualityTablesFormatter::BaselineDimension);
			
			Indices indices;
			indices.fill(qd);
			
			StatisticSaver saver;
			saver.dimension = QualityTablesFormatter::BaselineDimension;
			saver.frequency = centralFrequency();
			saver.qualityData = &qd;
			
			for(std::map<double, BaselineStatisticsMap>::const_iterator j=_baselineStatistics.begin();j!=_baselineStatistics.end();++j)
			{
				saver.frequency = j->first;
				const BaselineStatisticsMap &map = j->second;
				
				const std::vector<std::pair<unsigned, unsigned> > baselines = map.BaselineList();
			
				for(std::vector<std::pair<unsigned, unsigned> >::const_iterator i=baselines.begin();i!=baselines.end();++i)
				{
					saver.antenna1 = i->first;
					saver.antenna2 =  i->second;
					
					const DefaultStatistics &stat = map.GetStatistics(saver.antenna1, saver.antenna2);
					
					saveEachStatistic(saver, stat, indices);
				}
			}
		}
		
		DefaultStatistics &getTimeStatistic(double time, double centralFrequency)
		{
			// We use find() to see if the value exists, and only use insert() when it does not,
			// because insert is slow (because a "Statistic" needs to be created). Holds for both
			// frequency and time maps.
			std::map<double, DoubleStatMap>::iterator i = _timeStatistics.find(centralFrequency);
			if(i == _timeStatistics.end())
			{
				i = _timeStatistics.insert(std::pair<double, DoubleStatMap>(centralFrequency, DoubleStatMap())).first;
			}
			DoubleStatMap &selectedTimeStatistic = i->second;
			
			DoubleStatMap::iterator j = selectedTimeStatistic.find(time);
			if(j == selectedTimeStatistic.end())
			{
				j = selectedTimeStatistic.insert(std::pair<double, DefaultStatistics>(time, DefaultStatistics(_polarizationCount))).first;
			}
			return j->second;
		}
		
		DefaultStatistics &getFrequencyStatistic(double frequency)
		{
			// Use insert() only when not exist, as it is slower then find because a
			// Statistic is created.
			DoubleStatMap::iterator i = _frequencyStatistics.find(frequency);
			if(i == _frequencyStatistics.end())
			{
				i = _frequencyStatistics.insert(std::pair<double, DefaultStatistics>(frequency, DefaultStatistics(_polarizationCount))).first;
			}
			return i->second;
		}
		
		DefaultStatistics &getBaselineStatistic(unsigned antenna1, unsigned antenna2, double centralFrequency)
		{
			std::map<double, BaselineStatisticsMap>::iterator i = _baselineStatistics.find(centralFrequency);
			if(i == _baselineStatistics.end())
			{
				i = _baselineStatistics.insert(std::pair<double, BaselineStatisticsMap>(centralFrequency, BaselineStatisticsMap(_polarizationCount))).first;
			}
			BaselineStatisticsMap &selectedBaselineStatistic = i->second;
			return selectedBaselineStatistic.GetStatistics(antenna1, antenna2);
		}
		
		template<bool AddStatistics>
		void assignStatistic(DefaultStatistics &destination, const StatisticalValue &source, QualityTablesFormatter::StatisticKind kind)
		{
			if(AddStatistics)
			{
				for(unsigned p=0;p<_polarizationCount;++p)
				{
					switch(kind)
					{
						case QualityTablesFormatter::RFIRatioStatistic:
							destination.rfiCount[p] += round((double) destination.count[p] / ((1.0/source.Value(p).real())-1.0));
							break;
						case QualityTablesFormatter::CountStatistic:
							destination.count[p] += (long unsigned) source.Value(p).real();
							break;
						case QualityTablesFormatter::MeanStatistic:
							destination.sum[p] += source.Value(p) * (float) destination.count[p];
							break;
						case QualityTablesFormatter::SumP2Statistic:
							destination.sumP2[p] += source.Value(p);
							break;
						case QualityTablesFormatter::DCountStatistic:
							destination.dCount[p] += (long unsigned) source.Value(p).real();
							break;
						case QualityTablesFormatter::DMeanStatistic:
							destination.dSum[p] += source.Value(p) * (float) destination.dCount[p];
							break;
						case QualityTablesFormatter::DSumP2Statistic:
							destination.dSumP2[p] += source.Value(p);
							break;
						default:
							break;
					}
				}
			}
			else {
				for(unsigned p=0;p<_polarizationCount;++p)
				{
					switch(kind)
					{
						case QualityTablesFormatter::RFIRatioStatistic:
							destination.rfiCount[p] = round((double) destination.count[p] / ((1.0/source.Value(p).real())-1.0));
							break;
						case QualityTablesFormatter::CountStatistic:
							destination.count[p] = (long unsigned) source.Value(p).real();
							break;
						case QualityTablesFormatter::MeanStatistic:
							destination.sum[p] = source.Value(p) * (float) destination.count[p];
							break;
						case QualityTablesFormatter::SumP2Statistic:
							destination.sumP2[p] = source.Value(p);
							break;
						case QualityTablesFormatter::DCountStatistic:
							destination.dCount[p] = (long unsigned) source.Value(p).real();
							break;
						case QualityTablesFormatter::DMeanStatistic:
							destination.dSum[p] = source.Value(p) * (float) destination.dCount[p];
							break;
						case QualityTablesFormatter::DSumP2Statistic:
							destination.dSumP2[p] = source.Value(p);
							break;
						default:
							break;
					}
				}
			}
		}
		
		void forEachDefaultStatistic(QualityTablesFormatter &qd, void (StatisticsCollection::*functionName)(QualityTablesFormatter &, QualityTablesFormatter::StatisticKind))
		{
			(this->*functionName)(qd, QualityTablesFormatter::CountStatistic);
			(this->*functionName)(qd, QualityTablesFormatter::MeanStatistic);
			(this->*functionName)(qd, QualityTablesFormatter::SumP2Statistic);
			(this->*functionName)(qd, QualityTablesFormatter::DCountStatistic);
			(this->*functionName)(qd, QualityTablesFormatter::DMeanStatistic);
			(this->*functionName)(qd, QualityTablesFormatter::DSumP2Statistic);
			(this->*functionName)(qd, QualityTablesFormatter::RFIRatioStatistic);
		}
		
		template<bool AddStatistics>
		void loadSingleTimeStatistic(QualityTablesFormatter &qd, QualityTablesFormatter::StatisticKind kind)
		{
			std::vector<std::pair<QualityTablesFormatter::TimePosition, StatisticalValue> > values;
			unsigned kindIndex = qd.QueryKindIndex(kind);
			qd.QueryTimeStatistic(kindIndex, values);
			for(std::vector<std::pair<QualityTablesFormatter::TimePosition, StatisticalValue> >::const_iterator i=values.begin();i!=values.end();++i)
			{
				const QualityTablesFormatter::TimePosition &position = i->first;
				const StatisticalValue &statValue = i->second;
				
				DefaultStatistics &stat = getTimeStatistic(position.time, position.frequency);
				assignStatistic<AddStatistics>(stat, statValue, kind);
			}
		}
		
		template<bool AddStatistics>
		void loadTime(QualityTablesFormatter &qd)
		{
			forEachDefaultStatistic(qd, &StatisticsCollection::loadSingleTimeStatistic<AddStatistics>);
		}
		
		template<bool AddStatistics>
		void loadSingleFrequencyStatistic(QualityTablesFormatter &qd, QualityTablesFormatter::StatisticKind kind)
		{
			std::vector<std::pair<QualityTablesFormatter::FrequencyPosition, StatisticalValue> > values;
			unsigned kindIndex = qd.QueryKindIndex(kind);
			qd.QueryFrequencyStatistic(kindIndex, values);
			for(std::vector<std::pair<QualityTablesFormatter::FrequencyPosition, StatisticalValue> >::const_iterator i=values.begin();i!=values.end();++i)
			{
				const QualityTablesFormatter::FrequencyPosition &position = i->first;
				const StatisticalValue &statValue = i->second;
				
				DefaultStatistics &stat = getFrequencyStatistic(position.frequency);
				assignStatistic<AddStatistics>(stat, statValue, kind);
			}
		}
		
		template<bool AddStatistics>
		void loadFrequency(QualityTablesFormatter &qd)
		{
			forEachDefaultStatistic(qd, &StatisticsCollection::loadSingleFrequencyStatistic<AddStatistics>);
		}
		
		template<bool AddStatistics>
		void loadSingleBaselineStatistic(QualityTablesFormatter &qd, QualityTablesFormatter::StatisticKind kind)
		{
			std::vector<std::pair<QualityTablesFormatter::BaselinePosition, StatisticalValue> > values;
			unsigned kindIndex = qd.QueryKindIndex(kind);
			qd.QueryBaselineStatistic(kindIndex, values);
			for(std::vector<std::pair<QualityTablesFormatter::BaselinePosition, StatisticalValue> >::const_iterator i=values.begin();i!=values.end();++i)
			{
				const QualityTablesFormatter::BaselinePosition &position = i->first;
				const StatisticalValue &statValue = i->second;
				
				DefaultStatistics &stat = getBaselineStatistic(position.antenna1, position.antenna2, position.frequency);
				assignStatistic<AddStatistics>(stat, statValue, kind);
			}
		}
		
		template<bool AddStatistics>
		void loadBaseline(QualityTablesFormatter &qd)
		{
			forEachDefaultStatistic(qd, &StatisticsCollection::loadSingleBaselineStatistic<AddStatistics>);
		}
		
		double centralFrequency() const
		{
			double min =_frequencyStatistics.begin()->first;
			double max = _frequencyStatistics.rbegin()->first;
			return (min + max) / 2.0;
		}
		
		typedef std::map<double, DefaultStatistics> DoubleStatMap;
		DefaultStatistics getGlobalStatistics(const DoubleStatMap &statMap) const
		{
			DefaultStatistics global(_polarizationCount);
			for(DoubleStatMap::const_iterator i=statMap.begin();i!=statMap.end();++i)
			{
				const DefaultStatistics &stat = i->second;
				global += stat;
			}
			return global;
		}
		DefaultStatistics getGlobalStatistics(const std::map<double, DoubleStatMap> &statMap) const
		{
			DefaultStatistics global(_polarizationCount);
			for(std::map<double, DoubleStatMap>::const_iterator i=statMap.begin();i!=statMap.end();++i)
			{
				const DefaultStatistics &stat = getGlobalStatistics(i->second);
				global += stat;
			}
			return global;
		}
		
		template<bool AutoCorrelations>
		DefaultStatistics getGlobalBaselineStatistics() const
		{
			DefaultStatistics global(_polarizationCount);
			
			for(std::map<double, BaselineStatisticsMap>::const_iterator f=_baselineStatistics.begin();f!=_baselineStatistics.end();++f)
			{
				const BaselineStatisticsMap &map = f->second;
				const std::vector<std::pair<unsigned, unsigned> > baselines = map.BaselineList();
				
				for(std::vector<std::pair<unsigned, unsigned> >::const_iterator i=baselines.begin();i!=baselines.end();++i)
				{
					const unsigned
						antenna1 = i->first,
						antenna2 =  i->second;
					if( ((antenna1 == antenna2) && AutoCorrelations) || ((antenna1 != antenna2) && (!AutoCorrelations)))
					{
						const DefaultStatistics &stat = map.GetStatistics(antenna1, antenna2);
						global += stat;
					}
				}
			}
			return global;
		}
		
		void serializeTime(std::ostream &stream) const
		{
			SerializeToUInt64(stream, _timeStatistics.size());
			
			for(std::map<double, DoubleStatMap>::const_iterator i=_timeStatistics.begin();i!=_timeStatistics.end();++i)
			{
				const double frequency = i->first;
				const DoubleStatMap &map = i->second;
				
				SerializeToDouble(stream, frequency);
				serializeDoubleStatMap(stream, map);
			}
		}
		
		void unserializeTime(std::istream &stream)
		{
			_timeStatistics.clear();
			size_t count = (size_t) UnserializeUInt64(stream);
			
			for(size_t i=0;i<count;++i)
			{
				double frequency = UnserializeDouble(stream);
				std::map<double, DoubleStatMap>::iterator iterator =
					_timeStatistics.insert(std::pair<double, DoubleStatMap>(frequency, DoubleStatMap())).first;
				unserializeDoubleStatMap(stream, iterator->second);
			}
		}
		
		void serializeFrequency(std::ostream &stream) const
		{
			serializeDoubleStatMap(stream, _frequencyStatistics);
		}
		
		void unserializeFrequency(std::istream &stream)
		{
			_frequencyStatistics.clear();
			unserializeDoubleStatMap(stream, _frequencyStatistics);
		}
		
		void serializeBaselines(std::ostream &stream) const
		{
			SerializeToUInt64(stream, _baselineStatistics.size());
			
			for(std::map<double, BaselineStatisticsMap>::const_iterator i=_baselineStatistics.begin();i!=_baselineStatistics.end();++i)
			{
				const double frequency = i->first;
				const BaselineStatisticsMap &map = i->second;
				
				SerializeToDouble(stream, frequency);
				map.Serialize(stream);
			}
		}
		
		void unserializeBaselines(std::istream &stream)
		{
			_baselineStatistics.clear();
			size_t count = (size_t) UnserializeUInt64(stream);
			
			for(size_t i=0;i<count;++i)
			{
				double frequency = UnserializeDouble(stream);
				std::map<double, BaselineStatisticsMap>::iterator iterator =
					_baselineStatistics.insert(std::pair<double, BaselineStatisticsMap>(frequency, BaselineStatisticsMap(_polarizationCount))).first;
				iterator->second.Unserialize(stream);
			}
		}
		
		void serializeDoubleStatMap(std::ostream &stream, const DoubleStatMap &statMap) const
		{
			uint64_t statCount = statMap.size();
			stream.write(reinterpret_cast<char *>(&statCount), sizeof(statCount));
			for(DoubleStatMap::const_iterator i=statMap.begin();i!=statMap.end();++i)
			{
				const double &key = i->first;
				const DefaultStatistics &stat = i->second;
				stream.write(reinterpret_cast<const char *>(&key), sizeof(key));
				stat.Serialize(stream);
			}
		}
		
		void unserializeDoubleStatMap(std::istream &stream, DoubleStatMap &statMap) const
		{
			size_t count = (size_t) UnserializeUInt64(stream);
			
			for(size_t i=0;i<count;++i)
			{
				double key = UnserializeDouble(stream);
				std::map<double, DefaultStatistics>::iterator iterator =
					statMap.insert(std::pair<double, DefaultStatistics>(key, DefaultStatistics(_polarizationCount))).first;
				iterator->second.Unserialize(stream);
			}
		}
		
		void addTime(const StatisticsCollection &collection)
		{
			for(std::map<double, DoubleStatMap>::const_iterator i=collection._timeStatistics.begin();i!=collection._timeStatistics.end();++i)
			{
				const double frequency = i->first;
				const DoubleStatMap &map = i->second;
				
				for(DoubleStatMap::const_iterator j=map.begin();j!=map.end();++j)
				{
					const double time = j->first;
					const DefaultStatistics &stat = j->second;
					getTimeStatistic(time, frequency) += stat;
				}
			}
		}
		
		void addFrequency(const StatisticsCollection &collection)
		{
			for(DoubleStatMap::const_iterator j=collection._frequencyStatistics.begin();j!=collection._frequencyStatistics.end();++j)
			{
				const double frequency = j->first;
				const DefaultStatistics &stat = j->second;
				getFrequencyStatistic(frequency) += stat;
			}
		}
		
		void addBaseline(const StatisticsCollection &collection)
		{
			for(std::map<double, BaselineStatisticsMap>::const_iterator i=collection._baselineStatistics.begin();i!=collection._baselineStatistics.end();++i)
			{
				const double frequency = i->first;
				const BaselineStatisticsMap &map = i->second;
				
				vector<std::pair<unsigned, unsigned> > baselines = map.BaselineList();
				for(vector<std::pair<unsigned, unsigned> >::const_iterator j=baselines.begin();j!=baselines.end();++j)
				{
					const unsigned antenna1 = j->first;
					const unsigned antenna2 = j->second;
					const DefaultStatistics &stat = map.GetStatistics(antenna1, antenna2);
					getBaselineStatistic(antenna1, antenna2, frequency) += stat;
				}
			}
		}
		
		std::map<double, DoubleStatMap> _timeStatistics;
		DoubleStatMap _frequencyStatistics;
		std::map<double, BaselineStatisticsMap> _baselineStatistics;
		
		std::map<unsigned, std::vector< DefaultStatistics *> > _bands;
		std::map<unsigned, double> _centralFrequencies;
		
		unsigned _polarizationCount;
};

#endif
