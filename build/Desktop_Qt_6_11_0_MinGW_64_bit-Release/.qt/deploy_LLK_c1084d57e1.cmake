include("D:/OneDrive/Documents/LLK/build/Desktop_Qt_6_11_0_MinGW_64_bit-Release/.qt/QtDeploySupport.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/LLK-plugins.cmake" OPTIONAL)
set(__QT_DEPLOY_I18N_CATALOGS "qtbase")

qt6_deploy_runtime_dependencies(
    EXECUTABLE "D:/OneDrive/Documents/LLK/build/Desktop_Qt_6_11_0_MinGW_64_bit-Release/LLK.exe"
    GENERATE_QT_CONF
)
