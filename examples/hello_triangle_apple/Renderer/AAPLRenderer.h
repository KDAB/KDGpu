/*
See LICENSES/Apple-Sample-Code.txt for the full license text.

Abstract:
Metal Renderer for Metal View. Acts as the update and render delegate for the view controller and performs rendering.
*/

#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>

@interface AAPLRenderer : NSObject
- (nonnull instancetype)initWithMetalLayer:(nonnull CAMetalLayer *)layer;
- (void)render;
- (void)drawableResize:(CGSize)drawableSize;
@end
