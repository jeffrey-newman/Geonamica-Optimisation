include_directories(${Boost_INCLUDE_DIRS} )
INCLUDE_DIRECTORIES(${PARALLEL-NSGAII})

# Define required for how hypervolume is calculated.
add_definitions(-DVARIANT=4)

IF (USE_QT)
    # Find includes in corresponding build directories
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTOUIC ON)
    set(CMAKE_AUTORCC ON)
    find_package(Qt5GUI)
    find_package(Qt5Charts)
    find_package(Qt5Widgets)
    find_package(Qt5Core)
    add_definitions(-DWITH_QT)
ENDIF()

SET(Geon_Optimiser_GUI_SRCS
        TimeSeriesQtChart.h
        TimeSeriesQtChart.cpp
        main.cpp
#        nsgaiirunningpage.cpp
#        nsgaiirunningpage.h
#        optimisationwizardpage.cpp
#        optimisationwizardpage.h
#        optimiserthread.cpp
#        optimiserthread.h
        OptimiserController.cpp
        OptimiserController.h
        OptimiserWorker.cpp
        OptimiserWorker.h
        MetricQtSignal.cpp
        MetricQtSignal.h
        GeonamicOptGUICheckpoints.cpp
        GeonamicaOptGUICheckpoints.h
#        nsgaiiguimainwindow.ui
#        optimisationwizardpage.ui
        ParetoFrontQtChart.cpp
        ParetoFrontQtChart.h
        FrontQtSignal.cpp
        FrontQtSignal.h
        QtDialogueCheckpoint.cpp
        QtDialogueCheckpoint.h
        MainWindow.cpp
        MainWindow.hpp
        mdi.qrc
        ConfigDialog.h
        ConfigDialog.cpp
        ConfigPages.cpp
        ConfigPages.h
        GeonamicaPolicyParametersQtMetaTyping.cpp
        GeonamicaPolicyParametersQtMetaTyping.hpp

)



add_executable(geonoptgui ${Geon_Optimiser_GUI_SRCS} )
target_link_libraries(geonoptgui ${Boost_LIBRARIES} ${GDAL_LIBRARY}  ${MPI_LIBRARIES} ${PNGwriter_LIBRARIES}
        ${MagickLibs} Qt5::Widgets Qt5::Gui Qt5::Charts geonoptcommon ${OpenCV_LIBS})
target_compile_definitions(geonoptgui PUBLIC "-DMAGICKCORE_QUANTUM_DEPTH=16" PUBLIC "-DMAGICKCORE_HDRI_ENABLE=0")
#IF(USE_VTK)
#    if(VTK_LIBRARIES)
#        target_link_libraries(wdsoptgui ${VTK_LIBRARIES})
#    else()
#        target_link_libraries(wdsoptgui vtkHybrid vtkWidgets)
#    endif()
#ENDIF()