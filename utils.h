#pragma once
#include <charconv>
#include <fstream>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

std::vector<int> parse_ints_from_line(std::string_view line);
std::optional<std::vector<std::vector<int>>> parse_csv(const std::string& path);
