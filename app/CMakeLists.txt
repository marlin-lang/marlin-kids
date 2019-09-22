set(HEADERS
    Document.h
    DrawHelper.h
    DuplicateSourceTextField.h
    DuplicateViewController.h
    EditorViewController.h
    ExecuteViewController.h
    Formatter.h
    FunctionViewController.h
    LineNumberView.h
    MessageViewController.h
    Pasteboard.h
    SourceView.h
    SourceViewController.h
    Theme.h
    ToolboxViewController.h
    Types.h)

set(SOURCES
    Document.mm
    DuplicateSourceTextField.mm
    DuplicateViewController.mm
    EditorViewController.mm
    ExecuteViewController.mm
    Formatter.mm
    FunctionViewController.mm
    LineNumberView.mm
    MessageViewController.mm
    SourceView.mm
    SourceViewController.mm
    Theme.mm
    ToolboxViewController.mm
    Types.mm)

set(RESOURCES exec_env.html exec_env.js)

if(IOS)
  set(HEADERS
      ${HEADERS}
      ios/AppDelegate.h
      ios/DocumentBrowserViewController.h
      ios/IosDocument.h
      ios/IosEditorViewController.h
      ios/IosSourceView.h
      ios/IosSourceViewController.h
      ios/IosToolboxViewController.h
      ios/SplitViewController.h
      ios/ToolboxCell.h)

  set(SOURCES
      ${SOURCES}
      ios/AppDelegate.mm
      ios/DocumentBrowserViewController.mm
      ios/IosDocument.mm
      ios/IosEditorViewController.mm
      ios/IosSourceView.mm
      ios/IosSourceViewController.mm
      ios/IosToolboxViewController.mm
      ios/main.mm
      ios/SplitViewController.mm
      ios/ToolboxCell.mm)

  set(RESOURCES ${RESOURCES} ios/Assets.xcassets ios/Base.lproj/Main.storyboard)

  set(TARGET_NAME ${PROJECT_NAME}.app.ios)
  set(DEFINITIONS IOS)
  set(FRAMEWORKS "-framework CoreGraphics -framework UIKit -framework WebKit")
  set(INFO_PLIST "ios/Info.plist")
else(IOS)
  set(HEADERS
      ${HEADERS}
      mac/AppDelegate.h
      mac/MacDocument.h
      mac/MacEditorViewController.h
      mac/MacFunctionViewController.h
      mac/MacSourceView.h
      mac/MacSourceViewController.h
      mac/MacToolboxViewController.h
      mac/ParameterItem.h
      mac/SplitViewController.h
      mac/ToolboxItem.h
      mac/WindowController.h)

  set(SOURCES
      ${SOURCES}
      mac/AppDelegate.mm
      mac/MacDocument.mm
      mac/MacEditorViewController.mm
      mac/MacFunctionViewController.mm
      mac/MacSourceView.mm
      mac/MacSourceViewController.mm
      mac/MacToolboxViewController.mm
      mac/main.mm
      mac/ParameterItem.mm
      mac/SplitViewController.mm
      mac/ToolboxItem.mm
      mac/WindowController.mm)

  set(RESOURCES
      ${RESOURCES}
      mac/marlin.entitlements
      mac/Assets.xcassets
      mac/Base.lproj/Main.storyboard
      mac/ParameterItem.xib
      mac/ToolboxItem.xib)

  set(TARGET_NAME ${PROJECT_NAME}.app.mac)
  set(FRAMEWORKS "-framework CoreGraphics -framework AppKit -framework WebKit")
  set(INFO_PLIST "mac/Info.plist")
endif(IOS)

add_executable(${TARGET_NAME} MACOSX_BUNDLE ${SOURCES} ${RESOURCES})
target_sources(${TARGET_NAME} PRIVATE ${HEADERS})
target_compile_definitions(${TARGET_NAME} PRIVATE ${DEFINITIONS})
target_include_directories(${TARGET_NAME} PRIVATE .)

set_target_properties(${TARGET_NAME}
                      PROPERTIES OUTPUT_NAME
                                 marlin
                                 MACOSX_BUNDLE_INFO_PLIST
                                 "${CMAKE_CURRENT_SOURCE_DIR}/${INFO_PLIST}"
                                 RESOURCE
                                 "${RESOURCES}")

target_link_libraries(${TARGET_NAME} ${FRAMEWORKS} ${PROJECT_NAME}.core)

# XCode compile options
set_target_properties(
  ${TARGET_NAME}
  PROPERTIES XCODE_ATTRIBUTE_CLANG_ENABLE_MODULES
             YES
             XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC
             YES
             XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_WEAK
             YES
             XCODE_ATTRIBUTE_CLANG_WARN_BLOCK_CAPTURE_AUTORELEASING
             YES
             XCODE_ATTRIBUTE_CLANG_WARN_BOOL_CONVERSION
             YES
             XCODE_ATTRIBUTE_CLANG_WARN_COMMA
             YES
             XCODE_ATTRIBUTE_CLANG_WARN_CONSTANT_CONVERSION
             YES
             XCODE_ATTRIBUTE_CLANG_WARN_DEPRECATED_OBJC_IMPLEMENTATIONS
             YES
             XCODE_ATTRIBUTE_CLANG_WARN_DIRECT_OBJC_ISA_USAGE
             YES_ERROR
             XCODE_ATTRIBUTE_CLANG_WARN_DOCUMENTATION_COMMENTS
             YES
             XCODE_ATTRIBUTE_CLANG_WARN_EMPTY_BODY
             YES
             XCODE_ATTRIBUTE_CLANG_WARN_ENUM_CONVERSION
             YES
             XCODE_ATTRIBUTE_CLANG_WARN_INFINITE_RECURSION
             YES
             XCODE_ATTRIBUTE_CLANG_WARN_INT_CONVERSION
             YES
             XCODE_ATTRIBUTE_CLANG_WARN_NON_LITERAL_NULL_CONVERSION
             YES
             XCODE_ATTRIBUTE_CLANG_WARN_OBJC_IMPLICIT_RETAIN_SELF
             YES
             XCODE_ATTRIBUTE_CLANG_WARN_OBJC_LITERAL_CONVERSION
             YES
             XCODE_ATTRIBUTE_CLANG_WARN_OBJC_ROOT_CLASS
             YES_ERROR
             XCODE_ATTRIBUTE_CLANG_WARN_RANGE_LOOP_ANALYSIS
             YES
             XCODE_ATTRIBUTE_CLANG_WARN_STRICT_PROTOTYPES
             YES
             XCODE_ATTRIBUTE_CLANG_WARN_SUSPICIOUS_MOVE
             YES
             XCODE_ATTRIBUTE_CLANG_WARN_UNGUARDED_AVAILABILITY
             YES_AGGRESSIVE
             XCODE_ATTRIBUTE_CLANG_WARN_UNREACHABLE_CODE
             YES)

if(IOS)
  set_target_properties(${TARGET_NAME}
                        PROPERTIES XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET
                                   "11.0"
                                   XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY
                                   "1,2")
else(IOS)
  set_target_properties(${TARGET_NAME}
                        PROPERTIES XCODE_ATTRIBUTE_MACOSX_DEPLOYMENT_TARGET
                                   "10.13")
endif(IOS)
