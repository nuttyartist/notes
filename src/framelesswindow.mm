#include "framelesswindow.h"
#ifdef Q_OS_MAC
#  include <QDebug>
#  include <Cocoa/Cocoa.h>

CFramelessWindow::CFramelessWindow(QWidget *parent)
    : QMainWindow(parent),
      m_draggableHeight(0),
      m_bWinMoving(false),
      m_bMousePressed(false),
      m_bCloseBtnQuit(true),
      m_bNativeSystemBtn(false),
      m_bIsCloseBtnEnabled(true),
      m_bIsMinBtnEnabled(true),
      m_bIsZoomBtnEnabled(true),
      m_bTitleBarVisible(false)
{
    initUI();
}

@interface AppObserver : NSObject
@property (readwrite) CFramelessWindow *window;
- (void)appDidHide:(NSNotification *)notification;
- (void)appDidUnhide:(NSNotification *)notification;
@end

@implementation AppObserver
- (void)appDidHide:(NSNotification *)notification
{
    self.window->hide();
}
- (void)appDidUnhide:(NSNotification *)notification
{
    self.window->show();
}
@end

// 此类用于支持重载系统按钮的行为
// this Objective-c class is used to override the action of sysytem close button and zoom button
// https://stackoverflow.com/questions/27643659/setting-c-function-as-selector-for-nsbutton-produces-no-results
@interface ButtonPasser : NSObject {
}
@property (readwrite) CFramelessWindow *window;
+ (void)closeButtonAction:(id)sender;
- (void)zoomButtonAction:(id)sender;
@end

@implementation ButtonPasser {
}
+ (void)closeButtonAction:(id)sender
{
    Q_UNUSED(sender);
    [NSApp hide:nil];
}
- (void)zoomButtonAction:(id)sender
{
    Q_UNUSED(sender);
    if (0 == self.window)
        return;

    if (self.window->isFullScreen() || self.window->isMaximized()) {
        self.window->showNormal();
        emit self.window->toggleFullScreen(false);
    } else {
        NSView *view = sender;
        if (0 == view)
            return;
        NSWindow *window = view.window;
        if (0 == window)
            return;

        [window toggleFullScreen:window];
        emit self.window->toggleFullScreen(true);
    }
}
@end

void CFramelessWindow::initUI()
{
    m_bNativeSystemBtn = false;

    // 如果当前osx版本老于10.9，则后续代码不可用。转为使用定制的系统按钮，不支持自由缩放窗口及窗口阴影
    //    if (QSysInfo::MV_None == QSysInfo::macVersion())
    //    {
    //        if (QSysInfo::MV_None == QSysInfo::MacintoshVersion)
    //        {setWindowFlags(Qt::FramelessWindowHint); return;}
    //    }
    //    if (QSysInfo::MV_10_9 >= QSysInfo::MacintoshVersion)
    //    {setWindowFlags(Qt::FramelessWindowHint); return;}

    NSView *view = (NSView *)winId();
    if (0 == view) {
        setWindowFlags(Qt::FramelessWindowHint);
        return;
    }
    NSWindow *window = view.window;
    if (0 == window) {
        setWindowFlags(Qt::FramelessWindowHint);
        return;
    }

    AppObserver *observer = [[AppObserver alloc] init];
    if (observer) {
        observer.window = this;
        [[[NSWorkspace sharedWorkspace] notificationCenter]
                addObserver:observer
                   selector:@selector(appDidHide:)
                       name:NSWorkspaceDidHideApplicationNotification
                     object:nil];
        [[[NSWorkspace sharedWorkspace] notificationCenter]
                addObserver:observer
                   selector:@selector(appDidUnhide:)
                       name:NSWorkspaceDidUnhideApplicationNotification
                     object:nil];
    } else {
        qWarning() << "Failed to set up Notification Observer!";
    }

    // 设置标题文字和图标为不可见
    window.titleVisibility = NSWindowTitleHidden; // MAC_10_10及以上版本支持
    // 设置标题栏为透明
    window.titlebarAppearsTransparent = YES; // MAC_10_10及以上版本支持
    // 设置不可由标题栏拖动,避免与自定义拖动冲突
    [window setMovable:NO]; // MAC_10_6及以上版本支持
    // window.movableByWindowBackground = YES;
    // 设置view扩展到标题栏
    window.styleMask |= NSWindowStyleMaskFullSizeContentView; // MAC_10_10及以上版本支持

    m_bNativeSystemBtn = true;

    ButtonPasser *passer = [[ButtonPasser alloc] init];
    passer.window = this;
    // 重载全屏按钮的行为
    // override the action of fullscreen button
    NSButton *zoomButton = [window standardWindowButton:NSWindowZoomButton];
    [zoomButton setTarget:passer];
    [zoomButton setAction:@selector(zoomButtonAction:)];

    // TODO: move window buttons 2 pixels to the right to align with searchEdit and notesList
    // Currently, doesn't work
    NSButton *closeButton = [window standardWindowButton:NSWindowCloseButton];
    NSButton *minimizeButton = [window standardWindowButton:NSWindowMiniaturizeButton];
    closeButton.frame = NSMakeRect(closeButton.frame.origin.x + 10, closeButton.frame.origin.y,
                                   closeButton.frame.size.width, closeButton.frame.size.height);
    minimizeButton.frame =
            NSMakeRect(minimizeButton.frame.origin.x + 10, minimizeButton.frame.origin.y,
                       minimizeButton.frame.size.width, minimizeButton.frame.size.height);
    zoomButton.frame = NSMakeRect(zoomButton.frame.origin.x + 10, zoomButton.frame.origin.y,
                                  zoomButton.frame.size.width, zoomButton.frame.size.height);
}

void CFramelessWindow::setCloseBtnQuit(bool bQuit)
{
    if (bQuit || !m_bNativeSystemBtn)
        return;
    NSView *view = (NSView *)winId();
    if (0 == view)
        return;
    NSWindow *window = view.window;
    if (0 == window)
        return;

    // 重载关闭按钮的行为
    // override the action of close button
    // https://stackoverflow.com/questions/27643659/setting-c-function-as-selector-for-nsbutton-produces-no-results
    // https://developer.apple.com/library/content/documentation/General/Conceptual/CocoaEncyclopedia/Target-Action/Target-Action.html
    NSButton *closeButton = [window standardWindowButton:NSWindowCloseButton];
    [closeButton setTarget:[ButtonPasser class]];
    [closeButton setAction:@selector(closeButtonAction:)];
}

void CFramelessWindow::setCloseBtnEnabled(bool bEnable)
{
    if (!m_bNativeSystemBtn)
        return;
    NSView *view = (NSView *)winId();
    if (0 == view)
        return;
    NSWindow *window = view.window;
    if (0 == window)
        return;

    m_bIsCloseBtnEnabled = bEnable;
    if (bEnable) {
        [[window standardWindowButton:NSWindowCloseButton] setEnabled:YES];
    } else {
        [[window standardWindowButton:NSWindowCloseButton] setEnabled:NO];
    }
}

void CFramelessWindow::setMinBtnEnabled(bool bEnable)
{
    if (!m_bNativeSystemBtn)
        return;
    NSView *view = (NSView *)winId();
    if (0 == view)
        return;
    NSWindow *window = view.window;
    if (0 == window)
        return;

    m_bIsMinBtnEnabled = bEnable;
    if (bEnable) {
        [[window standardWindowButton:NSWindowMiniaturizeButton] setEnabled:YES];
    } else {
        [[window standardWindowButton:NSWindowMiniaturizeButton] setEnabled:NO];
    }
}

void CFramelessWindow::setZoomBtnEnabled(bool bEnable)
{
    if (!m_bNativeSystemBtn)
        return;
    NSView *view = (NSView *)winId();
    if (0 == view)
        return;
    NSWindow *window = view.window;
    if (0 == window)
        return;

    m_bIsZoomBtnEnabled = bEnable;
    if (bEnable) {
        [[window standardWindowButton:NSWindowZoomButton] setEnabled:YES];
    } else {
        [[window standardWindowButton:NSWindowZoomButton] setEnabled:NO];
    }
}

void CFramelessWindow::setDraggableAreaHeight(int height)
{
    if (height < 0)
        height = 0;
    m_draggableHeight = height;
}

void CFramelessWindow::hideEvent(QHideEvent *event)
{
    // FIXME: Figure out why this fails to trigger our AppObserver listener sometimes. :/
    [NSApp hide:nil];
    QMainWindow::hideEvent(event);
}

void CFramelessWindow::showEvent(QShowEvent *event)
{
    // FIXME: Figure out why this fails to trigger our AppObserver listener sometimes. :/
    [NSApp unhide:nil];
    QMainWindow::showEvent(event);
}

void CFramelessWindow::mousePressEvent(QMouseEvent *event)
{
    if ((event->button() != Qt::LeftButton) || isMaximized()) {
        return QMainWindow::mousePressEvent(event);
    }

    int height = size().height();
    if (m_draggableHeight > 0)
        height = m_draggableHeight;
    QRect rc;
    rc.setRect(0, 0, size().width(), height);
    if (rc.contains(this->mapFromGlobal(QCursor::pos())) == true) // 如果按下的位置
    {
        m_WindowPos = this->pos();
        m_MousePos = event->globalPos();
        m_bMousePressed = true;
    }
    return QMainWindow::mousePressEvent(event);
}

void CFramelessWindow::mouseReleaseEvent(QMouseEvent *event)
{
    m_bWinMoving = false;
    if ((event->button() == Qt::LeftButton)) {
        m_bMousePressed = false;
    }
    return QMainWindow::mouseReleaseEvent(event);
}

void CFramelessWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (!m_bMousePressed)
        return QMainWindow::mouseMoveEvent(event);
    m_bWinMoving = true;
    this->move(m_WindowPos + (event->globalPos() - m_MousePos));
    return QMainWindow::mouseMoveEvent(event);
}

void CFramelessWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    // TODO
    //    if (!isFullScreen())
    //    {
    //        emit restoreFromFullScreen();
    //    }
}

void CFramelessWindow::onRestoreFromFullScreen()
{
    setTitlebarVisible(false);
}

void CFramelessWindow::setTitlebarVisible(bool bTitlebarVisible)
{
    if (!m_bNativeSystemBtn)
        return;
    NSView *view = (NSView *)winId();
    if (0 == view)
        return;
    NSWindow *window = view.window;
    if (0 == window)
        return;

    m_bTitleBarVisible = bTitlebarVisible;
    if (bTitlebarVisible) {
        window.styleMask ^= NSWindowStyleMaskFullSizeContentView; // MAC_10_10及以上版本支持
    } else {
        window.styleMask |= NSWindowStyleMaskFullSizeContentView; // MAC_10_10及以上版本支持
    }
}

void CFramelessWindow::maximizeWindowMac()
{
    NSView *view = (NSView *)winId();
    if (0 == view)
        return;
    NSWindow *window = view.window;
    if (0 == window)
        return;

    [window zoom:window];
}

void CFramelessWindow::setWindowAlwaysOnTopMac(bool isAlwaysOnTop)
{
    NSView *view = (NSView *)winId();
    if (0 == view)
        return;
    NSWindow *window = view.window;
    if (0 == window)
        return;

    if (isAlwaysOnTop)
        [window setLevel:NSFloatingWindowLevel];
    else
        [window setLevel:NSNormalWindowLevel];
}

void CFramelessWindow::setStandardWindowButtonsMacVisibility(bool isVisible)
{
    NSView *view = (NSView *)winId();
    if (0 == view)
        return;
    NSWindow *window = view.window;
    if (0 == window)
        return;

    NSButton *closeButton = [window standardWindowButton:NSWindowCloseButton];
    NSButton *minimizeButton = [window standardWindowButton:NSWindowMiniaturizeButton];
    NSButton *zoomButton = [window standardWindowButton:NSWindowZoomButton];
    [closeButton setHidden:!isVisible];
    [minimizeButton setHidden:!isVisible];
    [zoomButton setHidden:!isVisible];
}

#endif // Q_OS_MAC
