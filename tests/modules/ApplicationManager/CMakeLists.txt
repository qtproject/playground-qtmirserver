set(
  APPLICATION_MANAGER_TEST_SOURCES
  application_manager_test.cpp
  ${CMAKE_SOURCE_DIR}/src/common/debughelpers.cpp
  ${CMAKE_SOURCE_DIR}/tests/modules/common/qtmir_test.cpp
  ../common/fake_mirsurfaceitem.h
)

include_directories(
  ${APPLICATION_API_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/src/platforms/mirserver
  ${CMAKE_SOURCE_DIR}/src/modules
  ${CMAKE_SOURCE_DIR}/tests/modules/common
  ${MIRSERVER_INCLUDE_DIRS}
)

add_executable(applicationmanager_test ${APPLICATION_MANAGER_TEST_SOURCES})

target_link_libraries(
  applicationmanager_test

  qpa-mirserver
  unityapplicationplugin

  Qt5::Test

  ${GTEST_BOTH_LIBRARIES}
  ${GMOCK_LIBRARIES}
)

add_test(ApplicationManager applicationmanager_test)
