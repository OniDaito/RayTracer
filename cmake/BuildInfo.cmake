#based on http://stackoverflow.com/questions/3780667/use-cmake-to-get-build-time-svn-revision
find_package(Git REQUIRED)
exec_program(${GIT_EXECUTABLE} 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ARGS "describe --tags"
    OUTPUT_VARIABLE GIT_REVISION)
set(RAYS_REVISION "${GIT_REVISION}")

#get the build host
site_name(RAYS_BUILD_HOST)

#get the build timestamp in the default format
string(TIMESTAMP BUILD_TIMESTAMP)

# write relevant information into template header 
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/buildinfo.txt
"#ifndef __version_hpp__
#define __version_hpp__

std::string GetVersionString() {
  return \"${RAYS_REVISION} ${RAYS_BUILD_HOST} ${BUILD_TIMESTAMP}\";
}
#endif
")

# copy the file to the final header only if the data changes (currently it will be every time due to the timestamp data) 
execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_BINARY_DIR}/buildinfo.txt ${CMAKE_CURRENT_BINARY_DIR}/version.hpp)

