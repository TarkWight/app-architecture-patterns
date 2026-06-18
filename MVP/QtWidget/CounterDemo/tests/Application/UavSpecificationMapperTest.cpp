#include "../../src/Application/Services/UavSpecificationMapper.hpp"

#include <gtest/gtest.h>

namespace {

TEST(UavSpecificationMapperTest, NormalizesDroneParametersIntoUavSpecification) {
    domain::TestProtocol protocol{};
    protocol.droneParameters = {
        {"uav_model", "Модель БПЛА", "BAS-1"},
        {"serial_number", "Серийный номер", "SN-42"},
        {"uav_empty_weight_kg", "Масса без нагрузки", "3,1"},
        {"uav_total_weight_kg", "Полная масса", "6,2"},
        {"flight_payload", "Полетная нагрузка", "15.00"},
        {"drag_ratio_percent", "DR (%)", "89,5"},
        {"frontal_area_m2", "Фронтальная площадь", "0,35"},
        {"drag_coefficient", "Коэффициент сопротивления", "1,1"},
        {"battery_weight", "Вес аккумулятора", "2,4"},
        {"battery_capacity_mah", "Номинальная емкость (mAh)", "18000"},
        {"battery_usable_capacity_percent", "Используемая емкость (%)", "80"},
        {"motor_count", "Количество двигателей", "4"},
        {"motor_peak_current_a", "Пиковый ток (A)", "13,990"},
        {"motor_hover_current_a", "Ток висения (A)", "7,5"},
        {"motor_nominal_current_a", "Номинальный ток (A)", "8,2"},
        {"motor_power_coefficient", "Коэффициент мощности", "1,3"},
        {"thrust_power_coefficient", "Коэффициент тяга-мощность", "1,4"},
        {"firmware_version", "Версия прошивки", "1.1.2.32"},
    };

    const auto specification = application::services::UavSpecificationMapper{}.map(protocol);

    ASSERT_TRUE(specification.has_value());
    EXPECT_EQ(specification->frame.model, "BAS-1");
    EXPECT_EQ(specification->frame.serialNumber, "SN-42");
    EXPECT_DOUBLE_EQ(specification->frame.emptyWeightKg, 3.1);
    EXPECT_DOUBLE_EQ(specification->frame.totalWeightKg, 6.2);
    EXPECT_DOUBLE_EQ(specification->frame.flightPayload, 15.0);
    EXPECT_DOUBLE_EQ(specification->frame.dragRatioPercent, 89.5);
    EXPECT_DOUBLE_EQ(specification->frame.frontalAreaM2, 0.35);
    EXPECT_DOUBLE_EQ(specification->frame.dragCoefficient, 1.1);
    EXPECT_DOUBLE_EQ(specification->battery.weight, 2.4);
    EXPECT_EQ(specification->battery.capacityMah, 18000);
    EXPECT_DOUBLE_EQ(specification->battery.usableCapacityPercent, 80.0);
    EXPECT_EQ(specification->motor.count, 4);
    EXPECT_DOUBLE_EQ(specification->motor.peakCurrentA, 13.990);
    EXPECT_DOUBLE_EQ(specification->motor.hoverCurrentA, 7.5);
    EXPECT_DOUBLE_EQ(specification->motor.nominalCurrentA, 8.2);
    EXPECT_DOUBLE_EQ(specification->motor.powerCoefficient, 1.3);
    EXPECT_DOUBLE_EQ(specification->motor.thrustPowerCoefficient, 1.4);
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
