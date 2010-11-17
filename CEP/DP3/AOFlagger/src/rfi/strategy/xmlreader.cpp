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
#include <AOFlagger/rfi/strategy/xmlreader.h>

#include <AOFlagger/rfi/strategy/action.h>
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

#define ENCODING "UTF-8"

namespace rfiStrategy {

XmlReader::XmlReader()
{
	LIBXML_TEST_VERSION ;
}


XmlReader::~XmlReader()
{
}

Strategy *XmlReader::CreateStrategyFromFile(const std::string &filename)
{
	_xmlDocument = xmlReadFile(filename.c_str(), NULL, 0);
	if (_xmlDocument == NULL)
		throw XmlReadError("Failed to parse file");

	xmlNode *rootElement = xmlDocGetRootElement(_xmlDocument);
	Strategy *strategy = 0;

	for (xmlNode *curNode=rootElement; curNode!=NULL; curNode=curNode->next)
	{
		if(curNode->type == XML_ELEMENT_NODE)
		{
			if(strategy != 0)
				throw XmlReadError("Multiple root elements found.");
			if(std::string((const char *) curNode->name) != "rfi-strategy")
				throw XmlReadError("Invalid structure in xml file: no rfi-strategy root node found. Maybe this is not an rfi strategy?");
			
			xmlChar *formatVersionCh = xmlGetProp(curNode, BAD_CAST "format-version");
			if(formatVersionCh == 0)
				throw XmlReadError("Missing attribute 'format-version'");
			double formatVersion = atof((const char*) formatVersionCh);
			xmlFree(formatVersionCh);

			xmlChar *readerVersionRequiredCh = xmlGetProp(curNode, BAD_CAST "reader-version-required");
			if(readerVersionRequiredCh == 0)
				throw XmlReadError("Missing attribute 'reader-version-required'");
			double readerVersionRequired = atof((const char*) readerVersionRequiredCh);
			xmlFree(readerVersionRequiredCh);
			
			if(readerVersionRequired > STRATEGY_FILE_FORMAT_VERSION)
				throw XmlReadError("This file requires a newer software version");
			if(formatVersion < STRATEGY_FILE_FORMAT_VERSION_REQUIRED)
				throw XmlReadError("This file is too old for the software, please recreate the strategy");
			
			strategy = parseRootChildren(curNode);
		}
	}
	if(strategy == 0)
		throw XmlReadError("Could not find root element in file.");

	xmlFreeDoc(_xmlDocument);

	return strategy;
}

Strategy *XmlReader::parseRootChildren(xmlNode *rootNode)
{
	Strategy *strategy = 0;
	for (xmlNode *curNode=rootNode->children; curNode!=NULL; curNode=curNode->next) {
		if(curNode->type == XML_ELEMENT_NODE)
		{
			if(strategy != 0)
				throw XmlReadError("More than one root element in file!");
			strategy = dynamic_cast<Strategy*>(parseAction(curNode));
			if(strategy == 0)
				throw XmlReadError("Root element was not a strategy!");
		}
	}
	if(strategy == 0)
		throw XmlReadError("Root element not found.");

	return strategy;
}

Action *XmlReader::parseChild(xmlNode *node)
{
	if (node->type == XML_ELEMENT_NODE) {
		std::string name((const char*) node->name);
		if(name == "action")
			return parseAction(node);
	}
	throw XmlReadError("Invalid structure in xml file: an action was expected");
}

class Strategy *XmlReader::parseStrategy(xmlNode *node)
{
	Strategy *strategy = new Strategy();
	parseChildren(node, strategy);
	return strategy;
}

void XmlReader::parseChildren(xmlNode *node, ActionContainer *parent) 
{
	for (xmlNode *curOuterNode=node->children; curOuterNode!=NULL; curOuterNode=curOuterNode->next) {
		if(curOuterNode->type == XML_ELEMENT_NODE)
		{
			std::string nameStr((const char *) curOuterNode->name);
			if(nameStr == "children")
			{
				for (xmlNode *curNode=curOuterNode->children; curNode!=NULL; curNode=curNode->next) {
					if (curNode->type == XML_ELEMENT_NODE) {
						parent->Add(parseChild(curNode));
					}
				}
			}
		}
	}
}

xmlNode *XmlReader::getTextNode(xmlNode *node, const char *subNodeName, bool allowEmpty) const 
{
	for (xmlNode *curNode=node->children; curNode!=NULL; curNode=curNode->next) {
		if(curNode->type == XML_ELEMENT_NODE)
		{
			std::string nameStr((const char *) curNode->name);
			if(nameStr == subNodeName)
			{
				curNode = curNode->children;
				if(curNode == 0 || curNode->type != XML_TEXT_NODE)
				{
					if(allowEmpty)
						return 0;
					else
						throw XmlReadError("Error occured in reading xml file: value node did not contain text");
				}
				return curNode;
			}
		}
	}
	throw XmlReadError("Error occured in reading xml file: could not find value node");
}

int XmlReader::getInt(xmlNode *node, const char *name) const 
{
	xmlNode *valNode = getTextNode(node, name);
	return atoi((const char *) valNode->content);
}

double XmlReader::getDouble(xmlNode *node, const char *name) const 
{
	xmlNode *valNode = getTextNode(node, name);
	return atof((const char *) valNode->content);
}

std::string XmlReader::getString(xmlNode *node, const char *name) const 
{
	xmlNode *valNode = getTextNode(node, name, true);
	if(valNode == 0)
		return std::string();
	else
		return std::string((const char *) valNode->content);
}

Action *XmlReader::parseAction(xmlNode *node)
{
	Action *newAction = 0;
	xmlChar *typeCh = xmlGetProp(node, BAD_CAST "type");
	if(typeCh == 0)
		throw XmlReadError("Action tag did not have 'type' parameter");
	std::string typeStr((const char*) typeCh);
	if(typeStr == "Adapter")
		newAction = parseAdapter(node);
	else if(typeStr == "AddStatisticsAction")
		newAction = parseAddStatistics(node);
	else if(typeStr == "BaselineSelectionAction")
		newAction = parseBaselineSelectionAction(node);
	else if(typeStr == "ChangeResolutionAction")
		newAction = parseChangeResolutionAction(node);
	else if(typeStr == "CombineFlagResults")
		newAction = parseCombineFlagResults(node);
	else if(typeStr == "CutAreaAction")
		newAction = parseCutAreaAction(node);
	else if(typeStr == "ForEachBaselineAction")
		newAction = parseForEachBaselineAction(node);
	else if(typeStr == "ForEachMSAction")
		newAction = parseForEachMSAction(node);
	else if(typeStr == "ForEachPolarisationBlock")
		newAction = parseForEachPolarisationBlock(node);
	else if(typeStr == "FrequencySelectionAction")
		newAction = parseFrequencySelectionAction(node);
	else if(typeStr == "FringeStopAction")
		newAction = parseFringeStopAction(node);
	else if(typeStr == "ImagerAction")
		newAction = parseImagerAction(node);
	else if(typeStr == "IterationBlock")
		newAction = parseIterationBlock(node);
	else if(typeStr == "PlotAction")
		newAction = parsePlotAction(node);
	else if(typeStr == "QuickCalibrateAction")
	newAction = parseQuickCalibrateAction(node);
	else if(typeStr == "SetFlaggingAction")
		newAction = parseSetFlaggingAction(node);
	else if(typeStr == "SetImageAction")
		newAction = parseSetImageAction(node);
	else if(typeStr == "SlidingWindowFitAction")
		newAction = parseSlidingWindowFitAction(node);
	else if(typeStr == "StatisticalFlagAction")
		newAction = parseStatisticalFlagAction(node);
	else if(typeStr == "SVDAction")
		newAction = parseSVDAction(node);
	else if(typeStr == "Strategy")
		newAction = parseStrategy(node);
	else if(typeStr == "ThresholdAction")
		newAction = parseThresholdAction(node);
	else if(typeStr == "TimeSelectionAction")
		newAction = parseTimeSelectionAction(node);
	else if(typeStr == "WriteFlagsAction")
		newAction = parseWriteFlagsAction(node);
	xmlFree(typeCh);
	if(newAction == 0)
	{
		std::stringstream s;
		s << "Unknown action type '" << typeStr << "' in xml file";
		throw XmlReadError(s.str());
	}
	return newAction;
}

Action *XmlReader::parseAdapter(xmlNode *node)
{
	Adapter *newAction = new Adapter();
	newAction->SetRestoreOriginals(getBool(node, "restore-originals"));
	parseChildren(node, newAction);
	return newAction;
}

Action *XmlReader::parseAddStatistics(xmlNode *node)
{
	AddStatisticsAction *newAction = new AddStatisticsAction();
	newAction->SetFilePrefix(getString(node, "file-prefix"));
	newAction->SetCompareOriginalAndAlternative(getBool(node, "compare-original-and-alternative"));
	return newAction;
}

Action *XmlReader::parseBaselineSelectionAction(xmlNode *node)
{
	BaselineSelectionAction *newAction = new BaselineSelectionAction();
	newAction->SetPreparationStep(getBool(node, "preparation-step"));
	newAction->SetFlagBadBaselines(getBool(node, "flag-bad-baselines"));
	newAction->SetThreshold(getDouble(node, "threshold"));
	newAction->SetAbsThreshold(getDouble(node, "abs-threshold"));
	newAction->SetSmoothingSigma(getDouble(node, "smoothing-sigma"));
	newAction->SetMakePlot(getBool(node, "make-plot"));
	return newAction;
}

Action *XmlReader::parseChangeResolutionAction(xmlNode *node)
{
	ChangeResolutionAction *newAction = new ChangeResolutionAction();
	newAction->SetTimeDecreaseFactor(getInt(node, "time-decrease-factor"));
	newAction->SetFrequencyDecreaseFactor(getInt(node, "frequency-decrease-factor"));
	newAction->SetRestoreRevised(getBool(node, "restore-revised"));
	newAction->SetRestoreMasks(getBool(node, "restore-masks"));
	parseChildren(node, newAction);
	return newAction;
}

Action *XmlReader::parseCombineFlagResults(xmlNode *node)
{
	CombineFlagResults *newAction = new CombineFlagResults();
	parseChildren(node, newAction);
	return newAction;
}

Action *XmlReader::parseCutAreaAction(xmlNode *node)
{
	CutAreaAction *newAction = new CutAreaAction();
	newAction->SetStartTimeSteps(getInt(node, "start-time-steps"));
	newAction->SetEndTimeSteps(getInt(node, "end-time-steps"));
	newAction->SetTopChannels(getInt(node, "top-channels"));
	newAction->SetBottomChannels(getInt(node, "bottom-channels"));
	parseChildren(node, newAction);
	return newAction;
}

Action *XmlReader::parseForEachBaselineAction(xmlNode *node)
{
	ForEachBaselineAction *newAction = new ForEachBaselineAction();
	newAction->SetSelection((BaselineSelection) getInt(node, "selection"));
	newAction->SetThreadCount(getInt(node, "thread-count"));
	newAction->SetDataKind((DataKind) getInt(node, "data-kind"));

	for (xmlNode *curNode=node->children; curNode!=NULL; curNode=curNode->next) {
		if(curNode->type == XML_ELEMENT_NODE)
		{
			std::string nameStr((const char *) curNode->name);
			if(nameStr == "antennae-to-skip")
			{
				for (xmlNode *curNode2=curNode->children; curNode2!=NULL; curNode2=curNode2->next) {
					if (curNode2->type == XML_ELEMENT_NODE) {
						std::string innerNameStr((const char *) curNode2->name);
						if(innerNameStr != "antenna")
							throw XmlReadError("Format of the for each baseline action is incorrect");
						xmlNode *textNode = curNode2->children;
						if(textNode->type != XML_TEXT_NODE)
							throw XmlReadError("Error occured in reading xml file: value node did not contain text");
						if(textNode->content != NULL)
						{
							newAction->AntennaeToSkip().insert(atoi((const char *) textNode->content));
						}
					}
				}
			}
		}
	}
	
	parseChildren(node, newAction);
	return newAction;
}

Action *XmlReader::parseForEachMSAction(xmlNode *node)
{
	ForEachMSAction *newAction = new ForEachMSAction();

	for (xmlNode *curNode=node->children; curNode!=NULL; curNode=curNode->next) {
		if(curNode->type == XML_ELEMENT_NODE)
		{
			std::string nameStr((const char *) curNode->name);
			if(nameStr == "filenames")
			{
				for (xmlNode *curNode2=curNode->children; curNode2!=NULL; curNode2=curNode2->next) {
					if (curNode2->type == XML_ELEMENT_NODE) {
						std::string innerNameStr((const char *) curNode2->name);
						if(innerNameStr != "filename")
							throw XmlReadError("Format of the for each MS action is incorrect");
						newAction->Filenames().push_back((const char *) curNode2->content);
					}
				}
			}
		}
	}
	
	parseChildren(node, newAction);
	return newAction;
}

Action *XmlReader::parseForEachPolarisationBlock(xmlNode *node)
{
	ForEachPolarisationBlock *newAction = new ForEachPolarisationBlock();
	newAction->SetIterateStokesValues(getBool(node, "iterate-stokes-values"));
	parseChildren(node, newAction);
	return newAction;
}

class Action *XmlReader::parseFrequencySelectionAction(xmlNode *node)
{
	FrequencySelectionAction *newAction = new FrequencySelectionAction();
	newAction->SetThreshold(getDouble(node, "threshold"));
	return newAction;
}

class Action *XmlReader::parseFringeStopAction(xmlNode *node)
{
	FringeStopAction *newAction = new FringeStopAction();
	newAction->SetFitChannelsIndividually(getBool(node, "fit-channels-individually"));
	newAction->SetFringesToConsider(getDouble(node, "fringes-to-consider"));
	newAction->SetOnlyFringeStop(getBool(node, "only-fringe-stop"));
	newAction->SetWindowSize(getInt(node, "window-size"));
	return newAction;
}

class Action *XmlReader::parseImagerAction(xmlNode *)
{
	ImagerAction *newAction = new ImagerAction();
	return newAction;
}

class Action *XmlReader::parseIterationBlock(xmlNode *node)
{
	IterationBlock *newAction = new IterationBlock();
	newAction->SetIterationCount(getInt(node, "iteration-count"));
	newAction->SetSensitivityStart(getDouble(node, "sensitivity-start"));
	parseChildren(node, newAction);
	return newAction;
}

Action *XmlReader::parseSetFlaggingAction(xmlNode *node)
{
	SetFlaggingAction *newAction = new SetFlaggingAction();
	newAction->SetNewFlagging((enum SetFlaggingAction::NewFlagging) getInt(node, "new-flagging"));
	return newAction;
}

Action *XmlReader::parsePlotAction(xmlNode *node)
{
	PlotAction *newAction = new PlotAction();
	newAction->SetPlotKind((enum PlotAction::PlotKind) getInt(node, "plot-kind"));
	newAction->SetLogarithmicYAxis(getBool(node, "logarithmic-y-axis"));
	return newAction;
}

Action *XmlReader::parseQuickCalibrateAction(xmlNode *)
{
	QuickCalibrateAction *newAction = new QuickCalibrateAction();
	return newAction;
}

class Action *XmlReader::parseSetImageAction(xmlNode *node)
{
	SetImageAction *newAction = new SetImageAction();
	newAction->SetNewImage((enum SetImageAction::NewImage) getInt(node, "new-image"));
	return newAction;
}

class Action *XmlReader::parseSlidingWindowFitAction(xmlNode *node)
{
	SlidingWindowFitAction *newAction = new SlidingWindowFitAction();
	newAction->Parameters().fitPrecision = getDouble(node, "fit-precision");
	newAction->Parameters().frequencyDirectionKernelSize = getDouble(node, "frequency-direction-kernel-size");
	newAction->Parameters().frequencyDirectionWindowSize = getInt(node, "frequency-direction-window-size");
	newAction->Parameters().method = (enum SlidingWindowFitParameters::Method) getInt(node, "method");
	newAction->Parameters().timeDirectionKernelSize = getDouble(node, "time-direction-kernel-size");
	newAction->Parameters().timeDirectionWindowSize = getInt(node, "time-direction-window-size");
	return newAction;
}

class Action *XmlReader::parseStatisticalFlagAction(xmlNode *node)
{
	StatisticalFlagAction *newAction = new StatisticalFlagAction();
	newAction->SetEnlargeFrequencySize(getInt(node, "enlarge-frequency-size"));
	newAction->SetEnlargeTimeSize(getInt(node, "enlarge-time-size"));
	newAction->SetMaxContaminatedFrequenciesRatio(getDouble(node, "max-contaminated-frequencies-ratio"));
	newAction->SetMaxContaminatedTimesRatio(getDouble(node, "max-contaminated-times-ratio"));
	newAction->SetMinimumGoodFrequencyRatio(getDouble(node, "minimum-good-frequency-ratio"));
	newAction->SetMinimumGoodTimeRatio(getDouble(node, "minimum-good-time-ratio"));
	return newAction;
}

class Action *XmlReader::parseSVDAction(xmlNode *node)
{
	SVDAction *newAction = new SVDAction();
	newAction->SetSingularValueCount(getInt(node, "singular-value-count"));
	return newAction;
}

class Action *XmlReader::parseThresholdAction(xmlNode *node)
{
	ThresholdAction *newAction = new ThresholdAction();
	newAction->SetBaseSensitivity(getDouble(node, "base-sensitivity"));
	newAction->SetTimeDirectionFlagging(getBool(node, "time-direction-flagging"));
	newAction->SetFrequencyDirectionFlagging(getBool(node, "frequency-direction-flagging"));
	return newAction;
}

class Action *XmlReader::parseTimeSelectionAction(xmlNode *node)
{
	TimeSelectionAction *newAction = new TimeSelectionAction();
	newAction->SetThreshold(getDouble(node, "threshold"));
	return newAction;
}

class Action *XmlReader::parseWriteFlagsAction(xmlNode *)
{
	WriteFlagsAction *newAction = new WriteFlagsAction();
	return newAction;
}

} // end of namespace
