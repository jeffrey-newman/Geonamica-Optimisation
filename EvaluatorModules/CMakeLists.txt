SET(ImplementationCostSrcs
        ImplementationCost.hpp
        ImplementationCost.cpp
        ModuleAPI.hpp
        )

add_library(implementcosts SHARED ${ImplementationCostSrcs})
target_link_libraries(implementcosts ${Boost_LIBRARIES} )

SET(ZonalCostingCellCountSrcs
        ZonalCostingCellCount.cpp
        ZonalCostingCellCount.h
        ModuleAPI.hpp
        ${AHHZ_Raster}
        ${AHHZ_Raster_Utility}
        )

add_library(zonalcostscellcount SHARED ${ZonalCostingCellCountSrcs})
target_link_libraries(zonalcostscellcount ${Boost_LIBRARIES} ${GDAL_LIBRARY} )