add_executable(test_version test_version.c)

target_link_libraries(test_version PRIVATE tengrad tengrad_warnings tengrad_sanitizers)

add_test(NAME test_version COMMAND test_version)
