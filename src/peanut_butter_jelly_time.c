#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"


#define MY_UUID { 0x9B, 0x5E, 0x69, 0x04, 0xCA, 0xAD, 0x4E, 0xFA, 0xB7, 0x19, 0x82, 0x10, 0x6B, 0x65, 0x47, 0x79 }

PBL_APP_INFO(MY_UUID,
             "Peanut Butter Jelly Time", "Stvsan", //Based off of Dansl's Silly Walk
             1, 0, /* App version */
             RESOURCE_ID_IMAGE_MENU_ICON,
             APP_INFO_WATCH_FACE);

Window window;

BmpContainer background_image_container;

TextLayer text_date_layer;

RotBmpPairContainer hour_hand_image_container;
RotBmpPairContainer minute_hand_image_container;
RotBmpPairContainer second_hand_image_container;

int last_wday = -1;     /* used to tell when the day changes */

void update_watch(PblTm* t){
    
    static char date_text[] = "XXXXXXXX, XXX 00";

	//rotbmp_pair_layer_set_angle(&hour_hand_image_container.layer, ((t->tm_hour % 12) * 30) + (t->tm_min/2));
	hour_hand_image_container.layer.white_layer.rotation = TRIG_MAX_ANGLE * (((t->tm_hour % 12) * 30) + (t->tm_min/2)) / 360;
  	hour_hand_image_container.layer.black_layer.rotation = TRIG_MAX_ANGLE * (((t->tm_hour % 12) * 30) + (t->tm_min/2)) / 360;
	hour_hand_image_container.layer.layer.frame.origin.x = (144/2) - (hour_hand_image_container.layer.layer.frame.size.w/2);
	hour_hand_image_container.layer.layer.frame.origin.y = (168/2) - (hour_hand_image_container.layer.layer.frame.size.h/2);
	layer_mark_dirty(&hour_hand_image_container.layer.layer);

	//rotbmp_pair_layer_set_angle(&minute_hand_image_container.layer, t->tm_min * 6);
	minute_hand_image_container.layer.white_layer.rotation = TRIG_MAX_ANGLE * (t->tm_min * 6) / 360;
  	minute_hand_image_container.layer.black_layer.rotation = TRIG_MAX_ANGLE * (t->tm_min * 6) / 360;
	minute_hand_image_container.layer.layer.frame.origin.x = (144/2) - (minute_hand_image_container.layer.layer.frame.size.w/2);
	minute_hand_image_container.layer.layer.frame.origin.y = (168/2) - (minute_hand_image_container.layer.layer.frame.size.h/2);
	layer_mark_dirty(&minute_hand_image_container.layer.layer);
    
    //Set Date text
    if (t->tm_wday != last_wday)
    {
        string_format_time(date_text, sizeof(date_text), "%A, %b %e", t);
        text_layer_set_text(&text_date_layer, date_text);
    
        last_wday = t->tm_wday;
    }
    
}

// Called once per second
void handle_minute_tick(AppContextRef ctx, PebbleTickEvent *t) {
	update_watch(t->tick_time);
}


// Handle the start-up of the app
void handle_init(AppContextRef app_ctx) {

	// Create our app's base window
	window_init(&window, "Peanut Butter Jelly Time");
	window_stack_push(&window, true);
	window_set_background_color(&window, GColorBlack);

    GFont font_date;
    
	resource_init_current_app(&APP_RESOURCES);
    
    font_date = fonts_get_system_font(FONT_KEY_GOTHIC_14);

	// Set up a layer for the static watch face background
	bmp_init_container(RESOURCE_ID_IMAGE_BACKGROUND, &background_image_container);
	layer_add_child(&window.layer, &background_image_container.layer.layer);


	// Set up a layer for the hour hand
	rotbmp_pair_init_container(RESOURCE_ID_IMAGE_HOUR_HAND_WHITE, RESOURCE_ID_IMAGE_HOUR_HAND_BLACK, &hour_hand_image_container);
	rotbmp_pair_layer_set_src_ic(&hour_hand_image_container.layer, GPoint(33, 40));
	layer_add_child(&window.layer, &hour_hand_image_container.layer.layer);


	// Set up a layer for the minute hand
	rotbmp_pair_init_container(RESOURCE_ID_IMAGE_MINUTE_HAND_WHITE, RESOURCE_ID_IMAGE_MINUTE_HAND_BLACK, &minute_hand_image_container);
	rotbmp_pair_layer_set_src_ic(&minute_hand_image_container.layer, GPoint(16, 60));
	layer_add_child(&window.layer, &minute_hand_image_container.layer.layer);
    
    // Set up a layer for the date
    text_layer_init(&text_date_layer, window.layer.frame);
    text_layer_set_text_color(&text_date_layer, GColorWhite);
    text_layer_set_background_color(&text_date_layer, GColorClear);
    layer_set_frame(&text_date_layer.layer, GRect(8, 152, 144-8, 168-92));
    text_layer_set_font(&text_date_layer, font_date);
    text_layer_set_text_alignment(&text_date_layer,GTextAlignmentCenter);
    layer_add_child(&window.layer, &text_date_layer.layer);
    

	PblTm t;
	get_time(&t);
	update_watch(&t);
    
    last_wday = -1;

}

void handle_deinit(AppContextRef ctx) {

	bmp_deinit_container(&background_image_container);
	rotbmp_pair_deinit_container(&hour_hand_image_container);
	rotbmp_pair_deinit_container(&minute_hand_image_container);
	rotbmp_pair_deinit_container(&second_hand_image_container);
}


// The main event/run loop for our app
void pbl_main(void *params) {
  PebbleAppHandlers handlers = {

    // Handle app start
    .init_handler = &handle_init,
    .deinit_handler = &handle_deinit,

    // Handle time updates
    .tick_info = {
      .tick_handler = &handle_minute_tick,
      .tick_units = MINUTE_UNIT
    }

  };
  app_event_loop(params, &handlers);
}