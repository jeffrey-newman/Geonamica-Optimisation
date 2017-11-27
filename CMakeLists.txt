 
cmake_minimum_required(VERSION 3.0)
PROJECT(Geon-Opt)
set (CMAKE_CXX_STANDARD 11)
#Variable for location of parallel-nsgaII-backend

FIND_PACKAGE(Boost REQUIRED COMPONENTS filesystem system program_options mpi serialization timer chrono date_time)
find_package(GDAL)
find_package(MPI REQUIRED)
include_directories(${MPI_CXX_INCLUDE_PATH})
#find_package(ImageMagick COMPONENTS Magick++ MagickWand MagickCore)
 find_package( OpenCV REQUIRED )


#SET(USE_VTK CACHE BOOL "Set to on to use vtk visualisation")
#IF (USE_VTK)
#        find_package(VTK REQUIRED)
#                IF(VTK_FOUND)
#                include(${VTK_USE_FILE})
#                add_definitions(-DWITH_VTK)
#                ENDIF()
#ENDIF()

set(AHHZ_RASTER_UTIL "Not Specified" CACHE PATH "Path to Alex Hagen-Zanker's Raster Utility Include direction (select above blink)")
set(AHHZ_RASTER_ITERATOR "Not Specified" CACHE PATH "Path to Alex Hagen-Zanker's Raster Iterator Include direction (select folder above blink)")
set(PARALLEL-NSGAII "Not Specified" CACHE PATH "Path to Jeff's parallel NSGAII code")
set(RASTER2IMAGE "Not specified" CACHE PATH "Path to the Raster2Image code")
#set(MCL "Not Specified" CACHE PATH "Path to MCL code")
set(MAC_FRAMEWORK_PATHS "FALSE" CACHE BOOL "Whether to use header paths in include statements for mac gdal framework")

if(MAC_FRAMEWORK_PATHS)
    add_definitions(-DWITH_MAC_FRAMEWORK)
ENDIF()


# Find where Magick++-config lives
find_program(MAGICK_CONFIG "Magick++-config")
# Ask about CXX and lib flags/locations
execute_process(COMMAND "${MAGICK_CONFIG}" "--cxxflags" OUTPUT_VARIABLE MAGICK_CXX_FLAGS)
execute_process(COMMAND "${MAGICK_CONFIG}" "--libs" OUTPUT_VARIABLE MAGICK_LD_FLAGS)
#message(STATUS "${MAGICK_CXX_FLAGS}")
#message(STATUS "${MAGICK_LD_FLAGS}")
# Remove trailing whitespace (CMAKE warns about this)
string(STRIP "${MAGICK_CXX_FLAGS}" MAGICK_CXX_FLAGS)
string(STRIP "${MAGICK_LD_FLAGS}" MAGICK_LD_FLAGS)


# Define required for how hypervolume is calculated.
add_definitions(-DVARIANT=4)
# define which fks code to use.
add_definitions(-Dfast_fks)
add_definitions(-Dclump_fast)

include_directories(${GDAL_INCLUDE_DIR} )
include_directories(${Boost_INCLUDE_DIRS} )
INCLUDE_DIRECTORIES(${AHHZ_RASTER_UTIL} )
INCLUDE_DIRECTORIES(${AHHZ_RASTER_ITERATOR})
INCLUDE_DIRECTORIES(${PARALLEL-NSGAII})
include_directories("${CMAKE_SOURCE_DIR}/pugixml-1.7/src/")
include_directories(${ImageMagick_Magick++_INCLUDE_DIR})
include_directories(${RASTER2IMAGE})
#INCLUDE_DIRECTORIES(${MCL})

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  add_definitions(-DCLANG)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  add_definitions(-DGNU)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
  add_definitions(-DINTEL)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  add_definitions(-DMSVC)
endif()

# add path to FindPNGwriter.cmake, e.g. in the directory in cmake/
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/)

# find PNGwriter installation
#   optional: prefer static libraries over shared ones (but do not force them)
#set(PNGwriter_USE_STATIC_LIBS ON)

#   optional: specifiy (minimal) version / require to find it
#           (PNGwriter 0.5.6 REQUIRED)
#find_package(PNGwriter)

#if(PNGwriter_FOUND)
#  # where to find the pngwriter.h header file (-I include for your compiler)
#  include_directories(${PNGwriter_INCLUDE_DIRS})
#  # additional compiler flags (e.g. -DNO_FREETYPE)
#  add_definitions(${PNGwriter_DEFINITIONS})
#  # libraries to link against (including dependencies)
##  set(LIBS ${LIBS} ${PNGwriter_LIBRARIES})
#endif(PNGwriter_FOUND)

# add_executable(yourBinary ${SOURCES})
# ...
# target_link_libraries(yourBinary ${LIBS})

SET(MagickLibs
        ${ImageMagick_Magick++_LIBRARY}
        ${ImageMagick_MagickWand_LIBRARY}
        ${ImageMagick_MagickCore_LIBRARY}
        )


file(GLOB_RECURSE AHHZ_Raster
 "${AHHZ_RASTER_UTIL}/*.h"
 "${AHHZ_RASTER_UTIL}/*.hpp"
 "${AHHZ_RASTER_UTIL}/*.cpp" 
 "${AHHZ_RASTER_ITERATOR}/*.h" 
 "${AHHZ_RASTER_ITERATOR}/*.hpp" 
 "${AHHZ_RASTER_ITERATOR}/*.cpp"
 )


 SET(PUGIXML-SRC
        "./pugixml-1.7/src/pugiconfig.hpp"
        "./pugixml-1.7/src/pugixml.hpp"
        "./pugixml-1.7/src/pugixml.cpp"
 )

# SET(AHHZ_Raster_Utility
#    "${AHHZ_RASTER_UTIL}/blink/raster/Utility.cpp"
#    "${AHHZ_RASTER_UTIL}/blink/raster/gdal_raster_lru.cpp"
#    )

SET(prll-nsga-src
        "${PARALLEL-NSGAII}/Checkpoint.hpp"
        "${PARALLEL-NSGAII}/Comparator.hpp"
        "${PARALLEL-NSGAII}/Crossover.hpp"
        "${PARALLEL-NSGAII}/DebsCrowdingDistance.hpp"
        "${PARALLEL-NSGAII}/DebsNondominatedSorting.hpp"
        "${PARALLEL-NSGAII}/Evaluation.hpp"
        "${PARALLEL-NSGAII}/Individual.hpp"
        "${PARALLEL-NSGAII}/Merge.hpp"
        "${PARALLEL-NSGAII}/Mutation.hpp"
        "${PARALLEL-NSGAII}/NSGAII.hpp"
        "${PARALLEL-NSGAII}/ParallelEvaluator.hpp"
        "${PARALLEL-NSGAII}/Population.hpp"
        "${PARALLEL-NSGAII}/Population.hpp"
        "${PARALLEL-NSGAII}/ProblemDefinitions.hpp"
        "${PARALLEL-NSGAII}/Selection.hpp"
#        "${PARALLEL-NSGAII}/Types.hpp"

        "${PARALLEL-NSGAII}/Checkpoints/MaxGenCheckpoint.hpp"
        "${PARALLEL-NSGAII}/Checkpoints/SavePopCheckpoint.hpp"
        "${PARALLEL-NSGAII}/Checkpoints/SerialiseCheckpoint.hpp"
        "${PARALLEL-NSGAII}/Checkpoints/PlotFronts.hpp"
        "${PARALLEL-NSGAII}/Checkpoints/colours.hpp"
        "${PARALLEL-NSGAII}/Checkpoints/ResetMutationXoverFlags.hpp"
        "${PARALLEL-NSGAII}/Checkpoints/MailCheckpoint.hpp"
        "${PARALLEL-NSGAII}/Checkpoints/SaveFirstFrontCheckpoint.hpp"
        "${PARALLEL-NSGAII}/Checkpoints/SignalCheckpoint.hpp"
        "${PARALLEL-NSGAII}/Checkpoints/SignalHandler.cpp"
        "${PARALLEL-NSGAII}/Checkpoints/SignalHandler.hpp"
        "${PARALLEL-NSGAII}/Checkpoints/BoolTerminateCheckpoint.hpp"

        "${PARALLEL-NSGAII}/Serialization/SerializeBoostPath.hpp"
        "${PARALLEL-NSGAII}/Serialization/SerializeRng.hpp"

        "${PARALLEL-NSGAII}/Metrics/hv.c"
        "${PARALLEL-NSGAII}/Metrics/hv.h"
        "${PARALLEL-NSGAII}/Metrics/Hypervolume.hpp"
    )

#SET(Rst2ImgSrc
#        "${RASTER2IMAGE}/MagickWriterClassified.cpp"
#        "${RASTER2IMAGE}/MagickWriterClassified.h"
#        "${RASTER2IMAGE}/MagickWriterGradient.cpp"
#        "${RASTER2IMAGE}/MagickWriterGradient.h"
#    )


#SET (MCL_HEADERS
#        "${MCL}/MapComparison_0_4.h"
#                "${MCL}/Map_Matrix.h"
#                "${MCL}/ReadInMap.h"
#                "${MCL}/Types.h"
#                "${MCL}/RasterComparisonModule/KappaSimulation.h"
#                "${MCL}/RasterComparisonModule/Kappa.h"
#        "${MCL}/Neighbourhood.h"
#        "${MCL}/FuzzyKappaSimulation.h"
#        "${MCL}/FuzzyKappaSimulationImproved.hpp"
#        "${MCL}/FuzzyKappaSimulationImprovedFast.hpp"
#        "${MCL}/ReadFuzzyTables.h"
#        "${MCL}/dt.h"
#        "${MCL}/Clumpiness.h"
#        "${MCL}/ClumpinessFast.h"
#        )

#SET (MCL_SOURCES
#        "${MCL}/MapComparison_0_4.cpp"
#        "${MCL}/Map_Matrix.cpp"
#                "${MCL}/Map_Matrix_Impl.cpp"
##	   		ReadInMap.cpp
#                "${MCL}/ReadInMap_Impl.cpp"
#                "${MCL}/RasterComparisonModule/KappaSimulation.cpp"
#                "${MCL}/RasterComparisonModule/Kappa.cpp"
#        "${MCL}/Neighbourhood.cpp"
#        "${MCL}/Neighbourhood_Impl.cpp"
#        "${MCL}/FuzzyKappaSimulation.cpp"
#        "${MCL}/FuzzyKappaSimulationImproved.cpp"
#        "${MCL}/FuzzyKappaSimulationImprovedFast.cpp"
#        "${MCL}/Clumpiness.cpp"
#        "${MCL}/ClumpinessFast.cpp"
#        )



link_directories(${Boost_LIBRARY_DIRS})

SET(OptCommon_Srcs
        GeonamicaPolicyOptimiser.hpp
        GeonamicaPolicyOptimiser.cpp
        GeonamicaPolicyParameters.hpp
        GeonamicaPolicyParameters.cpp
        GeonamicaPolicyPostProcess.cpp
        GeonamicaPolicyPostProcess.hpp
        GeonamicaPolicyCheckpoints.cpp
        GeonamicaPolicyCheckpoints.hpp
        Pathify.hpp
        Pathify.cpp
        ${prll-nsga-src}
        ${MCL_HEADERS}
        ${AHHZ_Raster}
        ${AHHZ_Raster_Utility}
        ${MCL_SOURCES}
        ${PUGIXML-SRC}
        GeonamicaNSGAII.cpp
        GeonamicaNSGAII.hpp
   )

add_library(geonoptcommon STATIC ${OptCommon_Srcs})
# target_compile_definitions(geonoptcommon PUBLIC "-DMAGICKCORE_QUANTUM_DEPTH=16" PUBLIC "-DMAGICKCORE_HDRI_ENABLE=0")
target_link_libraries(geonoptcommon ${GDAL_LIBRARY} ${CMAKE_DL_LIBS} ${Boost_LIBRARIES})

###################################################################################################################
##                                                                                                                #
##                 Parallel NSGAII for optimising zonal policy in Geonamica-Metronamica geoprojects               #
##                                                                                                                #
##                                                                                                                #
###################################################################################################################
#
#SET(Geonamica_optimiser_parallel_Src
#        MainPar.cpp
#    )
#
#add_executable(geooptpar ${Geonamica_optimiser_parallel_Src})
#target_link_libraries(geooptpar ${GDAL_LIBRARY} ${Boost_LIBRARIES} ${MPI_LIBRARIES} ${PNGwriter_LIBRARIES}
#        ${MagickLibs} geonoptcommon ${OpenCV_LIBS})
##IF(USE_VTK)
##    if(VTK_LIBRARIES)
##        target_link_libraries(geooptpar ${VTK_LIBRARIES} )
##    else()
##        target_link_libraries(geooptpar vtkHybrid vtkWidgets )
##    endif()
##ENDIF()
#target_compile_definitions(geooptpar PUBLIC "-DMAGICKCORE_QUANTUM_DEPTH=16" PUBLIC "-DMAGICKCORE_HDRI_ENABLE=0")
#
#
###################################################################################################################
##                                                                                                                #
##                 Serial NSGAII for optimising zonal policy in Geonamica-Metronamica geoprojects                 #
##                                                                                                                #
##                                                                                                                #
###################################################################################################################
#
#
#SET(Geonamica_optimiser_serial_Src
#       MainSer.cpp
#    )
#
#
#
#add_executable(geooptser ${Geonamica_optimiser_serial_Src} )
#target_link_libraries(geooptser ${GDAL_LIBRARY} ${Boost_LIBRARIES} ${MPI_LIBRARIES} ${PNGwriter_LIBRARIES}
#        ${MagickLibs} geonoptcommon ${OpenCV_LIBS})
##IF(USE_VTK)
##    if(VTK_LIBRARIES)
##        target_link_libraries(geooptser ${VTK_LIBRARIES} )
##    else()
##        target_link_libraries(geooptser vtkHybrid vtkWidgets )
##    endif()
##ENDIF()
#target_compile_definitions(geooptser PUBLIC "-DMAGICKCORE_QUANTUM_DEPTH=16" PUBLIC "-DMAGICKCORE_HDRI_ENABLE=0")


###################################################################################################################
##                                                                                                                #
##                 Test ZonalOF on zonal areas all restrict or all stimulate, or all nothing                      #
##                                                                                                                #
##                                                                                                                #
###################################################################################################################
#
#
#SET(zonal_opt_tst_src
#        TestZoneDVs.cpp
#        )
#
#
#
#add_executable(zonalopttest ${zonal_opt_tst_src})
#target_link_libraries(zonalopttest ${GDAL_LIBRARY} ${Boost_LIBRARIES} ${MPI_LIBRARIES} ${PNGwriter_LIBRARIES}
#        ${MagickLibs} geonoptcommon ${OpenCV_LIBS})
##IF(USE_VTK)
##    if(VTK_LIBRARIES)
##        target_link_libraries(zonalopttest ${VTK_LIBRARIES} )
##    else()
##        target_link_libraries(zonalopttest vtkHybrid vtkWidgets )
##    endif()
##ENDIF()
#target_compile_definitions(zonalopttest PUBLIC "-DMAGICKCORE_QUANTUM_DEPTH=16" PUBLIC "-DMAGICKCORE_HDRI_ENABLE=0")



SET(USE_QT CACHE BOOL "Set to on to build qt gui with chart visualisation")
add_subdirectory(GUI)
add_subdirectory(EvaluatorModules)