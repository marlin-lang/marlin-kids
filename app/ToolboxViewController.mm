#import "ToolboxViewController.h"

#import "NSString+StringView.h"
#import "Pasteboard.h"

using ToolIndex = std::pair<NSInteger, NSInteger>;

@interface ToolboxViewController () <CollectionViewDelegateFlowLayout>

@property(weak) IBOutlet StackView *categoryStackView;

@end

@implementation ToolboxViewController {
  std::vector<ToolIndex> _recentTools;
  __weak Button *_currentCategoryButton;
}

- (void)viewDidLoad {
  [super viewDidLoad];

  [self createButtonWithTitle:@"r" tag:-1];
  auto index = 0;
  for (auto &section : marlin::control::toolbox_model::categories) {
    auto title = [NSString stringWithStringView:{section.data(), 1}];
    [self createButtonWithTitle:title tag:index++];
  }
}

- (NSInteger)currentCategory {
  return _currentCategoryButton.tag;
}

- (NSInteger)sizeOfCurrentCategory {
  if (_currentCategoryButton.tag >= 0) {
    return marlin::control::toolbox_model::items[_currentCategoryButton.tag].size();
  } else {
    return _recentTools.size();
  }
}

- (const marlin::control::base_prototype &)prototypeOfCurrentCategoryItem:(NSInteger)item {
  if (_currentCategoryButton.tag >= 0) {
    return marlin::control::toolbox_model::prototype_at(_currentCategoryButton.tag, item);
  } else {
    auto [c, i] = _recentTools[item];
    return marlin::control::toolbox_model::prototype_at(c, i);
  }
}

- (NSString *)pasteboardTypeOfCurrentCategoryItem:(NSInteger)item {
  if (_currentCategoryButton.tag >= 0) {
    return pasteboardOfType(
        marlin::control::toolbox_model::items[_currentCategoryButton.tag][item].type);
  } else {
    auto [c, i] = _recentTools[item];
    return pasteboardOfType(marlin::control::toolbox_model::items[c][i].type);
  }
}

- (void)addRecentForCurrentCategoryItem:(NSInteger)item {
  if (_currentCategoryButton.tag >= 0) {
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
