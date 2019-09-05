set(HEADERS
    Document.h
    DrawContext.h
    DrawHelper.h
    EditorViewController.h
    ExecuteViewController.h
    LineNumberView.h
    MessageViewController.h
    NSData+DataView.h
    NSObject+Casting.h
    NSString+StringView.h
    Pasteboard.h
    SourceView.h
    SourceViewController.h
    Theme.h
    Types.h)

set(SOURCES
    Document.mm
    DrawContext.mm
    EditorViewController.mm
    ExecuteViewController.mm
    LineNumberView.mm
    MessageViewController.mm
    NSData+DataView.mm
    NSObject+Casting.mm
    NSString+StringView.mm
    SourceView.mm
    SourceViewController.mm
    Theme.mm)

if(IOS)
  set(HEADERS
      ${HEADERS}
      ios/AppDelegate.h
      ios/DocumentBrowserViewController.h
      ios/IosSourceView.h
      ios/IosSourceViewController.h
      ios/ToolboxCell.h
      ios/ToolboxHeaderView.h)

  set(SOURCES
      ${SOURCES}
      ios/AppDelegate.mm
      ios/DocumentBrowserViewController.mm
      ios/IosSourceView.mm
      ios/IosSourceViewController.mm
      ios/main.mm
      ios/ToolboxCell.mm
      ios/ToolboxHeaderView.mm)

  set(RESOURCES
      ${RESOURCES}
      ios/Info.plist
      ios/Assets.xcassets
      ios/Base.lproj/Main.storyboard)

  set(TARGET_NAME ${PROJECT_NAME}.app.ios)
  set(DEFINITIONS IOS)
  set(FRAMEWORKS "-framework CoreGraphics -framework UIKit")
  set(INFO_PLIST "ios/Info.plist")
else(IOS)
  set(HEADERS
      ${HEADERS}
      mac/AppDelegate.h
      mac/MacSourceView.h
      mac/MacSourceViewController.h
      mac/ToolboxHeaderView.h
      mac/ToolboxItem.h
      mac/WindowController.h)

  set(SOURCES
      ${SOURCES}
      mac/AppDelegate.mm
      mac/MacSourceView.mm
      mac/MacSourceViewController.mm
      mac/main.mm
      mac/ToolboxHeaderView.mm
      mac/ToolboxItem.mm
      mac/WindowController.mm)

  set(RESOURCES
      ${RESOURCES}
      mac/marlin.entitlements
      mac/Assets.xcassets
      mac/Base.lproj/Main.storyboard
      mac/ToolboxHeaderView.xib
      mac/ToolboxItem.xib)

  set(TARGET_NAME ${PROJECT_NAME}.app.mac)
  set(FRAMEWORKS "-framework CoreGraphics -framework AppKit")
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
endif(IOS)
