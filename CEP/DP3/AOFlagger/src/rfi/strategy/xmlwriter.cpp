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
#include <AOFlagger/rfi/strategy/xmlwriter.h>

#include <AOFlagger/rfi/strategy/adapter.h>
#include <AOFlagger/rfi/strategy/addstatisticsaction.h>
#include <AOFlagger/rfi/strategy/baselineselectionaction.h>
#include <AOFlagger/rfi/strategy/changeresolutionaction.h>
#include <AOFlagger/rfi/strategy/combineflagresults.h>
#include <AOFlagger/rfi/strategy/cutareaaction.h>
#include <AOFlagger/rfi/strategy/foreachbaselineaction.h>
#include <AOFlagger/rfi/strategy/foreachmsaction.h>
#include <AOFlagger/rfi/strategy/foreachpolarisationblock.h>
#include <AOFlagger/rfi/strategy/frequencyselectionaction.h>
#include <AOFlagger/rfi/strategy/fringestopaction.h>
#include <AOFlagger/rfi/strategy/imageraction.h>
#include <AOFlagger/rfi/strategy/iterationblock.h>
#include <AOFlagger/rfi/strategy/plotaction.h>
#include <AOFlagger/rfi/strategy/quickcalibrateaction.h>
#include <AOFlagger/rfi/strategy/setflaggingaction.h>
#include <AOFlagger/rfi/strategy/setimageaction.h>
#include <AOFlagger/rfi/strategy/slidingwindowfitaction.h>
#include <AOFlagger/rfi/strategy/statisticalflagaction.h>
#include <AOFlagger/rfi/strategy/strategy.h>
#include <AOFlagger/rfi/strategy/svdaction.h>
#include <AOFlagger/rfi/strategy/thresholdaction.h>
#include <AOFlagger/rfi/strategy/timeselectionaction.h>
#include <AOFlagger/rfi/strategy/writeflagsaction.h>

namespace rfiStrategy {

	XmlWriter::XmlWriter() : _writeDescriptions(false)
	{
	}
	
	
	XmlWriter::~XmlWriter()
	{
	}
	
	void XmlWriter::WriteStrategy(const Strategy &strategy, const std::string &filename)
	{
		_describedActions.clear();

		StartDocument(filename);

		std::string commentStr = 
			"This is a Strategy configuration file for the\n"
			"rfi detector by André Offringa (offringa@astro.rug.nl).\n";
		if(_writeDescriptions)
			commentStr += "\nIf you like to take a look at the structure of this file,\n"
				"try opening it in e.g. Firefox.\n";

		Comment(commentStr.c_str());

		Start("rfi-strategy");
		Attribute("format-version", STRATEGY_FILE_FORMAT_VERSION);
		Attribute("reader-version-required", STRATEGY_FILE_READER_VERSION_REQUIRED);
		writeAction(strategy);
		End();

		Close();
	}
	
	void XmlWriter::writeAction(const Action &action)
	{
		if(_writeDescriptions)
		{
			if(_describedActions.count(action.Type()) == 0)
			{
				const char *description = ActionFactory::GetDescription(action.Type());
				if(description != 0)
					Comment(wrap(description, 70).c_str());
				_describedActions.insert(action.Type());
			}
		}

		Start("action");
		switch(action.Type())
		{
			case ActionBlockType:
				throw std::runtime_error("Can not store action blocks");
			case AdapterType:
				writeAdapter(static_cast<const Adapter&>(action));
				break;
			case AddStatisticsActionType:
				writeAddStatisticsAction(static_cast<const AddStatisticsAction&>(action));
				break;
			case BaselineSelectionActionType:
				writeBaselineSelectionAction(static_cast<const BaselineSelectionAction&>(action));
				break;
			case ChangeResolutionActionType:
				writeChangeResolutionAction(static_cast<const ChangeResolutionAction&>(action));
				break;
			case CombineFlagResultsType:
				writeCombineFlagResults(static_cast<const CombineFlagResults&>(action));
				break;
			case CutAreaActionType:
				writeCutAreaAction(static_cast<const CutAreaAction&>(action));
				break;
			case ForEachBaselineActionType:
				writeForEachBaselineAction(static_cast<const ForEachBaselineAction&>(action));
				break;
			case ForEachMSActionType:
				writeForEachMSAction(static_cast<const ForEachMSAction&>(action));
				break;
			case ForEachPolarisationBlockType:
				writeForEachPolarisationBlock(static_cast<const ForEachPolarisationBlock&>(action));
				break;
			case FrequencySelectionActionType:
				writeFrequencySelectionAction(static_cast<const FrequencySelectionAction&>(action));
				break;
			case FringeStopActionType:
				writeFringeStopAction(static_cast<const FringeStopAction&>(action));
				break;
			case ImagerActionType:
				writeImagerAction(static_cast<const ImagerAction&>(action));
				break;
			case IterationBlockType:
				writeIterationBlock(static_cast<const IterationBlock&>(action));
				break;
			case PlotActionType:
				writePlotAction(static_cast<const PlotAction&>(action));
				break;
			case QuickCalibrateActionType:
				writeQuickCalibrateAction(static_cast<const QuickCalibrateAction&>(action));
				break;
			case SetFlaggingActionType:
				writeSetFlaggingAction(static_cast<const SetFlaggingAction&>(action));
				break;
			case SetImageActionType:
				writeSetImageAction(static_cast<const SetImageAction&>(action));
				break;
			case SlidingWindowFitActionType:
				writeSlidingWindowFitAction(static_cast<const SlidingWindowFitAction&>(action));
				break;
			case StatisticalFlagActionType:
				writeStatisticalFlagAction(static_cast<const StatisticalFlagAction&>(action));
				break;
			case StrategyType:
				writeStrategy(static_cast<const Strategy&>(action));
				break;
			case SVDActionType:
				writeSVDAction(static_cast<const SVDAction&>(action));
				break;
			case ThresholdActionType:
				writeThresholdAction(static_cast<const ThresholdAction&>(action));
				break;
			case TimeSelectionActionType:
				writeTimeSelectionAction(static_cast<const TimeSelectionAction&>(action));
				break;
			case WriteFlagsActionType:
				writeWriteFlagsAction(static_cast<const WriteFlagsAction&>(action));
				break;
		}
		End();
	}
	
	void XmlWriter::writeContainerItems(const ActionContainer &actionContainer)
	{
		Start("children");
		for(size_t i=0;i<actionContainer.GetChildCount();++i)
		{
			writeAction(actionContainer.GetChild(i));
		}
		End();
	}
	
	void XmlWriter::writeAdapter(const Adapter &action)
	{
		Attribute("type", "Adapter");
		Write<bool>("restore-originals", action.RestoreOriginals());
		writeContainerItems(action);
	}

	void XmlWriter::writeAddStatisticsAction(const AddStatisticsAction &action)
	{
		Attribute("type", "AddStatisticsAction");
		Write("file-prefix", action.FilePrefix().c_str());
		Write("compare-original-and-alternative", action.CompareOriginalAndAlternative());
	}

	void XmlWriter::writeBaselineSelectionAction(const class BaselineSelectionAction &action)
	{
		Attribute("type", "BaselineSelectionAction");
		Write<bool>("preparation-step", action.PreparationStep());
		Write<bool>("flag-bad-baselines", action.FlagBadBaselines());
		Write<num_t>("threshold", action.Threshold());
		Write<num_t>("smoothing-sigma", action.SmoothingSigma());
		Write<num_t>("abs-threshold", action.AbsThreshold());
		Write<bool>("make-plot", action.MakePlot());
	}

	void XmlWriter::writeChangeResolutionAction(const ChangeResolutionAction &action)
	{
		Attribute("type", "ChangeResolutionAction");
		Write<int>("time-decrease-factor", action.TimeDecreaseFactor());
		Write<int>("frequency-decrease-factor", action.FrequencyDecreaseFactor());
		Write<bool>("restore-revised", action.RestoreRevised());
		Write<bool>("restore-masks", action.RestoreMasks());
		writeContainerItems(action);
	}

	void XmlWriter::writeCombineFlagResults(const CombineFlagResults &action)
	{
		Attribute("type", "CombineFlagResults");
		writeContainerItems(action);
	}

	void XmlWriter::writeCutAreaAction(const class CutAreaAction &action)
	{
		Attribute("type", "CutAreaAction");
		Write<int>("start-time-steps", action.StartTimeSteps());
		Write<int>("end-time-steps", action.EndTimeSteps());
		Write<int>("top-channels", action.TopChannels());
		Write<int>("bottom-channels", action.BottomChannels());
		writeContainerItems(action);
	}

	void XmlWriter::writeForEachBaselineAction(const ForEachBaselineAction &action)
	{
		Attribute("type", "ForEachBaselineAction");
		Write<int>("selection", action.Selection());
		Write<int>("thread-count", action.ThreadCount());
		Write<int>("data-kind", action.DataKind());
		writeContainerItems(action);
	}

	void XmlWriter::writeForEachMSAction(const ForEachMSAction &action)
	{
		Attribute("type", "ForEachMSAction");
		Start("filenames");
		const std::vector<std::string> &filenames = action.Filenames();
		for(std::vector<std::string>::const_iterator i=filenames.begin();i!=filenames.end();++i)
		{
			Write("filename", i->c_str());
		}
		End();
		writeContainerItems(action);
	}

	void XmlWriter::writeForEachPolarisationBlock(const ForEachPolarisationBlock &action)
	{
		Attribute("type", "ForEachPolarisationBlock");
		Write<bool>("iterate-stokes-values", action.IterateStokesValues());
		writeContainerItems(action);
	}

	void XmlWriter::writeFrequencySelectionAction(const FrequencySelectionAction &action)
	{
		Attribute("type", "FrequencySelectionAction");
		Write<double>("threshold", action.Threshold());
	}

	void XmlWriter::writeFringeStopAction(const FringeStopAction &action)
	{
		Attribute("type", "FringeStopAction");
		Write<bool>("fit-channels-individually", action.FitChannelsIndividually());
		Write<num_t>("fringes-to-consider", action.FringesToConsider());
		Write<bool>("only-fringe-stop", action.OnlyFringeStop());
		Write<int>("window-size", action.WindowSize());
	}

	void XmlWriter::writeImagerAction(const ImagerAction &)
	{
		Attribute("type", "ImagerAction");
	}

	void XmlWriter::writeIterationBlock(const IterationBlock &action)
	{
		Attribute("type", "IterationBlock");
		Write<int>("iteration-count", action.IterationCount());
		Write<double>("sensitivity-start", action.SensitivityStart());
		writeContainerItems(action);
	}

	void XmlWriter::writePlotAction(const class PlotAction &action)
	{
		Attribute("type", "PlotAction");
		Write<int>("plot-kind", action.PlotKind());
		Write<bool>("logarithmic-y-axis", action.LogarithmicYAxis());
	}

	void XmlWriter::writeQuickCalibrateAction(const QuickCalibrateAction &)
	{
		Attribute("type", "QuickCalibrateAction");
	}

	void XmlWriter::writeSetFlaggingAction(const SetFlaggingAction &action)
	{
		Attribute("type", "SetFlaggingAction");
		Write<int>("new-flagging", action.NewFlagging());
	}

	void XmlWriter::writeSetImageAction(const SetImageAction &action)
	{
		Attribute("type", "SetImageAction");
		Write<int>("new-image", action.NewImage());
	}

	void XmlWriter::writeSlidingWindowFitAction(const SlidingWindowFitAction &action)
	{
		Attribute("type", "SlidingWindowFitAction");
		Write<num_t>("fit-precision", action.Parameters().fitPrecision);
		Write<num_t>("frequency-direction-kernel-size", action.Parameters().frequencyDirectionKernelSize);
		Write<int>("frequency-direction-window-size", action.Parameters().frequencyDirectionWindowSize);
		Write<int>("method", action.Parameters().method);
		Write<num_t>("time-direction-kernel-size", action.Parameters().timeDirectionKernelSize);
		Write<int>("time-direction-window-size", action.Parameters().timeDirectionWindowSize);
	}

	void XmlWriter::writeStatisticalFlagAction(const StatisticalFlagAction &action)
	{
		Attribute("type", "StatisticalFlagAction");
		Write<size_t>("enlarge-frequency-size", action.EnlargeFrequencySize());
		Write<size_t>("enlarge-time-size", action.EnlargeTimeSize());
		Write<num_t>("max-contaminated-frequencies-ratio", action.MaxContaminatedFrequenciesRatio());
		Write<num_t>("max-contaminated-times-ratio", action.MaxContaminatedTimesRatio());
		Write<num_t>("minimum-good-frequency-ratio", action.MinimumGoodFrequencyRatio());
		Write<num_t>("minimum-good-time-ratio", action.MinimumGoodTimeRatio());
	}

	void XmlWriter::writeStrategy(const class Strategy &action)
	{
		Attribute("type", "Strategy");
		writeContainerItems(action);
	}

	void XmlWriter::writeSVDAction(const SVDAction &action)
	{
		Attribute("type", "SVDAction");
		Write<int>("singular-value-count", action.SingularValueCount());
	}

	void XmlWriter::writeThresholdAction(const ThresholdAction &action)
	{
		Attribute("type", "ThresholdAction");
		Write<num_t>("base-sensitivity", action.BaseSensitivity());
		Write<bool>("time-direction-flagging", action.TimeDirectionFlagging());
		Write<bool>("frequency-direction-flagging", action.FrequencyDirectionFlagging());
	}

	void XmlWriter::writeTimeSelectionAction(const TimeSelectionAction &action)
	{
		Attribute("type", "TimeSelectionAction");
		Write<double>("threshold", action.Threshold());
	}

	void XmlWriter::writeWriteFlagsAction(const WriteFlagsAction &)
	{
		Attribute("type", "WriteFlagsAction");
	}

	std::string XmlWriter::wrap(const std::string &input, size_t max) const
	{
		int start = 0;
		bool first = true;
		std::stringstream s;
		int length = input.size();
		while(start < length)
		{
			int end = start + max;
			if(end > length)
				end = length;
			else {
				do {
					--end;
				} while(end > start && input[end] != ' ');
				if(end <= start)
					end = start + max;
				else
					++end;
			}
			int nextStart = end;
			while(end > start && input[end-1] == ' ') --end;

			if(!first)
				s << "\n";
			for(int i=start;i<end;++i)
				s << input[i];
				
			first = false;
			start = nextStart;
		}
		return s.str();
	}
}
