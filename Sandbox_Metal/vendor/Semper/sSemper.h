// Semper, v0.1 WIP
// 
// * An Immediate Mode Cross Platform Window/Input Abstraction
//
// * Heavily based on Dear ImGui's input system: https://github.com/ocornut/imgui
//

#pragma once

#include <cstring> // memset
#include <vector>  // temporary, replace with custom
#include "sMath.h"

#if defined(_MSC_VER)  && !defined(__clang__)
#include <wrl.h> // Microsoft::WRL::ComPtr
template <typename T>
using sComPtr = Microsoft::WRL::ComPtr<T>;
#elif defined(__APPLE__)
#import <Cocoa/Cocoa.h>
#import <Metal/Metal.h>
#include <Metal/Metal.hpp>
#import <MetalKit/MetalKit.h>
#endif

// Helper Macros
#ifndef S_ASSERT
#include <assert.h>
#define S_ASSERT(_EXPR) assert(_EXPR) // You can override the default assert handler by editing imconfig.h
#endif
#define S_ARRAYSIZE(_ARR) ((int)(sizeof(_ARR) / sizeof(*(_ARR))))     // Size of a static C-style array. Don't use on pointers!

// Helper: Unicode defines
#define S_UNICODE_CODEPOINT_INVALID 0xFFFD     // Invalid Unicode code point (standard value).
#ifdef S_USE_WCHAR32
#define S_UNICODE_CODEPOINT_MAX     0x10FFFF   // Maximum Unicode code point supported by this build.
#else
#define S_UNICODE_CODEPOINT_MAX     0xFFFF     // Maximum Unicode code point supported by this build.
#endif

// Warnings
#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable: 26495)    // [Static Analyzer] Variable 'XXX' is uninitialized. Always initialize a member variable (type.6).
#endif
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wold-style-cast"
#if __has_warning("-Wzero-as-null-pointer-constant")
#pragma clang diagnostic ignored "-Wzero-as-null-pointer-constant"
#endif
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpragmas"          // warning: unknown option after '#pragma GCC diagnostic' kind
#pragma GCC diagnostic ignored "-Wclass-memaccess"  // [__GNUC__ >= 8] warning: 'memset/memcpy' clearing/writing an object of type 'xxxx' with no trivial copy-assignment; use assignment or value-initialization instead
#endif

//-----------------------------------------------------------------------------
// [SECTION] Forward declarations and basic types
//-----------------------------------------------------------------------------

// forward declarations
struct sSemperContext;
struct sInputEvent;
struct sIO;
struct sKeyData;
struct sWindow;
struct sPlatformSpecifics;

// enums/flags
typedef int sInputEventType;     // -> enum sInputEventType_
typedef int sInputEventSource;   // -> enum sInputEventSource_
typedef int sMouseButton;        // -> enum sMouseButton_
typedef int sKey;                // -> enum sKey_
typedef int sKeyModFlags;        // -> enum sKeyModFlags_

// data types
typedef bool             b8;
typedef int              b32;
typedef float            f32;
typedef double           f64;
typedef signed char      i8;
typedef signed short     i16;
typedef signed int       i32;
#if defined(_MSC_VER) && !defined(__clang__)
typedef signed __int64   i64;
#else
typedef signed long long   i64;
#endif
typedef i8               s8;
typedef i16              s16;
typedef i32              s32;
typedef i64              s64;
typedef unsigned char    u8;
typedef unsigned short   u16;
typedef unsigned int     u32;
#if defined(_MSC_VER) && !defined(__clang__)
typedef unsigned __int64 u64;
#else
typedef unsigned long long   u64;
#endif

extern sSemperContext* GContext;

//-----------------------------------------------------------------------------
// [SECTION] Semper end-user API functions
//-----------------------------------------------------------------------------

namespace Semper
{

    // context creation & access
    void            create_context();
    void            destroy_context();
    sSemperContext* get_context();

    // os window
    sWindow*        create_window        (int cwidth, int cheight);
	void            cleanup_window       (sWindow* window);
	void            process_window_events(sWindow& window);
	void            enable_cursor        (sWindow& window);
	void            disable_cursor       (sWindow& window);

    // main
    sIO&            get_io();
    void            new_frame();
    void            end_frame();

    // input utilities: keyboard
    bool            is_key_down           (sKey key);
    bool            is_key_pressed        (sKey key, bool repeat = true);
    bool            is_key_released       (sKey key);
    int             get_key_pressed_amount(sKey key, float repeat_delay, float rate);  // uses provided repeat rate/delay. return a count, most often 0 or 1 but might be >1 if RepeatRate is small enough that DeltaTime > RepeatRate

    // input utilities: mouse
    bool            is_mouse_down          (sMouseButton button);                      // is mouse button held?
    bool            is_mouse_clicked       (sMouseButton button, bool repeat = false); // did mouse button clicked? (went from !Down to Down). Same as GetMouseClickedCount() == 1.
    bool            is_mouse_released      (sMouseButton button);                      // did mouse button released? (went from Down to !Down)
    bool            is_mouse_double_clicked(sMouseButton button);                      // did mouse button double-clicked? Same as GetMouseClickedCount() == 2. (note that a double-click will also report IsMouseClicked() == true)
    int             get_mouse_clicked_count(sMouseButton button);                      // return the number of successive mouse-clicks at the time where a click happen (otherwise 0).
    bool            is_mouse_pos_valid     (const sVec2* mouse_pos = nullptr);         // by convention we use (-FLT_MAX,-FLT_MAX) to denote that there is no mouse available
    sVec2           get_mouse_pos();                                                   // shortcut to Semper::get_io().mousePos provided by user, to be consistent with other calls
    bool            is_mouse_dragging     (sMouseButton button, float lock_threshold = -1.0f); // is mouse dragging? (if lock_threshold < -1.0f, uses io.MouseDraggingThreshold)
    sVec2           get_mouse_drag_delta  (sMouseButton button = 0, float lock_threshold = -1.0f); // return the delta from the initial clicking position while the mouse button is pressed or was just released. This is locked and return 0.0f until the mouse moves past a distance threshold at least once (if lock_threshold < -1.0f, uses io.MouseDraggingThreshold)
    void            reset_mouse_drag_delta(sMouseButton button = 0);
}

enum sInputSource_
{
    sInputSource_None = 0,
    sInputSource_Mouse,
    sInputSource_Keyboard,
    sInputSource_Gamepad,
    sInputSource_Nav,
    sInputSource_Clipboard,
    sInputSource_COUNT
};

enum sInputEventType_
{
    sInputEventType_None = 0,
    sInputEventType_MousePos,
    sInputEventType_MouseWheel,
    sInputEventType_MouseButton,
    sInputEventType_Key,
    sInputEventType_KeyMods,
    sInputEventType_Char,
    sInputEventType_Focus,
    sInputEventType_COUNT
};

enum sKeyModFlags_
{
    sKeyModFlags_None  = 0,
    sKeyModFlags_Ctrl  = 1 << 0,
    sKeyModFlags_Shift = 1 << 1,
    sKeyModFlags_Alt   = 1 << 2,
    sKeyModFlags_Super = 1 << 3    // Cmd/Super/Windows key
};

enum sMouseButton_
{
    sMouseButton_Left   = 0,
    sMouseButton_Right  = 1,
    sMouseButton_Middle = 2,
    sMouseButton_COUNT  = 5
};

enum sKey_
{
    sKey_None = 0,
    sKey_Tab,
    sKey_LeftArrow,
    sKey_RightArrow,
    sKey_UpArrow,
    sKey_DownArrow,
    sKey_PageUp,
    sKey_PageDown,
    sKey_Home,
    sKey_End,
    sKey_Insert,
    sKey_Delete,
    sKey_Backspace,
    sKey_Space,
    sKey_Enter,
    sKey_Escape,
    sKey_LeftCtrl, sKey_LeftShift, sKey_LeftAlt, sKey_LeftSuper,
    sKey_RightCtrl, sKey_RightShift, sKey_RightAlt, sKey_RightSuper,
    sKey_Menu,
    sKey_0, sKey_1, sKey_2, sKey_3, sKey_4, sKey_5, sKey_6, sKey_7, sKey_8, sKey_9,
    sKey_A, sKey_B, sKey_C, sKey_D, sKey_E, sKey_F, sKey_G, sKey_H, sKey_I, sKey_J,
    sKey_K, sKey_L, sKey_M, sKey_N, sKey_O, sKey_P, sKey_Q, sKey_R, sKey_S, sKey_T,
    sKey_U, sKey_V, sKey_W, sKey_X, sKey_Y, sKey_Z,
    sKey_F1, sKey_F2, sKey_F3, sKey_F4, sKey_F5, sKey_F6,
    sKey_F7, sKey_F8, sKey_F9, sKey_F10, sKey_F11, sKey_F12,
    sKey_Apostrophe,        // '
    sKey_Comma,             // ,
    sKey_Minus,             // -
    sKey_Period,            // .
    sKey_Slash,             // /
    sKey_Semicolon,         // ;
    sKey_Equal,             // =
    sKey_LeftBracket,       // [
    sKey_Backslash,         // \ (this text inhibit multiline comment caused by backslash)
    sKey_RightBracket,      // ]
    sKey_GraveAccent,       // `
    sKey_CapsLock,
    sKey_ScrollLock,
    sKey_NumLock,
    sKey_PrintScreen,
    sKey_Pause,
    sKey_Keypad0, sKey_Keypad1, sKey_Keypad2, sKey_Keypad3, sKey_Keypad4,
    sKey_Keypad5, sKey_Keypad6, sKey_Keypad7, sKey_Keypad8, sKey_Keypad9,
    sKey_KeypadDecimal,
    sKey_KeypadDivide,
    sKey_KeypadMultiply,
    sKey_KeypadSubtract,
    sKey_KeypadAdd,
    sKey_KeypadEnter,
    sKey_KeypadEqual,
    sKey_COUNT, // No valid sKey is ever greater than this value

    sKey_NamedKey_END = sKey_COUNT,
    sKey_NamedKey_COUNT = sKey_COUNT,
    sKey_KeysData_SIZE = sKey_NamedKey_COUNT,     // Size of KeysData[]: hold legacy 0..512 keycodes + named keys
    sKey_KeysData_OFFSET = 1    // First key stored in KeysData[0]
};

// FIXME: Structures in the union below need to be declared, as anonymous unions appears to be an extension?
//        'union member can't have a non-trivial default constructor'
struct sInputEventMousePos    { float posX, posY; };
struct sInputEventMouseWheel  { float wheelX, wheelY; };
struct sInputEventMouseButton { int button; bool down; };
struct sInputEventKey         { sKey key; bool down; };
struct sInputEventKeyMods     { sKeyModFlags mods; };
struct sInputEventText        { unsigned int c; };
struct sInputEventAppFocused  { bool focused; };

struct sInputEvent
{
    sInputEventType   type;
    sInputEventSource source;
    union
    {
        sInputEventMousePos     mousePos;       // if type == sInputEventType_MousePos
        sInputEventMouseWheel   mouseWheel;     // if type == sInputEventType_MouseWheel
        sInputEventMouseButton  mouseButton;    // if type == sInputEventType_MouseButton
        sInputEventKey          key;            // if type == sInputEventType_Key
        sInputEventKeyMods      keyMods;        // if type == sInputEventType_Modifiers
        sInputEventText         text;           // if type == sInputEventType_Text
        sInputEventAppFocused   appFocused;     // if type == sInputEventType_Focus
    };

    sInputEvent() { memset(this, 0, sizeof(*this)); }
};

struct sKeyData
{
    bool  down;             // True for if key is down
    float downDuration;     // Duration the key has been down (<0.0f: not pressed, 0.0f: just pressed, >0.0f: time held)
    float downDurationPrev; // Last frame duration the key has been down
};

#if defined(__APPLE__)
@interface SemperMetalView : MTKView
@end

struct sApplePass
{
    MTL::Drawable* drawable;
    double width;
    double height;
};
#endif

struct sPlatformSpecifics
{
#if defined(WIN32)
    HWND       handle;
    HWND       mouseHandle;
    WNDCLASSEX wc;
#elif defined(__APPLE__)
    NSWindow* handle;
    SemperMetalView *view;
#endif
};

struct sWindow
{
    int                width;
    int                height;
    int                client_width;
    int                client_height;
    int                xpos;
    int                ypos;
    bool               running;
    bool               sizeChanged;
    bool               mouseTracked;
    bool               cursorEnabled;
    bool               rawMouseMotion;
    int                mouseButtonsDown;
    sPlatformSpecifics platform;
};

struct sIO
{

    //------------------------------------------------------------------
    // Configuration (fill once)                // Default value
    //------------------------------------------------------------------
    float       deltaTime;               // = 1.0f/60.0f     // Time elapsed since last frame, in seconds.
    const char* logFilename;             // = "semper_log.txt"// Path to .log file (default parameter to ImGui::LogToFile when no file is specified).
    float       mouseDoubleClickTime;    // = 0.30f          // Time for a double-click, in seconds.
    float       mouseDoubleClickMaxDist; // = 6.0f           // Distance threshold to stay in to validate a double-click, in pixels.
    float       mouseDragThreshold;      // = 6.0f           // Distance threshold before considering we are dragging.
    float       keyRepeatDelay;          // = 0.250f         // When holding a key/button, time before it starts repeating, in seconds (for buttons in Repeat mode, etc.).
    float       keyRepeatRate;           // = 0.050f         // When holding a key/button, rate at which it repeats, in seconds.
    bool        configInputTrickleEventQueue; // = true      // Enable input queue trickling: some types of events submitted during the same frame (e.g. button down + up) will be spread over multiple frames, improving interactions with low framerates.
    void*       userData;                // = NULL           // Store your own data for retrieval by callbacks.

    //------------------------------------------------------------------
    // Input - Call before calling NewFrame()
    //------------------------------------------------------------------

    // Input Functions
    void  pushKeyEvent(sKey key, bool down);           // queue a new key down/up event. Key should be "translated" (as in, generally ImGuiKey_A matches the key end-user would use to emit an 'A' character)
    void  pushKeyModsEvent(sKeyModFlags modifiers);    // queue a change of Ctrl/Shift/Alt/Super modifiers
    void  pushMousePosEvent(float x, float y);         // queue a mouse position update. Use -FLT_MAX,-FLT_MAX to signify no mouse (e.g. app not focused and not hovered)
    void  pushMouseButtonEvent(int button, bool down); // queue a mouse button change
    void  pushMouseWheelEvent(float wh_x, float wh_y); // queue a mouse wheel update
    void  pushFocusEvent(bool focused);                // queue a gain/loss of focus for the application (generally based on OS/platform focus of your window)
    void  pushInputCharacter(unsigned int c);          // queue a new character input
    void  pushInputCharacterUTF16(unsigned short c);   // queue a new character input from an UTF-16 character, it can be a surrogate
    void  pushInputCharactersUTF8(const char* str);    // queue a new characters input from an UTF-8 string

    void  clearInputCharacters();                     // [Internal] clear the text input buffer manually
    void  clearInputKeys();                           // [Internal] release all keys

    //------------------------------------------------------------------
    // Output - Updated by NewFrame() or EndFrame()/Render()
    // (when reading from the io.WantCaptureMouse, io.WantCaptureKeyboard flags to dispatch your inputs, it is
    //  generally easier and more correct to use their state BEFORE calling NewFrame(). See FAQ for details!)
    //------------------------------------------------------------------
    float framerate;  // Rough estimate of application framerate, in frame per second. Solely for convenience. Rolling average estimation based on io.DeltaTime over 120 frames.
    sVec2 mouseDelta; // Mouse delta. Note that this is zero if either current or previous position are invalid (-FLT_MAX,-FLT_MAX), so a disappearing/reappearing mouse won't have a huge delta.


    //------------------------------------------------------------------
    // [Internal]
    //------------------------------------------------------------------

    // main Input State
    sVec2 mousePos;     // mouse position, in pixels. Set to ImVec2(-FLT_MAX, -FLT_MAX) if mouse is unavailable (on another screen, etc.)
    bool  mouseDown[5]; // mouse buttons: 0=left, 1=right, 2=middle + extras (sMouseButton_COUNT == 5). Dear ImGui mostly uses left and right buttons. Others buttons allows us to track if the mouse is being used by your application + available to user as a convenience via IsMouse** API.
    float mouseWheel;   // mouse wheel Vertical: 1 unit scrolls about 5 lines text.
    float mouseWheelH;  // mouse wheel Horizontal. Most users don't have a mouse with an horizontal wheel, may not be filled by all backends.
    bool  keyCtrl;      // keyboard modifier down: Control
    bool  keyShift;     // keyboard modifier down: Shift
    bool  keyAlt;       // keyboard modifier down: Alt
    bool  keySuper;     // keyboard modifier down: Cmd/Super/Windows
    
    // other state maintained from data above + IO function calls
    sKeyModFlags                keyMods;                            // key mods flags (same as io.KeyCtrl/KeyShift/KeyAlt/KeySuper but merged into flags), updated by NewFrame()
    sKeyModFlags                keyModsPrev;                        // key mods flags (from previous frame)
    sKeyData                    keysData[sKey_COUNT];               // key state for all known keys. Use IsKeyXXX() functions to access this.
    sVec2                       mousePosPrev;                       // Previous mouse position (note that MouseDelta is not necessary == MousePos-MousePosPrev, in case either position is invalid)
    sVec2                       mouseClickedPos[5];                 // Position at time of clicking
    double                      mouseClickedTime[5];                // Time of last click (used to figure out double-click)
    bool                        mouseClicked[5];                    // Mouse button went from !Down to Down (same as MouseClickedCount[x] != 0)
    bool                        mouseDoubleClicked[5];              // Has mouse button been double-clicked? (same as MouseClickedCount[x] == 2)
    unsigned short              mouseClickedCount[5];               // == 0 (not clicked), == 1 (same as MouseClicked[]), == 2 (double-clicked), == 3 (triple-clicked) etc. when going from !Down to Down
    unsigned short              mouseClickedLastCount[5];           // Count successive number of clicks. Stays valid after mouse release. Reset after another click is done.
    bool                        mouseReleased[5];                   // Mouse button went from Down to !Down
    bool                        mouseDownOwned[5];                  // Track if button was clicked inside a dear imgui window or over void blocked by a popup. We don't request mouse capture from the application if click started outside ImGui bounds.
    bool                        mouseDownOwnedUnlessPopupClose[5];  //Track if button was clicked inside a dear imgui window.
    float                       mouseDownDuration[5];               // Duration the mouse button has been down (0.0f == just clicked)
    float                       mouseDownDurationPrev[5];           // Previous time the mouse button has been down
    float                       mouseDragMaxDistanceSqr[5];         // Squared maximum distance of how much mouse has traveled from the clicking point (used for moving thresholds)
    bool                        appFocusLost;
    unsigned short              inputQueueSurrogate;                // for pushInputCharacterUTF16()
    std::vector<unsigned short> inputQueueCharacters;               // queue of _characters_ input (obtained by platform backend). Fill using pushInputCharacter() helper.

    double                      restoreCursorPos[2];
    double                      lastCursorPos[2];
};

struct sSemperContext
{
    sIO                      IO;
    std::vector<sInputEvent> inputEventsQueue; // Input events which will be trickled/written into IO structure.
    std::vector<sInputEvent> inputEventsTrail; // Past input events processed in new_frame(). This is to allow domain-specific application to access e.g mouse/pen trail.
    int                      frameCount;
    double                   time;
    sVec2                    mouseLastValidPos;
};