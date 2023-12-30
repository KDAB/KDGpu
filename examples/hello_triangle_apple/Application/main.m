/*
See LICENSES/Apple-Sample-Code.txt for the full license text.

Abstract:
Application entry point for all platforms
*/

#if defined(TARGET_IOS) || defined(TARGET_TVOS)
#import <UIKit/UIKit.h>
#import <TargetConditionals.h>
#import "AAPLAppDelegate.h"
#else
#import <Cocoa/Cocoa.h>
#endif

#if defined(TARGET_IOS) || defined(TARGET_TVOS)

int main(int argc, char *argv[])
{
    @autoreleasepool {
        return UIApplicationMain(argc, argv, nil, NSStringFromClass([AAPLAppDelegate class]));
    }
}

#elif defined(TARGET_MACOS)

int main(int argc, const char *argv[])
{
    return NSApplicationMain(argc, argv);
}

#endif
