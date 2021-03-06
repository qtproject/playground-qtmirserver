set(
  APPLICATION_TEST_SOURCES
  application_test.cpp
  ${CMAKE_SOURCE_DIR}/tests/modules/common/fake_session.h
  ${CMAKE_SOURCE_DIR}/tests/modules/common/qtmir_test.cpp
)

include_directories(
  ${APPLICATION_API_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/src/platforms/mirserver
  ${CMAKE_SOURCE_DIR}/src/modules
  ${CMAKE_SOURCE_DIR}/tests/modules/common
  ${MIRSERVER_INCLUDE_DIRS}
)

add_executable(application_test ${APPLICATION_TEST_SOURCES})

target_link_libraries(
  application_test

  unityapplicationplugin
  qpa-mirserver

  Qt5::Test

  ${GTEST_BOTH_LIBRARIES}
  ${GMOCK_LIBRARIES}
)

add_test(Application, application_test)
