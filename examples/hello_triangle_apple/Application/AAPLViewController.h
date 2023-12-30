/*
See LICENSES/Apple-Sample-Code.txt for the full license text.

Abstract:
Header for the cross-platform view controller
*/

#if defined(TARGET_IOS) || defined(TARGET_TVOS)
@import UIKit;
#define PlatformViewController UIViewController
#else
@import AppKit;
#define PlatformViewController NSViewController
#endif

#import "AAPLView.h"

@interface AAPLViewController : PlatformViewController <AAPLViewDelegate>

@end
