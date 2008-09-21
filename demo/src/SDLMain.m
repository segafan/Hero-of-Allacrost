/*   SDLMain.m - main entry point for our Cocoa-ized SDL app
       Initial Version: Darrell Walisser <dwaliss1@purdue.edu>
       Non-NIB-Code & other changes: Max Horn <max@quendi.de>
	   Refactor & cleanup for Hero of Allacrost: Alastair Lynn <arplynn@gmail.com>

    Feel free to customize this file to suit your needs
*/

#import <SDL/SDL.h>
#import <unistd.h>
#import <Cocoa/Cocoa.h>

@interface SDLMain : NSObject
{
}
@end

extern int NXArgc;
extern char** NXArgv;

// Create a window menu
static void SetupWindowMenu()
{
    NSMenu* windowMenu;
    NSMenuItem* windowMenuItem;
    NSMenuItem* menuItem;

    windowMenu = [[NSMenu alloc] initWithTitle:@"Window"];
    
    // "Minimize" item
    menuItem = [[NSMenuItem alloc] initWithTitle:@"Minimize" action:@selector(performMiniaturize:) keyEquivalent:@"m"];
    [windowMenu addItem:menuItem];
    [menuItem release];
    
    // Put menu into the menubar
    windowMenuItem = [[NSMenuItem alloc] initWithTitle:@"Window" action:nil keyEquivalent:@""];
    [windowMenuItem setSubmenu:windowMenu];
    [[NSApp mainMenu] addItem:windowMenuItem];
    
    // Tell the application object that this is now the window menu
    [NSApp setWindowsMenu:windowMenu];

    // Finally give up our references to the objects
    [windowMenu release];
    [windowMenuItem release];
}

// performs many of the same functions as NSApplicationMain
void CustomApplicationMain ()
{
    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
    SDLMain* sdlMain;

    // Ensure the application object is initialised
    [NSApplication sharedApplication];
    
    // Set up the menubar
    [NSApp setMainMenu:[[NSMenu alloc] init]];
    SetupWindowMenu();
    
    // Create SDLMain and make it the app delegate
    sdlMain = [[SDLMain alloc] init];
    [NSApp setDelegate:sdlMain];
    
    // Start the main event loop
    [NSApp run];
    
    [sdlMain release];
    [pool release];
}


// The main class of the application, the application's delegate
@implementation SDLMain

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication*)app
{
    SDL_Event event;
    event.type = SDL_QUIT;
    SDL_PushEvent(&event);
	return NSTerminateCancel;
}

// Set the working directory to the .app's parent directory
- (void)setupWorkingDirectory
{
	NSString* applicationPath = [[NSBundle mainBundle] bundlePath];
	[[NSFileManager defaultManager] changeCurrentDirectoryPath:[applicationPath stringByDeletingLastPathComponent]];
}

// Called when the internal event loop has just started running
- (void)applicationDidFinishLaunching:(NSNotification*) note
{
    int status;

    // Set the working directory to the .app's parent directory
    [self setupWorkingDirectory];

    // Hand off to main application code
    status = SDL_main (NXArgc, NXArgv);

    // We're done, thank you for playing
    exit(status);
}
@end


#ifdef main
#  undef main
#endif

// Main entry point to executable - should *not* be SDL_main!
int main (int argc, char **argv)
{
    CustomApplicationMain();
    return 0;
}
