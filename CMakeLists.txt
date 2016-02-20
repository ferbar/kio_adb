PROJECT( tutorial )
FIND_PACKAGE(KDE4 REQUIRED)
INCLUDE_DIRECTORIES( ${KDE4_INCLUDES} . )

set(kio_adb_PART_SRCS
	adb.cpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")

kde4_add_plugin(kio_adb ${kio_adb_PART_SRCS})

target_link_libraries(kio_adb ${KDE4_KIO_LIBS})

install(TARGETS kio_adb  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install(FILES adb.protocol DESTINATION ${SERVICES_INSTALL_DIR})
