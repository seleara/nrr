#include <nrr/resource/shader/shader.h>
#include <nrr/resource/shader/gl/glshader.h>

#include <fstream>
#include <sstream>

#include <nrr/util/string.h>

void ShaderWrapper::load(const std::string &path) {
	resource_ = ShaderLoader::load(path);
}

void ShaderWrapper::bind() {
	resource_->bind();
}

void ShaderWrapper::release() {
	resource_->release();
}

std::string ShaderResource::preprocessShaderSource(const std::string &path) {
	auto slash = path.rfind('/');
	std::string dir;
	if (slash == std::string::npos) dir = "";
	else dir = path.substr(0, slash + 1);
	std::ifstream ifs(path, std::ios_base::binary);
	if (!ifs.is_open()) return false;
	ifs.seekg(0, std::ios_base::end);
	size_t size = ifs.tellg();
	ifs.seekg(0, std::ios_base::beg);
	char *code = new char[size];
	ifs.read(code, size);
	ifs.close();
	std::string scode(code, size);
	delete[] code;

	// Check for #include's
	std::vector<std::string> includes;
	std::stringstream ss(scode);
	std::string line;
	bool findingIncludes = true;
	uint64_t lastPos;
	do {
		lastPos = ss.tellg();
		std::getline(ss, line);
		line = line.substr(0, line.find("//"));
		StringUtil::trim(line);
		if (line.length() == 0) continue;
		if (line.find("#include") != 0) {
			findingIncludes = false;
			ss.seekg(lastPos);
		} else {
			auto quoteOpen = line.find('"');
			auto quoteClose = line.rfind('"');
			auto includePath = line.substr(quoteOpen + 1, quoteClose - quoteOpen - 1);
			includes.push_back(includePath);
		}
	} while (findingIncludes);

	std::stringstream combined;
	for (const auto &includePath : includes) {
		std::ifstream f(dir + includePath, std::ios_base::binary);
		if (!f.is_open()) throw std::runtime_error("Unable to open included shader source '" + path + "'.");
		f.seekg(0, std::ios_base::end);
		size_t incSize = f.tellg();
		f.seekg(0, std::ios_base::beg);
		char *incCode = new char[incSize];
		f.read(incCode, incSize);
		f.close();
		combined.write(incCode, incSize);
		combined << "\n";
	}
	uint64_t strippedSourceSize = scode.size() - lastPos;
	char *strippedSource = new char[strippedSourceSize];
	ss.read(strippedSource, strippedSourceSize);
	combined.write(strippedSource, strippedSourceSize);
	delete[] strippedSource;

	return combined.str();
}