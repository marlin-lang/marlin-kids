#import "ToolboxViewController.h"

#include <memory>

#import "DuplicateViewController.h"
#import "mac/MacFunctionViewController.h"

using ToolIndex = std::pair<NSInteger, NSInteger>;

@interface ToolboxViewController () <CollectionViewDelegateFlowLayout>

@property(weak) IBOutlet View *duplicateView;
@property(weak) IBOutlet NSLayoutConstraint *duplicateViewHeightConstraint;

@property(weak) IBOutlet View *editorView;
@property(weak) IBOutlet NSLayoutConstraint *editorViewHeightConstraint;

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

- (void)showDuplicateViewControllerForSourceView:(SourceView *)view {
  [self dismissDuplicateViewController];
  DuplicateViewController *vc =
      [self.storyboard instantiateControllerWithIdentifier:@"DuplicateViewController"];
  vc.delegate = view;
  [self addChildViewController:vc
                        inView:self.duplicateView
              heightConstraint:self.duplicateViewHeightConstraint];
  _duplicateViewController = vc;
}

- (void)showEditorViewControllerForSourceView:(SourceView *)view
                                     withType:(marlin::control::literal_data_type)type
                                         data:(std::string_view)data {
  [self dismissEditorViewController];
  EditorViewController *vc =
      [self.storyboard instantiateControllerWithIdentifier:@"EditorViewController"];
  vc.delegate = view;

  [self addChildViewController:vc
                        inView:self.editorView
              heightConstraint:self.editorViewHeightConstraint];

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

  [self addChildViewController:vc
                        inView:self.editorView
              heightConstraint:self.editorViewHeightConstraint];

  [vc setFunctionSignature:std::move(signature)];
  _editorViewController = vc;
}

- (void)dismissEditorViewController {
  if (_editorViewController) {
    [self removeChildViewController:_editorViewController
                   heightConstraint:self.editorViewHeightConstraint];
    _editorViewController = nil;
  }
}

- (void)dismissChildViewControllers {
  [self dismissDuplicateViewController];
  [self dismissEditorViewController];
}

#pragma mark - CollectionViewDelegateFlowLayout

- (CGSize)collectionView:(CollectionView *)collectionView
                    layout:(CollectionViewLayout *)collectionViewLayout
    sizeForItemAtIndexPath:(NSIndexPath *)indexPath {
  return CGSizeMake(collectionView.bounds.size.width - 10, 25);
}

#pragma mark - Private Methods

- (void)addChildViewController:(ViewController *)vc
                        inView:(View *)view
              heightConstraint:(NSLayoutConstraint *)heightConstraint {
  [self addChildViewController:vc];
  [view addSubview:vc.view];
  heightConstraint.constant = vc.view.bounds.size.height;
  vc.view.translatesAutoresizingMaskIntoConstraints = NO;
  [vc.view.leftAnchor constraintEqualToAnchor:view.leftAnchor].active = YES;
  [vc.view.rightAnchor constraintEqualToAnchor:view.rightAnchor].active = YES;
  [vc.view.topAnchor constraintEqualToAnchor:view.topAnchor].active = YES;
  [vc.view.bottomAnchor constraintEqualToAnchor:view.bottomAnchor].active = YES;
#ifdef IOS
  [vc didMoveToParentViewController:self];
#endif
}

- (void)removeChildViewController:(ViewController *)vc
                 heightConstraint:(NSLayoutConstraint *)heightConstraint {
#ifdef IOS
  [vc willMoveToParentViewController:nil];
#endif
  [vc.view removeFromSuperview];
  [vc removeFromParentViewController];
  heightConstraint.constant = 0;
}

- (void)dismissDuplicateViewController {
  if (_duplicateViewController) {
    [self removeChildViewController:_duplicateViewController
                   heightConstraint:self.duplicateViewHeightConstraint];
    _duplicateViewController = nil;
  }
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
