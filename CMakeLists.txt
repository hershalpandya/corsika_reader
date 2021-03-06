cmake_minimum_required(VERSION 2.7)
project(corsika_reader CXX)
enable_language(C)

set (Corsika_PACKAGE_NAME "corsika")

set (CMAKE_BUILD_TYPE Debug)
set (CMAKE_CXX_FLAGS_DEBUG "-Wall -ggdb3 -O0 -fno-inline")
set (CMAKE_BUILD_TYPE Release)
set (CMAKE_CXX_FLAGS_DEBUG "-O3")

set (CMAKE_INCLUDE_CURRENT_DIR TRUE)
set (CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})

find_package (PythonLibs)
find_package (ZLIB REQUIRED)
find_package (BZip2 REQUIRED)
find_package (Boost REQUIRED COMPONENTS iostreams python unit_test_framework)

message ("${ZLIB_LIBRARIES}")
message ("${Boost_LIBRARIES}")
message ("${PYTHON_LIBRARIES}")

add_definitions(
  -DCORSIKA_TEST_DIR="${CMAKE_SOURCE_DIR}/share/data"
  -DCORSIKA_EXAMPLE_DATA_DIR="${CMAKE_INSTALL_PREFIX}/share/${Corsika_PACKAGE_NAME}/data"
)

include_directories(${CMAKE_SOURCE_DIR}/include ${Boost_INCLUDE_DIR})

set (Corsika_HEADERS
  include/corsika/Constants.h
  include/corsika/CorsikaUnits.h
  include/corsika/CorsikaBlock.h
  include/corsika/RawStream.h
  include/corsika/RawCorsikaFile.h
  include/corsika/RawParticleIterator.h
  include/corsika/CorsikaIOException.h
  include/corsika/CorsikaParticle.h
  include/corsika/CorsikaShower.h
  include/corsika/CorsikaShowerFile.h
  include/corsika/CorsikaShowerFileParticleIterator.h
  include/corsika/ShowerParticleList.h
  include/corsika/GaisserHillasParameter.h
  include/corsika/particle/NucleusProperties.h
  include/corsika/particle/ParticleList.h
  include/corsika/particle/ParticleProperties.h
  include/corsika/particle/VParticleProperties.h
  DESTINATION include
)

set (Corsika_SOURCES
  src/corsika/CorsikaBlock.cxx
  src/corsika/CorsikaParticle.cxx
  src/corsika/CorsikaShower.cxx
  src/corsika/CorsikaShowerFile.cxx
  src/corsika/CorsikaShowerFileParticleIterator.cxx
  src/corsika/GaisserHillasParameter.cxx
  src/corsika/RawStream.cxx
  src/corsika/RawCorsikaFile.cxx
  src/corsika/Index.cxx
  src/corsika/particle/NucleusProperties.cxx
  src/corsika/particle/ParticleProperties.cxx
  src/corsika/particle/ParticleList.cxx
  src/corsika/RawParticleIterator.cxx
)


set (Corsika_BoostPython_SOURCES
  src/pybindings/CorsikaBlock_py.cxx
  src/pybindings/CorsikaParticle_py.cxx
  src/pybindings/CorsikaShower_py.cxx
  src/pybindings/CorsikaShowerFile_py.cxx
  src/pybindings/CorsikaShowerFileParticleIterator_py.cxx
  src/pybindings/CorsikaUnits_py.cxx
  src/pybindings/MathConstants_py.cxx
  src/pybindings/ParticleList_py.cxx
  src/pybindings/ParticleProperties_py.cxx
  src/pybindings/PhysicalConstants_py.cxx
  src/pybindings/RawCorsikaFile_py.cxx
  src/pybindings/RawParticleIterator_py.cxx
  src/pybindings/ShowerParticleList_py.cxx
  src/pybindings/module.cxx
)

add_library (CorsikaReader SHARED
  ${Corsika_SOURCES}
)
target_link_libraries(CorsikaReader
  ${Boost_LIBRARIES}
  ${BZip2_LIBRARIES}
  ${ZLIB_LIBRARIES}
)

if (PYTHONLIBS_FOUND)
  include_directories (${PYTHON_INCLUDE_DIRS})
  add_library (${Corsika_PACKAGE_NAME} SHARED
    ${Corsika_BoostPython_SOURCES}
    )

  set_target_properties (${Corsika_PACKAGE_NAME} PROPERTIES
    PREFIX ""
    SUFFIX ".so"
    )

  target_link_libraries (${Corsika_PACKAGE_NAME}
    ${PYTHON_LIBRARIES}
    CorsikaReader
    )

  install(DIRECTORY share/examples/
    DESTINATION share/${Corsika_PACKAGE_NAME}/examples
    FILES_MATCHING PATTERN "*.py"
    )

  install (TARGETS ${Corsika_PACKAGE_NAME}
    LIBRARY DESTINATION lib
    )
endif (PYTHONLIBS_FOUND)

install (TARGETS CorsikaReader
  LIBRARY DESTINATION lib
)

install(FILES
  include/corsika/Constants.h
  include/corsika/CorsikaUnits.h
  include/corsika/CorsikaBlock.h
  include/corsika/RawStream.h
  include/corsika/RawCorsikaFile.h
  include/corsika/RawParticleIterator.h
  include/corsika/CorsikaIOException.h
  include/corsika/CorsikaParticle.h
  include/corsika/CorsikaShower.h
  include/corsika/CorsikaShowerFile.h
  include/corsika/CorsikaShowerFileParticleIterator.h
  include/corsika/ShowerParticleList.h
  include/corsika/GaisserHillasParameter.h
  DESTINATION include/icetop_corsika
)
install(FILES
  include/corsika/particle/NucleusProperties.h
  include/corsika/particle/ParticleList.h
  include/corsika/particle/ParticleProperties.h
  include/corsika/particle/VParticleProperties.h
  DESTINATION include/icetop_corsika/particle
)


add_executable(test_corsika
  test/main.cxx
  test/testRawStream.cxx
  test/testRawCorsikaFile.cxx
  test/testCorsikaFile.cxx
)

target_link_libraries(test_corsika CorsikaReader ${PYTHON_LIBRARIES})

SET(FILE         corsika_example_data)
SET(SOURCE       http://www.bartol.udel.edu/~javierg)
SET(TO_DIR ${CMAKE_CURRENT_SOURCE_DIR}/share)

OPTION(FETCH_CORSIKA_DATA "Fetch the example data" ON)
FIND_PACKAGE(Wget)
IF (WGET_FOUND)
  IF (FETCH_CORSIKA_DATA)
    IF (NOT EXISTS ${TO_DIR}/data)
      message(STATUS "Downloading and unpacking example CORSIKA data.")
      execute_process (COMMAND rm -f * WORKING_DIRECTORY ${TO_DIR}/${FILE})
      execute_process (COMMAND rmdir ${FILE} WORKING_DIRECTORY ${TO_DIR})
      execute_process (COMMAND ${WGET_EXECUTABLE} -P corsika_example_data -nd -r --no-parent ${SOURCE}/${FILE}/ WORKING_DIRECTORY ${TO_DIR})
      file (RENAME ${TO_DIR}/${FILE} ${TO_DIR}/data)
    ELSE (NOT EXISTS ${TO_DIR}/data)
      message(STATUS "${TO_DIR}/data already exists")
    ENDIF (NOT EXISTS ${TO_DIR}/data)
  ELSE (FETCH_CORSIKA_DATA)
    message(STATUS "Not downloading CORSIKA example data.")
  ENDIF (FETCH_CORSIKA_DATA)
  IF (EXISTS ${TO_DIR}/data)
    install (DIRECTORY ${TO_DIR}/data DESTINATION share/${Corsika_PACKAGE_NAME} FILES_MATCHING PATTERN "DAT*")
  ENDIF (EXISTS ${TO_DIR}/data)
ELSE (WGET_FOUND)
  message(STATUS "WGet not found. CORSIKA example data will not be available.")
ENDIF (WGET_FOUND)
