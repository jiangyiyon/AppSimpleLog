#include "../include/speckit/log/archive.h"
#include <filesystem>
#include <fstream>
#include <vector>
#include <string>

namespace fs = std::filesystem;

namespace speckit {
    namespace log {

        // NOTE: Minimal placeholder implementation: creates a single concatenated file with .zip extension.
        bool createArchive(const std::string& baseName, int processId, const std::string& timestamp) {
            try {
                std::string archiveName = baseName + "_" + std::to_string(processId) + "_" + timestamp + ".zip";
                std::ofstream out(archiveName, std::ios::binary);
                if (!out) return false;

                // Gather candidate files: baseName.log and baseName.*.log
                std::vector<fs::path> files;
                fs::path dir = fs::current_path();
                for (auto& p : fs::directory_iterator(dir)) {
                    auto name = p.path().filename().string();
                    if (name.rfind(baseName, 0) == 0 && name.find(".log") != std::string::npos) {
                        files.push_back(p.path());
                    }
                }

                // Write a simple header and then each file content
                for (auto& f : files) {
                    std::ifstream in(f, std::ios::binary);
                    if (!in) continue;
                    out << "---FILE:" << f.filename().string() << "\n";
                    out << in.rdbuf();
                    out << "\n";
                }
                out.close();

                // remove original files
                for (auto& f : files) {
                    try { fs::remove(f); }
                    catch (...) {}
                }

                return true;
            }
            catch (...) {
                return false;
            }
        }

    }
} // namespace speckit::log