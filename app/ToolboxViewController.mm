#import "ToolboxViewController.h"

#include "toolbox_model.hpp"

#import "NSData+DataView.h"
#import "NSString+StringView.h"
#import "Pasteboard.h"

@interface ToolboxViewController () <CollectionViewDelegateFlowLayout>

@property(weak) IBOutlet StackView *stackView;

@end

@implementation ToolboxViewController

- (void)viewDidLoad {
  [super viewDidLoad];

  [self createButtonWithTitle:@"r" tag:-1];
  auto index = 0;
  for (auto &section : marlin::control::toolbox_model::sections) {
    auto title = [NSString stringWithStringView:{section.data(), 1}];
    [self createButtonWithTitle:title tag:index++];
  }
}

- (void)sectionButtonPressed:(Button *)sender {
  if (sender.tag >= 0) {
    _currentSection = sender.tag;
    [self.toolboxView reloadData];
  }
}

- (void)createButtonWithTitle:(NSString *)title tag:(NSInteger)tag {
    auto button = [self buttonWithTitle:title action:@selector(sectionButtonPressed:)];
    button.tag = tag;
    button.translatesAutoresizingMaskIntoConstraints = NO;
    [self.stackView addArrangedSubview:button];
    [button.widthAnchor constraintEqualToAnchor:self.stackView.widthAnchor].active = YES;
    [button.widthAnchor constraintEqualToAnchor:button.heightAnchor].active = YES;
}

- (Button*)buttonWithTitle:(NSString*)title action:(SEL)selector {
    NSAssert(NO, @"Implemented by subclass");
    return nil;
}

#pragma mark - CollectionViewDelegateFlowLayout

- (CGSize)collectionView:(CollectionView *)collectionView
                    layout:(CollectionViewLayout *)collectionViewLayout
    sizeForItemAtIndexPath:(NSIndexPath *)indexPath {
  return CGSizeMake(collectionView.bounds.size.width - 10, 25);
}

@end
