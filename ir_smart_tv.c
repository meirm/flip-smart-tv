#include "dolphin/dolphin.h"
#include <gui/gui.h>
#include <infrared.h>
#include <infrared_worker.h>
#include <furi_hal_infrared.h>
#include <flipper_format.h>
#include <toolbox/args.h>
#include <toolbox/strint.h>
#include <m-dict.h>

#include "infrared_signal.h"
InfraredSignal* IR_UP = NULL;
InfraredSignal* IR_DOWN = NULL;
InfraredSignal* IR_LEFT = NULL;
InfraredSignal* IR_RIGHT = NULL;
InfraredSignal* IR_OK = NULL;
InfraredSignal* IR_BACK = NULL;
InfraredSignal* IR_HOME = NULL;
FuriString* IR_UP_STR = NULL;
FuriString* IR_DOWN_STR = NULL;
FuriString* IR_LEFT_STR = NULL;
FuriString* IR_RIGHT_STR = NULL;
FuriString* IR_OK_STR = NULL;
FuriString* IR_BACK_STR = NULL;
FuriString* IR_HOME_STR = NULL;

// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------

#define INFRARED_CLI_BUF_SIZE (10U)

static bool infrared_parse_message(const char* str, InfraredSignal* signal) {
    char protocol_name[32];
    InfraredMessage message;
    int parsed = sscanf(str, "%31s %lX %lX", protocol_name, &message.address, &message.command);

    if(parsed != 3) {
        return false;
    }

    message.protocol = infrared_get_protocol_by_name(protocol_name);
    message.repeat = false;
    infrared_signal_set_message(signal, &message);
    return infrared_signal_is_valid(signal);
}

static void infrared_start_ir_tx(FuriString* args) {
    const char* str = furi_string_get_cstr(args);
    InfraredSignal* signal = infrared_signal_alloc();

    bool success = infrared_parse_message(str, signal); // || infrared_parse_raw(str, signal);
    if(success) {
        infrared_signal_transmit(signal);
    }

    infrared_signal_free(signal);
}

// -----------------------------------------------------------------------------
// We keep the same data structure, but we only use the mutex for thread safety,
// not for x/y manipulations in this version.
// -----------------------------------------------------------------------------
typedef struct AppState {
    FuriMutex* mutex;
} AppState;

static void my_draw_callback(Canvas* canvas, void* context) {
    UNUSED(context);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 5, 10, "IR Remote Demo");
    canvas_draw_str(canvas, 5, 20, "Up/Down.");
    canvas_draw_str(canvas, 5, 30, "Use Left/Right.");
    canvas_draw_str(canvas, 5, 40, "OK (Hold for Home)");
    canvas_draw_str(canvas, 5, 50, "Back=(sends IR_BACK).");
    canvas_draw_str(canvas, 5, 60, "Hold Back to exit.");
}

static void my_input_callback(InputEvent* input_event, void* context) {
    // We'll just enqueue the input events to handle them in the main loop
    FuriMessageQueue* queue = (FuriMessageQueue*)context;
    furi_message_queue_put(queue, input_event, FuriWaitForever);
}

int32_t ir_smart_tv_app() {
    // Create a message queue to receive input events
    FuriMessageQueue* queue = furi_message_queue_alloc(8, sizeof(InputEvent));
    FuriString* IR_UP_SIGNAL = furi_string_alloc();
    furi_string_set(IR_UP_SIGNAL, "Samsung32 0x07 0x60");
    FuriString* IR_DOWN_SIGNAL = furi_string_alloc();
    furi_string_set(IR_DOWN_SIGNAL, "Samsung32 0x07 0x61");
    FuriString* IR_LEFT_SIGNAL = furi_string_alloc();
    furi_string_set(IR_LEFT_SIGNAL, "Samsung32 0x07 0x65");
    FuriString* IR_RIGHT_SIGNAL = furi_string_alloc();
    furi_string_set(IR_RIGHT_SIGNAL, "Samsung32 0x07 0x62");
    FuriString* IR_OK_SIGNAL = furi_string_alloc();
    furi_string_set(IR_OK_SIGNAL, "Samsung32 0x07 0x68");
    FuriString* IR_BACK_SIGNAL = furi_string_alloc();
    furi_string_set(IR_BACK_SIGNAL, "Samsung32 0x07 0x58");
    FuriString* IR_HOME_SIGNAL = furi_string_alloc();
    furi_string_set(IR_HOME_SIGNAL, "Samsung32 0x07 0x79");

    // Allocate our app state
    AppState* app_state = malloc(sizeof(AppState));
    app_state->mutex = furi_mutex_alloc(FuriMutexTypeNormal);

    // Create a ViewPort and set up its draw & input callbacks
    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, my_draw_callback, app_state);
    view_port_input_callback_set(view_port, my_input_callback, queue);

    // Uncomment different orientations to test them
    view_port_set_orientation(view_port, ViewPortOrientationHorizontal);
    // view_port_set_orientation(view_port, ViewPortOrientationHorizontalFlip);
    //view_port_set_orientation(view_port, ViewPortOrientationVertical);
    // view_port_set_orientation(view_port, ViewPortOrientationVerticalFlip);

    // Register this view_port with the GUI
    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    // Example Dolphin deed call
    dolphin_deed(DolphinDeedPluginGameStart);

    // Main event loop
    InputEvent event;
    bool keep_processing = true;
    while(keep_processing) {
        if(furi_message_queue_get(queue, &event, FuriWaitForever) == FuriStatusOk) {
            furi_mutex_acquire(app_state->mutex, FuriWaitForever);

            // Only respond to short or repeat presses
            if(event.type == InputTypeShort || event.type == InputTypeRepeat) {
                // We will transmit the decoded signals using the const char * IR_UP_SIGNAL,...
                switch(event.key) {
                case InputKeyUp:
                    infrared_start_ir_tx(IR_UP_SIGNAL);
                    break;
                case InputKeyDown:
                    infrared_start_ir_tx(IR_DOWN_SIGNAL);
                    break;
                case InputKeyLeft:
                    infrared_start_ir_tx(IR_LEFT_SIGNAL);
                    break;
                case InputKeyRight:
                    infrared_start_ir_tx(IR_RIGHT_SIGNAL);
                    break;
                case InputKeyOk:
                    infrared_start_ir_tx(IR_OK_SIGNAL);
                    break;
                case InputKeyBack:
                    infrared_start_ir_tx(IR_BACK_SIGNAL);
                    break;
                default:
                    break;
                }
            } else if(event.type == InputTypeLong && event.key == InputKeyBack) {
                // Long press of Back button exits the application
                keep_processing = false;
            } else if(event.type == InputTypeLong && event.key == InputKeyOk) {
                infrared_start_ir_tx(IR_HOME_SIGNAL);
                break;
            }

            furi_mutex_release(app_state->mutex);
            view_port_update(view_port);
        } else {
            // If we can't get an event, exit
            keep_processing = false;
        }
    }

    // Cleanup
    view_port_enabled_set(view_port, false);
    furi_message_queue_free(queue);
    gui_remove_view_port(gui, view_port);
    view_port_free(view_port);
    furi_record_close(RECORD_GUI);
    furi_mutex_free(app_state->mutex);
    free(app_state);

    return 0;
}
