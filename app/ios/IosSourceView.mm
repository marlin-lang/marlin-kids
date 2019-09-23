#import "IosSourceView.h"

#import "NSString+StringView.h"
#import "Pasteboard.h"

@interface IosSourceView () <UIDropInteractionDelegate>

@property(weak, nonatomic) UIScrollView* enclosingScrollView;

@end

@implementation IosSourceView

- (instancetype)initWithEnclosingScrollView:(UIScrollView*)enclosingScrollView
                                 dataSource:(id<SourceViewDataSource>)dataSource {
  if (self = [super init]) {
    self.backgroundColor = UIColor.whiteColor;
    _enclosingScrollView = enclosingScrollView;
    self.dataSource = dataSource;
    auto* dropInteraction = [[UIDropInteraction alloc] initWithDelegate:self];
    [self addInteraction:dropInteraction];
  }
  return self;
}

- (void)setFrame:(CGRect)frame {
  [super setFrame:frame];
  self.enclosingScrollView.contentSize = frame.size;
  [self.lineNumberView setNeedsDisplay];
}

- (void)touchesBegan:(NSSet<UITouch*>*)touches withEvent:(UIEvent*)event {
  auto location = [[touches anyObject] locationInView:self];
  [self touchDownAtLocation:location];
}

- (void)touchesEnded:(NSSet<UITouch*>*)touches withEvent:(UIEvent*)event {
  [self touchUp];
}

- (void)showEditorViewControllerFromRect:(CGRect)rect
                                withType:(EditorType)type
                                    data:(std::string_view)data {
}

- (BOOL)dropInteraction:(UIDropInteraction*)interaction
       canHandleSession:(id<UIDropSession>)session {
  return
      [session hasItemsConformingToTypeIdentifiers:@[ pasteboardOfType(
                                                       marlin::control::pasteboard_t::block) ]] ||
      [session hasItemsConformingToTypeIdentifiers:@[
        pasteboardOfType(marlin::control::pasteboard_t::statement)
      ]] ||
      [session hasItemsConformingToTypeIdentifiers:@[
        pasteboardOfType(marlin::control::pasteboard_t::expression)
      ]] ||
      [session hasItemsConformingToTypeIdentifiers:@[ pasteboardOfType(
                                                       marlin::control::pasteboard_t::reference) ]];
}

- (UIDropProposal*)dropInteraction:(UIDropInteraction*)interaction
                  sessionDidUpdate:(id<UIDropSession>)session {
  if (auto type = [self pasteboardTypeInSession:session]) {
    auto location = [session locationInView:self];
    if ([self draggingPasteboardOfType:*type toLocation:location]) {
      [self setNeedsDisplayInRect:self.bounds];
      return [[UIDropProposal alloc] initWithDropOperation:UIDropOperationMove];
    }
  }
  [self setNeedsDisplayInRect:self.bounds];
  return [[UIDropProposal alloc] initWithDropOperation:UIDropOperationCancel];
}

- (void)dropInteraction:(UIDropInteraction*)interaction performDrop:(id<UIDropSession>)session {
  if (auto type = [self pasteboardTypeInSession:session]) {
    for (UIDragItem* item in session.items) {
      [item.itemProvider loadDataRepresentationForTypeIdentifier:pasteboardOfType(*type)
                                               completionHandler:^(NSData* _Nullable data,
                                                                   NSError* _Nullable error) {
                                                 dispatch_async(dispatch_get_main_queue(), ^{
                                                   [self dropPasteboardOfType:*type
                                                                     withData:data
                                                        removingCurrentSource:YES];
                                                 });
                                               }];
    }
  }
}

#pragma mark - Private methods

- (std::optional<marlin::control::pasteboard_t>)pasteboardTypeInSession:(id<UIDropSession>)session {
  if ([session hasItemsConformingToTypeIdentifiers:@[ pasteboardOfType(
                                                       marlin::control::pasteboard_t::block) ]]) {
    return marlin::control::pasteboard_t::block;
  } else if ([session hasItemsConformingToTypeIdentifiers:@[
               pasteboardOfType(marlin::control::pasteboard_t::statement)
             ]]) {
    return marlin::control::pasteboard_t::statement;
  } else if ([session hasItemsConformingToTypeIdentifiers:@[
               pasteboardOfType(marlin::control::pasteboard_t::expression)
             ]]) {
    return marlin::control::pasteboard_t::expression;
  } else if ([session hasItemsConformingToTypeIdentifiers:@[
               pasteboardOfType(marlin::control::pasteboard_t::reference)
             ]]) {
    return marlin::control::pasteboard_t::reference;
  } else {
    return std::nullopt;
  }
}

@end
