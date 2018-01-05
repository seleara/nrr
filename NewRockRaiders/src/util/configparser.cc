#include <nrr/util/configparser.h>

#include <nrr/resource/wadarchive.h>

void ConfigParser::parse(WadArchive &archive, const std::string &path) {
	const auto &entry = archive.get(path);
	auto &stream = archive.getStream(entry);
	
	std::string line;
	uint64_t end = (uint64_t)stream.tellg() + entry.size;

	root_ = std::make_unique<ConfigBlock>();
	ConfigBlock *current = root_.get();

	// Debug
	int indent = 0;

	std::string lastLineWithText;

	while (stream.good() && stream.tellg() < end) {
		std::getline(stream, line);
		auto trimmed = StringUtil::trim(line);
		auto commentPos = trimmed.find(';');
		if (commentPos != std::string::npos) {
			//line.erase(commentPos, std::string::npos);
			trimmed = trimmed.substr(0, commentPos);
		}
		if (trimmed == "") continue;

		auto openBlockPos = trimmed.find('{');
		if (openBlockPos != std::string::npos) {
			//line.erase(openBlockPos, std::string::npos);
			trimmed = trimmed.substr(0, openBlockPos);
			trimmed = StringUtil::rtrim(trimmed);
			auto next = std::make_unique<ConfigBlock>();
			std::string id;
			if (trimmed != "") {
				id = trimmed;
			} else {
				id = lastLineWithText;
			}
			next->id = id;
			next->parent = current;
			next->type = ConfigNodeType::Block;
			current->children.insert({ next->id, std::move(next) });
			current = (ConfigBlock *)current->children[id].get();

#ifdef NRR_CONFIGPARSER_DEBUG
			for (int i = 0; i < indent; ++i)
				std::cout << "  ";
			std::cout << "Block \"" << id << "\" {\n";
			++indent;
#endif
			
			continue;
		}

		auto closeBlockPos = trimmed.find('}');
		if (closeBlockPos != std::string::npos) {
			//line.erase(closeBlockPos, std::string::npos);
			trimmed = trimmed.substr(0, closeBlockPos);
			trimmed = StringUtil::rtrim(trimmed);
		}

		if (trimmed != "") {
			auto trimmedStr = std::string(trimmed);
			auto tokens = StringUtil::splitRef(trimmedStr);
			if (tokens.size() == 2) {
				auto value = std::make_unique<ConfigValue>();
				value->id = tokens[0];
				value->value = tokens[1];
				value->parent = current;
				value->type = ConfigNodeType::Value;
				current->children.insert({ value->id, std::move(value) });

#ifdef NRR_CONFIGPARSER_DEBUG
				for (int i = 0; i < indent; ++i)
					std::cout << "  ";
				std::cout << "Value \"" << tokens[0] << "\" = " << tokens[1] << "\n";
#endif
			}
		}

		if (closeBlockPos != std::string::npos) {
			current = current->parent;

#ifdef NRR_CONFIGPARSER_DEBUG
			--indent;
			for (int i = 0; i < indent; ++i)
				std::cout << "  ";
			std::cout << "}\n";
			continue;
#endif
		}

		lastLineWithText = trimmed;

		//std::cout << line << std::endl;
		//std::cin.get();
	}
}