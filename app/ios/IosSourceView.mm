#import "IosSourceView.h"

#include "prototype_definition.hpp"
#include "toolbox_model.hpp"

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
  [self touchAtLocation:location];
}

- (void)showEditorViewControllerFromRect:(CGRect)rect
                                withType:(marlin::control::literal_data_type)type
                                    data:(std::string_view)data {
}

- (BOOL)dropInteraction:(UIDropInteraction*)interaction
       canHandleSession:(id<UIDropSession>)session {
  return [session hasItemsConformingToTypeIdentifiers:@[
           pasteboardOfType(marlin::control::pasteboard_t::statement)
         ]] ||
         [session hasItemsConformingToTypeIdentifiers:@[
           pasteboardOfType(marlin::control::pasteboard_t::expression)
         ]];
}

- (UIDropProposal*)dropInteraction:(UIDropInteraction*)interaction
                  sessionDidUpdate:(id<UIDropSession>)session {
  auto location = [session locationInView:self];
  if ([session hasItemsConformingToTypeIdentifiers:@[
        pasteboardOfType(marlin::control::pasteboard_t::statement)
      ]]) {
    if ([self draggingStatementAtLocation:location]) {
      [self setNeedsDisplayInRect:self.bounds];
      return [[UIDropProposal alloc] initWithDropOperation:UIDropOperationMove];
    }
  } else if ([session hasItemsConformingToTypeIdentifiers:@[
               pasteboardOfType(marlin::control::pasteboard_t::expression)
             ]]) {
    if ([self draggingExpressionAtLocation:location]) {
      [self setNeedsDisplayInRect:self.bounds];
      return [[UIDropProposal alloc] initWithDropOperation:UIDropOperationMove];
    }
  }
  [self setNeedsDisplayInRect:self.bounds];
  return [[UIDropProposal alloc] initWithDropOperation:UIDropOperationCancel];
}

- (void)dropInteraction:(UIDropInteraction*)interaction performDrop:(id<UIDropSession>)session {
  if ([session hasItemsConformingToTypeIdentifiers:@[
        pasteboardOfType(marlin::control::pasteboard_t::statement)
      ]]) {
    for (UIDragItem* item in session.items) {
      [item.itemProvider
          loadDataRepresentationForTypeIdentifier:pasteboardOfType(
                                                      marlin::control::pasteboard_t::statement)
                                completionHandler:^(NSData* _Nullable data,
                                                    NSError* _Nullable error) {
                                  dispatch_async(dispatch_get_main_queue(), ^{
                                    [self performStatementDropForData:data];
                                  });
                                }];
    }
  } else if ([session hasItemsConformingToTypeIdentifiers:@[
               pasteboardOfType(marlin::control::pasteboard_t::expression)
             ]]) {
    for (UIDragItem* item in session.items) {
      [item.itemProvider
          loadDataRepresentationForTypeIdentifier:pasteboardOfType(
                                                      marlin::control::pasteboard_t::expression)
                                completionHandler:^(NSData* _Nullable data,
                                                    NSError* _Nullable error) {
                                  dispatch_async(dispatch_get_main_queue(), ^{
                                    [self performExpressionDropForData:data];
                                  });
                                }];
    }
  }
}

@end
