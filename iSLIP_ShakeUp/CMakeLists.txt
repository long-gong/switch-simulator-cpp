include_directories(../common ../iSLIP_ShakeUp)
add_executable(ss_w_islip_shakeup iSLIP_ShakeUp.hpp iSLIP_ShakeUp_sim.cpp $<TARGET_OBJECTS:HDR_OBJ>)