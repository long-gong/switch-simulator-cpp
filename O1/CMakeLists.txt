include_directories(../common ../O1)
add_executable(ss_w_o1 O1.hpp O1_sim.cpp $<TARGET_OBJECTS:HDR_OBJ>)