IF(NOT EXISTS "D:/xProgramGit/glPractice_2018_10/glLibs/assimp-4.1.0/build/install_manifest.txt")
  MESSAGE(FATAL_ERROR "Cannot find install manifest: \"D:/xProgramGit/glPractice_2018_10/glLibs/assimp-4.1.0/build/install_manifest.txt\"")
ENDIF(NOT EXISTS "D:/xProgramGit/glPractice_2018_10/glLibs/assimp-4.1.0/build/install_manifest.txt")

FILE(READ "D:/xProgramGit/glPractice_2018_10/glLibs/assimp-4.1.0/build/install_manifest.txt" files)
STRING(REGEX REPLACE "\n" ";" files "${files}")
FOREACH(file ${files})
  MESSAGE(STATUS "Uninstalling \"$ENV{DESTDIR}${file}\"")
  EXEC_PROGRAM(
    "D:/xSoftware/CMake/bin/cmake.exe" ARGS "-E remove \"$ENV{DESTDIR}${file}\""
    OUTPUT_VARIABLE rm_out
    RETURN_VALUE rm_retval
    )
  IF(NOT "${rm_retval}" STREQUAL 0)
    MESSAGE(FATAL_ERROR "Problem when removing \"$ENV{DESTDIR}${file}\"")
  ENDIF(NOT "${rm_retval}" STREQUAL 0)
ENDFOREACH(file)
