#import "ColorWell.h"

@implementation ColorWell

- (void)activate:(BOOL)exclusive {
  if (self.showAlpha) {
    NSColorPanel.sharedColorPanel.showsAlpha = YES;
  }
  [super activate:exclusive];
}

- (void)deactivate {
  [super deactivate];
  if (self.showAlpha) {
    NSColorPanel.sharedColorPanel.showsAlpha = NO;
  }
}

@end
