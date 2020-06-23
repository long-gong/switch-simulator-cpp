include_directories(../common ../FQPS_iSLIP)
add_executable(ss_w_fqps_qpa_islip FQPP_QPA_iSLIP_sim.cpp $<TARGET_OBJECTS:HDR_OBJ>)