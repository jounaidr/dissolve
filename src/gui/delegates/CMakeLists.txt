# Don't forget to include output directory, otherwise the UI file won't be wrapped!

# Meta-Objects
set(delegates_MOC_HDRS comboList.hui exponentialSpin.hui integerSpin.hui isotopeCombo.hui)
qt6_wrap_cpp(delegates_MOC_SRCS ${delegates_MOC_HDRS})

# User Interface Files SET(delegates_UIS ) QT6_WRAP_UI(delegates_UIS_H ${delegates_UIS})

# Source files (not Qt UI files or custom .h [uih])
set(delegates_SRCS
    comboListFuncs.cpp
    exponentialSpinFuncs.cpp
    integerSpinFuncs.cpp
    isotopeComboFuncs.cpp
    null.cpp
    customComboDelegate.h
    intraFormCombo.h
    null.h
)

# Target 'delegates'
add_library(
  delegates
  # ${delegates_UIS_H}
  ${delegates_SRCS} ${delegates_MOC_SRCS}
)
target_include_directories(delegates PRIVATE ${PROJECT_SOURCE_DIR}/src ${PROJECT_BINARY_DIR}/src ${Qt6Widgets_INCLUDE_DIRS})
target_link_libraries(delegates PRIVATE base)

# Because Qt has its own way of checking for C++17
if(MSVC)
  target_compile_options(delegates PUBLIC /Zc:__cplusplus)
endif(MSVC)
