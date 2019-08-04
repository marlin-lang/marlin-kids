#import "Document.h"

#import <optional>

#import "NSObject+Casting.h"
#import "SourceViewController.h"

@interface Document () {
  std::optional<marlin::control::document> _content;
}

@property(nonatomic, strong) NSData *initialData;

@end

@implementation Document

+ (BOOL)autosavesInPlace {
  return YES;
}

- (instancetype)init {
  if (self = [super init]) {
    self.initialData = nil;
  }
  return self;
}

- (marlin::control::source_initialization)initialize {
  auto [doc, source] = marlin::control::document::make_document(nullptr, 0);
  self.initialData = nil;
  _content = std::move(doc);
  return source;
}

- (marlin::control::document &)content {
  return *_content;
}

- (void)makeWindowControllers {
  NSWindowController *controller = [[NSStoryboard storyboardWithName:@"Main" bundle:nil]
      instantiateControllerWithIdentifier:@"Document Window Controller"];
  [self addWindowController:controller];
  if (auto *vc = [SourceViewController cast:controller.contentViewController]) {
    vc.document = self;
  }
}

- (NSData *)dataOfType:(NSString *)typeName error:(NSError **)outError {
  // Insert code here to write your document to data of the specified type. If outError != NULL,
  // ensure that you create and set an appropriate error if you return nil. Alternatively, you could
  // remove this method and override -fileWrapperOfType:error:, -writeToURL:ofType:error:, or
  // -writeToURL:ofType:forSaveOperation:originalContentsURL:error: instead.
  [NSException raise:@"UnimplementedMethod"
              format:@"%@ is unimplemented", NSStringFromSelector(_cmd)];
  return nil;
}

- (BOOL)readFromData:(NSData *)data ofType:(NSString *)typeName error:(NSError **)outError {
  self.initialData = data;
  // NSString *source = [[NSString alloc] initWithData:data encoding:NSASCIIStringEncoding];
  // _content = marlin::control::document{source.UTF8String};
  // [self update];
  return YES;
}

@end
