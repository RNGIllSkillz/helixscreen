#include "lvgl/lvgl.h"
#include "mock_print_files.h"
#include <SDL.h>
#include <iostream>
#include <vector>

// Window dimensions
#define SCREEN_WIDTH  1024
#define SCREEN_HEIGHT 800

// Print file card dimensions (matching ui_xml/globals.xml constants)
#define FILE_CARD_WIDTH 260
#define FILE_CARD_PADDING 12
#define FILE_CARD_THUMBNAIL_SIZE 236  // FILE_CARD_WIDTH - (2 * FILE_CARD_PADDING)
#define FILE_CARD_SPACING 8
#define FILE_CARD_THUMBNAIL_BG 0x2a2a2a
#define FILE_CARD_BG 0x202020
#define FILE_CARD_RADIUS 8

// Helper to format print time as "19m", "1h20m", "2h1m"
std::string format_print_time(int minutes) {
    if (minutes < 60) {
        return std::to_string(minutes) + "m";
    } else {
        int hours = minutes / 60;
        int mins = minutes % 60;
        if (mins == 0) {
            return std::to_string(hours) + "h";
        }
        return std::to_string(hours) + "h" + std::to_string(mins) + "m";
    }
}

// Helper to format filament weight as "4g", "30g", "12.04g"
std::string format_filament_weight(float grams) {
    if (grams == static_cast<int>(grams)) {
        return std::to_string(static_cast<int>(grams)) + "g";
    } else {
        char buf[32];
        snprintf(buf, sizeof(buf), "%.1fg", grams);
        return std::string(buf);
    }
}

// Create a file card from mock data
lv_obj_t* create_file_card(lv_obj_t* parent, const test::MockPrintFile& file) {
    // Create card container (will be instantiated from XML in real impl)
    lv_obj_t* card = lv_obj_create(parent);
    lv_obj_set_size(card, FILE_CARD_WIDTH, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_color(card, lv_color_hex(FILE_CARD_BG), LV_PART_MAIN);
    lv_obj_set_style_radius(card, FILE_CARD_RADIUS, LV_PART_MAIN);
    lv_obj_set_style_border_width(card, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(card, FILE_CARD_PADDING, LV_PART_MAIN);
    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
    // Main axis (vertical): START, Cross axis (horizontal): START (left-aligned)
    lv_obj_set_flex_align(card, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

    // Thumbnail (FILE_CARD_THUMBNAIL_SIZE x FILE_CARD_THUMBNAIL_SIZE square) - center image within container
    lv_obj_t* thumbnail = lv_image_create(card);
    lv_image_set_src(thumbnail, file.thumbnail_path.c_str());
    lv_obj_set_size(thumbnail, FILE_CARD_THUMBNAIL_SIZE, FILE_CARD_THUMBNAIL_SIZE);
    lv_obj_set_style_radius(thumbnail, 6, LV_PART_MAIN);
    lv_image_set_inner_align(thumbnail, LV_IMAGE_ALIGN_CENTER);  // Center image content
    lv_obj_set_style_bg_color(thumbnail, lv_color_hex(FILE_CARD_THUMBNAIL_BG), LV_PART_MAIN);  // Subtle bg to show bounds

    // Filename label (single line, truncated) - must not expand in flex
    lv_obj_t* filename_label = lv_label_create(card);
    lv_label_set_text(filename_label, file.filename.c_str());
    lv_label_set_long_mode(filename_label, LV_LABEL_LONG_DOT);  // Set mode BEFORE setting size
    lv_obj_set_size(filename_label, FILE_CARD_THUMBNAIL_SIZE, 20);  // Fixed width and height (single line ~20px for 16pt font)
    lv_obj_set_flex_grow(filename_label, 0);  // Prevent flex expansion
    lv_obj_set_style_text_color(filename_label, lv_color_hex(0xffffff), LV_PART_MAIN);
    lv_obj_set_style_text_font(filename_label, &lv_font_montserrat_16, LV_PART_MAIN);
    lv_obj_set_style_pad_top(filename_label, FILE_CARD_SPACING, LV_PART_MAIN);
    lv_obj_set_style_pad_all(filename_label, 0, LV_PART_MAIN);  // Remove padding to prevent height increase

    // Metadata row (time + filament)
    lv_obj_t* metadata_row = lv_obj_create(card);
    lv_obj_set_size(metadata_row, FILE_CARD_THUMBNAIL_SIZE, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(metadata_row, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(metadata_row, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(metadata_row, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_top(metadata_row, 4, LV_PART_MAIN);
    lv_obj_set_flex_flow(metadata_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(metadata_row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(metadata_row, FILE_CARD_PADDING, LV_PART_MAIN);

    // Time metadata (icon + text)
    lv_obj_t* time_label = lv_label_create(metadata_row);
    std::string time_text = LV_SYMBOL_DOWNLOAD " " + format_print_time(file.print_time_minutes);
    lv_label_set_text(time_label, time_text.c_str());
    lv_obj_set_style_text_color(time_label, lv_color_hex(0xff4444), LV_PART_MAIN);
    lv_obj_set_style_text_font(time_label, &lv_font_montserrat_14, LV_PART_MAIN);

    // Filament metadata (icon + text)
    lv_obj_t* filament_label = lv_label_create(metadata_row);
    std::string filament_text = LV_SYMBOL_UPLOAD " " + format_filament_weight(file.filament_grams);
    lv_label_set_text(filament_label, filament_text.c_str());
    lv_obj_set_style_text_color(filament_label, lv_color_hex(0xff4444), LV_PART_MAIN);
    lv_obj_set_style_text_font(filament_label, &lv_font_montserrat_14, LV_PART_MAIN);

    return card;
}

// Simple BMP file writer for ARGB8888 format
static bool write_bmp(const char* filename, const uint8_t* data, int width, int height) {
    FILE* f = fopen(filename, "wb");
    if (!f) return false;

    // BMP header (54 bytes total)
    uint32_t file_size = 54 + (width * height * 4);
    uint32_t pixel_offset = 54;
    uint32_t dib_size = 40;
    uint16_t planes = 1;
    uint16_t bpp = 32;

    // BMP file header (14 bytes)
    fputc('B', f); fputc('M', f);                        // Signature
    fwrite(&file_size, 4, 1, f);                         // File size
    fwrite((uint32_t[]){0}, 4, 1, f);                    // Reserved
    fwrite(&pixel_offset, 4, 1, f);                      // Pixel data offset

    // DIB header (40 bytes)
    fwrite(&dib_size, 4, 1, f);                          // DIB header size
    fwrite(&width, 4, 1, f);                             // Width
    fwrite(&height, 4, 1, f);                            // Height
    fwrite(&planes, 2, 1, f);                            // Planes
    fwrite(&bpp, 2, 1, f);                               // Bits per pixel
    fwrite((uint32_t[]){0}, 4, 1, f);                    // Compression (none)
    uint32_t image_size = width * height * 4;
    fwrite(&image_size, 4, 1, f);                        // Image size
    fwrite((uint32_t[]){2835}, 4, 1, f);                 // X pixels per meter
    fwrite((uint32_t[]){2835}, 4, 1, f);                 // Y pixels per meter
    fwrite((uint32_t[]){0}, 4, 1, f);                    // Colors in palette
    fwrite((uint32_t[]){0}, 4, 1, f);                    // Important colors

    // Write pixel data (BMP is bottom-up, so flip rows)
    for (int y = height - 1; y >= 0; y--) {
        fwrite(data + (y * width * 4), 4, width, f);
    }

    fclose(f);
    return true;
}

static void save_screenshot() {
    // Generate unique filename with timestamp
    char filename[256];
    snprintf(filename, sizeof(filename), "/tmp/ui-screenshot-%lu.bmp",
             (unsigned long)time(NULL));

    // Take snapshot using LVGL's native API (platform-independent)
    lv_obj_t* screen = lv_screen_active();
    lv_draw_buf_t* snapshot = lv_snapshot_take(screen, LV_COLOR_FORMAT_ARGB8888);

    if (!snapshot) {
        std::cerr << "Failed to take screenshot" << std::endl;
        return;
    }

    // Write BMP file
    if (write_bmp(filename, snapshot->data, snapshot->header.w, snapshot->header.h)) {
        std::cout << "Screenshot saved: " << filename << std::endl;
    } else {
        std::cerr << "Failed to save screenshot" << std::endl;
    }

    // Free snapshot buffer
    lv_draw_buf_destroy(snapshot);
}

// Custom scroll event handler (currently unused but kept for future use)
static void scroll_event_cb(lv_event_t * e) {
    // This handler is called after scroll happens
    // We can use it for logging or other purposes
    (void)e;  // Suppress unused parameter warning
}

// Initialize LVGL with SDL
bool init_lvgl() {
    lv_init();
    lv_display_t* display = lv_sdl_window_create(SCREEN_WIDTH, SCREEN_HEIGHT);

    // Create input devices
    lv_indev_t* mouse = lv_sdl_mouse_create();
    lv_indev_set_display(mouse, display);

#if LV_SDL_MOUSEWHEEL_MODE == LV_SDL_MOUSEWHEEL_MODE_ENCODER
    // Create mousewheel encoder device for scrolling (only in ENCODER mode)
    lv_indev_t* mousewheel = lv_sdl_mousewheel_create();
    lv_indev_set_display(mousewheel, display);
#endif

    // Create keyboard device
    lv_indev_t* keyboard = lv_sdl_keyboard_create();
    lv_indev_set_display(keyboard, display);

    lv_display_set_default(display);

    return display != nullptr;
}

int main() {
    std::cout << "Print Select Panel Test Harness" << std::endl;
    std::cout << "================================" << std::endl;
    std::cout << std::endl;

    // Initialize LVGL
    if (!init_lvgl()) {
        std::cerr << "Failed to initialize LVGL" << std::endl;
        return 1;
    }

    std::cout << "LVGL initialized: " << SCREEN_WIDTH << "x" << SCREEN_HEIGHT << std::endl;

    // Create main screen
    lv_obj_t* screen = lv_screen_active();

    // Create scrollable grid container (simulating print select panel content area)
    lv_obj_t* container = lv_obj_create(screen);
    lv_obj_set_size(container, SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_obj_set_style_bg_color(container, lv_color_hex(0x111410), LV_PART_MAIN);
    lv_obj_set_style_border_width(container, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(container, 16, LV_PART_MAIN);
    lv_obj_set_flex_flow(container, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_flex_align(container, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(container, 20, LV_PART_MAIN);
    lv_obj_set_style_pad_row(container, 20, LV_PART_MAIN);
    lv_obj_set_scroll_dir(container, LV_DIR_VER);

    // Enable scrolling
    lv_obj_add_flag(container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(container, LV_OBJ_FLAG_SCROLL_WITH_ARROW);  // Disable arrow key scrolling
    lv_obj_set_scroll_snap_y(container, LV_SCROLL_SNAP_NONE);

    // Enable gesture recognition for better trackpad support
    lv_obj_add_flag(container, LV_OBJ_FLAG_GESTURE_BUBBLE);
    lv_obj_clear_flag(container, LV_OBJ_FLAG_SCROLL_ELASTIC);  // Disable elastic scroll

    // Generate mock data
    std::cout << "Generating mock print files..." << std::endl;
    std::vector<test::MockPrintFile> files = test::MockPrintFileGenerator::generate(30);

    // Sort by name (default)
    test::MockPrintFileGenerator::sort(files, test::SortOrder::NAME_ASC);
    std::cout << "Generated " << files.size() << " mock files (sorted by name)" << std::endl;

    // Register placeholder thumbnail
    lv_image_cache_drop(nullptr);
    std::cout << "Using placeholder thumbnail: " << test::MockPrintFileGenerator::get_placeholder_thumbnail() << std::endl;

    // Create cards from mock data
    std::cout << std::endl << "Creating file cards..." << std::endl;
    for (const auto& file : files) {
        create_file_card(container, file);
        std::cout << "  - " << file.get_full_path()
                  << " (" << format_print_time(file.print_time_minutes)
                  << ", " << format_filament_weight(file.filament_grams) << ")" << std::endl;
    }

    std::cout << std::endl << "✅ SUCCESS: " << files.size() << " cards created!" << std::endl;
    std::cout << "Auto-screenshot in 2 seconds..." << std::endl;

    // Auto-screenshot timer (2 seconds after UI creation)
    uint32_t screenshot_time = SDL_GetTicks() + 2000;
    bool screenshot_taken = false;

    // Main loop - standard LVGL event handling
    // Note: Mousewheel scroll direction is inverted due to LVGL SDL driver implementation
    // This is a known limitation that would require patching LVGL upstream
    while (1) {
        uint32_t time_till_next = lv_timer_handler();

        // Auto-screenshot after 2 seconds
        if (!screenshot_taken && SDL_GetTicks() >= screenshot_time) {
            save_screenshot();
            screenshot_taken = true;
            std::cout << "Screenshot captured. Close window to exit." << std::endl;
        }

        SDL_Delay(time_till_next);
    }

    return 0;
}
