#include <pebble.h>

static Window *window;

static TextLayer *step_layer;
static char s_step_layer[32];

static TextLayer *countdown_layer;
static char s_countdown_layer[32];

static TextLayer *nextstep_layer;
static char s_nextstep_layer[32];

int nextstep=0;
int nextstep_countdown=-1;

static VibePattern oneshort_pattern = {
  .durations = (uint32_t []) {100},
  .num_segments = 1
};

static VibePattern twoshort_pattern = {
  .durations = (uint32_t []) {100, 200, 100},
  .num_segments = 3
};

static VibePattern threeshort_pattern = {
  .durations = (uint32_t []) {100, 200, 100, 200, 100},
  .num_segments = 5
};

struct {
	int time; // in seconds
	char *text;
	VibePattern *vibe;
#if 0
} msgs[13] = {
	{ 10, "Upper left inner" },
	{ 10, "Upper left top" },
	{ 10, "Upper left outer" },
	{ 10, "Upper right inner" },
	{ 10, "Upper right top" },
	{ 10, "Upper right outer" },
	{  3, "Spit" },
	{ 10, "Lower left inner" },
	{ 10, "Lower left top" },
	{ 10, "Lower left outer" },
	{ 10, "Lower right inner" },
	{ 10, "Lower right top" },
	{ 10, "Lower right outer" },
#else //0
} msgs[10] = {
	{  3, "Get ready", NULL },

	{ 15, "Lower right outer", &twoshort_pattern },
	{ 15, "Lower left outer", &oneshort_pattern },

	{ 15, "Lower left inner", &oneshort_pattern },
	{ 15, "Lower right inner", &oneshort_pattern },

	{  3, "Spit", &twoshort_pattern },

	{ 15, "Upper right outer", &twoshort_pattern },
	{ 15, "Upper left outer", &oneshort_pattern },

	{ 15, "Upper left inner", &oneshort_pattern },
	{ 15, "Upper right inner", &oneshort_pattern },
#endif
};

const int msgs_size = sizeof(msgs)/sizeof(msgs[0]);
AppTimer *tooth_timer = NULL;

static void
exit_callback(void *data)
{
  window_stack_pop_all(false);  
}

static void 
timer_callback(void *data)
{
	
	if (nextstep > msgs_size-1)
	{
		s_step_layer[0] = 0;
		s_nextstep_layer[0] = 0;
		snprintf(s_countdown_layer, sizeof(s_countdown_layer), "Done!");
		layer_mark_dirty(text_layer_get_layer(step_layer));
		layer_mark_dirty(text_layer_get_layer(nextstep_layer));
		layer_mark_dirty(text_layer_get_layer(countdown_layer));
		vibes_enqueue_custom_pattern(threeshort_pattern);
    tooth_timer = app_timer_register(5*60*1000 /* 5 minutes */, exit_callback, NULL);
		return;
	}

	tooth_timer = app_timer_register(1000 /* milliseconds */, timer_callback, NULL);
	APP_LOG(APP_LOG_LEVEL_DEBUG, "timer_callback nextstep:%d, nextstep_countdown:%d", nextstep, nextstep_countdown);
	if (nextstep_countdown <= 0)
	{
    light_enable_interaction();
		nextstep_countdown = msgs[nextstep].time;
		if (msgs[nextstep].vibe)
			vibes_enqueue_custom_pattern(*msgs[nextstep].vibe);
		snprintf(s_step_layer, sizeof(s_step_layer), "%s", msgs[nextstep].text);
		APP_LOG(APP_LOG_LEVEL_DEBUG, "tc: new step: %s (%d)", msgs[nextstep].text, nextstep_countdown);
		if (nextstep < msgs_size-1)
			snprintf(s_nextstep_layer, sizeof(s_nextstep_layer), "Next:\n%s", msgs[nextstep+1].text);
		else
			snprintf(s_nextstep_layer, sizeof(s_nextstep_layer), "Next:\nDone!");
		layer_mark_dirty(text_layer_get_layer(step_layer));
		layer_mark_dirty(text_layer_get_layer(nextstep_layer));
	}
	snprintf(s_countdown_layer, sizeof(s_countdown_layer), "%d", nextstep_countdown);
	layer_mark_dirty(text_layer_get_layer(countdown_layer));
	nextstep_countdown --;
	if (nextstep_countdown == 0)
		nextstep ++;
}

static void
select_click_handler(ClickRecognizerRef recognizer, void *context)
{
	nextstep=0;
	nextstep_countdown = 0;
	if (tooth_timer != NULL)
	{
		app_timer_cancel(tooth_timer);
		tooth_timer = NULL;
	}
	timer_callback(NULL);
}

static void 
click_config_provider(void *context)
{
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  /* window_single_click_subscribe(BUTTON_ID_UP, up_click_handler); */
  /* window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler); */
}

#define TEXT_FONT FONT_KEY_GOTHIC_24_BOLD
static void 
window_load(Window *window)
{
	Layer *window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_bounds(window_layer);
	
	APP_LOG(APP_LOG_LEVEL_DEBUG, "window load. bounds.size w:%d, h:%d", bounds.size.w, bounds.size.h);

	s_step_layer[0] = 0;
	step_layer = text_layer_create((GRect) { .origin = { 0, 0 }, .size = { bounds.size.w, 48 } });
	text_layer_set_text(step_layer, s_step_layer);
	text_layer_set_font(step_layer, fonts_get_system_font(TEXT_FONT));
	// text_layer_set_text_alignment(step_layer, GTextAlignmentCenter);
	layer_add_child(window_layer, text_layer_get_layer(step_layer));
	
	strcpy(s_countdown_layer, "Start");
	countdown_layer = text_layer_create((GRect) { .origin = { 0, 48 }, .size = { bounds.size.w, 42 } });
	text_layer_set_text(countdown_layer, s_countdown_layer);
	text_layer_set_font(countdown_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
	text_layer_set_text_alignment(countdown_layer, GTextAlignmentCenter);
	layer_add_child(window_layer, text_layer_get_layer(countdown_layer));
	
	s_nextstep_layer[0] = 0;
  // nextstep is two lines (first is "Next:"), so needs to be taller than step
	nextstep_layer = text_layer_create((GRect) { .origin = { 0, 90 }, .size = { bounds.size.w, 54 } });
	text_layer_set_text(nextstep_layer, s_nextstep_layer);
	text_layer_set_font(nextstep_layer, fonts_get_system_font(TEXT_FONT));
	//text_layer_set_text_alignment(nextstep_layer, GTextAlignmentCenter);
	layer_add_child(window_layer, text_layer_get_layer(nextstep_layer));
	
}

static void 
window_unload(Window *window) 
{
  text_layer_destroy(step_layer);
  text_layer_destroy(countdown_layer);
  text_layer_destroy(nextstep_layer);
}

static void 
init(void) 
{
  window = window_create();
  window_set_click_config_provider(window, click_config_provider);
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  const bool animated = true;
  window_stack_push(window, animated);
}

static void 
deinit(void) 
{
  window_destroy(window);
}

int 
main(void) 
{
  init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);

  app_event_loop();
  deinit();
}
