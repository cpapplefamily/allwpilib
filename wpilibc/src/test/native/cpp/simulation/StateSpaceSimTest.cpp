/*----------------------------------------------------------------------------*/
/* Copyright (c) 2020 FIRST. All Rights Reserved.                             */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in the root directory of */
/* the project.                                                               */
/*----------------------------------------------------------------------------*/

#include <iostream>

#include <units/angular_acceleration.h>
#include <units/angular_velocity.h>

#include "frc/Encoder.h"
#include "frc/PWMVictorSPX.h"
#include "frc/RobotController.h"
#include "frc/controller/PIDController.h"
#include "frc/controller/SimpleMotorFeedforward.h"
#include "frc/simulation/BatterySim.h"
#include "frc/simulation/DifferentialDrivetrainSim.h"
#include "frc/simulation/ElevatorSim.h"
#include "frc/simulation/EncoderSim.h"
#include "frc/simulation/FlywheelSim.h"
#include "frc/simulation/LinearSystemSim.h"
#include "frc/simulation/PWMSim.h"
#include "frc/simulation/RoboRioSim.h"
#include "frc/simulation/SingleJointedArmSim.h"
#include "frc/system/plant/LinearSystemId.h"
#include "gtest/gtest.h"

TEST(StateSpaceSimTest, TestFlywheelSim) {
  const frc::LinearSystem<1, 1, 1> plant =
      frc::LinearSystemId::IdentifyVelocitySystem(0.02, 0.01);
  frc::sim::FlywheelSim sim{plant, frc::DCMotor::NEO(2), 1.0};
  frc2::PIDController controller{0.2, 0.0, 0.0};
  frc::SimpleMotorFeedforward<units::radian> feedforward{
      0_V, 0.02_V / 1_rad_per_s, 0.01_V / 1_rad_per_s_sq};
  frc::Encoder encoder{0, 1};
  frc::sim::EncoderSim encoderSim{encoder};
  frc::PWMVictorSPX motor{0};

  for (int i = 0; i < 100; i++) {
    // RobotPeriodic runs first
    auto voltageOut = controller.Calculate(encoder.GetRate(), 200.0);
    motor.SetVoltage(units::volt_t(voltageOut) +
                     feedforward.Calculate(200_rad_per_s));

    // Then, SimulationPeriodic runs
    frc::sim::RoboRioSim::SetVInVoltage(
        frc::sim::BatterySim::Calculate({sim.GetCurrentDraw()}).to<double>());
    sim.SetInput(frc::MakeMatrix<1, 1>(
        motor.Get() * frc::RobotController::GetInputVoltage()));
    sim.Update(20_ms);
    encoderSim.SetRate(sim.GetAngularVelocity().to<double>());
  }

  ASSERT_TRUE(std::abs(200 - encoder.GetRate()) < 0.1);
}
