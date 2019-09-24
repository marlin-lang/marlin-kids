#import "ToolboxViewController.h"

#include <memory>

#import "DuplicateViewController.h"
#import "mac/MacFunctionViewController.h"

using ToolIndex = std::pair<NSInteger, NSInteger>;

@interface ToolboxViewController () <CollectionViewDelegateFlowLayout>

@property(weak) IBOutlet View *duplicateView;
@property(weak) IBOutlet View *editorView;
@property(weak) IBOutlet StackView *categoryStackView;

@end

@implementation ToolboxViewController {
  std::shared_ptr<marlin::control::toolbox> _model;

  __weak ViewController *_duplicateViewController;
  __weak ViewController *_editorViewController;
  __weak Button *_currentCategoryButton;
}

- (void)viewDidLoad {
  [super viewDidLoad];

  _model = std::make_shared<marlin::control::toolbox>();

  for (size_t i = 0; i < self.model.categories().size(); i++) {
    auto &category = self.model.categories()[i];
    auto title = [NSString stringWithStringView:{category.name.data(), 1}];
    [self createButtonWithTitle:title tag:i];
  }
}

- (marlin::control::toolbox &)model {
  return *_model;
}

- (void)registerModelToDocument:(marlin::control::document &)document {
  document.register_toolbox(_model);
}

- (void)showArrayViewControllerForSourceView:(SourceView *)view
                                   withCount:(NSUInteger)count
                                minimalCount:(NSUInteger)minimalCount {
  [self dismissEditorViewController];
  ArrayViewController *vc =
      [self.storyboard instantiateControllerWithIdentifier:@"ArrayViewController"];
  vc.delegate = view;

  [self addChildViewController:vc inView:self.editorView];

  vc.minimalCount = minimalCount;
  vc.countTextField.stringValue = [NSString stringWithFormat:@"%lu", count];
  _editorViewController = vc;
}

- (void)showDuplicateViewControllerForSourceView:(SourceView *)view
                                      withString:(NSString *)string
                                    draggingData:(const DraggingData &)draggingData {
  [self dismissDuplicateViewController];
  DuplicateViewController *vc =
      [self.storyboard instantiateControllerWithIdentifier:@"DuplicateViewController"];
  vc.delegate = view;
  [self addChildViewController:vc inView:self.duplicateView];
  vc.sourceString = string;
  vc.draggingData = draggingData;
  _duplicateViewController = vc;
}

- (void)showEditorViewControllerForSourceView:(SourceView *)view
                                     withType:(EditorType)type
                                         data:(std::string_view)data {
  [self dismissEditorViewController];
  EditorViewController *vc =
      [self.storyboard instantiateControllerWithIdentifier:@"EditorViewController"];
  vc.delegate = view;

  [self addChildViewController:vc inView:self.editorView];

  vc.type = type;
  vc.editorTextField.stringValue = [NSString stringWithStringView:data];
  _editorViewController = vc;
}

- (void)showFunctionViewControllerForSourceView:(SourceView *)view
                          withFunctionSignature:(marlin::function_definition)signature {
  [self dismissEditorViewController];
  MacFunctionViewController *vc =
      [self.storyboard instantiateControllerWithIdentifier:@"FunctionViewController"];
  vc.delegate = view;

  [self addChildViewController:vc inView:self.editorView];

  [vc setFunctionSignature:std::move(signature)];
  _editorViewController = vc;
}

- (void)dismissDuplicateViewController {
  if (_duplicateViewController) {
    [self removeChildViewController:_duplicateViewController];
    _duplicateViewController = nil;
  }
}

- (void)dismissEditorViewController {
  if (_editorViewController) {
    [self removeChildViewController:_editorViewController];
    _editorViewController = nil;
  }
}

#pragma mark - CollectionViewDelegateFlowLayout

- (CGSize)collectionView:(CollectionView *)collectionView
                    layout:(CollectionViewLayout *)collectionViewLayout
    sizeForItemAtIndexPath:(NSIndexPath *)indexPath {
  return CGSizeMake(collectionView.bounds.size.width - 10, 25);
}

#pragma mark - Private Methods

- (void)addChildViewController:(ViewController *)vc inView:(View *)view {
  [self addChildViewController:vc];
  [view addSubview:vc.view];
  vc.view.translatesAutoresizingMaskIntoConstraints = NO;
  [vc.view.leftAnchor constraintEqualToAnchor:view.leftAnchor].active = YES;
  [vc.view.rightAnchor constraintEqualToAnchor:view.rightAnchor].active = YES;
  [vc.view.topAnchor constraintEqualToAnchor:view.topAnchor].active = YES;
  [vc.view.bottomAnchor constraintEqualToAnchor:view.bottomAnchor].active = YES;
#ifdef IOS
  [vc didMoveToParentViewController:self];
#endif
}

- (void)removeChildViewController:(ViewController *)vc {
#ifdef IOS
  [vc willMoveToParentViewController:nil];
#endif
  [vc.view removeFromSuperview];
  [vc removeFromParentViewController];
}

- (void)sectionButtonPressed:(Button *)sender {
  [self setCurrentcategoryButton:sender];
  [self.toolboxView reloadData];
}

- (void)createButtonWithTitle:(NSString *)title tag:(NSInteger)tag {
  auto button = [self buttonWithTitle:title action:@selector(sectionButtonPressed:)];
  button.tag = tag;
  if (tag == self.model.current_category_index()) {
    [self setCurrentcategoryButton:button];
  }
  button.translatesAutoresizingMaskIntoConstraints = NO;
  [self.categoryStackView addArrangedSubview:button];
  [button.widthAnchor constraintEqualToAnchor:self.categoryStackView.widthAnchor].active = YES;
  [button.widthAnchor constraintEqualToAnchor:button.heightAnchor].active = YES;
}

- (Button *)buttonWithTitle:(NSString *)title action:(SEL)selector {
  NSAssert(NO, @"Implemented by subclass");
  return nil;
}

- (void)setCurrentcategoryButton:(Button *)button {
  [self setBackgroundColor:Color.whiteColor forButton:_currentCategoryButton];
  _currentCategoryButton = button;
  self.model.set_current_category(button.tag);
  [self setBackgroundColor:[Color colorWithWhite:0.92 alpha:1] forButton:_currentCategoryButton];
}

- (void)setBackgroundColor:(Color *)color forButton:(Button *)button {
  NSAssert(NO, @"Implemented by subclass");
}

@end
