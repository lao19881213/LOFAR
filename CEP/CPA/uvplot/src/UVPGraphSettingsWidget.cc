//
// Copyright (C) 2002
// ASTRON (Netherlands Foundation for Research in Astronomy)
// P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//



#include <uvplot/UVPGraphSettingsWidget.h>

#include <qlayout.h>

#include <sstream>


//=================>>>  UVPGraphSettingsWidget::UVPGraphSettingsWidget  <<<=================

UVPGraphSettingsWidget::UVPGraphSettingsWidget(unsigned int numOfAntennae,
                                               QWidget *    parent,
                                               const char * name,
                                               WFlags       f)
  : QWidget(parent, name, f),
    itsNumberOfAntennae(0)
{
  itsAntenna1Label  = new QLabel("Antenna 1: ", this);
  itsAntenna1Slider = new QSlider(1, numOfAntennae, 1, 1,
                                  QSlider::Horizontal, this, "Antenna 1");

  itsColumnLabel      = new QLabel("Load column:", this);
  itsColumnCombo      = new QComboBox(true, this);

  itsLoadButton    = new QPushButton("Load ant1/column", this);


  itsCorrelationCombo = new QComboBox(false, this);
  itsCorrelationLabel = new QLabel("Correlation:", this);
  
  itsAntenna2Label  = new QLabel("Antenna 2: ", this);
  itsAntenna2Slider = new QSlider(1, numOfAntennae, 1, numOfAntennae,
                                  QSlider::Horizontal, this, "Antenna 2");



  itsSettings.setAntenna1(itsAntenna1Slider->value()-1);
  itsSettings.setAntenna2(itsAntenna2Slider->value()-1);
  
  QVBoxLayout *BaselineVLayout = new QVBoxLayout(this);
  BaselineVLayout->addWidget(itsAntenna1Label);
  BaselineVLayout->addWidget(itsAntenna1Slider);
  BaselineVLayout->addWidget(itsColumnLabel);
  BaselineVLayout->addWidget(itsColumnCombo);
  BaselineVLayout->addWidget(itsLoadButton);
  BaselineVLayout->addWidget(itsAntenna2Label);
  BaselineVLayout->addWidget(itsAntenna2Slider);
  BaselineVLayout->addWidget(itsCorrelationLabel);
  BaselineVLayout->addWidget(itsCorrelationCombo);
  BaselineVLayout->addStretch();
  BaselineVLayout->activate();

  itsCorrelationCombo->insertItem("Not selected", 0);
  itsCorrelationCombo->insertItem("XX", 1);
  itsCorrelationCombo->insertItem("XY", 2);
  itsCorrelationCombo->insertItem("YX", 3);
  itsCorrelationCombo->insertItem("YY", 4);
  itsCorrelationCombo->insertItem("RR", 5);
  itsCorrelationCombo->insertItem("RL", 6);
  itsCorrelationCombo->insertItem("LR", 7);
  itsCorrelationCombo->insertItem("LL", 8);

  itsColumnCombo->insertItem("", 0);
  itsColumnCombo->insertItem("DATA", 1);
  itsColumnCombo->insertItem("MODEL_DATA", 2);
  itsColumnCombo->insertItem("CORRECTED_DATA", 3);
  itsColumnCombo->insertItem("RESIDUAL_DATA", 4);
  

  QObject::connect(itsAntenna1Slider, SIGNAL(valueChanged(int)),
                   this, SLOT(slot_antenna1Changed(int)) );

  QObject::connect(itsAntenna2Slider, SIGNAL(valueChanged(int)),
                   this, SLOT(slot_antenna2Changed(int)) );

  QObject::connect(itsLoadButton, SIGNAL(clicked()),
                   this, SIGNAL(signalLoadButtonClicked()));

  QObject::connect(itsColumnCombo, SIGNAL(activated(int)),
                   this, SLOT(slot_columnChanged(int)));

  QObject::connect(itsCorrelationCombo, SIGNAL(activated(int)),
                   this, SLOT(slot_correlationChanged(int)) );


  slot_antenna1Changed(itsAntenna1Slider->value());
  slot_antenna2Changed(itsAntenna2Slider->value());
}







//=================>>>  UVPGraphSettingsWidget::~UVPGraphSettingsWidget  <<<=================

UVPGraphSettingsWidget::~UVPGraphSettingsWidget()
{
}





//=================>>>  UVPGraphSettingsWidget::setNumberOfAntennae  <<<=================

void UVPGraphSettingsWidget::setNumberOfAntennae(unsigned int numberOfAntennae)
{
  itsNumberOfAntennae = numberOfAntennae;
  itsAntenna1Slider->setRange(1, numberOfAntennae);
  itsAntenna2Slider->setRange(1, numberOfAntennae);
}






//===============>>>  UVPGraphSettingsWidget::slot_antenna1Changed  <<<===============

void UVPGraphSettingsWidget::slot_antenna1Changed(int antenna1)
{
  std::ostringstream out;
  out << "Antenna 1: " << antenna1;
  itsAntenna1Label->setText(out.str().c_str());

  itsSettings.setAntenna1(antenna1-1);
  emit signalAntenna1Changed(antenna1-1);
}








//===============>>>  UVPGraphSettingsWidget::slot_antenna2Changed  <<<===============

void UVPGraphSettingsWidget::slot_antenna2Changed(int antenna2)
{
  std::ostringstream out;
  out << "Antenna 2: " << antenna2;
  itsAntenna2Label->setText(out.str().c_str());

  itsSettings.setAntenna2(antenna2-1);
  emit signalAntenna2Changed(antenna2-1);
}





//============>>>  UVPGraphSettingsWidget::slot_correlationChanged  <<<============

void UVPGraphSettingsWidget::slot_correlationChanged(int corr)
{
  itsSettings.setCorrelation(UVPDataAtomHeader::Correlation(corr));
  emit signalCorrelationChanged(UVPDataAtomHeader::Correlation(corr));
}





//============>>>  UVPGraphSettingsWidget::slot_columnChanged  <<<============

void UVPGraphSettingsWidget::slot_columnChanged(int /*column*/)
{
  itsSettings.setColumnName(itsColumnCombo->currentText().latin1());
  emit signalColumnChanged(itsColumnCombo->currentText().latin1());
}





//===============>>>  UVPGraphSettingsWidget::getSettings  <<<===============

const UVPGraphSettings &UVPGraphSettingsWidget::getSettings() const
{
  return itsSettings;
}
