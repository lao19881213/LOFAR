#include <UVPMainWindow.h>


#include <qapplication.h>       // qApp
#include <qmessagebox.h>


#include <sstream>              // std::ostringstream

#if(DEBUG_MODE)
InitDebugContext(UVPMainWindow, "DEBUG_CONTEXT");
#endif

//====================>>>  UVPMainWindow::UVPMainWindow  <<<====================

UVPMainWindow::UVPMainWindow():QMainWindow()
{
  m_file_menu = new QPopupMenu;
  m_file_menu->insertItem("&Open", mc_open);
  m_file_menu->insertItem("&Quit", qApp, SLOT(quit()));

  m_plot_menu = new QPopupMenu;
  m_plot_menu->insertItem("&Image", this, SLOT(slot_plotTimeFrequencyImage()));

  m_help_menu = new QPopupMenu;
  m_help_menu->insertItem("&About uvplot", this, SLOT(slot_about_uvplot()));

  m_menu_bar = new QMenuBar(this);
  m_menu_bar->insertItem("&File", m_file_menu);
  m_menu_bar->insertItem("&Plot", m_plot_menu);
  m_menu_bar->insertSeparator();
  m_menu_bar->insertItem("&Help", m_help_menu);


  itsStatusBar   = new QStatusBar(this);
  itsProgressBar = new QProgressBar(itsStatusBar);
  itsXPosLabel   = new QLabel(itsStatusBar);
  itsYPosLabel   = new QLabel(itsStatusBar);

  itsStatusBar->addWidget(itsXPosLabel, 2, true);
  itsStatusBar->addWidget(itsYPosLabel, 2, true);
  itsStatusBar->addWidget(itsProgressBar, 5, true);
  
  itsStatusBar->show();

#ifdef UVC_PLOT_FSBKB
  // Small example view

  itsCube = new UVPImageCube(500, 800);

  // End small example view

  itsCanvas = new UVPUVCoverageArea(this, itsCube);
  itsCanvas->setGeometry(0, m_menu_bar->height(), width(), height()-m_menu_bar->height() -itsStatusBar->height());
  itsCanvas->show();

  // Update itsCube
  slot_setProgressTotalSteps(500);

  for(int x = 0; x < 500; x++) {
    slot_setProgress(x+1);
    for(int y = 0; y < 800; y++) {
      itsCube->getPixel(x, y)->addPointUniform(sin(double(x)*double(y)/30.0), x*y);
    }
  }
#endif

#if(DEBUG_MODE)
  TRACER1("itsCanvas = new UVPTimeFrequencyPlot(this, Channels);");
#endif
  itsCanvas = new UVPTimeFrequencyPlot(this);
  itsCanvas->drawView();

  connect(itsCanvas, SIGNAL(signal_mouseWorldPosChanged(double, double)),
          this, SLOT(slot_mouse_world_pos(double, double)));

    // End update itsCube
}






//==================>>>  UVPMainWindow::~UVPMainWindow  <<<==================

UVPMainWindow::~UVPMainWindow()
{
#ifdef UVC_PLOT_FSBKB
  itsCanvas->setData(0);
  delete itsCube;
#endif

}





//==================>>>  UVPMainWindow::resizeEvent  <<<==================

void UVPMainWindow::resizeEvent(QResizeEvent */*event*/)
{
  itsCanvas->setGeometry(0, m_menu_bar->height(), width(), height()-m_menu_bar->height()-itsStatusBar->height());
}




//==================>>>  UVPMainWindow::slot_about_uvplot  <<<==================

void UVPMainWindow::slot_about_uvplot()
{
  QMessageBox::information(this, "About uvplot",
                           "UV data visualiser for the LOFAR project\n"
                           "by Michiel Brentjens (brentjens@astron.nl)");
}



//====================>>>  UVPMainWindow::slot_mouse_world_pos  <<<====================

void UVPMainWindow::slot_mouse_world_pos(double x,
                                        double y)
{
  std::ostringstream x_out;
  std::ostringstream y_out;

  x_out << itsCanvas->getXAxis()->getType()<< ": " << x
        << " " << itsCanvas->getXAxis()->getUnit();

  y_out << itsCanvas->getYAxis()->getType()<< ": " << y
        << " " << itsCanvas->getYAxis()->getUnit();


  itsXPosLabel->setText(x_out.str().c_str());
  itsYPosLabel->setText(y_out.str().c_str());
}




//====================>>>  UVPMainWindow::slot_setProgressTotalSteps <<<====================

void UVPMainWindow::slot_setProgressTotalSteps(int steps)
{
  itsProgressBar->setTotalSteps(steps);
}






//====================>>>  UVPMainWindow::slot_setProgress <<<====================

void UVPMainWindow::slot_setProgress(int steps)
{
  itsProgressBar->setProgress(steps);
}





//==========>>>  UVPMainWindow::slot_plotTimeFrequencyImage  <<<==========

void UVPMainWindow::slot_plotTimeFrequencyImage()
{
  // First experiment to see how fast I can paint...
  const unsigned int Channels = 512;
  double     Values[Channels];
  UVPSpectrum   Spectrum(Channels);

  itsCanvas->setChannels(Channels);

  slot_setProgressTotalSteps(600);
  for(unsigned int t = 0; t < 600; t++) {
    for(unsigned int i = 0; i < Channels; i++) {
      Values[i] = sin(double(i)*double(t)/30.0);
    }
    Spectrum.copyFast(Values);
    itsCanvas->slot_addSpectrum(Spectrum);
    slot_setProgress(t+1);
  }

  itsCanvas->drawView();
  slot_setProgress(0);
}
