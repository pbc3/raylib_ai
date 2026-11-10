
#include "utils.h"

std::vector<int> parse_ints_from_line(std::string_view line) {
	std::vector<int> vals;

	while (!line.empty()) {
		while (!line.empty() &&
			(std::isspace(static_cast<unsigned char>(line.front())) ||
				std::isalpha(static_cast<unsigned char>(line.front())) ||
				(std::ispunct(static_cast<unsigned char>(line.front())) && line.front() != '-')))
		{
			line.remove_prefix(1);
		}

		if (line.empty()) {
			break;
		}

		int value;
		auto [ptr, errorcode] = std::from_chars(line.data(), line.data() + line.size(), value);

		if ((errorcode != std::errc())) {
			// TODO - HANDLE
			break;
		}
		vals.push_back(value);

		// advance the ptr past what we just parsed
		line.remove_prefix(ptr - line.data());
	}
	return vals;
}
std::optional<std::vector<std::vector<int>>> parse_csv(std::string path) {
	std::ifstream file(path);
	if (!file.is_open()) {
		return std::nullopt;
	}

	std::vector<std::vector<int>> grid;

	//std::string total;
	////while (std::getline(file, line)) {

	////	total += line + "\n";
	////}


	int row_idx = 0;
	std::string line;

	while (std::getline(file, line)) {
		// take all the numbers
		std::vector<int> tiles = parse_ints_from_line(line);
		//grid[row_idx] = tiles;
		grid.push_back(tiles);
		row_idx++;
	}
	return grid;
}