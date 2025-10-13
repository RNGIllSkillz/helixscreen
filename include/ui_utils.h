#pragma once

#include <string>
#include <ctime>

/**
 * Format print time from minutes to human-readable string
 * Examples: "5m", "1h30m", "8h"
 * @param minutes Total print time in minutes
 * @return Formatted string
 */
std::string format_print_time(int minutes);

/**
 * Format filament weight from grams to human-readable string
 * Examples: "2.5g", "45g", "120g"
 * @param grams Filament weight in grams
 * @return Formatted string
 */
std::string format_filament_weight(float grams);

/**
 * Format file size from bytes to human-readable string
 * Examples: "1.2 KB", "45 MB", "1.5 GB"
 * @param bytes File size in bytes
 * @return Formatted string
 */
std::string format_file_size(size_t bytes);

/**
 * Format timestamp to date/time string
 * Examples: "Jan 15 14:30", "Dec 5 09:15"
 * @param timestamp Unix timestamp (time_t)
 * @return Formatted date/time string
 */
std::string format_modified_date(time_t timestamp);
