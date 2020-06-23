include_directories(../common ../MWM)
add_executable(ss_w_mwm MWM.hpp MWM_sim.cpp $<TARGET_OBJECTS:HDR_OBJ>)
target_link_libraries(ss_w_mwm ${LEMON_LIB})