include_directories(../common ../QPS_iSLIP)
add_executable(ss_w_qps_islip QPS_iSLIP.hpp QPS_iSLIP_sim.cpp $<TARGET_OBJECTS:HDR_OBJ>)