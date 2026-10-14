add_library(unity STATIC ${PROJECT_SOURCE_DIR}/third_party/Unity/src/unity.c)
target_include_directories(unity PUBLIC ${PROJECT_SOURCE_DIR}/third_party/Unity/src)

# Catch memory errors and UB on the host that would be silent on the MCU.
set(SANITIZE -fsanitize=address,undefined -fno-omit-frame-pointer)
target_compile_options(corelib PRIVATE ${SANITIZE})

foreach(name ringbuf cli)
    add_executable(test_${name} test_${name}.c)
    target_compile_options(test_${name} PRIVATE -Wall -Wextra ${SANITIZE})
    target_link_options(test_${name} PRIVATE ${SANITIZE})
    target_link_libraries(test_${name} PRIVATE corelib unity)
    add_test(NAME ${name} COMMAND test_${name})
endforeach()
