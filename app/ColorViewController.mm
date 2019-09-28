#import "ColorViewController.h"

#import "mac/ColorWell.h"

@interface ColorViewController ()

@property(weak) IBOutlet ColorWell *colorWall;

@end

@implementation ColorViewController

- (void)setColor:(Color *)color showAlpha:(BOOL)showAlpha {
  self.colorWall.color = color;
  self.colorWall.showAlpha = showAlpha;
}

- (IBAction)okButtonPressed:(id)sender {
  [self.delegate colorViewController:self finishEditingWithColor:self.colorWall.color];
}

@end
