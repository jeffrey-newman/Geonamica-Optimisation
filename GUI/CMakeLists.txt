include_directories(${Boost_INCLUDE_DIRS} )
INCLUDE_DIRECTORIES(${PARALLEL-NSGAII})
include_directories(${MPI_CXX_INCLUDE_PATH})

# Define required for how hypervolume is calculated.
add_definitions(-DVARIANT=4)

IF (USE_QT)
    # Find includes in corresponding build directories
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTOUIC ON)
    set(CMAKE_AUTORCC ON)
    find_package(Qt5Gui)
    find_package(Qt5Charts)
    find_package(Qt5Widgets)
    find_package(Qt5Core)
    add_definitions(-DWITH_QT)

    SET(Geon_Optimiser_GUI_SRCS
            TimeSeriesQtChart.h
            TimeSeriesQtChart.cpp
            OptimiserController.cpp
            OptimiserController.h
            OptimiserWorker.cpp
            OptimiserWorker.h
            MetricQtSignal.cpp
            MetricQtSignal.h
            GeonamicaOptGUICheckpoints.cpp
            GeonamicaOptGUICheckpoints.h
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
            PathifyQt.cpp
            PathifyQt.hpp
            main.cpp
            )
ELSE(USE_QT)
    SET(Geon_Optimiser_GUI_SRCS
            main.cpp
            RandomVector.hpp )
ENDIF()

SET(UI_LINK_LIBS
        ${Boost_LIBRARIES} ${GDAL_LIBRARY}  ${MPI_LIBRARIES} ${PNGwriter_LIBRARIES} geonoptcommon ${OpenCV_LIBS})
IF (USE_QT)
    SET(UI_LINK_LIBS ${UI_LINK_LIBS} Qt5::Widgets Qt5::Gui Qt5::Charts)
ENDIF()

add_executable(geonopt ${Geon_Optimiser_GUI_SRCS} )
target_link_libraries(geonopt ${UI_LINK_LIBS} )

#target_compile_definitions(geonoptgui PUBLIC "-DMAGICKCORE_QUANTUM_DEPTH=16" PUBLIC "-DMAGICKCORE_HDRI_ENABLE=0")
#IF(USE_VTK)
#    if(VTK_LIBRARIES)
#        target_link_libraries(wdsoptgui ${VTK_LIBRARIES})
#    else()
#        target_link_libraries(wdsoptgui vtkHybrid vtkWidgets)
#    endif()
#ENDIF()