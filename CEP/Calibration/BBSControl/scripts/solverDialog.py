#!/opt/local/bin/python

# Solver statistics dialog
#
# File:			solverDialog.py
# Author:		Sven Duscha (duscha@astron.nl)
# Date:			2010-08-05
# Last change;		2010-10-07
#
#

# Import
import sys, os, random
import SolverQuery as sq
import lofar.parmdb as parmdb

from PyQt4.QtCore import *
from PyQt4.QtGui import *

import numpy as np
import unicodedata
import time        # for timing functions
import math
import matplotlib
from matplotlib.backends.backend_qt4agg import FigureCanvasQTAgg as FigureCanvas
from matplotlib.backends.backend_qt4agg import NavigationToolbar2QTAgg as NavigationToolbar
from matplotlib.figure import Figure
import pylab as pl   # do we need that, too?


class SolverAppForm(QMainWindow):

    def __init__(self, parent=None):
        QMainWindow.__init__(self, parent)
        self.setWindowTitle('Solver statistics')

        self.solverQuery = sq.SolverQuery()       # attribute to hold SolverQuery object
        self.table=False                          # attribute to check if we have an open table
        self.solutionsPlot=None                   # attribute to define if solutions are shown or not
        self.parameterPlot=None

        self.solutions_plot=False                 # Plot solutions, too
        self.xaxis=None
        self.scatter=False                        # Plot scatter plots?
        self.clf=True                             # clear figure before replot

        self.parmMap={}                           # dictionary mapping parmDB names to solution indices

        #self.fig
        self.Figures=[]                           # list to hold all the dialog's figures
        self.currentFigure=None                   # pointer to current figure

        self.perIteration=False                   # per default set per iteration to False
        self.parmDB=None                          # parmDB database of parameters contained the MS
        self.parms=[]                             # Parameters that were solved for in this solver run
        self.currentSolution=None                 # current solution to plot solver parameters for, consisting of a tuple of
                                                  # start_time, end_time, start_freq and end_freq
        self.create_main_frame()
        self.create_status_bar()
        
        self.__styles = ["%s%s" % (x, y) for y in ["-", ":"] for x in ["b", "g", "r", "c",
    "m", "y", "k"]]


    # Save the plot to a graphics file (PNG, PDF, EPS, etc.)
    #
    def save_plot(self):
        file_choices = "PNG (*.png)|*.png"
        
        path = unicode(QFileDialog.getSaveFileName(self, 'Save file', '', file_choices))

        if path:
            self.canvas.print_figure(path, dpi=self.dpi)
            self.statusBar().showMessage('Saved to %s' % path, 2000)


    # Load the Measurementset solver table
    #
    def load_table(self):
        if self.table is True:
            self.close_table()

        # Customization: check if ~/Cluster/SolutionTests exists
        if os.path.exists('/Users/duscha/Cluster/SolutionTests'):
            setDir=QString('/Users/duscha/Cluster/SolutionTests')
        else:
            setDir=QString('')

        path = unicode(QFileDialog.getExistingDirectory(self, 'Load table', setDir, QFileDialog.ShowDirsOnly))
        path=str(path)  # Convert to string so that it can be used by load table

        if path:
            self.statusBar().showMessage("Loading solver statistics table")
            self.solverQuery=self.solverQuery.open(path)
            self.statusBar().clearMessage()

            # Fill in information from the table into the Dialog object for faster access?
            # strictly not necessary, since we can access them in the solverQuery object
            
            self.open_table(path)
        else:
            print "load_table: invalid path"


    # Open a table (on startup, if a command line argument is given)
    #
    def open_table(self, tableName):
        # If a table is already loaded, close it:
        if self.table is True:
            print "closing table..."
            self.close_table()

        # Check if path exists and it is a director (i.e. a table)
        if os.path.exists(tableName) is False:
            raise IOError
        else:
            if os.path.isdir(tableName) is False:
                raise IOError

        # Open it and set object attributes
        self.solverQuery=self.solverQuery.open(tableName)

        self.create_table_widgets()
        self.table=True

        self.determineTableType()

        # Enable checkboxes
        if self.tableType == "PERITERATION" or self.tableType == "PERITERATION_CORRMATRIX":
            self.showIterationsCheckBox.setEnabled(True)
            self.perIteration=False   # initially set that to False
        else:
            self.showIterationsCheckBox.setEnabled(False)
            self.perIteration=False

        # By default we do not want to show plots per iteration
        self.showIterationsCheckBox.setCheckState(False)
  
        # Decide on type which plotting to do (PERSOLUTION,PERITERATION, with or without CORRMATRIX)
        #tableType=self.solverQuery.getType() 
        #if tableType == "PERSOLUTION" or tableType == "PERSOLUTION_CORRMATRIX":
        #    self.perIteration=False                          # set internal attribute to False
        #    self.showIterationsCheckBox.setEnabled(False)    # Deactivate the per iteration checkbox
        #elif tableType == "PERITERATION" or tableType == "PERITERATION_CORRMATRIX":
        #    self.perIteration=True                           # set internal attribute to True
        #    self.showIterationsCheckBox.setEnabled(True)     # Activate the per iteration checkbox


        self.singleCellCheckBox.setEnabled(True)
        self.solutionsButton.setEnabled(True)
        self.drawButton.setEnabled(True)
        self.histogramButton.setEnabled(True) 

        # Update layouts to get correct GUI
        self.buttonsLayout.update()
        #self.mainLayout.update()


    # Close the table that is currently loaded
    #
    def close_table(self):
        # Remove buttons?
        self.buttonsLayout.removeWidget()

        # Remove table specific widgets
        self.buttonsLayout.removeWidget(self.timeStartSlider)
        self.buttonsLayout.removeWidget(self.timeEndSlider)
        self.buttonsLayout.removeWidget(self.timeStartSliderLabel)
        self.buttonsLayout.removeWidget(self.timeEndSliderLabel)
        self.buttonsLayout.removeWidget(self.frequencySlider)
        self.buttonsLayout.removeWidget(self.frequencySliderLabel)
        self.buttonsLayout.removeWidget(self.parametersComboBox)
        self.buttonsLayout.removeWidget(self.histogramButton)

        self.buttonsLayout.update()
        #self.mainLayout.update()
        self.solverQuery.close()
        self.table=False       # we don't have an open table anymore


    # Open parmDB
    #
    def open_parmDB(self, parmDBname):
        self.parmDB=parmdb.parmdb(parmDBname)


    # Close parmDB
    # TODO: What is the proper way to close the parmDB?
    #
    def close_parmDB(self):
        self.buttonsLayoyt.removeWidget(self.parmsComboBox)
        self.buttonsLayout.removeWidget(self.parmValueCombobox)

        self.parmDB.close()
        self.parmDB=None


    # Function to handle table index slider has changed
    #
    def on_timeStartSlider(self, index):
        # Read time at index
        starttime=self.solverQuery.timeSlots[index]['STARTTIME']

        self.timeStartSliderLabel.setText("S:" +  str(starttime) + " s")

        # Handle behaviour of timeEndSlider in combination with timeStartSlider
        # If timeEndSlider is smaller than timeStartSlider adjust the latter
        if self.timeEndSlider.sliderPosition() < self.timeStartSlider.sliderPosition():
            self.timeEndSlider.setValue(self.timeStartSlider.sliderPosition())

        self.repaint()

    # Function to handle table index slider has changed
    #
    def on_timeEndSlider(self, index):
        # Read time at index
        endtime=self.solverQuery.timeSlots[index]['ENDTIME']
        self.timeEndSliderLabel.setText("E:" + str(endtime) + " s")

        # Handle behaviour of timeEndSlider in combination with timeStartSlider
        # If timeEndSlider is smaller than timeStartSlider adjust the latter
        if self.timeEndSlider.sliderPosition() < self.timeStartSlider.sliderPosition():
            self.timeStartSlider.setValue(self.timeEndSlider.sliderPosition())

        self.repaint()


    # If only a single cell is to be shown, adjust timeEndSlider
    # to the position of timeStartSlider, and lock them
    def on_singleCell(self):
        if self.singleCellCheckBox.isChecked():
            self.timeEndSlider.setValue(self.timeStartSlider.sliderPosition())
            self.syncSliders()
            self.timeEndSlider.emit(SIGNAL('valueChanged()'))
            self.showIterationsCheckBox.setEnabled(True)
        else:
            self.unsyncSliders()
            self.timeEndSlider.emit(SIGNAL('valueChanged()'))
            self.showIterationsCheckBox.setCheckState(False)     # deactivate show iterations if we show interval
            self.showIterationsCheckBox.setEnabled(False)


    # Put sliders into sync, so that moving one also moves the other
    #
    def syncSliders(self):
        self.connect(self.timeStartSlider, SIGNAL('valueChanged(int)'), self.timeEndSlider, SLOT('setValue(int)')) 
        self.connect(self.timeEndSlider, SIGNAL('valueChanged(int)'), self.timeStartSlider, SLOT('setValue(int)'))


    # Unsync sliders, so that they can move independently again
    #
    def unsyncSliders(self):
       self.disconnect(self.timeStartSlider, SIGNAL('valueChanged(int)'), self.timeEndSlider, SLOT('setValue(int)'))
       self.disconnect(self.timeEndSlider, SIGNAL('valueChanged(int)'), self.timeStartSlider, SLOT('setValue(int)'))


    # Function to handle frequency index slider has changed
    #
    def on_frequencySlider(self, index):
        print "on_frequencySlider(self, index):"
        # Read frequency at index
        self.solverQuery.getTimeSlots()
        startfreq=self.solverQuery.timeSlots[index]['STARTTIME']
        endfreq=self.solverQuery.timeSlots[index]['ENDTIME']
        self.timeFreqSliderLabel.setText("Freq: \n" + str(startfreq))

    
    # Handler for showSolutionsPlot button
    # changes the GUI interface accordingly and calls a redraw of the
    # figure through the on_draw() function
    #
    def on_solutions(self):
        self.fig.clf()
      
        self.delAllAxes()         # clear all the axes
        self.canvas.draw()

        # If solutions are currently shown in the plot
        if self.solutions_plot==True:
            # Change GUI elements to include parms selection etc.
            self.addSolutionsplotButton.setText("Add Solutions")
            #self.parmsComboBox.hide()
            #self.parmValueComboBox.hide()

            #self.ParameterSubplot=self.fig.add_subplot(211)
            self.solutions_plot=False   # Indicate in attribute that we now do not show solutions anymore
            
            self.ParameterSubplot=self.fig.add_subplot(111)

            # TODO
            #titleString="Solution: " +  str(self.parmsComboBox.currentText()) + "& solver parameter " +  + str(self.parametersComboBox.currentText())

        else:
            # Remove solutions parms GUI elements, i.e. do not show them anymore
            self.addSolutionsplotButton.setText("Remove Solutions")
            self.parmsComboBox.show()
            self.parmValueComboBox.show()

            self.SolutionsSubplot=self.fig.add_subplot(211)
            self.ParameterSubplot=self.fig.add_subplot(212)

            self.solutions_plot=True   # indicate in attribute that we now do show solutions

            # TODO define title for subplots
            #titleString="Solver parameter:" + str(self.parametersComboBox.currentText())

        # Call self.on_draw() function to replot current parameter (+solution if toggled)
        #self.setTitle(titleString)
        self.on_draw()


    # If the selected solution changed, then also change 
    # items in self.parmValueComboBox
    #
    def on_solution_changed(self):

        self.parmValueComboBox.clear()
        # add items again
        parameter=str(self.parmsComboBox.currentText())  # need to convert QString to Python string
        if parameter.find("Gain") is not -1:
            self.parmValueComboBox.addItem("Amplitude")
            self.parmValueComboBox.addItem("Phase")

        # TODO different string searching in parameter name to define values for other parms
        

    # Display a histogram of the converged solutions (i.e. LASTITER=TRUE)
    # for the currently selected parameter
    #
    # nbins - number of bins to use (default=50)
    #
    def on_histogram(self, nbins=50):
        print "on_histogram()"    # DEBUG

        # Get time and frequency intervals from the QWidgets
        start_time=self.solverQuery.timeSlots[self.timeStartSlider.value()]['STARTTIME']
        end_time=self.solverQuery.timeSlots[self.timeEndSlider.value()]['ENDTIME']
        start_freq=self.solverQuery.frequencies[self.frequencySlider.value()]['STARTFREQ']
        end_freq=self.solverQuery.frequencies[self.frequencySlider.value()]['ENDFREQ']

        #solutionIndex=self.parmsComboBox.currentIndex()
        parameter=self.parametersComboBox.currentText()

        print "DEBUG"
        print "on_histogram(): parameter = ", parameter   # DEBUG
        print "on_histogram(): start_time = ", start_time
        print "on_histogram(): end_time = ", end_time

        # TODO: This does not work here (but works in tSolverQuery.py?? Why?
        convergedParameter=self.solverQuery.getConvergedParameter(parameter, start_time, end_time, start_freq, end_freq)
        
        # Create and display histogram
        n, bins, patches = pl.hist(convergedParameter, nbins, histtype='stepfilled')



    #**************************************************
    #
    # Mainwindow creation, create Widgets etc.
    #
    #**************************************************
            
    # Create the main frame of the dialog
    #
    def create_main_frame(self):
        self.main_frame = QWidget()

        #**********************************************************
        #
        # Canvas / Matplotlib
        #
        #**********************************************************

        # Create the mpl Figure and FigCanvas objects
        # 5x4 inches, 100 dots-per-inch
        #self.dpi = 100
        #self.fig = Figure((5.0, 4.0), dpi=self.dpi, frameon=False)
        #self.canvas = FigureCanvas(self.fig)
        #self.canvas.setParent(self.main_frame)
        #self.fig.set_canvas(self.canvas)

        #self.parmaxis=self.fig.gca()

        # If we do not show solutions yet  (this is now in the plot-handler functions)
        #self.ParameterSubplot = self.fig.add_subplot(111)

        # Create the navigation toolbar, tied to the canvas
        #self.mpl_toolbar = NavigationToolbar(self.canvas, self.main_frame)

        # NEW
        self.fig = Figure((5.0, 4.0), dpi=100, frameon=False)
        self.canvas = FigureCanvas(self.fig)
        self.canvas.setParent(self.main_frame)
        self.fig.set_canvas(self.canvas)

        # These are needed that the layout is not messed up
        self.setMinimumWidth(250)
        self.setMinimumHeight(600)


        #**********************************************************
        #
        # Widgets
        #
        #**********************************************************


        # Create buttons to access solverStatistics
        self.loadButton=QPushButton("&Load solver table")      # Load MS/solver button
        self.loadButton.setToolTip("Load a solver statistics table")
        self.saveButton=QPushButton("&Save plot")              # Save plot button
        self.saveButton.setToolTip("Save the plot to an image")
        self.drawButton=QPushButton("&Draw")                   # Draw button
        self.drawButton.setToolTip("Redraw the plot")
        self.addSolutionsplotButton=QPushButton("&Solutions plot")      # Add a plot of the solutions to the window
        self.addSolutionsplotButton.setToolTip("Show solutions plot")
        self.solutionsButton=QPushButton("Show Solutions")     # Show a plot of the corresponding solutions        
        self.solutionsButton.setToolTip("Show solutions plot along-side")
        self.quitButton=QPushButton("&Quit")                   # Quit the application
        self.quitButton.setToolTip("Quit application")

        self.plottingOptions=QLabel('Plotting options')
        self.showIterationsCheckBox=QCheckBox()                # Checkbox to show individual iterations parameters
        self.showIterationsCheckBox.setCheckState(False)       # Default: False
        self.showIterationsCheckBox.setText('Show iterations')
        self.showIterationsCheckBox.setToolTip('Show all iterations for this solution')
        self.showIterationsCheckBox.setTristate(False)

        self.singleCellCheckBox=QCheckBox()                    # Checkbox to show individual iterations parameters
        self.singleCellCheckBox.setCheckState(False)           # Default: False
        self.singleCellCheckBox.setText('Show single cell')
        self.singleCellCheckBox.setToolTip('Show only a single cell solution')
        self.singleCellCheckBox.setTristate(False)             # we seem to need these to have "normal" CheckBoxes

        self.scatterCheckBox=QCheckBox()                       # Checkbox to set plot to scatter plot
        self.scatterCheckBox.setCheckState(False)
        self.scatterCheckBox.setText('Scatter plot')
        self.scatterCheckBox.setTristate(False)

        self.colorizeCheckBox=QCheckBox()                      # Checkbox to create alternating colours
        self.colorizeCheckBox.setCheckState(True)
        self.colorizeCheckBox.setText('Colourize')
        self.colorizeCheckBox.setToolTip('Colourize plot points')
        self.colorizeCheckBox.setTristate(False)

        self.clfCheckBox=QCheckBox()
        self.clfCheckBox.setCheckState(True)
        self.clfCheckBox.setText('Clear figure')
        self.clfCheckBox.setToolTip('Clear the current figure on replot')
        self.clfCheckBox.setTristate(False)

        self.newCheckBox=QCheckBox()
        self.newCheckBox.setCheckState(False)
        self.newCheckBox.setText('New figure')
        self.newCheckBox.setToolTip('Plot in new figure window')
        self.newCheckBox.setTristate(False)

        self.histogramButton=QPushButton("&Histogram")              # Create a histogram
        self.histogramButton.setToolTip("Create a histogram of the current parameter")


        # TODO
        self.messageLabel=QLabel()     # Used to display the last solver message of a solution

        # Check if a table has been loaded
        if self.table is False:
            self.showIterationsCheckBox.setEnabled(False)
            self.singleCellCheckBox.setEnabled(False)
            self.solutionsButton.setEnabled(False)
            self.drawButton.setEnabled(False)
            self.histogramButton.setEnabled(False)                   # by default disable it            


        #**********************************************************
        #
        # Layouts
        #
        #**********************************************************

        #self.mainLayout=QHBoxLayout()
        self.buttonsLayout=QVBoxLayout()
        plotLayout=QVBoxLayout()

        # Add the button widgets to the buttonsLayout  (self.colorizeCheckBox) left out for the moment
        for widget in [self.loadButton, self.saveButton, self.drawButton, self.addSolutionsplotButton, self.quitButton, self.plottingOptions, self.showIterationsCheckBox, self.singleCellCheckBox, self.scatterCheckBox, self.clfCheckBox, self.newCheckBox, self.histogramButton]:
            self.buttonsLayout.addWidget(widget)
            widget.setMaximumWidth(170)  # restrain all widgets to that maximum width
            widget.show()    # DEBUG does this fix the display update issue?

        self.buttonsLayout.insertStretch(-1);    # add a stretcher at the end
        self.buttonsLayout.setSizeConstraint(3)  # enum 3 = QLayout::SetFixedSize
        
        # Add the canvas and the mpl toolbar to the plotLayout
        #plotLayout.addWidget(self.canvas)
        #plotLayout.addWidget(self.mpl_toolbar)

        #self.mainLayout.addLayout(self.buttonsLayout)
        #self.mainLayout.addLayout(plotLayout)

        #self.main_frame.setLayout(self.mainLayout)
        self.main_frame.setLayout(self.buttonsLayout)
        self.setCentralWidget(self.main_frame)



        #**********************************************************
        #
        # Connections
        #
        #**********************************************************

        # Set connections:
        self.connect(self.loadButton, SIGNAL('clicked()'), self.load_table)
        self.connect(self.saveButton, SIGNAL('clicked()'), self.save_plot)
        self.connect(self.drawButton, SIGNAL('clicked()'), self.on_draw)
        self.connect(self.addSolutionsplotButton, SIGNAL('clicked()'), self.on_solutions)
        self.connect(self.showIterationsCheckBox, SIGNAL('stateChanged(int)'), self.on_showIterations)
        self.connect(self.singleCellCheckBox, SIGNAL('stateChanged(int)'), self.on_singleCell)
        self.connect(self.clfCheckBox, SIGNAL('stateChanged(int)'), self.on_clf)
        self.connect(self.newCheckBox, SIGNAL('stateChanged(int)'), self.on_newFigure)
        #self.connect(self.showMessageCheckBox, SIGNAL('stateChanged(int)'), self.on_showMessage)
        self.connect(self.histogramButton, SIGNAL('clicked()'), self.on_histogram)



    # Create widgets that depend on information from the
    # solver table, and can only be created after table has
    # been opened
    #
    def create_table_widgets(self):
        # Create QComboBox with entries from casa table
        # get all available solver parameters from casa table
        # Before we opened the table this is kept empty
        # If we have a table opened
        self.createSliders()
 
        self.create_solver_dropdown(self)
        self.create_parms_dropdown()
        self.create_parms_value_dropdown()

        # Update layouts
        self.buttonsLayout.update()
        #self.mainLayout.update()    # mainLayout has been removed

        self.fig.canvas.draw()


    # Create sliders from table information
    #
    def createSliders(self):
            # Slider to access time axis range of solver statistics
            self.solverQuery.getTimeSlots()
            self.solverQuery.getFreqs()

            # tracking is disabled, the slider emits the valueChanged() signal only when the user releases the slider.
            self.timeStartSlider=QSlider(Qt.Horizontal)
            self.timeStartSliderLabel = QLabel("S:")
            self.timeStartSlider.setTracking(False)
            starttime=self.solverQuery.timeSlots[0]['STARTTIME']              # read first STARTTIME
            self.timeStartSliderLabel.setText("S:" +  str(starttime) + " s")  # initialize StartTimeLabel with it
            self.timeStartSlider.setMaximumWidth(170)

            self.timeEndSlider=QSlider(Qt.Horizontal)
            self.timeEndSliderLabel = QLabel("E:")
            self.timeEndSlider.setTracking(False)
            endtime=self.solverQuery.timeSlots[0]['ENDTIME']              # read first ENDTIME
            self.timeEndSliderLabel.setText("E:" +  str(endtime) + " s")  # initialize EndTimeLabel with it
            self.timeEndSlider.setMaximumWidth(170)

            self.frequencySlider=QSlider(Qt.Horizontal)
            startfreq=self.solverQuery.frequencies[0]['STARTFREQ']
            self.frequencySlider.setTracking(False)
            self.frequencySliderLabel = QLabel("Freq: " + str(startfreq) + " Hz")
            self.frequencySlider.setMaximumWidth(170)

            self.timeStartSlider.setRange(0, self.solverQuery.getNumTimeSlots()-1)
            self.connect(self.timeStartSlider, SIGNAL('valueChanged(int)'), self.on_timeStartSlider) 
            self.timeEndSlider.setRange(0, self.solverQuery.getNumTimeSlots()-1)
            self.connect(self.timeEndSlider, SIGNAL('valueChanged(int)'), self.on_timeEndSlider) 

            self.frequencySlider.setRange(0, self.solverQuery.getNumFreqs()-1)
            self.connect(self.frequencySlider, SIGNAL('valueChanged(int)'), self.on_frequencySlider)

            # Get solver parameter names from table
            self.parameters=self.solverQuery.getParameterNames()

            # Add widgets to buttonLayout (these are now class attributes to make them accessible)
            # Only add sliders and their labels to the buttonLayout if it is not there yet:
            self.buttonsLayout.addWidget(self.timeStartSliderLabel)
            self.buttonsLayout.addWidget(self.timeEndSliderLabel)
            self.buttonsLayout.addWidget(self.timeStartSlider)
            self.buttonsLayout.addWidget(self.timeEndSlider)
            self.buttonsLayout.addWidget(self.frequencySliderLabel)
            self.buttonsLayout.addWidget(self.frequencySlider)

            self.buttonsLayout.setSizeConstraint(1)


    # Create drop down menu with solver parameters to choose
    # from
    #
    def create_solver_dropdown(self, solver):
        # Get parameter names from solver table
        parameterNames=self.solverQuery.getParameterNames()
        self.parametersComboBox=QComboBox()
        self.parametersComboBox.setMaximumWidth(170)

        for p in parameterNames:
            self.parametersComboBox.addItem(p)

        # Finally add the ComboBox to the buttonsLayout
        self.buttonsLayout.addWidget(self.parametersComboBox)


    # Create a drop down menu that contains the different parameters that BBS
    # was used to solve for; if no parmDB was given, these will only be numbers
    # 1 to rank
    #
    def create_parms_dropdown(self):
        self.parmsComboBox=QComboBox()
        self.parmsComboBox.setMaximumWidth(170)


        # If no parmDB is given, load the parameters from the solver Table
        # and can refer to them only by number (which is in a physical interpretation
        # not really useful)
        if self.parmDB==None:
            rank=self.solverQuery.getRank()   # get the rank, i.e. the number of solved parameters for
            for item in range(1, rank+1):
                self.parmsComboBox.addItem(str(item))

        # if we have been given a parmDB, get parameters and their real names from the parmDB
        else:
            # Get parameters from parmDB
            #names = self.parmDB.getNames()
            parmnames=self.populate()    # now use Joris' populate function that reads from the parmDB
            for parm in parmnames:
                #print "create_parms_dropdown(): parm: ", parm
                # Sort them into ComboBox dropdown menu
                self.parmsComboBox.addItem(parm)

        self.connect(self.parmsComboBox, SIGNAL('currentIndexChanged(int)'), self.on_solution_changed)
        self.buttonsLayout.addWidget(self.parmsComboBox)

        self.parmsComboBox.hide()                          # by default it is not visible, only if solutions are also plotted



    # Create a drop down menu to choose which physical value
    # of a parameter is being plotted (e.g. Amplitude / Phase)
    #
    # TODO: This is at the moment hard coded to Amplitude & Phase
    #
    def create_parms_value_dropdown(self):
        print "create_parms_value_dropdown()"   # DEBUG

        self.parmValueComboBox=QComboBox()
        self.parmValueComboBox.setMaximumWidth(170)

        # Make this a bit more intelligently, decide depending on parameter
        # names what physical value is contained in them
        parameter=str(self.parmsComboBox.currentText())   # need to convert QString to python string

        if parameter.find("Gain") is not -1:
            self.parmValueComboBox.addItem("Amplitude")
            self.parmValueComboBox.addItem("Phase")

        self.buttonsLayout.addWidget(self.parmValueComboBox)
        self.parmValueComboBox.hide()



    # Create a status bar at the bottom of the MainWindow
    #
    def create_status_bar(self):
        print "create_status_bar()"
        self.status_text = QLabel("Solver statistics")
        self.statusBar().addWidget(self.status_text, 1)



    #******************************************************
    #
    # Checkbox handler functions
    #
    #******************************************************


    # Redraw the plot with the current parameters
    #
    def on_draw(self):
        self.fig.clf()      # Clear the figure
        #self.delAllAxes()

        scatter=self.scatterCheckBox.isChecked()
        
        print "on_draw(): self.perIteration = ", self.perIteration    # DEBUG

        solclf=self.clf
        newfig=self.newCheckBox.isChecked()

        print "solclf = ", solclf   # DEBUG
        print "newfig = ", newfig   # DEBUG

        # Solver parameter
        parameter=str(self.parametersComboBox.currentText())
        
        if self.solutions_plot == True:
            self.plotSolutions(self.fig, clf=solclf, scatter=scatter, periteration=self.perIteration)
            
        self.plot_parameter(parameter)


    # Set clear figure attribute self.clf depending on state
    # of clfCeckBox
    #
    def on_clf(self):
        self.clf=self.clfCheckBox.isChecked()


    # Set new figure attribute to generate another plot in a
    # new figure window
    #
    def on_newFigure(self):
        self.newfigure=self.newCheckBox.isChecked()
    

    # Set class attribute when showIterationsCheckBox is clicked
    # and changes its state
    #
    def on_showIterations(self):
        self.perIteration=self.showIterationsCheckBox.isChecked()

        # If periteration has been enabled, set everything to singleCell mode
        if self.perIteration:
            self.timeEndSlider.setValue(self.timeStartSlider.sliderPosition())
            self.syncSliders()
            self.timeEndSlider.emit(SIGNAL('valueChanged()'))
            self.showIterationsCheckBox.setEnabled(True)
            self.singleCellCheckBox.setCheckState(True)
            self.singleCellCheckBox.setTristate(False)             # we seem to need these to have "normal" CheckBoxes

    # Determine the table type PERSOLUTION, PERITERATION or
    # PERSOLUTION_CORRMATRIX or PERITERATION_CORRMATRIX
    #
    def determineTableType(self):
        # Decide on type which plotting to do (PERSOLUTION,PERITERATION, with or without CORRMATRIX)
        self.tableType=self.solverQuery.getType() 
        if self.tableType == "PERSOLUTION" or self.tableType == "PERSOLUTION_CORRMATRIX":
            self.perIteration=False
        elif self.tableType == "PERITERATION" or self.tableType == "PERITERATION_CORRMATRIX":
            self.perIteration=True


    #******************************************************
    #
    # Plotting helper functions
    #
    #******************************************************


    # Plotting function that does the actual plotting on the canvas
    #
    # Parameters are passed on as dictionaries with entries "result" = false, last, all, iteration
    # and the corresponding entry "last", "all", or "iteration" containing the actual result as
    # a numpy.ndarray
    #
    def plot_parameter(self, parameter): 
        # Get time and frequency intervals from the QWidgets
        start_time=self.solverQuery.timeSlots[self.timeStartSlider.value()]['STARTTIME']
        end_time=self.solverQuery.timeSlots[self.timeEndSlider.value()]['ENDTIME']
        start_freq=self.solverQuery.frequencies[self.frequencySlider.value()]['STARTFREQ']
        end_freq=self.solverQuery.frequencies[self.frequencySlider.value()]['ENDFREQ']


        # Picking the solution that has been choosen in the solved parameters comboBox
        solutionIndex=self.parmsComboBox.currentIndex()


        # Determine plot options
        scatter=self.scatterCheckBox.isChecked()

        if self.singleCellCheckBox.isChecked() == True or self.timeStartSlider.value() == self.timeEndSlider.value():
            singleCell=True
        elif self.singleCellCheckBox.isChecked() == False and self.timeStartSlider.value() != self.timeEndSlider.value():
            singleCell=False

        if self.solutions_plot==True:   # If solutions plot is also to be shown...
            parsub=212                  # ... plot parameters at the bottom with solutions plot on top
        else:                           # otherwise...
            parsub=111                  # plot them as single subplot


        # Decide on type which plotting to do (PERSOLUTION,PERITERATION, with or without CORRMATRIX)
        #tableType=self.solverQuery.getType() 
        #if tableType == "PERSOLUTION" or tableType == "PERSOLUTION_CORRMATRIX":
        #    perIteration=False
        #elif tableType == "PERITERATION" or tableType == "PERITERATION_CORRMATRIX":
        #    perIteration=True

        periteration=self.perIteration        # get a local copy from the class attribute (TODO: change this to use self.perIteration)

        print "plot_parameter(): periteration = ", periteration   # DEBUG

        clffig=self.clfCheckBox.isChecked()   # clear the figure before plotting new curve

        # DEBUG
        #print "plot_parameter(): start_time = ", start_time
        #print "plot_parameter(): end_time = ", end_time
        #print "plot_parameter(): start_freq = ", start_freq
        #print "plot_parameter(): end_freq = ", end_freq
        #print "--------------------------------------------"
        #print "parameter = ", parameter
        #print "type = ", tableType
        #print "scatter = ", scatter
        #print "singleCell = ", singleCell
        #print "timeInterval = ", end_time - start_time
        #print "parsub = ", parsub

        # We have to distinguish between a single solver value (from just one solution cell)
        # and the case we were given a series of values within an interval

        # There are two generic plotting cases:
        # (1) Only a single cell is displayed, then the plot
        #     displays the solver parameters per iteration (if present in the table)
        # (2) If the time sliders specify a time interval
        #     then the trend of the selected solver parameter over time is plotted
        #
        # ----------------------------------------
        # (1) Plotting a single cell (if there, with iterations)
        if singleCell==True:
            
            # If we only plot per solution
            if periteration == False:
                # If we also want to plot the solutions
                if self.solutions_plot==True:
                    self.plotSolutions(self.fig, scatter=scatter, clf=self.clf, periteration=False)

                x = self.solverQuery.getMidTimes()                

                # Get data from table per solutions
                # Check if special parameter is asked for, e.g. getSolution
                if parameter == "SOLUTION":
                    print "plot_parameter(): SOLUTION"               # DEBUG
                
                    y=self.solverQuery.getSolution(start_time, end_time, start_freq, end_freq)
                    #print "type(y).__name__ : ", type(y).__name__    # DEBUG
                    #print "x: ", x                                   # DEBUG
                    #print "y: ". y                                   # DEBUG

                    # This then calls Joris' plot function
                    self.plot(self.fig, y["last"], x, sub=parsub, scatter=scatter, clf=self.clf)

                elif parameter == "CORRMATRIX":
                    print "plot_parameter(): CORRMATRIX"             # DEBUG
                    corrmatrix=self.solverQuery.getCorrMatrix(start_time, end_time, start_freq, end_freq, iteration="last")
                    rank=self.solverQuery.getRank()
                    self.plotCorrMatrix(self.fig, corrmatrix, rank)

                # "Normal parameter"
                else:     
                    y=self.solverQuery.readParameter(parameter, start_time, end_time, start_freq, end_freq)
                    x=self.solverQuery.getMidTimes(start_time, end_time)

                    self.plot(self.fig, y["last"], x, sub=parsub, scatter=scatter, clf=self.clf)


            # If we plot a single solution per iteration
            elif periteration == True:  

                # If we also want to plot the solutions
                if self.solutions_plot==True:
                    self.plotSolutions(self.fig, scatter=scatter, clf=self.clf, periteration=True)

                y=self.solverQuery.readParameter(parameter, start_time, end_time, start_freq, end_freq, iteration='all')
                
                #print "plot_parameter(): y = ", y    # DEBUG

                # Set x to go from iteration 1 to the last one found in the dictionary for y
                x = range(1, len(y)-1)
                y = self.rearrangeIteration(y) 
                self.plot(self.fig, y, x, sub=parsub, scatter=scatter, clf=self.clf)


            self.canvas.draw()   # redraw canvas


        # (2) Plotting a range of values over a time interval
        elif singleCell==False:
            print "plot(): plotting a time interval from time_start till time_end"  # DEBUG

            x=self.solverQuery.getMidTimes(start_time, end_time)
            
            # Get data from table per iterations
            # Check if special parameter is asked for, e.g. getSolution
            if parameter == "SOLUTION":
                print "plot_parameter(): SOLUTION"               # DEBUG

                y=self.solverQuery.getSolution(start_time, end_time, start_freq, end_freq)
                
                # This then calls Joris' plot function
                self.plot(self.fig, y["last"], x, sub=parsub, scatter=scatter, clf=self.clf)

            elif parameter == "CORRMATRIX":
                print "plot_parameter(): CORRMATRIX"             # DEBUG
                corrmatrix=self.solverQuery.getCorrMatrix(start_time, end_time, start_freq, end_freq, iteration="last")
                # TODO!
                # Reorder in a 2D-Array? or just plot line-wise on canvas?
                # To be done in plotCorrMatrix() function
                self.plotCorrMatrix(corrmatrix, rank=24)

            # "Normal parameter"
            else:
                y=self.solverQuery.readParameter(parameter, start_time, end_time, start_freq, end_freq)
                x=self.solverQuery.getMidTimes(start_time, end_time)

                self.plot(self.fig, y['last'], x, sub=parsub, scatter=scatter, clf=self.clf)

            # If we also want to plot the solutions
            if self.solutions_plot==True:
                self.plotSolutions(self.fig, scatter=scatter, clf=self.clf, periteration=periteration)

            #if self.solutions_plot==True:
            #    solutions=self.solverQuery.getSolution(start_time, end_time, start_freq, end_freq, iteration='last')
            #    # Pick particular solution from the solutions
            #    z=self.solverQuery.selectSolution(solutions, solutionIndex)
            #    self.plot(self.fig, z, x, sub=211, scatter=scatter)
            #else:
            #    y=y['last']
            #    self.plot(self.fig, y, x, sub=212, scatter=scatter)


            #if start_time != end_time:
            #    self.ParameterSubplot.set_xlim(start_time, end_time)
            #    #self.SolutionsSubplot.set_xlim(start_time, end_time)
            #else:
            #    self.ParameterSubplot.set_xlim(start_time-5, end_time+5)
            #    #self.SolutionsSubplot.set_xlim(start_time, end_time)
 
            # This then calls Joris' plot function
            #self.plot(self.fig, y["last"], x, sub=sub, scatter=scatter)
            
            self.canvas.draw()   # redraw canvas

        else:
            print "plot_parameter(): can't plot with these options"


    # Plot solutions plot in a subplot 211, it uses the same xaxis as
    # the parameters plot
    #
    # fig          - plot into this figure object (instead of default)
    # periteration - plot improvement of a solution per iteration
    # scatter      - determines if we plot a scatter plot (default False)
    # clf          - clear the figure (default=False)
    #
    def plotSolutions(self, fig=None, periteration=True, scatter=False, clf=True):
        #print "plotSolutions(): "   # DEBUG

        solsub=211 # the solutions are plotted into the top subplot

        # Get time and frequency intervals from the QWidgets
        start_time=self.solverQuery.timeSlots[self.timeStartSlider.value()]['STARTTIME']
        end_time=self.solverQuery.timeSlots[self.timeEndSlider.value()]['ENDTIME']
        start_freq=self.solverQuery.frequencies[self.frequencySlider.value()]['STARTFREQ']
        end_freq=self.solverQuery.frequencies[self.frequencySlider.value()]['ENDFREQ']

        solutions_array=[]

        if periteration == True:
            solutions=self.solverQuery.getSolution(start_time, end_time, start_freq, end_freq, iteration='all')

            for iter in range(1, len(solutions)):
                solutions_array.append(solutions[iter])
        else:
            solutions=self.solverQuery.getSolution(start_time, end_time, start_freq, end_freq, iteration='last')
            for iter in range(0, len(solutions['last'])):
                solutions_array.append(solutions['last'][iter])

        #print "len(solutions_array) = ", len(solutions_array)   # DEBUG
        #print "solutions_array = ", solutions_array             # DEBUG

        # Picking the solution that has been choosen in the solved parameters comboBox
        solutionIndex=self.parmsComboBox.currentIndex()
        parameter=self.parmsComboBox.currentText()
        physValue=self.parmValueComboBox.currentText()


        # Read particular solution from the solutions
        if periteration == True:
            #solution=self.solverQuery.selectSolution(solutions, solutionIndex, result='all')
            x=range(1, len(solutions)+1)
        else:
            #solution=self.solverQuery.selectSolution(solutions, solutionIndex, result='last')
            x=self.solverQuery.getMidTimes(start_time, end_time)
 
        #solution=solution[solutionIndex]
        #print "plotSolutions():  x = ", x                  # DEBUG
        #print "plotSolutions(): solution = ", solution    # DEBUG

        #x, solution=self.cutMinLength(x, solution)    # DEBUG old
        
        print "physValue: ", physValue
        if physValue == "Amplitude":
            solution=self.computeAmplitude(parameter, solutions_array)
        elif physValue == "Phase":
            solution=self.computePhase(parameter, solutions_array)

        x, solution=self.cutMinLength(x, solution)

        #self.plot(fig, solution, x, sub=solsub, scatter=scatter, clf=self.clf)  # modified Joris' plot function
        self.plot(fig, solution, x, sub=solsub, scatter=scatter, clf=self.clf)  # modified Joris' plot function


    # Plot a corrmatrix on the lower subplot
    # fig - figure to plot on
    # sub - if a subplot is given plot in subplot
    # corrmatrix - (linear) array with correlation matrix to plot
    #
    def plotCorrMatrix(self, fig, corrmatrix, sub=None, rank=None, start_time=None, end_time=None, start_freq=None, end_freq=None):
        print "plotCorrMatrix():"                       # DEBUG

        print "type(corrmatrix): ", type(corrmatrix)    # DEBUG

        # If rank=None then try to read from the solutions table
        if rank == None:
            if start_time == None or end_time == None or start_freq == None or end_freq == None:
                print "plotCorrMatrix(): no valid time/freq cell given"
                return False
            else:
                rank=self.readParameter("RANK", start_time, end_time, start_freq, end_freq)

        if isinstance(corrmatrix, pyrap.table.table):
            print "plotCorrMatrix(): pyrap.table.table"

            # Read pyrap table into a 2D array
            corrmatrix2D
            
        elif isinstance(corrmatrix, numpy.ndarray):
            print "len(corrmatrix): ", len(corrmatrix)

            # Reshape the 1-D array into a 2-D matrix
            corrmatrix.shape=(rank, rank)  # reaarange into a 2-D array

            print "plotCorrMatrix(): corrmatrix=", corrmatrix    # DEBUG

            if sub is not None:  # if subplot was specified
                self.ParameterSubplot=fig.add_subplot(sub, autoscaleon=True)

            #axes = fig.gca()
            #axes.set_title("Corr Matrix")
            #axes.set_xlabel("ME parameters")
            #axes.set_ylabel("ME parameters")

            # Plot array as "colour map"
            im = a.imshow(data.getNumpyArray(), interpolation=None, cmap = corrmatrix.getCustomColorMap())

        else:
            print "plotCorrMatrix(): can not print unknown corrmatrix of type ", type(corrmatrix)
            return False


    # Modified Joris'plot function
    #
    def plot(self, fig, y, x=None, axes=None, clf=False, sub=None, scatter=False, stack=False,
        sep=5.0, sep_abs=False, labels=None, show_legend=False, title=None,
        xlabel=None, ylabel=None):
        """
        Plot a list of signals.

        If 'fig' is equal to None, a new figure will be created. Otherwise, the
        specified figure number is used. The 'sub' argument can be used to create
        subplots.

        The 'scatter' argument selects between scatter and line plots.

        The 'stack', 'sep', and 'sep_abs' arguments can be used to control placement
        of the plots in the list. If 'stack' is set to True, each plot will be
        offset by the mean plus sep times the standard deviation of the previous
        plot. If 'sep_abs' is set to True, 'sep' is used as is.

        The 'labels' argument can be set to a list of labels and 'show_legend' can
        be set to True to show a legend inside the plot.

        The figure number of the figure used to plot in is returned.
        """
        #global __styles
        tplotStart=time.time()

        #axes = fig.gca(autoscale_on=True)

        axes=pl.subplot(sub, autoscale_on=True, sharex=self.xaxis)


        if not title is None:
            axes.set_title(title)
        if not xlabel is None:
            axes.set_xlabel(xlabel)
        if not ylabel is None:
            axes.set_ylabel(ylabel)

        if x is None:
            x = [range(len(yi)) for yi in y]

        if clf:              # If a clear figure was sent, too
            #print "plot(): clearing figure"
            fig=pl.gcf()
            pl.cla()


        # DEBUG
        #print "plot(): self.fig = ", self.fig
        #print "plot(): fig = ", fig
        #print "type(sub) = ", type(sub)
        #print "sub = ", sub
        #print "self.clf = ", self.clf
        #print "min(x): ", min(x)
        #print "max(x): ", max(x)
        #axes.set_xlim(min(x), max(x))
        #self.ParameterSubplot.set_xlim(min(x), max(x))

        #fig.add_subplot(sub, autoscale_on=True)
        
        #if sub != None:
        #    print "plot(): adding subplot %", sub # DEBUG
        #    #fig.add_subplot(sub)

        # Depending on which sub (i.e. ParameterSubplot or SolutionsSubplot) was supplied
        #if sub is not None and sub == 111 or sub==212:  # if no subplot was specified
        #     print "we got a parameter plot"  # DEBUG
        #     #self.ParameterSubplot=fig.add_subplot(sub, autoscale_on=True)
        #     self.ParameterSubplot=fig.add_subplot(sub, autoscale_on=True)
        #elif sub is not None and sub == 211:
        #     print "we got a solution plot"  # DEBUG
        #     #self.SolutionsSubplot=fig.add_subplot(sub, autoscale_on=True)
        #     self.SolutionsSubplot=fig.add_subplot(sub, sharex=self.ParameterSubplot, autoscale_on=True)


        # If only one plot-set was provided
        if isinstance(y, np.int32) or isinstance(y, int) or isinstance(y, float) or (isinstance(y, np.ndarray) and len(y)==1) or (isinstance(y, list) and len(y)==1):

            if scatter:
                axes.scatter(x, y, edgecolors="None",
                             c=self.__styles[0][0], marker="o")
            else:
                axes.plot(x, y, marker="o")
        elif len(y) > 1:  # or if more than one plot in the set was provided
    
            # Really dirty hack: midTimes and Parameters differ often in size by 1
            # TODO: FIX this in solverQuery, most likely readParamter()
            # for the time being use common length

            length=min(len(x), len(y))
            x=x[0:length]   # drop remainder of x
            y=y[0:length]   # drop remainder of y

                    
            if labels is None:
                if scatter:
                    axes.scatter(x, y, edgecolors="None",
                        c=self.__styles[len(self.__styles)-1][0], marker="o")
                else:
                    axes.plot(x, y , marker="o")
            else:
                if scatter:
                    axes.scatter(x, y, edgecolors="None",
                        c=__styles[len(self.__styles)][0], marker="o",
                        label=labels[0])
                else:
                    axes.plot(x, y, label=labels[0])

        if not labels is None and show_legend:
            axes.legend(prop=FontProperties(size="x-small"), markerscale=0.5)

        self.fig.canvas.draw()  # "redraw" figure

        tplotEnd=time.time()   # take final time after redrawing of the canvas

        tplotTime=tplotEnd-tplotStart

        print "plot(): plotting took %6.2f ms" % (tplotTime*1000)


    # Cut two arrays x,y to the length of the smaller one
    # 
    def cutMinLength(self, x, y):
        length=min(len(x), len(y))
        x=x[0:length]   # drop remainder of x
        y=y[0:length]   # drop remainder of y

        return x,y


    # Plot a vertical line marker on the solutions plot showing the solution
    # of the currently plotted solver parameters
    #
    #
    def update_plotMarker(self):
        print "plotMarker()"

        # get current cell
        if self.currentSolution != None:
            axvline()


    # TODO
    # Get the current solution from a mouse click in the solutions plot
    # this will be marked with the plotMarker
    #
    def getCurrentSolution(self):
        print "getCurrentSolution()"
        
        return self.currentSolution


    #*****************************************************
    # 
    # Helper functions
    #
    #*****************************************************

    
    # Rearrange a dictionary of iterations returned by SolverQuery
    # into an array that then can be plotted
    # Each iteration no. is a dictionary keyword
    #
    def rearrangeIteration(self, parameterDict):
        #print "rearrangeIteration()"    # DEBUG

        if isinstance(parameterDict, dict) == False:
            raise "plotIterations(): parameterDict is not a dictionary"

        y=[]    # list to hold individual iteration results
        length=len(parameterDict)   # the first entry is "result" giving the type of dictionary

        for i in range(1, length):
            y.append(parameterDict[i])   # read from dictionary and rearrange (starting with index 0)

        return y


    # Delete all axes in a figure, figure defaults to None
    # then self.fig is used
    #
    def delAllAxes(self):
        for ax in self.fig.axes:
            print "delAllAxes(): deleting ", ax    # DEBUG
            ax.delaxes()

    
    # DEBUG function
    def printAllAxes(self):
        i=0
        for ax in self.fig.axes:
            i=i+1
            print "printAllAxes(): ax(%2d) = %s" % (i, ax)


    # Set the title for this figure
    #
    def setTitle(self, title=""):
        print "setTitle(): "   # DEBUG
        self.fig.set_title(title)


    #*****************************************************
    # 
    # Export functions
    #
    #*****************************************************

    # Export the plot to an ASCII text file
    #
    # subplot - subplot to export to ASCII
    # filename - name of file to write to
    #
    def export_plot(self, subplot, filename):
        print "export_plot()"

        fh=open(filename, 'w')   # open "filename" for writing

        if fh == 0:   # If we did not get a file handle
            print "export_plot() could not open file ", filename, " for writing."
            return False
        else:
            # Get currently displayed data points from Matplotlib

            # Loop over data points

            # write them to the file
            data=str(x) + "\t" + str(y) + "\n"
            fh.write(data)


#*****************************************************
#
# parmDB functions
#
#*****************************************************

    # Populate the parameter selection (menu) with available
    # parms from the parmDB stored in the casa subtable
    #
    def populate(self):
        for parm in self.parmDB.getNames():

            split = parm.split(":")

            if contains(split, "Real") or contains(split, "Imag"):
                if contains(split, "Real"):
                    idx = split.index("Real")
                    split[idx] = "Imag"
                    elements = [parm, ":".join(split)]
                else:
                    idx = split.index("Imag")
                    split[idx] = "Real"
                    elements = [":".join(split), parm]

                split.pop(idx)
                name = ":".join(split)

                found = False
                for i in range(len(self.parms)):
                    if self.parms[i].name() == name and not self.parms[i].isPolar():
                        found = True
                        break

                if not found:
                    self.parms.append(Parm(self.parmDB, name, elements))
            elif contains(split, "Ampl") or contains(split, "Phase"):
                if contains(split, "Ampl"):
                    idx = split.index("Ampl")
                    split[idx] = "Phase"
                    elements = [parm, ":".join(split)]
                else:
                    idx = split.index("Phase")
                    split[idx] = "Ampl"
                    elements = [":".join(split), parm]

                split.pop(idx)
                name = ":".join(split)

                found = False
                for i in range(len(self.parms)):
                    if self.parms[i].name() == name and self.parms[i].isPolar():
                        found = True
                        break

                if not found:
                    self.parms.append(Parm(self.parmDB, name, elements, True))
            else:
                self.parms.append(Parm(self.parmDB, parm))

        self.parms = [parm for parm in self.parms if not parm.empty()]
        self.parms.sort(cmp=lambda x, y: cmp(x.name(), y.name()))

        #domain = common_domain(self.parms)
        #if not domain is None:
        #    self.resolution[0].setText("%.6f" % ((domain[1] - domain[0]) / 100.0))
        #    self.resolution[1].setText("%.6f" % ((domain[3] - domain[2]) / 100.0))

        parameters=[]

        i=0
        for parm in self.parms:
            name = parm.name()

            if parm.isPolar():
                name = "%s (polar)" % name

            parameters.append(name)
            self.parameters.append(name)      # also store them in the class attribute
            self.parmMap[name]=(i,i+1)
            i=i+2

            #QListWidgetItem(name, self.list)

        #print "parameters = ", parameters # DEBUG
        #print "self.parmMap = ", self.parmMap  # DEBUG

        return parameters


    # Joris "Fetch value" function
    #
    def __fetch_value(self, name, domain=None, resolution=None):
        if domain is None:
            tmp = self._db.getValuesGrid(name)[name]
        else:
            if resolution is None:
                tmp = self._db.getValues(name, domain[0], domain[1], domain[2], domain[3])[name]
            else:
                tmp = self._db.getValuesStep(name, domain[0], domain[1], resolution[0], domain[2], domain[3], resolution[1])[name]

        if type(tmp) is dict:
            return tmp["values"]

        # Old parmdb interface.
        print "tmp = ", tmp  # DEBUG

        return tmp


    # Create parmMap that maps parameter name to indices in solution
    # array
    #
    def createParmMap(self):
        print "createParmMap()"   # DEBUG


    # Unwrap phase
    #
    def unwrap(phase, tol=0.25, delta_tol=0.25):
        """
        Unwrap phase by restricting phase[n] to fall within a range [-tol, tol]
        around phase[n - 1].

        If this is impossible, the closest phase (modulo 2*pi) is used and tol is
        increased by delta_tol (tol is capped at pi).
        """

        assert(tol < math.pi)

        # Allocate result.
        out = numpy.zeros(phase.shape)

        # Effective tolerance.
        eff_tol = tol

        ref = phase[0]
        for i in range(0, len(phase)):
            delta = math.fmod(phase[i] - ref, 2.0 * math.pi)

            if delta < -math.pi:
                delta += 2.0 * math.pi
            elif delta > math.pi:
                delta -= 2.0 * math.pi

            out[i] = ref + delta

            if abs(delta) <= eff_tol:
                # Update reference phase and reset effective tolerance.
                ref = out[i]
                eff_tol = tol
            elif eff_tol < math.pi:
                # Increase effective tolerance.
                eff_tol += delta_tol * tol
                if eff_tol > math.pi:
                    eff_tol = math.pi

        return out

    
    # Unwrap phase, windowed mode
    #
    def unwrap_windowed(phase, window_size=5):
        """
        Unwrap phase by estimating the trend of the phase signal.
        """

        # Allocate result.
        out = numpy.zeros(phase.shape)

        windowl = numpy.array([math.fmod(phase[0], 2.0 * math.pi)] * window_size)

        delta = math.fmod(phase[1] - windowl[0], 2.0 * math.pi)
        if delta < -math.pi:
            delta += 2.0 * math.pi
        elif delta > math.pi:
            delta -= 2.0 * math.pi
        windowu = numpy.array([windowl[0] + delta] * window_size)

        out[0] = windowl[0]
        out[1] = windowu[0]

        meanl = windowl.mean()
        meanu = windowu.mean()
        slope = (meanu - meanl) / float(window_size)

        for i in range(2, len(phase)):
            ref = meanu + (1.0 + (float(window_size) - 1.0) / 2.0) * slope
            delta = math.fmod(phase[i] - ref, 2.0 * math.pi)

            if delta < -math.pi:
                delta += 2.0 * math.pi
            elif delta > math.pi:
                delta -= 2.0 * math.pi

            out[i] = ref + delta

            windowl[:-1] = windowl[1:]
            windowl[-1] = windowu[0]
            windowu[:-1] = windowu[1:]
            windowu[-1] = out[i]

            meanl = windowl.mean()
            meanu = windowu.mean()
            slope = (meanu - meanl) / float(window_size)

        return out

    
    # Normalize the phase
    #
    def normalize(phase):
        """
        Normalize phase to the range [-pi, pi].
        """

        # Convert to range [-2*pi, 2*pi].
        out = numpy.fmod(phase, 2.0 * numpy.pi)

        # Convert to range [-pi, pi]
        out[out < -numpy.pi] += 2.0 * numpy.pi
        out[out > numpy.pi] -= 2.0 * numpy.pi

        return out




    # Compute amplitude for parameter
    #
    def computeAmplitude(self, parameter, solutions):
        #print "computeAmplitude()"   # DEBUG
        #print "computeAmplitude(): parameter = ", parameter   # DEBUG

        parameter=str(parameter)

        amplitude=[]
        real_idx=self.parmMap[parameter][0]
        imag_idx=self.parmMap[parameter][1]

        print "real_idx = ", real_idx   # DEBUG
        print "imag_idx = ", imag_idx   # DEBUG
        print "type(solutions) = ", type(solutions)  # DEBUG

        # Decide on data type of solutions
        if isinstance(solutions, int):
            print "int"
            amplitude=math.sqrt(solutions[real_idx]^2 + solutions[imag_idx]^2)

        elif isinstance(solutions, np.ndarray) or isinstance(solutions, list):
            print "np.ndarray"    # DEBUG
            
            length=len(solutions)

            # compute amplitude for each entry and append to vector
            if length == 1:
                amplitude.append(math.sqrt(solutions[0][real_idx]**2 + solutions[0][imag_idx]**2))
            else:
                for iter in range(0, length):   # Loop over solutions
                    amplitude.append(math.sqrt(solutions[iter][real_idx]**2 + solutions[iter][imag_idx]**2))
        
        return amplitude


    # Compute phase for parameter
    #
    def computePhase(self, parameter, solutions):
        print "computePhase(): parameter = ", parameter   # DEBUG

        phase=[]

        parameter=str(parameter)   # convert QString to string
        real_idx=self.parmMap[parameter][0]
        imag_idx=self.parmMap[parameter][1]

        print "real_idx = ", real_idx   # DEBUG
        print "imag_idx = ", imag_idx   # DEBUG
        print "type(solutions) = ", type(solutions)  # DEBUG

        # Decide on data type of solutions
        if isinstance(solutions, int):
            print "int"
            amplitude=math.sqrt(solutions[real_idx]^2 + solutions[imag_idx]^2)

        elif isinstance(solutions, np.ndarray) or isinstance(solutions, list):
            print "np.ndarray"    # DEBUG
            
            length=len(solutions)

            # compute amplitude for each entry and append to vector
            if length == 1:
                phase.append(math.atan(solutions[0][imag_idx]/solutions[0][real_idx]))
            else:
                for iter in range(0, length):   # Loop over solutions
                    phase.append(math.atan(solutions[iter][imag_idx]/solutions[iter][real_idx]))
        

        #phase=math.atan(solutions[self.parms[parameter][1]]/solutions[self.parms[parameter][0]])

        return phase


# Joris' helper function
#
def contains(container, item):
    try:
        return container.index(item) >= 0
    except ValueError:
        return False





# Joris' high-level class for accessing the parmDB
#
#    
class Parm:
    def __init__(self, db, name, elements=None, isPolar=False):
        self._db = db
        self._name = name
        self._elements = elements
        self._isPolar = isPolar
        self._value = None
        self._value_domain = None
        self._value_resolution = None

        self._readDomain()

    def name(self):
        return self._name

    def isPolar(self):
        return self._isPolar

    def empty(self):
        return self._empty

    def domain(self):
        return self._domain

    def value(self, domain=None, resolution=None, asPolar=True, unwrap_phase=False):
        if self.empty():
            assert(False)
            return (numpy.zeros((1,1)), numpy.zeros((1,1)))

        if self._value is None or self._value_domain != domain or self._value_resolution != resolution:
            self._readValue(domain, resolution)

        if asPolar:
            if self.isPolar():
                ampl = self._value[0]
                phase = normalize(self._value[1])
            else:
                ampl = numpy.sqrt(numpy.power(self._value[0], 2) + numpy.power(self._value[1], 2))
                phase = numpy.arctan2(self._value[1], self._value[0])

            if unwrap_phase:
                for i in range(0, phase.shape[1]):
                    phase[:, i] = unwrap(phase[:, i])

            return (1.0/ampl, phase)

        if not self.isPolar():
            re = self._value[0]
            im = self._value[1]
        else:
            re = self._value[0] * numpy.cos(self._value[1])
            im = self._value[0] * numpy.sin(self._value[1])

        return (1.0/re, im)

    def _readDomain(self):
        if self._elements is None:
            self._domain = self._db.getRange(self.name())
        else:
            domain_el0 = self._db.getRange(self._elements[0])
            domain_el1 = self._db.getRange(self._elements[1])
            self._domain = [max(domain_el0[0], domain_el1[0]), min(domain_el0[1], domain_el1[1]), max(domain_el0[2], domain_el1[2]), min(domain_el0[3], domain_el1[3])]

        self._empty = (self._domain[0] >= self._domain[1]) or (self._domain[2] >= self._domain[3])

    def _readValue(self, domain=None, resolution=None):
#        print "fetching:", self.name()

        if self._elements is None:
            value = numpy.array(self.__fetch_value(self.name(), domain, resolution))
            self._value = (value, numpy.zeros(value.shape))
        else:
            el0 = numpy.array(self.__fetch_value(self._elements[0], domain, resolution))
            el1 = numpy.array(self.__fetch_value(self._elements[1], domain, resolution))
            self._value = (el0, el1)

        self._value_domain = domain
        self._value_resolution = resolution

    def __fetch_value(self, name, domain=None, resolution=None):
        if domain is None:
            tmp = self._db.getValuesGrid(name)[name]
        else:
            if resolution is None:
                tmp = self._db.getValues(name, domain[0], domain[1], domain[2], domain[3])[name]
            else:
                tmp = self._db.getValuesStep(name, domain[0], domain[1], resolution[0], domain[2], domain[3], resolution[1])[name]

        if type(tmp) is dict:
            return tmp["values"]

        # Old parmdb interface.
        return tmp


#****************************************************************
#
# Main function
#
#****************************************************************
#
def main():
    app = QApplication(sys.argv)
    form = SolverAppForm()

    # Strip solver-filename from sys.argv[0] and add "instrument" to it


    # for some reason this must be here, and can not be in create_main_frame()
    form.connect(form.quitButton, SIGNAL('clicked()'), app, SLOT('quit()'))

    form.show()

    # If there is a table given as command line argument, open that table
    if len(sys.argv) == 2 and sys.argv[1] is not "":
        tableName=sys.argv[1]
        form.open_table(tableName)
    # If a second argument is given, take this as the instrument parmDB
    if len(sys.argv) == 3 and sys.argv[2] is not "":
        print "sys.argv[2]: ", sys.argv[2]

        form.open_parmDB(sys.argv[2])
        form.open_table(sys.argv[1])

    app.exec_()


# Main entry function
#
if __name__ == "__main__":
    main()

