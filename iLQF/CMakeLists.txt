include_directories(../common ../iLQF)
add_executable(ss_w_ilqf iLQF_sim.cpp iLQF.hpp $<TARGET_OBJECTS:HDR_OBJ>)