include(ios.toolchain)

set(HEADERS
    AppDelegate.h
    Document.h
    DocumentBrowserViewController.h
    SourceViewController.h)

set(SOURCES
    AppDelegate.mm
    Document.mm
    DocumentBrowserViewController.mm
    SourceViewController.mm
    main.mm)

set(RESOURCES Info.plist Assets.xcassets Base.lproj/Main.storyboard)

add_executable(${PROJECT_NAME}.ios MACOSX_BUNDLE ${SOURCES} ${RESOURCES})
target_sources(${PROJECT_NAME}.ios PRIVATE ${HEADERS})

set_target_properties(${PROJECT_NAME}.ios
                      PROPERTIES OUTPUT_NAME
                                 marlin
                                 MACOSX_BUNDLE_INFO_PLIST
                                 "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist"
                                 RESOURCE
                                 "${RESOURCES}")

target_link_libraries(${PROJECT_NAME}.ios "-framework UIKit"
                      ${PROJECT_NAME}.core)

# XCode compile options
set_target_properties(
  ${PROJECT_NAME}.ios
  PROPERTIES XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET
             "11.0"
             XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY
             "1,2"
             XCODE_ATTRIBUTE_CLANG_ENABLE_MODULES
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
