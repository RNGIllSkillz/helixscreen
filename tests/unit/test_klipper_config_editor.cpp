// Copyright (C) 2025-2026 356C LLC
// SPDX-License-Identifier: GPL-3.0-or-later

#include "klipper_config_editor.h"

#include "../catch_amalgamated.hpp"

using namespace helix::system;

TEST_CASE("KlipperConfigEditor - section parsing", "[config][parser]") {
    KlipperConfigEditor editor;

    SECTION("Finds simple section") {
        std::string content = "[printer]\nkinematics: corexy\n\n[probe]\npin: PA1\nz_offset: 1.5\n";
        auto result = editor.parse_structure(content);
        REQUIRE(result.sections.count("probe") == 1);
        REQUIRE(result.sections["probe"].line_start > 0);
    }

    SECTION("Handles section with space in name") {
        std::string content = "[bed_mesh default]\nversion: 1\n";
        auto result = editor.parse_structure(content);
        REQUIRE(result.sections.count("bed_mesh default") == 1);
    }

    SECTION("Finds key within section") {
        std::string content = "[probe]\npin: PA1\nz_offset: 1.5\nsamples: 3\n";
        auto result = editor.parse_structure(content);
        auto key = result.find_key("probe", "z_offset");
        REQUIRE(key.has_value());
        REQUIRE(key->value == "1.5");
    }

    SECTION("Handles both : and = delimiters") {
        std::string content = "[probe]\npin: PA1\nz_offset = 1.5\n";
        auto result = editor.parse_structure(content);
        auto key1 = result.find_key("probe", "pin");
        auto key2 = result.find_key("probe", "z_offset");
        REQUIRE(key1->delimiter == ":");
        REQUIRE(key2->delimiter == "=");
    }

    SECTION("Skips multi-line values correctly") {
        std::string content =
            "[gcode_macro START]\ngcode:\n    G28\n    G1 Z10\n\n[probe]\npin: PA1\n";
        auto result = editor.parse_structure(content);
        auto key = result.find_key("probe", "pin");
        REQUIRE(key.has_value());
        REQUIRE(key->value == "PA1");
    }

    SECTION("Identifies SAVE_CONFIG boundary") {
        std::string content = "[probe]\npin: PA1\n\n"
                              "#*# <---------------------- SAVE_CONFIG ---------------------->\n"
                              "#*# DO NOT EDIT THIS BLOCK OR BELOW.\n"
                              "#*#\n"
                              "#*# [probe]\n"
                              "#*# z_offset = 1.234\n";
        auto result = editor.parse_structure(content);
        REQUIRE(result.save_config_line > 0);
    }

    SECTION("Preserves comments - not treated as keys") {
        std::string content = "# My config\n[probe]\n# Z offset\nz_offset: 1.5\n";
        auto result = editor.parse_structure(content);
        auto key = result.find_key("probe", "z_offset");
        REQUIRE(key.has_value());
        // Should only have z_offset as a key, not comments
        REQUIRE(result.sections["probe"].keys.size() == 1);
    }

    SECTION("Detects include directives") {
        std::string content =
            "[include hardware/*.cfg]\n[include macros.cfg]\n[printer]\nkinematics: corexy\n";
        auto result = editor.parse_structure(content);
        REQUIRE(result.includes.size() == 2);
        REQUIRE(result.includes[0] == "hardware/*.cfg");
        REQUIRE(result.includes[1] == "macros.cfg");
    }

    SECTION("Option names are lowercased") {
        std::string content = "[probe]\nZ_Offset: 1.5\n";
        auto result = editor.parse_structure(content);
        auto key = result.find_key("probe", "z_offset");
        REQUIRE(key.has_value());
    }

    SECTION("Handles empty file") {
        auto result = editor.parse_structure("");
        REQUIRE(result.sections.empty());
        REQUIRE(result.includes.empty());
    }

    SECTION("Handles file with only comments") {
        auto result = editor.parse_structure("# Just a comment\n; Another\n");
        REQUIRE(result.sections.empty());
    }

    SECTION("Multi-line value with empty lines preserved") {
        std::string content =
            "[gcode_macro M]\ngcode:\n    G28\n\n    G1 Z10\n\n[probe]\npin: PA1\n";
        auto result = editor.parse_structure(content);
        // The gcode macro's multi-line value spans across the empty line
        auto gcode_key = result.find_key("gcode_macro M", "gcode");
        REQUIRE(gcode_key.has_value());
        REQUIRE(gcode_key->is_multiline);
        // probe section should still be found after the multi-line value
        REQUIRE(result.sections.count("probe") == 1);
    }

    SECTION("Section line ranges are correct") {
        std::string content =
            "[printer]\nkinematics: corexy\nmax_velocity: 300\n\n[probe]\npin: PA1\n";
        auto result = editor.parse_structure(content);
        auto& printer = result.sections["printer"];
        auto& probe = result.sections["probe"];
        REQUIRE(printer.line_start < probe.line_start);
        REQUIRE(printer.line_end < probe.line_start);
    }
}

TEST_CASE("KlipperConfigEditor - value editing", "[config][editor]") {
    KlipperConfigEditor editor;

    SECTION("set_value replaces existing value") {
        std::string content = "[probe]\npin: PA1\nz_offset: 1.5\nsamples: 3\n";
        auto result = editor.set_value(content, "probe", "samples", "5");
        REQUIRE(result.has_value());
        REQUIRE(result->find("samples: 5") != std::string::npos);
        // Other values unchanged
        REQUIRE(result->find("pin: PA1") != std::string::npos);
        REQUIRE(result->find("z_offset: 1.5") != std::string::npos);
    }

    SECTION("set_value preserves delimiter style") {
        std::string content = "[probe]\nz_offset = 1.5\n";
        auto result = editor.set_value(content, "probe", "z_offset", "2.0");
        REQUIRE(result.has_value());
        REQUIRE(result->find("z_offset = 2.0") != std::string::npos);
    }

    SECTION("set_value preserves comments") {
        std::string content = "[probe]\n# Important comment\nz_offset: 1.5\n";
        auto result = editor.set_value(content, "probe", "z_offset", "2.0");
        REQUIRE(result.has_value());
        REQUIRE(result->find("# Important comment") != std::string::npos);
    }

    SECTION("set_value returns nullopt for missing key") {
        std::string content = "[probe]\npin: PA1\n";
        auto result = editor.set_value(content, "probe", "samples", "5");
        REQUIRE_FALSE(result.has_value());
    }

    SECTION("set_value returns nullopt for missing section") {
        std::string content = "[printer]\nkinematics: corexy\n";
        auto result = editor.set_value(content, "probe", "pin", "PA1");
        REQUIRE_FALSE(result.has_value());
    }

    SECTION("add_key adds to end of section") {
        std::string content = "[probe]\npin: PA1\nz_offset: 1.5\n\n[printer]\nkinematics: corexy\n";
        auto result = editor.add_key(content, "probe", "samples", "3");
        REQUIRE(result.has_value());
        REQUIRE(result->find("samples: 3") != std::string::npos);
        // Should be in [probe] section, before [printer]
        auto samples_pos = result->find("samples: 3");
        auto printer_pos = result->find("[printer]");
        REQUIRE(samples_pos < printer_pos);
    }

    SECTION("add_key returns nullopt for missing section") {
        std::string content = "[printer]\nkinematics: corexy\n";
        auto result = editor.add_key(content, "probe", "pin", "PA1");
        REQUIRE_FALSE(result.has_value());
    }

    SECTION("add_key respects custom delimiter") {
        std::string content = "[probe]\npin = PA1\n";
        auto result = editor.add_key(content, "probe", "samples", "3", " = ");
        REQUIRE(result.has_value());
        REQUIRE(result->find("samples = 3") != std::string::npos);
    }

    SECTION("remove_key comments out the line") {
        std::string content = "[probe]\npin: PA1\nsamples: 3\nz_offset: 1.5\n";
        auto result = editor.remove_key(content, "probe", "samples");
        REQUIRE(result.has_value());
        REQUIRE(result->find("#samples: 3") != std::string::npos);
        // Other keys untouched
        REQUIRE(result->find("pin: PA1") != std::string::npos);
        REQUIRE(result->find("z_offset: 1.5") != std::string::npos);
    }

    SECTION("remove_key returns nullopt for missing key") {
        std::string content = "[probe]\npin: PA1\n";
        auto result = editor.remove_key(content, "probe", "nonexistent");
        REQUIRE_FALSE(result.has_value());
    }

    SECTION("set_value handles value with spaces") {
        std::string content = "[probe]\nsamples_result: median\n";
        auto result = editor.set_value(content, "probe", "samples_result", "average");
        REQUIRE(result.has_value());
        REQUIRE(result->find("samples_result: average") != std::string::npos);
    }
}
