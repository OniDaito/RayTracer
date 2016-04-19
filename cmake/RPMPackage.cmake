#utility function to generate a shell script from the input list containing commands
function(generate_shell_script SCRIPTFILENAME INPUTLIST)
    file(WRITE ${SCRIPTFILENAME} "#!/bin/sh\n")
    FOREACH(command ${INPUTLIST})
        file(APPEND ${SCRIPTFILENAME} "${command}\n")
    ENDFOREACH(command)
endfunction(generate_shell_script)

#define the generator and other required settings
set(CPACK_GENERATOR "RPM")
set(CPACK_PACKAGE_CONTACT "Benjamin Blundell <b.blundell@qmul.ac.uk>")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "RayTracer application")
set(CPACK_RPM_PACKAGE_DESCRIPTION "A simple Raytracer that works with OpenMP")
set(CPACK_PACKAGE_VENDOR "ITS Research")
SET(CPACK_PACKAGE_NAME "rays")
set(CPACK_PACKAGE_VERSION_MAJOR "${RAYS_PACKAGE_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${RAYS_PACKAGE_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${RAYS_PACKAGE_VERSION_PATCH}")
set(CPACK_PACKAGING_INSTALL_PREFIX "/usr/local/bin")

#RPM specific options
set(CPACK_RPM_PACKAGE_RELEASE "${RAYS_RPM_PACKAGE_RELEASE}")
set(CPACK_RPM_PACKAGE_ARCHITECTURE "x86_64")
#set(CPACK_RPM_PACKAGE_REQUIRES "policycoreutils >= 2.2.5")
#set(CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION "/var/www/html")

#set the os release
string (REGEX MATCH "\\el[1-9]" OS_VERSION ${CMAKE_SYSTEM})

#set the output file name
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${RAYS_PACKAGE_VERSION}.${OS_VERSION}.${CPACK_RPM_PACKAGE_ARCHITECTURE}")

set(RAYS_POST_INSTALL_CMDS "restorecon -R ${CPACK_PACKAGING_INSTALL_PREFIX}")
set(RAYS_POST_INSTALL ${CMAKE_CURRENT_BINARY_DIR}/rpm/postinstall)
generate_shell_script(${RAYS_POST_INSTALL} "${RAYS_POST_INSTALL_CMDS}")
set(CPACK_RPM_POST_INSTALL_SCRIPT_FILE "${RAYS_POST_INSTALL}")
#set(CPACK_RPM_POST_INSTALL_SCRIPT_FILE "")

include(CPack)



