#include "sSemper.h"
#define SEMPER_MATH_IMPLEMENTATION
#include "sMath.h"
#include "sSemper_Internal.h"

extern sSemperContext* GContext = nullptr;

namespace Semper
{
#if defined(WIN32)
	extern sPlatformSpecifics initialize_platform    (sWindow& window, int cwidth, int cheight);
	extern void               cleanup_platform       (sWindow& window);
	extern void               process_platform_events(sWindow& window);
	extern void               enable_platform_cursor (sWindow& window);
	extern void               disable_platform_cursor(sWindow& window);
	extern void               process_windows_workarounds();
#elif defined(__APPLE__)
    extern sPlatformSpecifics initialize_platform    (sWindow& window, int cwidth, int cheight);
	extern void               cleanup_platform       (sWindow& window);
	extern void               process_platform_events(sWindow& window);
	extern void               enable_platform_cursor (sWindow& window);
	extern void               disable_platform_cursor(sWindow& window);
	void                      process_windows_workarounds(){}
#endif
}

sIO&
Semper::get_io()
{
	S_ASSERT(GContext != nullptr && "No current context. Did you call Semper::create_semper_context() and ImGui::SetCurrentContext() ?");
	return GContext->IO;
}

void
Semper::create_context()
{
	if (GContext)
	{
		assert(false && "Context already created.");
		return;
	}

	GContext = new sSemperContext();

	GContext->frameCount = 0;
	GContext->IO.deltaTime = 1.0f / 60.0f;
	GContext->IO.logFilename = "semper_log.txt";
	GContext->IO.mouseDoubleClickTime = 0.30f;
	GContext->IO.mouseDoubleClickMaxDist = 6.0f;
	GContext->IO.mouseDragThreshold = 6.0f;
	GContext->IO.keyRepeatDelay = 0.250f;
	GContext->IO.keyRepeatRate = 0.050f;
	GContext->IO.configInputTrickleEventQueue = true;
	GContext->IO.userData = nullptr;
}

void
Semper::destroy_context()
{
	if (GContext)
	{
		delete GContext;
		GContext = nullptr;
		return;
	}

	assert(false && "Context already destroyed.");
}

sSemperContext*
Semper::get_context()
{
	return GContext;
}

void
Semper::new_frame()
{
	sSemperContext* ctx = get_context();
	sIO& io = get_io();

	#if defined(WIN32)
	process_windows_workarounds();
	#endif

	ctx->time += ctx->IO.deltaTime;
	ctx->frameCount += 1;

	// Process input queue (trickle as many events as possible)
	ctx->inputEventsTrail.resize(0);
	update_input_events(ctx->IO.configInputTrickleEventQueue);

	// Update keyboard input state
	update_keyboard_inputs();

	// Update mouse input state
	update_mouse_inputs();

	// Mouse wheel scrolling, scale
	update_mouse_wheel();
}

void
Semper::end_frame()
{
	sSemperContext* ctx = get_context();
	sIO& io = get_io();

	// Clear Input data for next frame
	io.mouseWheel = io.mouseWheelH = 0.0f;
	io.inputQueueCharacters.resize(0);
	io.keyModsPrev = io.keyMods; // doing it here is better than in new_frame() as we'll tolerate backend writing to keyMods. If we want to firmly disallow it we should detect it.
}

sKeyData*
Semper::get_key_data(sKey key)
{
	sSemperContext* ctx = get_context();
	int index;
	//IM_ASSERT(IsNamedKey(key) && "Support for user key indices was dropped in favor of ImGuiKey. Please update backend & user code.");
	index = key-1;
	return &ctx->IO.keysData[index];
}

bool
Semper::is_key_down(sKey key)
{
	const sKeyData* key_data = get_key_data(key);
	return key_data->down;
}

bool
Semper::is_key_pressed(sKey key, bool repeat)
{
	sSemperContext& ctx = *GContext;
	const sKeyData* key_data = get_key_data(key);
	const float t = key_data->downDuration;
	if (t == 0.0f)
		return true;
	if (repeat && t > ctx.IO.keyRepeatDelay)
		return get_key_pressed_amount(key, ctx.IO.keyRepeatDelay, ctx.IO.keyRepeatRate) > 0;
	return false;
}

int
Semper::get_key_pressed_amount(sKey key, float repeat_delay, float repeat_rate)
{
	sSemperContext& ctx = *GContext;
	const sKeyData* key_data = get_key_data(key);
	const float t = key_data->downDuration;
	return calc_typematic_repeat_amount(t - ctx.IO.deltaTime, t, repeat_delay, repeat_rate);
}

bool
Semper::is_key_released(sKey key)
{
	const sKeyData* key_data = get_key_data(key);
	return key_data->downDurationPrev >= 0.0f && !key_data->down;
}

bool  
Semper::is_mouse_down(sMouseButton button)
{
	sSemperContext& ctx = *GContext;
	S_ASSERT(button >= 0 && button < S_ARRAYSIZE(ctx.IO.mouseDown));
	return ctx.IO.mouseDown[button];
}

bool  
Semper::is_mouse_clicked(sMouseButton button, bool repeat)
{
	sSemperContext& ctx = *GContext;
	S_ASSERT(button >= 0 && button < S_ARRAYSIZE(ctx.IO.mouseDown));
	const float t = ctx.IO.mouseDownDuration[button];
	if (t == 0.0f)
		return true;

	if (repeat && t > ctx.IO.keyRepeatDelay)
	{
		int amount = calc_typematic_repeat_amount(t - ctx.IO.deltaTime, t, ctx.IO.keyRepeatDelay, ctx.IO.keyRepeatRate * 0.50f);
		if (amount > 0)
			return true;
	}
	return false;
}

bool  
Semper::is_mouse_released(sMouseButton button)
{
	sSemperContext& ctx = *GContext;
	S_ASSERT(button >= 0 && button < S_ARRAYSIZE(ctx.IO.mouseDown));
	return ctx.IO.mouseReleased[button];
}

bool  
Semper::is_mouse_double_clicked(sMouseButton button)
{
	sSemperContext& ctx = *GContext;
	S_ASSERT(button >= 0 && button < S_ARRAYSIZE(ctx.IO.mouseDown));
	return ctx.IO.mouseClickedCount[button] == 2;
}

int   
Semper::get_mouse_clicked_count(sMouseButton button)
{
	sSemperContext& ctx = *GContext;
	S_ASSERT(button >= 0 && button < S_ARRAYSIZE(ctx.IO.mouseDown));
	return ctx.IO.mouseClickedCount[button];
}

bool  
Semper::is_mouse_pos_valid(const sVec2* mouse_pos)
{
	// The assert is only to silence a false-positive in XCode Static Analysis.
	// Because GImGui is not dereferenced in every code path, the static analyzer assume that it may be NULL (which it doesn't for other functions).
	S_ASSERT(GContext != nullptr);
	const float MOUSE_INVALID = -256000.0f;
	sVec2 p = mouse_pos ? *mouse_pos : GContext->IO.mousePos;
	return p.x >= MOUSE_INVALID && p.y >= MOUSE_INVALID;
}

sVec2 
Semper::get_mouse_pos()
{
	sSemperContext& ctx = *GContext;
	return ctx.IO.mousePos;
}

// Return if a mouse click/drag went past the given threshold. Valid to call during the MouseReleased frame.
// [Internal] This doesn't test if the button is pressed
bool Semper::is_mouse_drag_past_threshold(sMouseButton button, float lock_threshold)
{
	sSemperContext& ctx = *GContext;
	S_ASSERT(button >= 0 && button < S_ARRAYSIZE(ctx.IO.mouseDown));
	if (lock_threshold < 0.0f)
		lock_threshold = ctx.IO.mouseDragThreshold;
	return ctx.IO.mouseDragMaxDistanceSqr[button] >= lock_threshold * lock_threshold;
}

bool  
Semper::is_mouse_dragging(sMouseButton button, float lock_threshold)
{
	sSemperContext& ctx = *GContext;
	S_ASSERT(button >= 0 && button < S_ARRAYSIZE(ctx.IO.mouseDown));
	if (!ctx.IO.mouseDown[button])
		return false;
	return is_mouse_drag_past_threshold(button, lock_threshold);
}

// Return the delta from the initial clicking position while the mouse button is clicked or was just released.
// This is locked and return 0.0f until the mouse moves past a distance threshold at least once.
// NB: This is only valid if is_mouse_pos_valid(). backends in theory should always keep mouse position valid when dragging even outside the client window.
sVec2 
Semper::get_mouse_drag_delta(sMouseButton button, float lock_threshold)
{
	sSemperContext& ctx = *GContext;
	S_ASSERT(button >= 0 && button < S_ARRAYSIZE(ctx.IO.mouseDown));
	if (lock_threshold < 0.0f)
		lock_threshold = ctx.IO.mouseDragThreshold;
	if (ctx.IO.mouseDown[button] || ctx.IO.mouseReleased[button])
		if (ctx.IO.mouseDragMaxDistanceSqr[button] >= lock_threshold * lock_threshold)
			if (is_mouse_pos_valid(&ctx.IO.mousePos) && is_mouse_pos_valid(&ctx.IO.mouseClickedPos[button]))
				return ctx.IO.mousePos - ctx.IO.mouseClickedPos[button];
	return sVec2(0.0f, 0.0f);
}

void  
Semper::reset_mouse_drag_delta(sMouseButton button)
{
	sSemperContext& ctx = *GContext;
	S_ASSERT(button >= 0 && button < S_ARRAYSIZE(ctx.IO.mouseDown));
	// NB: We don't need to reset g.IO.MouseDragMaxDistanceSqr
	ctx.IO.mouseClickedPos[button] = ctx.IO.mousePos;
}

void
sIO::pushKeyEvent(sKey key, bool down)
{
	if (key == sKey_None)
		return;

	sSemperContext* ctx = Semper::get_context();

	sInputEvent e;
	e.type = sInputEventType_Key;
	e.source = sInputSource_Keyboard;
	e.key.key = key;
	e.key.down = down;
	ctx->inputEventsQueue.push_back(e);
}

void
sIO::pushKeyModsEvent(sKeyModFlags modifiers)
{
	sSemperContext* ctx = Semper::get_context();

	sInputEvent e;
	e.type = sInputEventType_KeyMods;
	e.source = sInputSource_Keyboard;
	e.keyMods.mods = modifiers;
	ctx->inputEventsQueue.push_back(e);
}

void
sIO::pushMousePosEvent(float x, float y)
{
	sSemperContext* ctx = Semper::get_context();

	sInputEvent e;
	e.type = sInputEventType_MousePos;
	e.source = sInputSource_Mouse;
	e.mousePos.posX = x;
	e.mousePos.posY = y;
	ctx->inputEventsQueue.push_back(e);
}

void
sIO::pushMouseButtonEvent(int mouse_button, bool down)
{
	sSemperContext* ctx = Semper::get_context();

	assert(mouse_button >= 0 && mouse_button < sMouseButton_COUNT);

	sInputEvent e;
	e.type = sInputEventType_MouseButton;
	e.source = sInputSource_Mouse;
	e.mouseButton.button = mouse_button;
	e.mouseButton.down = down;
	ctx->inputEventsQueue.push_back(e);
}

void
sIO::pushMouseWheelEvent(float wheel_x, float wheel_y)
{
	sSemperContext* ctx = Semper::get_context();

	if (wheel_x == 0.0f && wheel_y == 0.0f)
		return;

	sInputEvent e;
	e.type = sInputEventType_MouseWheel;
	e.source = sInputSource_Mouse;
	e.mouseWheel.wheelX = wheel_x;
	e.mouseWheel.wheelY = wheel_y;
	ctx->inputEventsQueue.push_back(e);
}

void
sIO::pushFocusEvent(bool focused)            
{
	sSemperContext* ctx = Semper::get_context();

	sInputEvent e;
	e.type = sInputEventType_Focus;
	e.appFocused.focused = focused;
	ctx->inputEventsQueue.push_back(e);
}

// Pass in translated ASCII characters for text input.
// - with glfw you can get those from the callback set in glfwSetCharCallback()
// - on Windows you can get those using ToAscii+keyboard state, or via the WM_CHAR message
// FIXME: Should in theory be called "AddCharacterEvent()" to be consistent with new API
void
sIO::pushInputCharacter(unsigned int c)        
{
	sSemperContext* ctx = Semper::get_context();

	if (c == 0)
		return;

	sInputEvent e;
	e.type = sInputEventType_Char;
	e.source = sInputSource_Keyboard;
	e.text.c = c;
	ctx->inputEventsQueue.push_back(e);
}

// UTF16 strings use surrogate pairs to encode codepoints >= 0x10000, so
// we should save the high surrogate.
void 
sIO::pushInputCharacterUTF16(unsigned short c)
{
	sSemperContext* ctx = Semper::get_context();

	if (c == 0 && inputQueueSurrogate == 0)
		return;

	if ((c & 0xFC00) == 0xD800) // High surrogate, must save
	{
		if (inputQueueSurrogate != 0)
			pushInputCharacter(S_UNICODE_CODEPOINT_INVALID);
		inputQueueSurrogate = c;
		return;
	}

	unsigned char cp = c;
	if (inputQueueSurrogate != 0)
	{
		if ((c & 0xFC00) != 0xDC00) // Invalid low surrogate
		{
			pushInputCharacter(S_UNICODE_CODEPOINT_INVALID);
		}
		else
		{
#if S_UNICODE_CODEPOINT_MAX == 0xFFFF
			cp = S_UNICODE_CODEPOINT_INVALID; // Codepoint will not fit in ImWchar
#else
			cp = (unsigned char)(((inputQueueSurrogate - 0xD800) << 10) + (c - 0xDC00) + 0x10000);
#endif
		}

		inputQueueSurrogate = 0;
	}
	pushInputCharacter((unsigned)cp);
}

void
sIO::pushInputCharactersUTF8(const char* utf8_chars)
{
	sSemperContext* ctx = Semper::get_context();

	while (*utf8_chars != 0)
	{
		unsigned int c = 0;
		utf8_chars += Semper::sTextCharFromUtf8(&c, utf8_chars, NULL);
		if (c != 0)
			pushInputCharacter(c);
	}

}

void 
sIO::clearInputCharacters()
{
	inputQueueCharacters.resize(0);
}

void 
sIO::clearInputKeys()
{
	for (int n = 0; n < sKey_COUNT; n++)
	{
		keysData[n].down = false;
		keysData[n].downDuration = -1.0f;
		keysData[n].downDurationPrev = -1.0f;
	}
	keyCtrl = keyShift = keyAlt = keySuper = false;
	keyMods = keyModsPrev = sKeyModFlags_None;
}

// t0 = previous time (e.g.: g.Time - g.IO.DeltaTime)
// t1 = current time (e.g.: g.Time)
// An event is triggered at:
//  t = 0.0f     t = repeat_delay,    t = repeat_delay + repeat_rate*N
int
Semper::calc_typematic_repeat_amount(float t0, float t1, float repeat_delay, float repeat_rate)
{
	if (t1 == 0.0f)
		return 1;
	if (t0 >= t1)
		return 0;
	if (repeat_rate <= 0.0f)
		return (t0 < repeat_delay) && (t1 >= repeat_delay);
	const int count_t0 = (t0 < repeat_delay) ? -1 : (int)((t0 - repeat_delay) / repeat_rate);
	const int count_t1 = (t1 < repeat_delay) ? -1 : (int)((t1 - repeat_delay) / repeat_rate);
	const int count = count_t1 - count_t0;
	return count;
}

void
Semper::update_input_events(bool trickleFastInputs)
{
	sSemperContext& ctx = *GContext;
	sIO& io = ctx.IO;

	bool mouse_moved = false;
	bool mouse_wheeled = false;
	bool key_changed = false;
	bool text_inputed = false;
	int  mouse_button_changed = 0x00;
	int key_mods_changed = 0x00;

	sBitArray<sKey_KeysData_SIZE> key_changed_mask;

	int event_n = 0;
	for (; event_n < ctx.inputEventsQueue.size(); event_n++)
	{
		const sInputEvent* e = &ctx.inputEventsQueue[event_n];
		if (e->type == sInputEventType_MousePos)
		{
			sVec2 event_pos(e->mousePos.posX, e->mousePos.posY);

			if (is_mouse_pos_valid(&event_pos))
				event_pos = sVec2(get_floor(event_pos.x), get_floor(event_pos.y)); // Apply same flooring as UpdateMouseInputs()
			if (io.mousePos.x != event_pos.x || io.mousePos.y != event_pos.y)
			{
				// Trickling Rule: Stop processing queued events if we already handled a mouse button change
				if (trickleFastInputs && (mouse_button_changed != 0 || mouse_wheeled || key_changed || key_mods_changed || text_inputed))
					break;
				io.mousePos = event_pos;
				mouse_moved = true;
			}
		}
		else if (e->type == sInputEventType_MouseButton)
		{
			const sMouseButton button = e->mouseButton.button;
			S_ASSERT(button >= 0 && button < sMouseButton_COUNT);
			if (io.mouseDown[button] != e->mouseButton.down)
			{
				// Trickling Rule: Stop processing queued events if we got multiple action on the same button
				if (trickleFastInputs && ((mouse_button_changed & (1 << button)) || mouse_wheeled))
					break;
				io.mouseDown[button] = e->mouseButton.down;
				mouse_button_changed |= (1 << button);
			}
		}
		else if (e->type == sInputEventType_MouseWheel)
		{
			if (e->mouseWheel.wheelX != 0.0f || e->mouseWheel.wheelY != 0.0f)
			{
				// Trickling Rule: Stop processing queued events if we got multiple action on the event
				if (trickleFastInputs && (mouse_wheeled || mouse_button_changed != 0))
					break;
				io.mouseWheelH += e->mouseWheel.wheelX;
				io.mouseWheel += e->mouseWheel.wheelY;
				mouse_wheeled = true;
			}
		}
		else if (e->type == sInputEventType_Key)
		{
			S_ASSERT(e->key.key != sKey_None);
			const int keydata_index = (e->key.key - sKey_KeysData_OFFSET);
			sKeyData* keydata = &io.keysData[keydata_index];
			if (keydata->down != e->key.down)
			{
				// Trickling Rule: Stop processing queued events if we got multiple action on the same button
				if (trickleFastInputs && (key_changed_mask.TestBit(keydata_index) || text_inputed || mouse_button_changed != 0))
					break;
				keydata->down = e->key.down;
				key_changed = true;
				key_changed_mask.SetBit(keydata_index);
			}
		}
		else if (e->type == sInputEventType_KeyMods)
		{
			const sKeyModFlags modifiers = e->keyMods.mods;
			if (io.keyMods != modifiers)
			{
				// Trickling Rule: Stop processing queued events if we got multiple action on the same button
				sKeyModFlags modifiers_that_are_changing = (io.keyMods ^ modifiers);
				if (trickleFastInputs && (key_mods_changed & modifiers_that_are_changing) != 0)
					break;
				io.keyMods = modifiers;
				io.keyCtrl = (modifiers & sKeyModFlags_Ctrl) != 0;
				io.keyShift = (modifiers & sKeyModFlags_Shift) != 0;
				io.keyAlt = (modifiers & sKeyModFlags_Alt) != 0;
				io.keySuper = (modifiers & sKeyModFlags_Super) != 0;
				key_mods_changed |= modifiers_that_are_changing;
			}
		}
		else if (e->type == sInputEventType_Char)
		{
			// Trickling Rule: Stop processing queued events if keys/mouse have been interacted with
			if (trickleFastInputs && (key_changed || mouse_button_changed != 0 || mouse_moved || mouse_wheeled))
				break;
			unsigned int c = e->text.c;
			io.inputQueueCharacters.push_back(c <= S_UNICODE_CODEPOINT_MAX ? (u16)c : S_UNICODE_CODEPOINT_INVALID);
			text_inputed = true;
		}
		else if (e->type == sInputEventType_Focus)
		{
			// We intentionally overwrite this and process lower, in order to give a chance
			// to multi-viewports backends to queue AddFocusEvent(false) + AddFocusEvent(true) in same frame.
			io.appFocusLost = !e->appFocused.focused;
		}
		else
		{
			S_ASSERT(0 && "Unknown event!");
		}
	}

	// Record trail (for domain-specific applications wanting to access a precise trail)
	for (int n = 0; n < event_n; n++)
		ctx.inputEventsTrail.push_back(ctx.inputEventsQueue[n]);

	// Remaining events will be processed on the next frame
	if (event_n == ctx.inputEventsQueue.size())
		ctx.inputEventsQueue.resize(0);
	else
		ctx.inputEventsQueue.erase(ctx.inputEventsQueue.begin(), ctx.inputEventsQueue.begin() + event_n);
		//ctx.inputEventsQueue.erase(ctx.inputEventsQueue.data(), ctx.inputEventsQueue.data() + event_n);

	// Clear buttons state when focus is lost
	// (this is useful so e.g. releasing Alt after focus loss on Alt-Tab doesn't trigger the Alt menu toggle)
	if (ctx.IO.appFocusLost)
	{
		ctx.IO.clearInputKeys();
		ctx.IO.appFocusLost = false;
	}
}

void
Semper::update_keyboard_inputs()
{
	sSemperContext& ctx = *GContext;
	sIO& io = get_io();

	// Synchronize io.KeyMods with individual modifiers io.KeyXXX bools
	io.keyMods = sKeyModFlags_None;
	if (io.keyCtrl)  { io.keyMods |= sKeyModFlags_Ctrl; }
	if (io.keyShift) { io.keyMods |= sKeyModFlags_Shift; }
	if (io.keyAlt)   { io.keyMods |= sKeyModFlags_Alt; }
	if (io.keySuper) { io.keyMods |= sKeyModFlags_Super; }

	// Update keys
	for (int i = 0; i < S_ARRAYSIZE(io.keysData); i++)
	{
		sKeyData& key_data = io.keysData[i];
		key_data.downDurationPrev = key_data.downDuration;
		key_data.downDuration = key_data.down ? (key_data.downDuration < 0.0f ? 0.0f : key_data.downDuration + io.deltaTime) : -1.0f;
	}
}

void
Semper::update_mouse_inputs()
{
	sSemperContext& ctx = *GContext;
	sIO& io = get_io();

	// Round mouse position to avoid spreading non-rounded position (e.g. UpdateManualResize doesn't support them well)
	if (is_mouse_pos_valid(&io.mousePos))
		io.mousePos = ctx.mouseLastValidPos = sVec2(get_floor(io.mousePos.x), get_floor(io.mousePos.y));

	// If mouse just appeared or disappeared (usually denoted by -FLT_MAX components) we cancel out movement in MouseDelta
	if (is_mouse_pos_valid(&io.mousePos) && is_mouse_pos_valid(&io.mousePosPrev))
		io.mouseDelta = io.mousePos - io.mousePosPrev;
	else
		io.mouseDelta = sVec2(0.0f, 0.0f);

	io.mousePosPrev = io.mousePos;
	for (int i = 0; i < S_ARRAYSIZE(io.mouseDown); i++)
	{
		io.mouseClicked[i] = io.mouseDown[i] && io.mouseDownDuration[i] < 0.0f;
		io.mouseClickedCount[i] = 0; // Will be filled below
		io.mouseReleased[i] = !io.mouseDown[i] && io.mouseDownDuration[i] >= 0.0f;
		io.mouseDownDurationPrev[i] = io.mouseDownDuration[i];
		io.mouseDownDuration[i] = io.mouseDown[i] ? (io.mouseDownDuration[i] < 0.0f ? 0.0f : io.mouseDownDuration[i] + io.deltaTime) : -1.0f;
		if (io.mouseClicked[i])
		{
			bool is_repeated_click = false;
			if ((float)(ctx.time - io.mouseClickedTime[i]) < io.mouseDoubleClickTime)
			{
				sVec2 delta_from_click_pos = is_mouse_pos_valid(&io.mousePos) ? (io.mousePos - io.mouseClickedPos[i]) : sVec2(0.0f, 0.0f);
				if (lengthSqr(delta_from_click_pos) < io.mouseDoubleClickMaxDist * io.mouseDoubleClickMaxDist)
					is_repeated_click = true;
			}
			if (is_repeated_click)
				io.mouseClickedLastCount[i]++;
			else
				io.mouseClickedLastCount[i] = 1;
			io.mouseClickedTime[i] = ctx.time;
			io.mouseClickedPos[i] = io.mousePos;
			io.mouseClickedCount[i] = io.mouseClickedLastCount[i];
			io.mouseDragMaxDistanceSqr[i] = 0.0f;
		}
		else if (io.mouseDown[i])
		{
			// Maintain the maximum distance we reaching from the initial click position, which is used with dragging threshold
			float delta_sqr_click_pos = is_mouse_pos_valid(&io.mousePos) ? lengthSqr(io.mousePos - io.mouseClickedPos[i]) : 0.0f;
			io.mouseDragMaxDistanceSqr[i] = get_max(io.mouseDragMaxDistanceSqr[i], delta_sqr_click_pos);
		}

		// We provide io.MouseDoubleClicked[] as a legacy service
		io.mouseDoubleClicked[i] = (io.mouseClickedCount[i] == 2);
	}
}

void
Semper::update_mouse_wheel()
{
	sSemperContext& ctx = *GContext;
	sIO& io = get_io();

	if (ctx.IO.mouseWheel == 0.0f && ctx.IO.mouseWheelH == 0.0f)
		return;
}

sWindow*
Semper::create_window(int cwidth, int cheight)
{
	sWindow* window = new sWindow();

	window->client_width = cwidth;
	window->client_height = cheight;
	window->xpos = 0;
	window->ypos = 0;
	window->running = true;
	window->sizeChanged = false;
	window->mouseButtonsDown = 0;
	window->mouseTracked = false;
	window->cursorEnabled = true;

	window->platform = initialize_platform(*window, cwidth, cheight);

	return window;
}

void
Semper::cleanup_window(sWindow* window)
{
	cleanup_platform(*window);
	delete window;
	window = nullptr;
}

void
Semper::process_window_events(sWindow& window)
{
	process_platform_events(window);
}

void
Semper::enable_cursor(sWindow& window)
{
	if (window.cursorEnabled)
		return;

	window.cursorEnabled = true;

	enable_platform_cursor(window);
}

void
Semper::disable_cursor(sWindow& window)
{
	if (!window.cursorEnabled)
		return;

	sIO& io = get_io();

	window.cursorEnabled = false;

	disable_platform_cursor(window);
}