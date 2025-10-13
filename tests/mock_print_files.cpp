#include "mock_print_files.h"
#include <algorithm>
#include <ctime>
#include <cstdlib>

namespace test {

// Sample filenames with variety of lengths and styles
static const char* SAMPLE_FILENAMES[] = {
    "Benchy.gcode",
    "Calibration_Cube.gcode",
    "Large_Vase_With_Very_Long_Filename_That_Should_Truncate.gcode",
    "Dragon_Model.gcode",
    "Miniature_House.gcode",
    "Phone_Stand_v2.gcode",
    "Cable_Organizer.gcode",
    "Gear_Assembly.gcode",
    "Puzzle_Box.gcode",
    "Flower_Pot.gcode",
    "Desk_Organizer_Final.gcode",
    "Keychain.gcode",
    "Lithophane_Test.gcode",
    "PETG_Test_Print.gcode",
    "Support_Test_Model.gcode",
    "Screw_Container.gcode",
    "Raspberry_Pi_Case.gcode",
    "Arduino_Enclosure.gcode",
    "Warhammer_Mini.gcode",
    "Dice_Tower.gcode",
    "Plant_Marker.gcode",
    "Cookie_Cutter_Star.gcode",
    "Bearing_Holder.gcode",
    "Filament_Clip.gcode",
    "Spool_Holder_Adapter.gcode",
    "Headphone_Stand.gcode",
    "Business_Card_Holder.gcode",
    "Pen_Holder.gcode",
    "Coaster_Set.gcode",
    "Wall_Hook.gcode"
};

static const char* SAMPLE_SUBDIRS[] = {
    "",                     // Root directory
    "calibration/",
    "functional/",
    "decorative/",
    "projects/toys/",
    "projects/enclosures/",
    "models/miniatures/",
    "test_prints/",
};

std::vector<MockPrintFile> MockPrintFileGenerator::generate(size_t count) {
    std::vector<MockPrintFile> files;
    files.reserve(count);

    // Use current time as base for timestamps
    time_t base_time = std::time(nullptr);

    for (size_t i = 0; i < count; ++i) {
        MockPrintFile file;

        // Cycle through sample filenames, adding variety
        file.filename = generate_filename(i);
        file.subdirectory = generate_subdirectory(i);
        file.print_time_minutes = generate_print_time(i);
        file.filament_grams = generate_filament_weight(i);
        file.modified_timestamp = base_time - (i * 3600 * 24); // Spread over days

        // 80% have thumbnails, 20% use placeholder
        file.has_thumbnail = (i % 5) != 0;

        if (file.has_thumbnail) {
            // For now, all point to same placeholder (will be replaced with real thumbs)
            file.thumbnail_path = "A:assets/images/placeholder_thumb_centered.png";
        } else {
            file.thumbnail_path = get_placeholder_thumbnail();
        }

        files.push_back(file);
    }

    return files;
}

void MockPrintFileGenerator::sort(std::vector<MockPrintFile>& files, SortOrder order) {
    switch (order) {
        case SortOrder::NAME_ASC:
            std::sort(files.begin(), files.end(), [](const MockPrintFile& a, const MockPrintFile& b) {
                return a.filename < b.filename;
            });
            break;

        case SortOrder::NAME_DESC:
            std::sort(files.begin(), files.end(), [](const MockPrintFile& a, const MockPrintFile& b) {
                return a.filename > b.filename;
            });
            break;

        case SortOrder::DATE_NEWEST_FIRST:
            std::sort(files.begin(), files.end(), [](const MockPrintFile& a, const MockPrintFile& b) {
                return a.modified_timestamp > b.modified_timestamp;
            });
            break;

        case SortOrder::DATE_OLDEST_FIRST:
            std::sort(files.begin(), files.end(), [](const MockPrintFile& a, const MockPrintFile& b) {
                return a.modified_timestamp < b.modified_timestamp;
            });
            break;
    }
}

std::string MockPrintFileGenerator::get_placeholder_thumbnail() {
    return "A:assets/images/placeholder_thumb_centered.png";
}

// Private helper implementations

std::string MockPrintFileGenerator::generate_filename(int index) {
    const int num_filenames = sizeof(SAMPLE_FILENAMES) / sizeof(SAMPLE_FILENAMES[0]);
    return SAMPLE_FILENAMES[index % num_filenames];
}

std::string MockPrintFileGenerator::generate_subdirectory(int index) {
    const int num_subdirs = sizeof(SAMPLE_SUBDIRS) / sizeof(SAMPLE_SUBDIRS[0]);
    // Distribute files across subdirectories
    return SAMPLE_SUBDIRS[(index / 4) % num_subdirs];
}

int MockPrintFileGenerator::generate_print_time(int index) {
    // Generate varied print times: 5 minutes to 8 hours
    static const int PRINT_TIMES[] = {
        5,      // 5 minutes
        15,     // 15 minutes
        30,     // 30 minutes
        45,     // 45 minutes
        60,     // 1 hour
        80,     // 1h 20m
        120,    // 2 hours
        150,    // 2h 30m
        180,    // 3 hours
        240,    // 4 hours
        360,    // 6 hours
        480     // 8 hours
    };
    const int num_times = sizeof(PRINT_TIMES) / sizeof(PRINT_TIMES[0]);
    return PRINT_TIMES[index % num_times];
}

float MockPrintFileGenerator::generate_filament_weight(int index) {
    // Generate varied filament weights: 2g to 120g
    static const float FILAMENT_WEIGHTS[] = {
        2.0f,
        4.5f,
        8.0f,
        12.0f,
        18.5f,
        25.0f,
        30.0f,
        45.0f,
        65.0f,
        85.0f,
        100.0f,
        120.0f
    };
    const int num_weights = sizeof(FILAMENT_WEIGHTS) / sizeof(FILAMENT_WEIGHTS[0]);
    return FILAMENT_WEIGHTS[index % num_weights];
}

time_t MockPrintFileGenerator::generate_timestamp(int index) {
    // Generate timestamps spread over past 30 days
    time_t now = std::time(nullptr);
    return now - (index * 3600 * 24); // Each file 1 day older
}

} // namespace test
