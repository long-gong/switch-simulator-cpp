include_directories(../common ../QPP_QPA_Serena)
add_executable(ss_w_qpp_qpa_serena QPP_QPA_Serena.hpp QPP_QPA_Serena_sim.cpp $<TARGET_OBJECTS:HDR_OBJ>)