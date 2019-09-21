#import "ToolboxViewController.h"

#include <memory>

#import "mac/MacFunctionViewController.h"

using ToolIndex = std::pair<NSInteger, NSInteger>;

@interface ToolboxViewController () <CollectionViewDelegateFlowLayout>

@property(weak) IBOutlet View *editorView;
@property(weak) IBOutlet NSLayoutConstraint *editorViewHeightConstraint;

@property(weak) IBOutlet StackView *categoryStackView;

@end

@implementation ToolboxViewController {
  std::shared_ptr<marlin::control::toolbox> _model;

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

- (void)showEditorViewControllerForSourceView:(SourceView *)view
                                     withType:(marlin::control::literal_data_type)type
                                         data:(std::string_view)data {
  [self dismissEditorViewControllers];
  EditorViewController *vc =
      [self.storyboard instantiateControllerWithIdentifier:@"EditorViewController"];
  vc.delegate = view;

  [self addChildViewController:vc];
  [self.editorView addSubview:vc.view];
  self.editorViewHeightConstraint.constant = vc.view.bounds.size.height;
  vc.view.translatesAutoresizingMaskIntoConstraints = NO;
  [vc.view.leftAnchor constraintEqualToAnchor:self.editorView.leftAnchor].active = YES;
  [vc.view.rightAnchor constraintEqualToAnchor:self.editorView.rightAnchor].active = YES;
  [vc.view.topAnchor constraintEqualToAnchor:self.editorView.topAnchor].active = YES;
  [vc.view.bottomAnchor constraintEqualToAnchor:self.editorView.bottomAnchor].active = YES;
#ifdef IOS
  [vc didMoveToParentViewController:self];
#endif

  vc.type = type;
  vc.editorTextField.stringValue = [NSString stringWithStringView:data];
}

- (void)showFunctionViewControllerForSourceView:(SourceView *)view
                          withFunctionSignature:(marlin::function_definition)signature {
  [self dismissEditorViewControllers];
  MacFunctionViewController *vc =
      [self.storyboard instantiateControllerWithIdentifier:@"FunctionViewController"];
  vc.delegate = view;

  [self addChildViewController:vc];
  [self.editorView addSubview:vc.view];
  self.editorViewHeightConstraint.constant = vc.view.bounds.size.height;
  vc.view.translatesAutoresizingMaskIntoConstraints = NO;
  [vc.view.leftAnchor constraintEqualToAnchor:self.editorView.leftAnchor].active = YES;
  [vc.view.rightAnchor constraintEqualToAnchor:self.editorView.rightAnchor].active = YES;
  [vc.view.topAnchor constraintEqualToAnchor:self.editorView.topAnchor].active = YES;
  [vc.view.bottomAnchor constraintEqualToAnchor:self.editorView.bottomAnchor].active = YES;
#ifdef IOS
  [vc didMoveToParentViewController:self];
#endif

  [vc setFunctionSignature:std::move(signature)];
}

- (void)dismissEditorViewControllers {
  for (ViewController *vc in self.childViewControllers) {
#ifdef IOS
    [vc.willMoveToParentViewController:nil];
#endif
    [vc.view removeFromSuperview];
    [vc removeFromParentViewController];
  }
  self.editorViewHeightConstraint.constant = 0;
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

#pragma mark - CollectionViewDelegateFlowLayout

- (CGSize)collectionView:(CollectionView *)collectionView
                    layout:(CollectionViewLayout *)collectionViewLayout
    sizeForItemAtIndexPath:(NSIndexPath *)indexPath {
  return CGSizeMake(collectionView.bounds.size.width - 10, 25);
}

@end
