set(HEADERS
    AppDelegate.h
    Document.h
    EditorViewController.h
    ExecuteViewController.h
    LineNumberView.h
    MessageViewController.h
    NSData+DataView.h
    NSObject+Casting.h
    NSString+StringView.h
    Pasteboard.h
    SourceTextView.h
    SourceTheme.h
    SourceViewController.h
    ToolBoxHeaderView.h
    ToolBoxItem.h
    WindowController.h)

set(SOURCES
    AppDelegate.mm
    Document.mm
    EditorViewController.mm
    ExecuteViewController.mm
    LineNumberView.mm
    main.mm
    MessageViewController.mm
    NSData+DataView.mm
    NSObject+Casting.mm
    NSString+StringView.mm
    SourceTextView.mm
    SourceTheme.mm
    SourceViewController.mm
    ToolBoxHeaderView.mm
    ToolBoxItem.mm
    WindowController.mm)

set(RESOURCES
    Info.plist
    marlin.entitlements
    Assets.xcassets
    Base.lproj/Main.storyboard
    ToolBoxHeaderView.xib
    ToolBoxItem.xib)

add_executable(${PROJECT_NAME}.mac MACOSX_BUNDLE ${SOURCES} ${RESOURCES})
target_sources(${PROJECT_NAME}.mac PRIVATE ${HEADERS})

set_target_properties(${PROJECT_NAME}.mac
                      PROPERTIES OUTPUT_NAME
                                 marlin
                                 MACOSX_BUNDLE_INFO_PLIST
                                 "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist"
                                 RESOURCE
                                 "${RESOURCES}")

target_link_libraries(${PROJECT_NAME}.mac "-framework AppKit"
                      ${PROJECT_NAME}.core)

# XCode compile options
set_target_properties(
  ${PROJECT_NAME}.mac
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
