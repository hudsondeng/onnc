
include_directories(${ONNC_INCLUDE_DIRS})
add_executable(onnc main.cpp ONNCApp.cpp ONNCConfig.cpp)
target_link_libraries(onnc libonnc onnc-rt)

install(TARGETS onnc
    RUNTIME DESTINATION bin)
