#include "macminimizewidget.h"
#include <Cocoa/Cocoa.h>


void MacMinimizeWidget::AllowMinimizeForFramelessWindow(QWidget *window)
{
#if defined __APPLE__ && __MAC_OS_X_VERSION_MAX_ALLOWED >= 101300
  if (@available(macOS 10.13, *)) {
    NSWindow *nsWindow = [(NSView *)(window->winId()) window];
    [nsWindow
        setStyleMask:(NSWindowStyleMaskResizable | NSWindowStyleMaskTitled |
                      NSWindowStyleMaskFullSizeContentView |
                      NSWindowStyleMaskMiniaturizable)];
    [nsWindow setTitlebarAppearsTransparent:YES];      // 10.10+
    [nsWindow setTitleVisibility:NSWindowTitleHidden]; // 10.10+
    [nsWindow setShowsToolbarButton:NO];
    [[nsWindow standardWindowButton:NSWindowFullScreenButton] setHidden:YES];
    [[nsWindow standardWindowButton:NSWindowMiniaturizeButton] setHidden:YES];
    [[nsWindow standardWindowButton:NSWindowCloseButton] setHidden:YES];
    [[nsWindow standardWindowButton:NSWindowZoomButton] setHidden:YES];
  }
#else
  Q_UNUSED(window);
#endif
}
