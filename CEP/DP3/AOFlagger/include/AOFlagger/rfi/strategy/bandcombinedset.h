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

#ifndef BANDCOMBINEDSET_H
#define BANDCOMBINEDSET_H

#include <string>
#include <sstream>
#include <cstring>
#include <vector>

#include <AOFlagger/msio/types.h>
#include <AOFlagger/msio/timefrequencymetadata.h>

#include <AOFlagger/rfi/strategy/msimageset.h>

namespace rfiStrategy {

	class BandCombinedSet;
	
	class BandCombinedSetIndex : public ImageSetIndex {
		public:
			BandCombinedSetIndex(ImageSet &set);

			~BandCombinedSetIndex()
			{
				for(size_t i=0;i<_setCount;++i)
					delete _indices[i];
				delete[] _indices;
			}
			virtual void Previous()
			{
				for(size_t i=0;i<_setCount;++i)
					_indices[i]->Previous();
			}
			virtual void Next()
			{
				for(size_t i=0;i<_setCount;++i)
					_indices[i]->Next();
			}
			virtual void LargeStepPrevious()
			{
				for(size_t i=0;i<_setCount;++i)
					_indices[i]->LargeStepPrevious();
			}
			virtual void LargeStepNext()
			{
				for(size_t i=0;i<_setCount;++i)
					_indices[i]->LargeStepNext();
			}
			virtual std::string Description() const
			{
				std::stringstream s;
				s << "Combination, starting with " << _indices[0]->Description();
				return s.str();
			}
			virtual bool IsValid() const
			{
				for(size_t i=0;i<_setCount;++i)
					if(!_indices[i]->IsValid()) return false;
				return true;
			}
			virtual BandCombinedSetIndex *Copy() const
			{
				return new BandCombinedSetIndex(imageSet(), this);
			}
			class ImageSetIndex *GetIndex(size_t i) const { return _indices[i]; }
		private:
			BandCombinedSetIndex(ImageSet &bcSet, const BandCombinedSetIndex *index) : ImageSetIndex(bcSet), _setCount(index->_setCount)
			{
				_indices = new ImageSetIndex*[_setCount];
				for(size_t i=0;i<_setCount;++i)
					_indices[i] = index->_indices[i]->Copy();
			}
			BandCombinedSet &bcSet() const;

			ImageSetIndex **_indices;
			size_t _setCount;
	};
	
	class BandCombinedSet : public ImageSet {
		public:
			BandCombinedSet(const std::vector<std::string> setNames)
			{
				for(std::vector<std::string>::const_iterator i=setNames.begin();i!=setNames.end();++i)
					_sets.push_back(new MSImageSet(*i));
			}
			virtual ~BandCombinedSet()
			{
				for(std::vector<MSImageSet*>::const_iterator i=_sets.begin();i!=_sets.end();++i)
					delete *i;
			}
			virtual BandCombinedSet *Copy()
			{
				return new BandCombinedSet(*this);
			}

			virtual BandCombinedSetIndex *StartIndex()
			{
				return new BandCombinedSetIndex(*this);
			}
			
			virtual void Initialize()
			{
				for(std::vector<MSImageSet*>::iterator i=_sets.begin();i!=_sets.end();++i)
					(*i)->Initialize();
			}

			virtual std::string Name() { return "Combined set"; }
			virtual std::string File() { return ""; }

			virtual TimeFrequencyData *LoadData(ImageSetIndex &index)
			{
				BandCombinedSetIndex &bcIndex = static_cast<BandCombinedSetIndex&>(index);
				TimeFrequencyData *first = _sets[0]->LoadData(*bcIndex.GetIndex(0));
				unsigned width = first->ImageWidth(), height = first->ImageHeight();
				TimeFrequencyData *data = new TimeFrequencyData(*first);
				data->SetImageSize(width, height*_sets.size());
				data->CopyFrom(*first, 0, 0);
				delete first;

				for(size_t i=1;i<_sets.size();++i)
				{
					TimeFrequencyData *current = _sets[i]->LoadData(*bcIndex.GetIndex(i));
					data->CopyFrom(*current, 0, height*i);
					delete current;
				}
				return data;
			}

			virtual void LoadFlags(ImageSetIndex &index, TimeFrequencyData &destination)
			{
			}

			virtual TimeFrequencyMetaDataCPtr LoadMetaData(ImageSetIndex &index)
			{
				return TimeFrequencyMetaDataCPtr();
			}
			virtual void WriteFlags(ImageSetIndex &index, TimeFrequencyData &data)
			{
				throw std::runtime_error("Not implemented");
			}
			virtual size_t GetPart(ImageSetIndex &index)
			{
				throw std::runtime_error("Not implemented");
			}
			virtual size_t GetAntenna1(ImageSetIndex &index)
			{
				throw std::runtime_error("Not implemented");
			}
			virtual size_t GetAntenna2(ImageSetIndex &index)
			{
				throw std::runtime_error("Not implemented");
			}

			size_t SetCount() const
			{
				return _sets.size();
			}
			MSImageSet &GetSet(size_t i) const { return *_sets[i]; }
			virtual void AddReadRequest(ImageSetIndex &index)
			{
				_data = BaselineData(index);
			}
			virtual void PerformReadRequests()
			{
				TimeFrequencyData *data = LoadData(_data.Index());
				_data.SetData(*data);
				delete data;
			}
			virtual BaselineData *GetNextRequested()
			{
				return new BaselineData(_data);
			}
			virtual void AddWriteFlagsTask(ImageSetIndex &index, std::vector<Mask2DCPtr> &flags)
			{
			}
			virtual void PerformWriteFlagsTask()
			{
			}
		private:
			BandCombinedSet(const BandCombinedSet &source) : ImageSet(source)
			{
				for(std::vector<MSImageSet*>::const_iterator i=source._sets.begin();i!=source._sets.end();++i)
				{
					_sets.push_back((*i)->Copy());
				}
			}

			std::vector<MSImageSet*> _sets;
			BaselineData _data;
	};

	BandCombinedSetIndex::BandCombinedSetIndex(ImageSet &set) : ImageSetIndex(set)
	{
		_setCount = bcSet().SetCount();
		_indices = new ImageSetIndex*[_setCount];
		for(size_t i=0;i<_setCount;++i)
			_indices[i] = bcSet().GetSet(i).StartIndex();
	}

	BandCombinedSet &BandCombinedSetIndex::bcSet() const { return static_cast<BandCombinedSet&>(imageSet()); }

}

#endif //BANDCOMBINEDSET_H
