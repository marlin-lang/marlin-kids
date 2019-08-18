#import "Document.h"

#import <optional>

#import "NSData+DataView.h"
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

- (std::optional<marlin::control::source_initialization>)initialize {
  if (auto result{self.initialData == nil
                      ? marlin::control::document::make_document()
                      : marlin::control::document::make_document(self.initialData.dataView)}) {
    auto [doc, source]{*std::move(result)};
    self.initialData = nil;
    _content = std::move(doc);
    return source;
  } else {
    return std::nullopt;
  }
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
  return [NSData dataWithDataView:_content->write()];
}

- (BOOL)readFromData:(NSData *)data ofType:(NSString *)typeName error:(NSError **)outError {
  self.initialData = data;
  return YES;
}

@end
