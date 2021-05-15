#include <fstream>

#include "cc.hpp"

namespace cc {

static std::unordered_map<String, FileCache> s_fileMap;

bool file_exists(const char* path) {
    std::ifstream fs(path);
    return fs.good();
}
bool file_exists(const String& path) {
    std::ifstream fs(path);
    return fs.good();
}

FileCache::FileCache(const String& path) : m_path(path) {
    std::ifstream file(path);
    assert(file.is_open());

    file.seekg(0, std::ios::end);
    m_source.reserve(file.tellg());
    file.seekg(0, std::ios::beg);
    m_source.assign((std::istreambuf_iterator<char>(file)),
                    std::istreambuf_iterator<char>());

    const char* begin = m_source.c_str();
    while (1) {
        const char* end = strchr(begin, '\n');

        if (end == nullptr) {
            m_lines.push_back(String(begin));
            break;
        }

        m_lines.push_back(String(begin, end - begin));
        begin = end + 1;
    }
}

bool fcache_has(const char* path) {
    return s_fileMap.find(path) != s_fileMap.end();
}

bool fcache_has(const String& path) {
    String spath(path);
    return fcache_has(spath);
}

void fcache_set_tokens(const char* path, const TokenList& tokens) {
    String spath(path);
    fcache_set_tokens(spath, tokens);
}

void fcache_set_tokens(const String& path, const TokenList& tokens) {
    auto it = s_fileMap.find(path);
    assertfmt(it != s_fileMap.end(), "path %s does not exists in cache", path.c_str());
    it->second.SetTokens(tokens);
}

FileCache& fcache_get(const char* path) {
    String spath(path);
    return fcache_get(spath);
}

FileCache& fcache_get(const String& path) {
    const auto it = s_fileMap.find(path);
    if (it != s_fileMap.end()) {
        return it->second;
    }

    s_fileMap.insert(std::pair(path, FileCache(path)));
    return s_fileMap.find(path)->second;
}

}  // namespace cc
