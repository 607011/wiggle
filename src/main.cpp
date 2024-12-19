#include <ApplicationServices/ApplicationServices.h>
#include <CoreFoundation/CoreFoundation.h>
#include <CoreGraphics/CoreGraphics.h>
#include <IOKit/IOKitLib.h>
#ifdef WITH_DISPLAY_SLEEP
#include <IOKit/pwr_mgt/IOPM.h>
#include <IOKit/pwr_mgt/IOPMLib.h>
#include <IOKit/pwr_mgt/IOPMLibDefs.h>
#endif
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <thread>

constexpr int64_t DEFAULT_IDLE_TIMEOUT_SECS{30};
constexpr std::chrono::duration DEFAULT_CHECK_INTERVAL{std::chrono::milliseconds(1000)};

#ifdef WITH_DISPLAY_SLEEP
constexpr int64_t DEFAULT_DISPLAY_SLEEP_SECS{120};
#endif

int64_t system_idle_secs(void)
{
    io_iterator_t it = 0;
    if (IOServiceGetMatchingServices(kIOMainPortDefault, IOServiceMatching("IOHIDSystem"), &it) != KERN_SUCCESS)
        return -1;
    int64_t idle_secs = -1;
    io_registry_entry_t entry = IOIteratorNext(it);
    if (entry)
    {
        CFMutableDictionaryRef dict = nullptr;
        if (IORegistryEntryCreateCFProperties(entry, &dict, kCFAllocatorDefault, 0) == KERN_SUCCESS)
        {
            CFNumberRef obj = (CFNumberRef)CFDictionaryGetValue(dict, CFSTR("HIDIdleTime"));
            if (obj != nullptr)
            {
                int64_t ns = 0;
                if (CFNumberGetValue(obj, kCFNumberSInt64Type, &ns))
                {
                    idle_secs = ns / 1'000'000'000LL;
                }
            }
            CFRelease(dict);
        }
        IOObjectRelease(entry);
    }
    IOObjectRelease(it);
    return idle_secs;
}

#ifdef WITH_DISPLAY_SLEEP
bool is_display_asleep(void)
{
    // We need a way to detect if the display is asleep.
    // Otherwise 
    return false;
}

void turn_off_display(void)
{
    // Calling the command-line tool pmset seems to be the only option
    // to put the display(s) to sleep. I'm clueless on how to do it
    // using API calls.
    system("pmset displaysleepnow");

#if 0
    // The following code taken from
    // https://github.com/apple-oss-distributions/PowerManagement/blob/main/pmset/pmset.m#L972
    // doesn't do anything :-/
    io_registry_entry_t disp_wrangler = IO_OBJECT_NULL;
    kern_return_t kr;

    disp_wrangler = IORegistryEntryFromPath(kIOMainPortDefault, kIOServicePlane ":/IOResources/IODisplayWrangler");
    if (disp_wrangler == IO_OBJECT_NULL)
        return;
    kr = IORegistryEntrySetCFProperty(disp_wrangler, CFSTR("IORequestIdle"), kCFBooleanTrue);

    if (kr)
        fprintf(stderr, "pmset: Failed to set the display to sleep(err:0x%x)\n", kr);
    IOObjectRelease(disp_wrangler);
#endif

#if 0
    // The following deactivated code remains here for documentation purposes.
    // CGAcquireDisplayFadeReservation() always returns with kCGErrorNotImplemented.
    // But why?
    CGDisplayFadeReservationToken res_token;
    CGError err = CGAcquireDisplayFadeReservation(2.0, &res_token);
    if (err != kCGErrorSuccess)
    {
        std::cerr << "CGAcquireDisplayFadeReservation() failed with code " << err << "\n";
    }
    CGDisplayFade(res_token,
                2.0,                       // 2 seconds
                kCGDisplayBlendNormal,     // starting state
                kCGDisplayBlendSolidColor, // ending state
                0.0, 0.0, 0.0,             // black
                true                       // wait for completion
    );
    CGReleaseDisplayFadeReservation(res_token);
#endif

}
#endif

CGPoint get_mouse_pos()
{
    CGEventRef event = CGEventCreate(nullptr);
    CGPoint cursor = CGEventGetLocation(event);
    CFRelease(event);
    return cursor;
}

void move_mouse_to(CGPoint const& point)
{
    CGEventRef event = CGEventCreateMouseEvent(nullptr, kCGEventMouseMoved, point, kCGMouseButtonLeft);
    CGEventPost(kCGHIDEventTap, event);
    CFRelease(event);
}

void wiggle_mouse_cursor(void)
{
    CGPoint current_mouse_pos = get_mouse_pos();
    move_mouse_to(CGPointMake(current_mouse_pos.x + 1, current_mouse_pos.y));
    move_mouse_to(current_mouse_pos);
}

void press_shift_key()
{
    CGEventRef key_down = CGEventCreateKeyboardEvent(nullptr, 56, true);
    CGEventRef key_up = CGEventCreateKeyboardEvent(nullptr, 56, false);
    CGEventPost(kCGHIDEventTap, key_down);
    CGEventPost(kCGHIDEventTap, key_up);
    CFRelease(key_down);
    CFRelease(key_up);
}

int main(void)
{
    static constexpr int64_t idle_timeout_secs = DEFAULT_IDLE_TIMEOUT_SECS;
#ifdef WITH_DISPLAY_SLEEP
    static constexpr int64_t display_off_secs = DEFAULT_DISPLAY_SLEEP_SECS;
    int64_t total_idle_secs = 0;
    bool display_off = false;
#endif
    std::cout << "Preventing sleep after " << idle_timeout_secs << " seconds of inactivity.\n";
    while (true)
    {
        int64_t idle_secs = system_idle_secs();
        if (idle_secs > idle_timeout_secs)
        {
            std::cout << "System idle for " << idle_secs << " seconds. Preventing sleep ...\n";
            wiggle_mouse_cursor();
            press_shift_key();
#ifdef WITH_DISPLAY_SLEEP
            total_idle_secs += idle_secs;
#endif
        }
#ifdef WITH_DISPLAY_SLEEP
        if (total_idle_secs > display_off_secs && !display_off)
        {
            std::cout << "Turning display off ...\n";
            turn_off_display();
            display_off = true;
        }
#endif
        std::this_thread::sleep_for(DEFAULT_CHECK_INTERVAL);
    }
    return 0;
}
