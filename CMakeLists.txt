cmake_minimum_required(VERSION 3.16)

project(tickets VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Sql)

set(PROJECT_SOURCES
        main.cpp
        view/mainwindow.cpp
        view/mainwindow.h
        view/mainwindow.ui
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(tickets
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        global.cpp

        # entity层
        entity/user.h entity/user.cpp
        entity/client.h entity/client.cpp
        entity/admin.h entity/admin.cpp
        entity/company.h entity/company.cpp
        entity/flight.h entity/flight.cpp
        entity/plane.h entity/plane.cpp
        entity/order.h entity/order.cpp  # 主分支新增的order相关
        entity/food.h entity/food.cpp

        # model层
        model/usermodel.h model/usermodel.cpp
        model/adminmodel.h model/adminmodel.cpp
        model/clientmodel.h model/clientmodel.cpp
        model/companymodel.h model/companymodel.cpp
        model/flightmodel.h model/flightmodel.cpp
        model/planemodel.h model/planemodel.cpp
        model/flightfilterproxymodel.h model/flightfilterproxymodel.cpp
        model/ordermodel.h model/ordermodel.cpp  # 主分支新增的ordermodel
        model/foodmodel.h model/foodmodel.cpp

        # controller层
        controller/controlleruser.h controller/controlleruser.cpp
        controller/controllerflight.h controller/controllerflight.cpp
        controller/controllercompany.h controller/controllercompany.cpp
        controller/foodservice.h controller/foodservice.cpp
        controller/controllerfood.h controller/controllerfood.cpp

        # view层
        view/logindialog.h view/logindialog.cpp view/logindialog.ui
        view/registerdialog.h view/registerdialog.cpp view/registerdialog.ui
        view/flightwidget.h view/flightwidget.cpp view/flightwidget.ui
        view/adminform.h view/adminform.cpp view/adminform.ui  # 主分支新增/补充的adminform
        view/companyregisterdialog.h view/companyregisterdialog.cpp view/companyregisterdialog.ui
        view/weatherforecastform.h view/weatherforecastform.cpp view/weatherforecastform.ui
        view/addfoodwindow.h view/addfoodwindow.cpp view/addfoodwindow.ui
        view/seatitem.h view/seatitem.cpp  # 主分支新增的seat相关
        view/seatselectionform.h view/seatselectionform.cpp view/seatselectionform.ui
        view/personalpage.cpp view/personalpage.h view/personalpage.ui
        view/passwordchange.cpp view/passwordchange.h view/passwordchange.ui

        # tool层
        tool/encoder.h tool/encoder.cpp
        tool/recordtool.h tool/recordtool.cpp
        entity/record.h entity/record.cpp
        controller/controllerplane.h
        controller/controllerplane.cpp
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET tickets APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(tickets SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(tickets
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(tickets PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Sql)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.tickets)
endif()
set_target_properties(tickets PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS tickets
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(tickets)
endif()
