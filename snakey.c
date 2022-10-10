/* Copyright
 *   2021 solartempest
 *   2021 Luna code adapted from HellSingCoder and Jackasaur
 *   2021 QMK
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty o
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
 
//SSD1306 OLED update loop
#ifdef OLED_ENABLE
	uint32_t oled_timer = 0; //OLED timeout
	led_t led_usb_state;

	// KEYBOARD PET START
	#define KEYBOARD_PET
	 
	// settings for WPM-reactive animations
	#define MIN_WALK_SPEED 10
	#define MIN_RUN_SPEED 40
	 
	// advanced settings
	#define ANIM_FRAME_DURATION 300 // how long each frame lasts in ms. Default is 200.
	#define ANIM_SIZE 96 // number of bytes in array. If you change sprites, minimize for adequate firmware size. max is 1024
	 
	// timers
	uint32_t anim_timer = 0;
	 
	// current frame
	uint8_t current_frame = 0;
	 
	// status variables
	int current_wpm_read = 0;
	#ifndef SNEAK_DISABLE
		bool isSneaking = false;
	#endif
	bool isJumping = false;
	bool showedJump = true;
	bool isBarking = false;
	 
	#if defined(OLED_LOGO) && !defined(OLED_NO_SLAVE)
		static void render_logo(int LUNA_X, int LUNA_Y) {
			static const char PROGMEM luna_logo[] = {
				0x00, 0x00, 0x04, 0xf6, 0xb8, 0xf8, 0xd8, 0xf0, 0xe0, 0xc0, 0x00, 0x00, 0x00, 0x80, 0xc0, 0xe0, 
				0xe0, 0xf0, 0xf0, 0xf0, 0xf0, 0xe0, 0xe0, 0xc0, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
				0x00, 0x80, 0xc0, 0xe1, 0xf3, 0xfb, 0x7f, 0x1f, 0x0f, 0x00, 0x00, 0xf0, 0xff, 0xff, 0x3f, 0x03, 
				0x01, 0x00, 0x00, 0x00, 0x80, 0xc1, 0xf3, 0xff, 0x7f, 0x1f, 0x00, 0x00, 0x80, 0xc0, 0xe0, 0x30, 
				0x1e, 0x3f, 0x3f, 0x7f, 0xf9, 0xf0, 0xf0, 0x70, 0x78, 0x3c, 0x3f, 0x1f, 0x0f, 0x03, 0x00, 0x00, 
				0x00, 0x00, 0x1c, 0x3f, 0x7f, 0x7f, 0xfb, 0xf1, 0xf0, 0x78, 0x3c, 0x1e, 0x0f, 0x03, 0x00, 0x00, 
				};
			oled_set_cursor(LUNA_X,LUNA_Y);
			oled_write_raw_P(luna_logo, sizeof(luna_logo));
		}
	#endif
	 
	// logic
	static void render_luna(int LUNA_X, int LUNA_Y) {
	 
		// Sit
		static const char PROGMEM sit[2][ANIM_SIZE] = {
			// 'sit1', 32x22px
			{
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xf0, 0xf0, 0xf8, 0xf8, 0xcc, 
				0xcc, 0xf8, 0xf8, 0xf0, 0xf0, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x3f, 0x7f, 0xfc, 0xf9, 0xf1, 
				0xe1, 0xc1, 0x81, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x38, 0x70, 0x60, 0xe0, 0xe0, 0xf0, 0xf0, 0xf8, 0xf9, 0xff, 
				0xff, 0xff, 0xff, 0xff, 0x7e, 0x1c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
			},
	 
			// 'sit2', 32x22px
			{
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xf0, 0xf0, 0xf8, 0xf8, 0xcc, 
				0xcc, 0xf8, 0xf8, 0xf0, 0xf0, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x3f, 0x7f, 0xfc, 0xf9, 0xf1, 
				0xe1, 0xc1, 0x81, 0x00, 0x00, 0x01, 0x01, 0x02, 0x06, 0x0c, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x38, 0x7c, 0x60, 0xe0, 0xe0, 0xf0, 0xf0, 0xf8, 0xf9, 0xff, 
				0xff, 0xff, 0xff, 0xff, 0x7e, 0x1c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
			}
		};
	 
		// Walk
		static const char PROGMEM walk[2][ANIM_SIZE] = {
			// 'walk1', 32x22px
			{
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0xf8, 0xfc, 0xfe, 0xfe, 0x1f, 0x3f, 0x39, 0x39, 0x3f, 0x3f, 0x1e, 0x1e, 0x0c, 0x00, 
				0x00, 0x00, 0x01, 0x0f, 0x3e, 0x78, 0x70, 0xf8, 0xfc, 0xfe, 0x7e, 0x3e, 0x3c, 0x3c, 0x78, 0x78, 
				0x78, 0xf0, 0xf1, 0xff, 0x7f, 0x7f, 0x3e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
			},
	 
			// 'walk2', 32x22px
			{
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0x80, 0x80, 0xc0, 0xc0, 0x60, 0x60, 0xc0, 0xc0, 0xc0, 0x80, 0x00, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0x20, 0x60, 0xc0, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
				0x3e, 0x7f, 0xff, 0xff, 0xe7, 0xc7, 0x0e, 0x0e, 0x0f, 0x0f, 0x07, 0x07, 0x03, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0x00, 0x00, 0x01, 0x07, 0x1f, 0x3e, 0x78, 0xf0, 0xfc, 0x7e, 0x1e, 0x1f, 0x0f, 0x0e, 
				0x1e, 0x1c, 0x3d, 0x7f, 0xff, 0xff, 0xff, 0xfc, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
			}
		};
	 
		// Run
		static const char PROGMEM run[2][ANIM_SIZE] = {
			// 'run1', 32x22px
			{
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
				0x80, 0xc0, 0xe0, 0xf0, 0xf0, 0xf8, 0x98, 0xf8, 0xf8, 0xf8, 0xf0, 0x70, 0x20, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0x00, 0x60, 0xc0, 0x80, 0x00, 0x00, 0x00, 0x00, 0xc0, 0xf0, 0xf0, 0xf0, 0xe0, 0xc0, 
				0x87, 0x7f, 0xff, 0xff, 0xf1, 0xc3, 0x03, 0x03, 0x03, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0x00, 0x01, 0x03, 0x07, 0x1f, 0x3e, 0x78, 0xf7, 0xff, 0x7f, 0x3d, 0x03, 0x07, 0x0f, 
				0x1e, 0x3d, 0x7f, 0xff, 0xff, 0xff, 0xfc, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
			},
	 
			// 'run2', 32x22px
			{
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0x80, 0xe0, 0xf0, 0xf0, 0xf8, 0x98, 0xf8, 0xf8, 0xf8, 0xf0, 0x70, 0x20, 0x00, 0x00, 
				0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0xc0, 0xf8, 0xfc, 0x3e, 0xde, 0xfe, 0xfc, 0xf8, 0x00, 0x00, 
				0xe0, 0xfe, 0xff, 0x7f, 0x07, 0x01, 0x03, 0x03, 0x03, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 
				0x00, 0x07, 0x1e, 0x3c, 0x78, 0xff, 0xff, 0x7f, 0x01, 0x00, 0x07, 0x1f, 0x3f, 0x7c, 0x78, 0xff, 
				0xff, 0x7f, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
			}
		};
	 
		// Bark
		static const char PROGMEM bark[2][ANIM_SIZE] = {
			// 'bark1', 32x22px
			{
				0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xc0, 0xc0, 0xe0, 0xe0, 0x30, 0x70, 0xe0, 0xe0, 0xc0, 0xc0, 
				0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0x00, 0x00, 0x1f, 0x7f, 0xff, 0xff, 0xe7, 0xe7, 0xef, 0xef, 0xcf, 0xcb, 0x83, 0x83, 
				0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0x06, 0x1c, 0x38, 0x78, 0xf1, 0xe3, 0xe3, 0xe7, 0xf7, 0xf7, 0xff, 0xff, 0xff, 0xff, 
				0x7f, 0x3e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
			},
	 
			// 'bark2', 32x22px
			{
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0x80, 0xc0, 0xe0, 0xe0, 0x60, 0xe0, 0xc0, 0xc0, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0xf8, 0xfe, 0xff, 0xff, 0xff, 0x9f, 
				0x0f, 0x0e, 0x1f, 0x1f, 0x3f, 0x3f, 0x7e, 0xe6, 0x83, 0x03, 0x0f, 0x43, 0x43, 0x90, 0x88, 0x08, 
				0x00, 0x00, 0x03, 0x1e, 0x3c, 0x78, 0xf0, 0xe0, 0xe0, 0xf0, 0xf0, 0xfb, 0xff, 0xff, 0xff, 0x7f, 
				0x7f, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
			}
		};
	 
		// Sneak
		#ifndef SNEAK_DISABLE
			static const char PROGMEM sneak[2][ANIM_SIZE] = {
				// 'sneak1', 32x22px
				{
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
					0x00, 0x00, 0x00, 0xcc, 0xf8, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0xf0, 0xf8, 0xf8, 
					0x7c, 0xfc, 0xee, 0xe6, 0xfc, 0xfc, 0x78, 0x78, 0xb0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
					0x00, 0x00, 0x3f, 0x7f, 0xf1, 0xf8, 0xf8, 0x7c, 0x7c, 0xf8, 0xf0, 0xf0, 0xf1, 0xe7, 0xef, 0xef, 
					0xfe, 0xfe, 0xfe, 0xfc, 0xfc, 0xf8, 0xf0, 0x60, 0x00, 0x01, 0x03, 0x06, 0x02, 0x00, 0x00, 0x00, 
				},
		 
				// 'sneak2', 32x22px
				{
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
					0x00, 0x00, 0x80, 0xe0, 0xf8, 0x1c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0xf0, 0xf8, 0xf8, 
					0x7c, 0xfc, 0xee, 0xe6, 0xfc, 0xfc, 0x78, 0x78, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
					0x00, 0x00, 0x3f, 0x7f, 0xf0, 0xf8, 0xf8, 0x7c, 0x7c, 0xf8, 0xf0, 0xf0, 0xf1, 0xe7, 0xef, 0xef, 
					0xfe, 0xfe, 0xfe, 0xfc, 0xfc, 0xf8, 0xf0, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
				}
			};
		#endif
	 
		// animation
		void animate_luna(void) {
			// jump
			if (isJumping || !showedJump) {
				// clear
				oled_set_cursor(LUNA_X,LUNA_Y +2);
				oled_write("     ", false);
				oled_set_cursor(LUNA_X,LUNA_Y -1);
				showedJump = true;
			} else {
				// clear
				oled_set_cursor(LUNA_X,LUNA_Y -1);
				oled_write("     ", false);
				oled_set_cursor(LUNA_X,LUNA_Y);
			}
	 
			// switch frame
			current_frame = (current_frame + 1) % 2;
	 
			// current status
			if(isBarking) {
				oled_write_raw_P(bark[abs(1 - current_frame)], ANIM_SIZE);
			#ifndef SNEAK_DISABLE
				} else if(isSneaking) {
					oled_write_raw_P(sneak[abs(1 - current_frame)], ANIM_SIZE);
			#endif
			} else if(current_wpm_read <= MIN_WALK_SPEED) {
				oled_write_raw_P(sit[abs(1 - current_frame)], ANIM_SIZE);
			} else if(current_wpm_read <= MIN_RUN_SPEED) {
				oled_write_raw_P(walk[abs(1 - current_frame)], ANIM_SIZE);
			} else {
				oled_write_raw_P(run[abs(1 - current_frame)], ANIM_SIZE);
			}
		}
	 
		// animation timer
		if(timer_elapsed32(anim_timer) > ANIM_FRAME_DURATION) {
			anim_timer = timer_read32();
			animate_luna();
		}
	}
	// KEYBOARD PET END

	#ifndef OLED_NO_SLAVE
		static void print_logo_narrow(void) {
			oled_set_cursor(0,2);
			oled_write("ROHAN", false);
			oled_set_cursor(0,3);
			oled_write("NIGHT", false);
		 
			#ifdef OLED_LOGO
				render_logo(0,7); //Not defining this in config.h will save space (+112).
			#endif
			
			/* wpm counter */
			oled_set_cursor(0,13);
			oled_write_P(PSTR("WPM: "), false);
			oled_write(get_u8_str(get_current_wpm(), ' '), false);
		}
	#endif
	 
	static void print_status_narrow(void) {
		/*switch (get_highest_layer(default_layer_state)) { //May be useful if you use multiple non-QWERTY layouts to show which is active.
			case 0:
				oled_write("SOLAR", false);
				break;
			case 1:
				oled_write("GAME", false);
				break;
			default:
				oled_write("UNDEF", false);
		}*/
	 
		/* Print current layer */
		oled_set_cursor(0,2);
		oled_write("LAYER", false);
	 
		oled_set_cursor(0,3);
		switch (get_highest_layer(layer_state)) {
			case 0:
			case 1:
				oled_write("Base ", false);
				break;
			case 2:
				oled_write("Raise ", false);
				break;
			case 3:
				oled_write("Lower", false);
				break;
			case 4:
				oled_write("Undef", false);
				break;
		}
		
		/* lock status */
		oled_set_cursor(0,6);
		oled_write("LOCK", false);
		oled_set_cursor(0,7);
		oled_write("Caps", led_usb_state.caps_lock);
		oled_set_cursor(0,8);
		oled_write("Num", !(led_usb_state.num_lock));
		oled_set_cursor(0,9);
		oled_write("Scrl", led_usb_state.scroll_lock);
	 
		/* KEYBOARD PET RENDER START */
		render_luna(0,12);
		/* KEYBOARD PET RENDER END */
	}
	 
	oled_rotation_t oled_init_user(oled_rotation_t rotation) {
		return OLED_ROTATION_270;
	}
	 
	bool oled_task_user(void) {
		/* KEYBOARD PET VARIABLES START */
		current_wpm_read = get_current_wpm();
		/* KEYBOARD PET VARIABLES END */
		led_usb_state = host_keyboard_led_state();
		
		if (is_keyboard_master()) {
			if (timer_elapsed32(oled_timer) > 30000) {//OLED timeout 30000ms = 30s
				oled_off();
				return false;
			} else {
				oled_on();
			}
			print_status_narrow();
		} else {
			print_logo_narrow();
		}
		return false;
	}
#endif