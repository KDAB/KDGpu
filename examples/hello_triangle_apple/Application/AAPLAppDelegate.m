/*
See LICENSES/Apple-Sample-Code.txt for the full license text.

Abstract:
Application delegate for Metal Sample Code
*/

#import "AAPLAppDelegate.h"

@implementation AAPLAppDelegate

#if TARGET_MACOS
// Close app when window is closed
- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)app
{
    return YES;
}
#endif

@end
