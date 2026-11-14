add_executable(test_version test_version.c)
target_link_libraries(test_version PRIVATE tengrad tengrad_warnings tengrad_sanitizers)
add_test(NAME test_version COMMAND test_version)

add_executable(test_storage test_storage.c)
target_include_directories(test_storage PRIVATE ${PROJECT_SOURCE_DIR}/src/)
target_link_libraries(test_storage PRIVATE tengrad tengrad_warnings tengrad_sanitizers)
add_test(NAME test_storage COMMAND test_storage)
