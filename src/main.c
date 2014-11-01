#include <pebble.h>
  
#define KEY_PARK 0
#define KEY_OPEN 1
#define KEY_UNTIL 2

Window *hip_main_window;
TextLayer *hip_time_layer;
TextLayer *hip_park_layer;
TextLayer *hip_open_layer;
TextLayer *hip_until_layer;
static GFont hip_time_font;
static GFont hip_park_font;
static GFont hip_hours_font;

static void update_time() {
  // Get tm struct
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  
  // Create long-lived buffer
  static char buffer[] = "00:00";
  
  // Write current hours and minutes into buffer
  if(clock_is_24h_style() == true) {
    // Use 24hr format
    strftime(buffer, sizeof("00:00"), "%H:%M", tick_time);
  } else {
    // Use 12hr format
    strftime(buffer, sizeof("00:00"), "%I:%M", tick_time);
  }
  
  // Display time on TextLayer
  text_layer_set_text(hip_time_layer, buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
  
  if(tick_time->tm_min % 60 == 0) {
    // Begin Dictionary
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);
    
    // Add a key-value pair
    dict_write_uint8(iter, 0, 0);
    
    // Send the message
    app_message_outbox_send();
  }
}

static void main_window_load(Window *window) {
  // Custom Font references
  hip_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_DIN_BOLD_48));
  hip_park_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_DIN_LIGHT_18));
  hip_hours_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_DIN_LIGHT_24));
  
  // Create base time TextLayer
  hip_time_layer = text_layer_create(GRect(0, 42, 144, 50));
  text_layer_set_background_color(hip_time_layer, GColorClear);
  text_layer_set_text_color(hip_time_layer, GColorBlack);
  text_layer_set_font(hip_time_layer, hip_time_font);
  text_layer_set_text_alignment(hip_time_layer, GTextAlignmentCenter);
  
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(hip_time_layer));
  
  // Create Park Title TextLayer
  hip_park_layer = text_layer_create(GRect(0, 90, 144, 24));
  text_layer_set_background_color(hip_park_layer, GColorBlack);
  text_layer_set_text_color(hip_park_layer, GColorWhite);
  text_layer_set_font(hip_park_layer, hip_park_font);
  text_layer_set_text_alignment(hip_park_layer, GTextAlignmentCenter);
  text_layer_set_text(hip_park_layer, "Loading...");
  
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(hip_park_layer));
  
  // Create Open Time TextLayer
  hip_open_layer = text_layer_create(GRect(0, 4, 144, 32));
  text_layer_set_background_color(hip_open_layer, GColorClear);
  text_layer_set_text_color(hip_open_layer, GColorBlack);
  text_layer_set_font(hip_open_layer, hip_hours_font);
  text_layer_set_text_alignment(hip_open_layer, GTextAlignmentCenter);
  text_layer_set_text(hip_open_layer, "OPEN: ..am");
  
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(hip_open_layer));
  
  // Create Until Time TextLayer
  hip_until_layer = text_layer_create(GRect(0, 136, 144, 32));
  text_layer_set_background_color(hip_until_layer, GColorClear);
  text_layer_set_text_color(hip_until_layer, GColorBlack);
  text_layer_set_font(hip_until_layer, hip_hours_font);
  text_layer_set_text_alignment(hip_until_layer, GTextAlignmentCenter);
  text_layer_set_text(hip_until_layer, "UNTIL: ..pm");
  
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(hip_until_layer));
  
}

static void main_window_unload(Window *window) {
  // Destroy Layers
  text_layer_destroy(hip_time_layer);
  text_layer_destroy(hip_park_layer);
  text_layer_destroy(hip_open_layer);
  text_layer_destroy(hip_until_layer);
  
  // Destroy Fonts
  fonts_unload_custom_font(hip_time_font);
  fonts_unload_custom_font(hip_park_font);
  fonts_unload_custom_font(hip_hours_font);
  
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  // Store incoming info
  static char park_buffer[32];
  static char open_buffer[32];
  static char until_buffer[32];
  
  // Read first item
  Tuple *t = dict_read_first(iterator);
  
  // For all items
  while(t != NULL) {
    // Which key was received
    switch(t->key) {
      case KEY_PARK:
        snprintf(park_buffer, sizeof(park_buffer), "%s", t->value->cstring);
        //APP_LOG(APP_LOG_LEVEL_INFO, "")
        break;
      case KEY_OPEN:
        snprintf(open_buffer, sizeof(open_buffer), "OPEN: %sam", t->value->cstring);
        break;
      case KEY_UNTIL:
          if( strcmp(t->value->cstring, "12") == 0 ) {
            snprintf(until_buffer, sizeof(until_buffer), "UNTIL: %sam", t->value->cstring);
          } else {
            snprintf(until_buffer, sizeof(until_buffer), "UNTIL: %spm", t->value->cstring);
          }
        break;
      default:
        APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d not recognized.", (int)t->key);
        break;
    }
    
    // Look for next item
    t = dict_read_next(iterator);
  }
  
  // Display info to right text layer
  text_layer_set_text(hip_park_layer, park_buffer);
  text_layer_set_text(hip_open_layer, open_buffer);
  text_layer_set_text(hip_until_layer, until_buffer);
  
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message Dropped");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed.");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox sent successully.");
}

static void handle_init(void) {
  hip_main_window = window_create();
  
  // Register with TimeTicker Services
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  
  window_set_window_handlers(hip_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  
  window_stack_push(hip_main_window, true);
  
  update_time();
  
  // Register AppMessage Callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  
  // Open AppMessage
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
}

static void handle_deinit(void) {
  window_destroy(hip_main_window);
}

int main(void) {
  handle_init();
  app_event_loop();
  handle_deinit();
}
