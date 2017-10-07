PROJECT( kio_adb )

cmake_minimum_required(VERSION 2.8.12)

include(FeatureSummary)

set(PROJECT_VERSION 0.2)

set(QT_MIN_VERSION 5.2.0)
set(KF5_MIN_VERSION 5.14.0)
set(KGAPI_MIN_VERSION 5.3.1)
set(QTKEYCHAIN_MIN_VERSION 0.6.0)

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    Gui
	Network
	Widgets)


find_package(ECM 5.14.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMQtDeclareLoggingCategory)
include(ECMSetupVersion)

#find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS
#    I18n
#	KIO)

find_package(KF5KIO ${KF5_MIN_VERSION} REQUIRED COMPONENTS
    I18n
	KIO)

ecm_setup_version(${PROJECT_VERSION}
    VARIABLE_PREFIX ADB
	VERSION_HEADER adbversion.h)

add_definitions(-DQT_NO_URL_CAST_FROM_STRING)
add_definitions(-DQT_STRICT_ITERATORS)
add_definitions(-DTRANSLATION_DOMAIN=\"kio5_src\")

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE5_ENABLE_EXCEPTIONS}")


#install(TARGETS kio_adb  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

#install(FILES adb.protocol DESTINATION ${SERVICES_INSTALL_DIR})

# add_subdirectory(doc)
add_subdirectory(src)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)

