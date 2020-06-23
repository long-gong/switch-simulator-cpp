include_directories(../common ../iSLIP)
add_executable(ss_w_islip iSLIP.hpp iSLIP_sim.cpp $<TARGET_OBJECTS:HDR_OBJ>)