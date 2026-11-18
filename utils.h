#pragma once
#include <string>
#include <charconv>
#include <vector>
#include <optional>
#include <sstream>
#include <fstream>


std::vector<int> parse_ints_from_line(std::string_view line);
std::optional<std::vector<std::vector<int>>> parse_csv(const std::string& path);