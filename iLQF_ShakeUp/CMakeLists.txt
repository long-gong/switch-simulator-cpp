include_directories(../common)
add_executable(ss_w_ilqf_shakeup iLQF_ShakeUp_sim.cpp $<TARGET_OBJECTS:HDR_OBJ>)