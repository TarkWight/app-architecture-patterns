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

TEST(ConfigTemplateServiceTest, ResolvesDefaultTelemetryPathWhenOperatorPathIsEmpty) {
    const auto dir = testDir();
    std::filesystem::remove_all(dir);
    std::filesystem::create_directories(dir);

    const auto defaultTelemetry = dir / "telemetry.toml";
    {
        std::ofstream stream{defaultTelemetry};
        stream << "custom = true\n";
    }

    FileLocationProviderStub provider{dir};
    infrastructure::configTemplates::ConfigTemplateService service{provider};

    const auto resolution =
        service.resolvePath(infrastructure::configTemplates::ConfigTemplateType::Telemetry, std::filesystem::path{});

    EXPECT_TRUE(resolution.exists());
    EXPECT_EQ(resolution.path, defaultTelemetry);
    EXPECT_EQ(resolution.status, infrastructure::configTemplates::ConfigTemplateResolutionStatus::Found);

    std::filesystem::remove_all(dir);
}

TEST(ConfigTemplateServiceTest, ResolvesOperatorSelectedTelemetryPathWhenItExists) {
    const auto dir = testDir();
    std::filesystem::remove_all(dir);
    std::filesystem::create_directories(dir);

    const auto selectedTelemetry = dir / "selected" / "telemetry.toml";
    std::filesystem::create_directories(selectedTelemetry.parent_path());
    {
        std::ofstream stream{selectedTelemetry};
        stream << "custom = true\n";
    }

    FileLocationProviderStub provider{dir};
    infrastructure::configTemplates::ConfigTemplateService service{provider};

    const auto resolution =
        service.resolvePath(infrastructure::configTemplates::ConfigTemplateType::Telemetry, selectedTelemetry);

    EXPECT_TRUE(resolution.exists());
    EXPECT_EQ(resolution.path, selectedTelemetry);
    EXPECT_EQ(resolution.status, infrastructure::configTemplates::ConfigTemplateResolutionStatus::Found);

    std::filesystem::remove_all(dir);
}

TEST(ConfigTemplateServiceTest, DoesNotFallbackToDefaultWhenOperatorSelectedPathIsMissing) {
    const auto dir = testDir();
    std::filesystem::remove_all(dir);
    std::filesystem::create_directories(dir);

    const auto defaultTelemetry = dir / "telemetry.toml";
    {
        std::ofstream stream{defaultTelemetry};
        stream << "custom = true\n";
    }

    const auto missingSelectedTelemetry = dir / "missing" / "telemetry.toml";

    FileLocationProviderStub provider{dir};
    infrastructure::configTemplates::ConfigTemplateService service{provider};

    const auto resolution =
        service.resolvePath(infrastructure::configTemplates::ConfigTemplateType::Telemetry, missingSelectedTelemetry);

    EXPECT_FALSE(resolution.exists());
    EXPECT_EQ(resolution.path, missingSelectedTelemetry);
    EXPECT_EQ(resolution.status,
              infrastructure::configTemplates::ConfigTemplateResolutionStatus::MissingOperatorSelected);

    std::filesystem::remove_all(dir);
}
