#include "../../src/Application/Services/UavSpecificationMapper.hpp"

#include <gtest/gtest.h>

namespace {

TEST(UavSpecificationMapperTest, NormalizesDroneParametersIntoUavSpecification) {
    domain::TestProtocol protocol{};
    protocol.droneParameters = {
        {"uav_model", "Модель БПЛА", "BAS-1"},
        {"serial_number", "Серийный номер", "SN-42"},
        {"flight_payload", "Полетная нагрузка", "15.00"},
        {"drag_ratio_percent", "DR (%)", "89,5"},
        {"battery_weight", "Вес аккумулятора", "2,4"},
        {"battery_capacity_mah", "Номинальная емкость (mAh)", "18000"},
        {"motor_count", "Количество двигателей", "4"},
        {"motor_peak_current_a", "Пиковый ток (A)", "13,990"},
        {"firmware_version", "Версия прошивки", "1.1.2.32"},
    };

    const auto specification = application::services::UavSpecificationMapper{}.map(protocol);

    ASSERT_TRUE(specification.has_value());
    EXPECT_EQ(specification->frame.model, "BAS-1");
    EXPECT_EQ(specification->frame.serialNumber, "SN-42");
    EXPECT_DOUBLE_EQ(specification->frame.flightPayload, 15.0);
    EXPECT_DOUBLE_EQ(specification->frame.dragRatioPercent, 89.5);
    EXPECT_DOUBLE_EQ(specification->battery.weight, 2.4);
    EXPECT_EQ(specification->battery.capacityMah, 18000);
    EXPECT_EQ(specification->motor.count, 4);
    EXPECT_DOUBLE_EQ(specification->motor.peakCurrentA, 13.990);
    EXPECT_EQ(specification->firmwareVersion, "1.1.2.32");
}

TEST(UavSpecificationMapperTest, ReturnsEmptySpecificationWhenDroneParametersAreEmptyDefaults) {
    domain::TestProtocol protocol{};
    protocol.droneParameters = {
        {"uav_model", "Модель БПЛА", ""},
        {"flight_payload", "Полетная нагрузка", "0"},
        {"battery_weight", "Вес аккумулятора", "0,0"},
    };

    const auto specification = application::services::UavSpecificationMapper{}.map(protocol);

    EXPECT_FALSE(specification.has_value());
}

} // namespace
