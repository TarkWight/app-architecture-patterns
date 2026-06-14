#include "../../src/Infrastructure/ConfigTemplates/ConfigTemplateService.hpp"

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <string>

namespace {

class FileLocationProviderStub final : public infrastructure::configTemplates::IAppFileLocationProvider {
  public:
    explicit FileLocationProviderStub(std::filesystem::path dir) : dir(std::move(dir)) {
    }

    [[nodiscard]] std::filesystem::path applicationDir() const override {
        return dir;
    }

  private:
    std::filesystem::path dir;
};

std::filesystem::path testDir() {
    return std::filesystem::temp_directory_path() / "counterdemo-config-template-service-test";
}

} // namespace

TEST(ConfigTemplateServiceTest, BuildsTemplatePathsRelativeToApplicationDirectory) {
    const auto dir = testDir();
    FileLocationProviderStub provider{dir};
    infrastructure::configTemplates::ConfigTemplateService service{provider};

    EXPECT_EQ(service.pathFor(infrastructure::configTemplates::ConfigTemplateType::Telemetry), dir / "telemetry.toml");
    EXPECT_EQ(service.pathFor(infrastructure::configTemplates::ConfigTemplateType::PdfReport), dir / "pdf_report.toml");
}

TEST(ConfigTemplateServiceTest, ExistsCheckDoesNotCreateTemplates) {
    const auto dir = testDir();
    std::filesystem::remove_all(dir);

    FileLocationProviderStub provider{dir};
    infrastructure::configTemplates::ConfigTemplateService service{provider};

    EXPECT_FALSE(service.exists(infrastructure::configTemplates::ConfigTemplateType::Telemetry));
    EXPECT_FALSE(std::filesystem::exists(dir));
}

TEST(ConfigTemplateServiceTest, CreatesTelemetryTemplateAtOperatorSelectedPathWithoutOverwritingExistingFile) {
    const auto dir = testDir();
    std::filesystem::remove_all(dir);
    std::filesystem::create_directories(dir);

    const auto existingTelemetry = dir / "existing-telemetry.toml";
    {
        std::ofstream stream{existingTelemetry};
        stream << "custom = true\n";
    }

    FileLocationProviderStub provider{dir};
    infrastructure::configTemplates::ConfigTemplateService service{provider};

    const auto selectedPath = dir / "operator-selected" / "telemetry.toml";
    service.createTemplate(infrastructure::configTemplates::ConfigTemplateType::Telemetry, selectedPath);
    service.createTemplate(infrastructure::configTemplates::ConfigTemplateType::Telemetry, existingTelemetry);

    EXPECT_TRUE(std::filesystem::exists(selectedPath));

    std::ifstream stream{existingTelemetry};
    std::string content;
    std::getline(stream, content);
    EXPECT_EQ(content, "custom = true");

    std::filesystem::remove_all(dir);
}
