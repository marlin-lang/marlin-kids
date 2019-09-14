#import "ToolboxViewController.h"

#include "toolbox_model.hpp"

#import "NSData+DataView.h"
#import "NSString+StringView.h"
#import "Pasteboard.h"

@interface ToolboxViewController () <CollectionViewDelegateFlowLayout>

@property(weak) IBOutlet StackView *stackView;

@end

@implementation ToolboxViewController {
  __weak Button *_currentSectionButton;
}

- (void)viewDidLoad {
  [super viewDidLoad];

  [self createButtonWithTitle:@"r" tag:-1];
  auto index = 0;
  for (auto &section : marlin::control::toolbox_model::sections) {
    auto title = [NSString stringWithStringView:{section.data(), 1}];
    [self createButtonWithTitle:title tag:index++];
  }
}

- (NSInteger)currentSection {
  return _currentSectionButton.tag;
}

- (void)sectionButtonPressed:(Button *)sender {
  if (sender.tag >= 0) {
    [self setCurrentSectionButton:sender];
    [self.toolboxView reloadData];
  }
}

- (void)createButtonWithTitle:(NSString *)title tag:(NSInteger)tag {
  auto button = [self buttonWithTitle:title action:@selector(sectionButtonPressed:)];
  button.tag = tag;
  if (tag == 0) {
    [self setCurrentSectionButton:button];
  }
  button.translatesAutoresizingMaskIntoConstraints = NO;
  [self.stackView addArrangedSubview:button];
  [button.widthAnchor constraintEqualToAnchor:self.stackView.widthAnchor].active = YES;
  [button.widthAnchor constraintEqualToAnchor:button.heightAnchor].active = YES;
}

- (Button *)buttonWithTitle:(NSString *)title action:(SEL)selector {
  NSAssert(NO, @"Implemented by subclass");
  return nil;
}

- (void)setCurrentSectionButton:(Button *)button {
  [self setBackgroundColor:Color.whiteColor forButton:_currentSectionButton];
  _currentSectionButton = button;
  [self setBackgroundColor:[Color colorWithWhite:0.92 alpha:1] forButton:_currentSectionButton];
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
