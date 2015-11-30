# #####################################################################
# Automatically generated by qmake (2.01a) Tue Mar 15 12:58:58 2011
# #####################################################################
TEMPLATE = lib
#TEMPLATE = app
TARGET = libScheduler
QT += core \
    gui \
    sql
CONFIG += debug_and_release
CONFIG += staticlib  # dynamic link: Problem with linking into app.
# This needs to be fixed with help of Marcel / JD

CONFIG(debug, debug|release) { 
    message(Building Makefile.Debug)
    DEFINES += _DEBUG_
    DESTDIR = debug
    OBJECTS_DIR = debug
    MOC_DIR = debug
    UI_DIR = debug
    RCC_DIR = debug
    INCLUDEPATH += . \
        debug
    DEPENDPATH += . \
        debug
}
else { 
    message(Building Makefile.Release)
    DEFINES += _RELEASE_
    DESTDIR = release
    OBJECTS_DIR = release
    MOC_DIR = release
    UI_DIR = release
    RCC_DIR = release
    INCLUDEPATH += . \
        release
    DEPENDPATH += . \
        release
}

# Input
HEADERS += blocksize.h \
    ListWidget.h \
    redistributetasksdialog.h \
    sasconnectdialog.h \
    DataTreeWidgetItem.h \
    shifttasksdialog.h \
    tiedarraybeamdialog.h \
    DigitalBeam.h \
    TiedArrayBeam.h \
    SpinBox.h \
    parsettreeviewer.h \
    FileUtils.h \
    Angle.h \
    astrodate.h \
    astrodatetime.h \
    astrotime.h \
    ComboBox.h \
    conflictdialog.h \
    Controller.h \
    DataHandler.h \
    DataMonitorConnection.h \
    dataslotdialog.h \
    DateEdit.h \
    DateTimeEdit.h \
    digitalbeamdialog.h \
    GraphicCurrentTimeLine.h \
    GraphicResourceScene.h \
    GraphicStationTaskLine.h \
    graphicstoragescene.h \
    GraphicStorageTimeLine.h \
    GraphicTask.h \
    GraphicTimeLine.h \
    LineEdit.h \
    longbaselinepipeline.h \
    lofar_scheduler.h \
    lofar_utils.h \
    neighboursolution.h \
    OTDBnode.h \
    OTDBtree.h \
    publishdialog.h \
    qlofardatamodel.h \
    SASConnection.h \
    sasprogressdialog.h \
    sasstatusdialog.h \
    sasuploaddialog.h \
    Scheduler.h \
    schedulerdata.h \
    schedulerdatablock.h \
    schedulergui.h \
    schedulersettings.h \
    schedulesettingsdialog.h \
    scheduletabledelegate.h \
    statehistorydialog.h \
    station.h \
    stationlistwidget.h \
    stationtreewidget.h \
    Storage.h \
    StorageNode.h \
    tablecolumnselectdialog.h \
    tableview.h \
    task.h \
    taskcopydialog.h \
    taskdialog.h \
    thrashbin.h \
    TimeEdit.h \
    doublespinbox.h \
    pipeline.h \
    pulsarpipeline.h \
    imagingpipeline.h \
    calibrationpipeline.h \
    observation.h \
    taskstorage.h \
    stationtask.h \
    storage_definitions.h \
    demixingsettings.h \
    CheckBox.h \
    schedulerLib.h \
    signalhandler.h
FORMS += \
    redistributetasksdialog.ui \
    sasconnectdialog.ui \
    shifttasksdialog.ui \
    tiedarraybeamdialog.ui \
    parsettreeviewer.ui \
    conflictdialog.ui \
    dataslotdialog.ui \
    digitalbeamdialog.ui \
    graphicstoragescene.ui \
    publishdialog.ui \
    sasprogressdialog.ui \
    sasstatusdialog.ui \
    sasuploaddialog.ui \
    schedulergui.ui \
    schedulesettingsdialog.ui \
    statehistorydialog.ui \
    stationlistwidget.ui \
    stationtreewidget.ui \
    tablecolumnselectdialog.ui \
    taskcopydialog.ui \
    taskdialog.ui \
    thrashbin.ui \
    storageresourceview.ui
SOURCES += \
    ListWidget.cpp \
    redistributetasksdialog.cpp \
    sasconnectdialog.cpp \
    DataTreeWidgetItem.cpp \
    shifttasksdialog.cpp \
    tiedarraybeamdialog.cpp \
    DigitalBeam.cpp \
    TiedArrayBeam.cpp \
    SpinBox.cpp \
    parsettreeviewer.cpp \
    FileUtils.cpp \
    Angle.cpp \
    astrodate.cpp \
    astrodatetime.cpp \
    astrotime.cpp \
    ComboBox.cpp \
    conflictdialog.cpp \
    Controller.cpp \
    DataHandler.cpp \
    DataMonitorConnection.cpp \
    dataslotdialog.cpp \
    DateEdit.cpp \
    DateTimeEdit.cpp \
    debug_lofar.cpp \
    digitalbeamdialog.cpp \
    GraphicCurrentTimeLine.cpp \
    GraphicResourceScene.cpp \
    GraphicStationTaskLine.cpp \
    graphicstoragescene.cpp \
    GraphicStorageTimeLine.cpp \
    GraphicTask.cpp \
    GraphicTimeLine.cpp \
    LineEdit.cpp \
    longbaselinepipeline.cpp \
    lofar_utils.cpp \
    main.cpp \
    neighboursolution.cpp \
    OTDBnode.cpp \
    OTDBtree.cpp \
    publishdialog.cpp \
    qlofardatamodel.cpp \
    SASConnection.cpp \
    sasprogressdialog.cpp \
    sasstatusdialog.cpp \
    sasuploaddialog.cpp \
    Scheduler.cpp \
    schedulerdata.cpp \
    schedulerdatablock.cpp \
    schedulergui.cpp \
    schedulersettings.cpp \
    schedulesettingsdialog.cpp \
    scheduletabledelegate.cpp \
    statehistorydialog.cpp \
    station.cpp \
    stationlistwidget.cpp \
    stationtreewidget.cpp \
    Storage.cpp \
    StorageNode.cpp \
    tablecolumnselectdialog.cpp \
    tableview.cpp \
    task.cpp \
    taskcopydialog.cpp \
    taskdialog.cpp \
    thrashbin.cpp \
    TimeEdit.cpp \
    doublespinbox.cpp \
    pipeline.cpp \
    pulsarpipeline.cpp \
    imagingpipeline.cpp \
    calibrationpipeline.cpp \
    observation.cpp \
    taskstorage.cpp \
    stationtask.cpp \
    demixingsettings.cpp \
    blocksize.cpp \
    CheckBox.cpp \
    schedulerLib.cpp \
    signalhandler.cpp
RESOURCES += scheduler_resources.qrc


