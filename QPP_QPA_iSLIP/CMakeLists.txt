include_directories(../common ../QPP_QPA_iSLIP)
add_executable(ss_w_qpp_qpa_islip QPP_QPA_iSLIP.hpp QPP_QPA_iSLIP_sim.cpp $<TARGET_OBJECTS:HDR_OBJ>)