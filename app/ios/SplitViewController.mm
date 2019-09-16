#import "SplitViewController.h"

#import "IosSourceViewController.h"
#import "IosToolboxViewController.h"
#import "NSObject+Casting.h"

@interface SplitViewController ()

@property(weak, nonatomic) IBOutlet UIView *leftView;
@property(weak, nonatomic) IBOutlet UIView *rightView;

@end

@implementation SplitViewController

- (void)viewDidLoad {
  [self addChildViewControllerForView:self.leftView identifier:@"ToolboxViewController"];
  [self addChildViewControllerForView:self.rightView identifier:@"SourceViewController"];
  NSAssert(self.childViewControllers.count > 0, @"Should has child view controllers");
}

- (void)addChildViewControllerForView:(UIView *)view identifier:(NSString *)identifier {
  auto vc = [self.storyboard instantiateViewControllerWithIdentifier:identifier];
  [self addChildViewController:vc];
  [view addSubview:vc.view];
  vc.view.translatesAutoresizingMaskIntoConstraints = NO;
  [vc.view.topAnchor constraintEqualToAnchor:view.topAnchor].active = YES;
  [vc.view.bottomAnchor constraintEqualToAnchor:view.bottomAnchor].active = YES;
  [vc.view.leftAnchor constraintEqualToAnchor:view.leftAnchor].active = YES;
  [vc.view.rightAnchor constraintEqualToAnchor:view.rightAnchor].active = YES;
  [vc didMoveToParentViewController:self];
}

- (IBAction)close:(id)sender {
  [self.navigationController dismissViewControllerAnimated:YES completion:nil];
}

- (IBAction)run:(id)sender {
  [self.sourceViewController execute];
}

- (void)setDocument:(IosDocument *)document {
  self.sourceViewController.document = document;
  [self.toolboxViewController registerModelToDocument:document.content];
}

- (IosToolboxViewController *)toolboxViewController {
  for (id vc in self.childViewControllers) {
    if (auto sourceVC = [IosToolboxViewController cast:vc]) {
      return sourceVC;
    }
  }
  NSAssert(NO, @"Should has toolbox view controller");
  return nil;
}

- (IosSourceViewController *)sourceViewController {
  for (id vc in self.childViewControllers) {
    if (auto sourceVC = [IosSourceViewController cast:vc]) {
      return sourceVC;
    }
  }
  NSAssert(NO, @"Should has source view controller");
  return nil;
}

@end
