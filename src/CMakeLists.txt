include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_BINARY_DIR})

kde_enable_exceptions()

set(kio_adb_SRCS
    adb.cpp
	)

ecm_qt_declare_logging_category(kio_adb_SRCS
    HEADER adbdebug.h
	IDENTIFIER ADB
	CATEGORY_NAME kf5.kio.adb)


kcoreaddons_add_plugin(kio_adb
    SOURCES ${kio_adb_SRCS}
	INSTALL_NAMESPACE kf5/kio)

# target_link_libraries(kio_adb ${KDE5_KIO_LIBS})
target_link_libraries(kio_adb KF5::KIOCore KF5::KIOFileWidgets KF5::KIOWidgets KF5::KIONTLM)

set_target_properties(kio_adb PROPERTIES OUTPUT_NAME "adb")
