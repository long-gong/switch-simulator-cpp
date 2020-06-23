include_directories(../common ../Serena)
add_executable(ss_w_serena Serena.hpp Serena_sim.cpp $<TARGET_OBJECTS:HDR_OBJ>)