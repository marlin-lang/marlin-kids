#import "MacColorViewController.h"

#import "ColorWell.h"

@interface MacColorViewController ()

@property(weak) IBOutlet ColorWell *colorWall;

@end

@implementation MacColorViewController

- (void)setColor:(Color *)color showAlpha:(BOOL)showAlpha {
  self.colorWall.color = color;
  self.colorWall.showAlpha = showAlpha;
}

- (IBAction)okButtonPressed:(id)sender {
  [self.delegate colorViewController:self finishEditingWithColor:self.colorWall.color];
}

@end
