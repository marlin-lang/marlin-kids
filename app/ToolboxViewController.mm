#import "ToolboxViewController.h"

#import "NSString+StringView.h"

using ToolIndex = std::pair<NSInteger, NSInteger>;

@interface ToolboxViewController () <CollectionViewDelegateFlowLayout>

@property(weak) IBOutlet StackView *categoryStackView;

@end

@implementation ToolboxViewController {
  marlin::control::toolbox _model;

  std::vector<ToolIndex> _recentTools;
  __weak Button *_currentCategoryButton;
}

- (void)viewDidLoad {
  [super viewDidLoad];

  for (size_t i = 0; i < _model.categories().size(); i++) {
    auto &category = _model.categories()[i];
    auto title = [NSString stringWithStringView:{category.name.data(), 1}];
    [self createButtonWithTitle:title tag:i];
  }
}

- (marlin::control::toolbox &)model {
  return _model;
}

- (void)addRecentForCurrentCategoryItem:(NSInteger)item {
  if (_model.current_category().type != marlin::control::toolbox::category::category_type::recent) {
    auto it = std::find(_recentTools.begin(), _recentTools.end(),
                        ToolIndex{_currentCategoryButton.tag, item});
    if (it != _recentTools.end()) {
      _recentTools.erase(it);
    }
    _recentTools.insert(_recentTools.begin(), {_currentCategoryButton.tag, item});
    constexpr size_t maxRecentTools = 20;
    if (_recentTools.size() > maxRecentTools) {
      _recentTools.erase(_recentTools.begin() + maxRecentTools, _recentTools.end());
    }
  }
}

- (void)sectionButtonPressed:(Button *)sender {
  [self setCurrentcategoryButton:sender];
  [self.toolboxView reloadData];
}

- (void)createButtonWithTitle:(NSString *)title tag:(NSInteger)tag {
  auto button = [self buttonWithTitle:title action:@selector(sectionButtonPressed:)];
  button.tag = tag;
  if (tag == 0) {
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
  _model.set_current_category(button.tag);
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
