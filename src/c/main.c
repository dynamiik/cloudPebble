#include <pebble.h>
//APP_LOG(APP_LOG_LEVEL_DEBUG, "hahahah");

static Window *s_main_window;
static TextLayer *s_time_layer;
static GFont s_time_font;

// Declare globally
static TextLayer *s_weather_layer;
static GFont s_weather_font;

static TextLayer *s_weatherTown_layer;
static GFont s_weatherTown_font;

static TextLayer *s_dayDate_layer;
static GFont s_dayDate_font;

static TextLayer *s_monthDay_layer;
static GFont s_monthDay_font;

static BitmapLayer *s_bt_icon_layer;
static GBitmap *s_bt_icon_bitmap, *s_nbt_icon_bitmap;
static bool btState;

static BitmapLayer *s_quiet_icon_layer;
static GBitmap *s_quietOn_icon_bitmap, *s_quietOff_icon_bitmap;

static int s_battery_level;
static Layer *s_battery_layer;

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  // Write the current hours and minutes into a buffer
  static char s_buffer[8];
  strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ?
                                          "%H:%M" : "%I:%M", tick_time);
  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, s_buffer);

}
static void update_date(){
	time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
	static char wochentage[7][12]= {"Montag", "Dienstag", "Mittwoch", "Donnerstag", "Freitag", "Samstag", "Sonntag"}; 
	static char monate[12][12]={"Januar", "Februar", "März", "April", "Mai", "Juni", "Juli", 
															"August", "September", "Oktober", "November", "Dezember"};
	static char date_weekday[] = "x";
	static char date_month[] = "xx";
	static char date_monthDay[] = "xx";
	static int dayNumber = 0;
	static int monthNumber = 0;
	
	strftime(date_weekday,
			sizeof(date_weekday),
			"%u",
			tick_time);
	
	strftime(date_month,
			sizeof(date_month),
			"%m",
			tick_time);
	
	strftime(date_monthDay,
			sizeof(date_monthDay),
			"%d",
			tick_time);
	
	if(strcmp(date_weekday,"7")==0){
		dayNumber=6;
	} else if(strcmp(date_weekday,"6")==0){
		dayNumber=5;
	} else if(strcmp(date_weekday,"5")==0){
		dayNumber=4;
	} else if(strcmp(date_weekday,"4")==0){
		dayNumber=3;
	} else if(strcmp(date_weekday,"3")==0){
		dayNumber=2;
	} else if(strcmp(date_weekday,"2")==0){
		dayNumber=1;
	} else if(strcmp(date_weekday,"1")==0){
		dayNumber=0;
	}

	if(strcmp(date_month,"01")==0){
		monthNumber=0;
	} else if(strcmp(date_month,"02")==0){
		monthNumber=1;
	} else if(strcmp(date_month,"03")==0){
		monthNumber=2;
	} else if(strcmp(date_month,"04")==0){
		monthNumber=3;
	} else if(strcmp(date_month,"05")==0){
		monthNumber=4;
	} else if(strcmp(date_month,"06")==0){
		monthNumber=5;
	} else if(strcmp(date_month,"07")==0){
		monthNumber=6;
	} else if(strcmp(date_month,"08")==0){
		monthNumber=7;
	} else if(strcmp(date_month,"09")==0){
		monthNumber=8;
	} else if(strcmp(date_month,"10")==0){
		monthNumber=9;
	} else if(strcmp(date_month,"11")==0){
		monthNumber=10;
	} else if(strcmp(date_month,"12")==0){
		monthNumber=11;
	}  
	static char day_buffer[12];
	static char month_buffer[15];
	snprintf(day_buffer, sizeof(day_buffer), "%s", wochentage[dayNumber]);
	snprintf(month_buffer, sizeof(month_buffer), "%s.%s", date_monthDay, monate[monthNumber]);
  text_layer_set_text(s_dayDate_layer, day_buffer);
  text_layer_set_text(s_monthDay_layer, month_buffer);


}
static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  if (units_changed & SECOND_UNIT) {}	
	if (units_changed & MINUTE_UNIT) {
		update_time();		
		// Get weather update every 30 minutes
		if(tick_time->tm_min % 15 == 0) {
			// Begin dictionary
			DictionaryIterator *iter;
			app_message_outbox_begin(&iter);

			// Add a key-value pair
			dict_write_uint8(iter, 0, 0);

			// Send the message!
			app_message_outbox_send();
		}
	}	
	if (units_changed & DAY_UNIT) {
		update_date();
	}	
}

static void battery_callback(BatteryChargeState state) {
  // Record the new battery level
  s_battery_level = state.charge_percent;
	
	// Update meter
	layer_mark_dirty(s_battery_layer);

}
static void battery_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);

  // Find the width of the bar (total width = 114px)
  int width = (s_battery_level * bounds.size.w) / 100;
	
  // Draw the background
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

  // Draw the bar
	if(s_battery_level >= 50){
		graphics_context_set_fill_color(ctx, GColorWhite);	
	}
	if(s_battery_level < 50){
		graphics_context_set_fill_color(ctx, GColorYellow);	
	}
	if(s_battery_level < 30){
		graphics_context_set_fill_color(ctx, GColorRed);	
	}
  graphics_fill_rect(ctx, GRect(0, 0, width, bounds.size.h), 0, GCornerNone);
}

static void bluetooth_callback(bool connected) {
  // Show icon if disconnected
  //layer_set_hidden(bitmap_layer_get_layer(s_bt_icon_layer), connected);
		bitmap_layer_set_compositing_mode(s_bt_icon_layer, GCompOpSet);
  if(connected) {
		bitmap_layer_set_bitmap(s_bt_icon_layer, s_bt_icon_bitmap);
  }else{
		bitmap_layer_set_bitmap(s_bt_icon_layer, s_nbt_icon_bitmap);
		if(btState != connected){
			vibes_long_pulse();	
		}
	}
	btState = connected;
}



static void main_window_load(Window *window) {
  // Get information about the Window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
	
  // Create the TextLayer with specific bounds
  s_time_layer = text_layer_create(
      GRect(0, PBL_IF_ROUND_ELSE(30, 30), bounds.size.w, 50));

  // Improve the layout to be more like a watchface
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorBlack);
  text_layer_set_text(s_time_layer, "00:00");
  //text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
	// Create GFont
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_PERFECT_DOS_42));
  // Apply to TextLayer
  text_layer_set_font(s_time_layer, s_time_font);
	
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);

  // Add it as a child layer to the Window's root layer
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
	
	// Create temperature Layer
	s_weather_layer = text_layer_create(
			GRect(0, PBL_IF_ROUND_ELSE(125, 120), bounds.size.w, 25));

	// Style the text
	text_layer_set_background_color(s_weather_layer, GColorClear);
	text_layer_set_text_color(s_weather_layer, GColorWhite);
	text_layer_set_text_alignment(s_weather_layer, GTextAlignmentCenter);
	text_layer_set_text(s_weather_layer, "Loading...");
	
	// Create second custom font, apply it and add to Window
	s_weather_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_PERFECT_DOS_20));
	text_layer_set_font(s_weather_layer, s_weather_font);
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_weather_layer));
	
	// Create Town Layer
	s_weatherTown_layer = text_layer_create(
			GRect(0, PBL_IF_ROUND_ELSE(140, 145), bounds.size.w-5, 25));

	// Style the text
	text_layer_set_background_color(s_weatherTown_layer, GColorClear);
	text_layer_set_text_color(s_weatherTown_layer, GColorWhite);
	text_layer_set_text_alignment(s_weatherTown_layer, GTextAlignmentRight);
	text_layer_set_text(s_weatherTown_layer, "Loading...");
	
	// Create second custom font, apply it and add to Window
	s_weatherTown_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_PERFECT_DOS_12));
	text_layer_set_font(s_weatherTown_layer, s_weatherTown_font);
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_weatherTown_layer));
	
	// Create dayDate Layer
	s_dayDate_layer = text_layer_create(
			GRect(5, PBL_IF_ROUND_ELSE(140, 3), bounds.size.w, 25));

	// Style the text
	text_layer_set_background_color(s_dayDate_layer, GColorClear);
	text_layer_set_text_color(s_dayDate_layer, GColorBlack);
	text_layer_set_text_alignment(s_dayDate_layer, GTextAlignmentLeft);
	text_layer_set_text(s_dayDate_layer, "Loading...");
	
	// Create second custom font, apply it and add to Window
	s_dayDate_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_PERFECT_DOS_12));
	text_layer_set_font(s_dayDate_layer, s_dayDate_font);
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_dayDate_layer));
	
	// Create monthDay Layer
	s_monthDay_layer = text_layer_create(
			GRect(0, PBL_IF_ROUND_ELSE(140, 100), bounds.size.w-5, 25));

	// Style the text
	text_layer_set_background_color(s_monthDay_layer, GColorClear);
	text_layer_set_text_color(s_monthDay_layer, GColorBlack);
	text_layer_set_text_alignment(s_monthDay_layer, GTextAlignmentRight);
	text_layer_set_text(s_monthDay_layer, "Loading...");
	
	// Create second custom font, apply it and add to Window
	s_monthDay_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_PERFECT_DOS_12));
	text_layer_set_font(s_monthDay_layer, s_monthDay_font);
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_monthDay_layer));
	
	// Create battery meter Layer
	s_battery_layer = layer_create(GRect(0, 0, bounds.size.w, 2));
	layer_set_update_proc(s_battery_layer, battery_update_proc);

	// Add to Window
	layer_add_child(window_get_root_layer(window), s_battery_layer);
	
	// Create the Bluetooth icon GBitmap
	s_bt_icon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BT_ON);
	s_nbt_icon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BT_OFF);

	// Create the BitmapLayer to display the GBitmap
	s_bt_icon_layer = bitmap_layer_create(GRect(bounds.size.w-23,3, 20, 20));
	
	bitmap_layer_set_bitmap(s_bt_icon_layer, s_bt_icon_bitmap);
	layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_bt_icon_layer));
	// Show the correct state of the BT connection from the start
	btState = connection_service_peek_pebble_app_connection();
	bluetooth_callback(connection_service_peek_pebble_app_connection());
	
	// Create the Quiet icon GBitmap
	s_quietOn_icon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_QUIET_ON);
	s_quietOff_icon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_QUIET_OFF);

	// Create the BitmapLayer to display the GBitmap
	s_quiet_icon_layer = bitmap_layer_create(GRect(bounds.size.w-53,3, 20, 20));
	bitmap_layer_set_compositing_mode(s_quiet_icon_layer, GCompOpSet);
	
	if(quiet_time_is_active()){
		bitmap_layer_set_bitmap(s_quiet_icon_layer,s_quietOn_icon_bitmap);
	}else{
		bitmap_layer_set_bitmap(s_quiet_icon_layer,s_quietOff_icon_bitmap);
	}
	layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_quiet_icon_layer));
}

static void main_window_unload(Window *window) {
  // Destroy TextLayer
  text_layer_destroy(s_time_layer);
	// Unload GFont
  fonts_unload_custom_font(s_time_font);
	// Destroy weather elements
	text_layer_destroy(s_weather_layer);
	fonts_unload_custom_font(s_weather_font);
	// Destroy weatherTown elements
	text_layer_destroy(s_weatherTown_layer);
	fonts_unload_custom_font(s_weatherTown_font);
	// Destroy dayDate elements
	text_layer_destroy(s_dayDate_layer);
	fonts_unload_custom_font(s_dayDate_font);
	// Destroy monthDay elements
	text_layer_destroy(s_monthDay_layer);
	fonts_unload_custom_font(s_monthDay_font);
	
	layer_destroy(s_battery_layer);

	gbitmap_destroy(s_bt_icon_bitmap);
	gbitmap_destroy(s_nbt_icon_bitmap);
	bitmap_layer_destroy(s_bt_icon_layer);
	
	gbitmap_destroy(s_quietOn_icon_bitmap);
	gbitmap_destroy(s_quietOff_icon_bitmap);
	bitmap_layer_destroy(s_quiet_icon_layer);

}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
	// Store incoming information
	static char temperature_buffer[8];
	static char conditions_buffer[32];
	static char weather_layer_buffer[32];
	// Read tuples for data
	Tuple *temp_tuple = dict_find(iterator, MESSAGE_KEY_TEMPERATURE);
	Tuple *conditions_tuple = dict_find(iterator, MESSAGE_KEY_CONDITIONS);

	// If all data is available, use it
	if(temp_tuple && conditions_tuple) {
		snprintf(temperature_buffer, sizeof(temperature_buffer), "%dC", (int)temp_tuple->value->int32);
		snprintf(conditions_buffer, sizeof(conditions_buffer), "%s", conditions_tuple->value->cstring);
		// Assemble full string and display
		snprintf(weather_layer_buffer, sizeof(weather_layer_buffer), "%s, %s", temperature_buffer, conditions_buffer);
		text_layer_set_text(s_weather_layer, weather_layer_buffer);
	}
	static char town_buffer[32];
	static char weatherTown_layer_buffer[32];
	// Read tuples for data
	Tuple *town_tuple = dict_find(iterator, MESSAGE_KEY_NAME);

	// If all data is available, use it
	if(town_tuple) {
		snprintf(town_buffer, sizeof(town_buffer), "%s", town_tuple->value->cstring);
		// Assemble full string and display
		text_layer_set_text(s_weatherTown_layer, town_buffer);
	}
}
static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}
static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

static void init() {
 // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
	
	// Register with TickTimerService
	tick_timer_service_subscribe(SECOND_UNIT | MINUTE_UNIT | HOUR_UNIT | DAY_UNIT, tick_handler);
	
	// Make sure the time is displayed from the start
	update_time();
	update_date();	
	
	window_set_background_color(s_main_window, GColorCyan);
	// Register callbacks
	app_message_register_inbox_received(inbox_received_callback);
	app_message_register_inbox_dropped(inbox_dropped_callback);
	app_message_register_outbox_failed(outbox_failed_callback);
	app_message_register_outbox_sent(outbox_sent_callback);
	// Open AppMessage
	const int inbox_size = 128;
	const int outbox_size = 128;
	app_message_open(inbox_size, outbox_size);
	
	// Register for battery level updates
	battery_state_service_subscribe(battery_callback);
	// Ensure battery level is displayed from the start
	battery_callback(battery_state_service_peek());
	
	// Register for Bluetooth connection updates
	connection_service_subscribe((ConnectionHandlers) {
		.pebble_app_connection_handler = bluetooth_callback
	});
}

static void deinit() {
  // Destroy Window
 window_destroy(s_main_window);
	
	//Destroy Battery callback
	//layer_destroy(s_battery_layer);

}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
