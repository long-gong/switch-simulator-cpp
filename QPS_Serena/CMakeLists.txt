include_directories(../common ../QPS_Serena)
add_executable(ss_w_qps_serena QPS_Serena.hpp QPS_Serena_sim.cpp $<TARGET_OBJECTS:HDR_OBJ>)