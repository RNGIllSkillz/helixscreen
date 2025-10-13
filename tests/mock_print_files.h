#pragma once

#include <string>
#include <vector>
#include <ctime>

namespace test {

/**
 * Mock print file data structure for testing Print Select Panel
 */
struct MockPrintFile {
    std::string filename;           // e.g., "Benchy.gcode"
    std::string subdirectory;       // e.g., "projects/boats/" or "" for root
    std::string thumbnail_path;     // Path to thumbnail image (prefer large)
    int print_time_minutes;         // Total print time in minutes
    float filament_grams;           // Filament usage in grams
    bool has_thumbnail;             // true if thumbnail exists
    time_t modified_timestamp;      // File modification time (for sorting)

    // Helper to get full path
    std::string get_full_path() const {
        if (subdirectory.empty()) {
            return filename;
        }
        return subdirectory + filename;
    }
};

/**
 * Sort order for print files
 */
enum class SortOrder {
    NAME_ASC,           // A → Z
    NAME_DESC,          // Z → A
    DATE_NEWEST_FIRST,  // Newest → Oldest
    DATE_OLDEST_FIRST   // Oldest → Newest
};

/**
 * Mock data generator for testing
 */
class MockPrintFileGenerator {
public:
    /**
     * Generate a diverse set of mock print files for testing
     * @param count Number of files to generate (default: 30)
     * @return Vector of mock print files
     */
    static std::vector<MockPrintFile> generate(size_t count = 30);

    /**
     * Sort files by specified order
     * @param files Files to sort (modified in place)
     * @param order Sort order
     */
    static void sort(std::vector<MockPrintFile>& files, SortOrder order);

    /**
     * Generate placeholder thumbnail path
     * @return Path to generic placeholder image
     */
    static std::string get_placeholder_thumbnail();

private:
    // Internal helpers for generating varied test data
    static std::string generate_filename(int index);
    static std::string generate_subdirectory(int index);
    static int generate_print_time(int index);
    static float generate_filament_weight(int index);
    static time_t generate_timestamp(int index);
};

} // namespace test
