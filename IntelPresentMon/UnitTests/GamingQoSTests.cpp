// Copyright (C) 2026 Intel Corporation
// SPDX-License-Identifier: MIT

#include <CppUnitTest.h>

#include <CommonUtilities/mc/GamingQoS.h>

#include <cmath>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace pmon::util::metrics;

namespace UtilityTests
{
    TEST_CLASS(TestGamingQoS)
    {
    public:
        TEST_METHOD(PerfectInputs_ScoreNear100)
        {
            GamingQoSInputs inputs{
                .avgFps = 120.,
                .low1Fps = 120.,
                .low5Fps = 120.,
                .pcLatencyMs = 20.,
                .aeP95Ms = 0.5,
            };
            const auto result = ComputeGamingQoS(inputs);
            Assert::IsTrue(result.scoreValid);
            Assert::IsTrue(result.score >= 99.9);
            Assert::AreEqual(std::string("S"), GamingQoSGradeFromScore(result.score));
        }

        TEST_METHOD(LatencyAtBad_ZeroLatencySubscore)
        {
            GamingQoSInputs inputs{
                .avgFps = 60.,
                .low1Fps = 60.,
                .low5Fps = 60.,
                .pcLatencyMs = 60.,
                .aeP95Ms = 0.5,
            };
            const auto result = ComputeGamingQoS(inputs);
            Assert::IsTrue(result.latencySubscore.has_value());
            Assert::AreEqual(0., *result.latencySubscore, 0.0001);
        }

        TEST_METHOD(MissingPcLatency_ReweightsRemainingPillars)
        {
            GamingQoSInputs inputs{
                .avgFps = 100.,
                .low1Fps = 100.,
                .low5Fps = 100.,
                .pcLatencyMs = std::nullopt,
                .aeP95Ms = 0.5,
            };
            const auto result = ComputeGamingQoS(inputs);
            Assert::IsTrue(result.scoreValid);
            Assert::IsTrue(result.score >= 99.);
        }

        TEST_METHOD(NoValidInputs_ScoreInvalid)
        {
            const auto result = ComputeGamingQoS({});
            Assert::IsFalse(result.scoreValid);
            Assert::IsTrue(std::isnan(result.score));
            Assert::AreEqual(std::string("NA"), GamingQoSGradeFromScore(result.score));
        }

        TEST_METHOD(FpsRatio_ClampedToOne)
        {
            GamingQoSInputs inputs{
                .avgFps = 60.,
                .low1Fps = 90.,
                .low5Fps = 60.,
                .pcLatencyMs = 20.,
                .aeP95Ms = 0.5,
            };
            const auto result = ComputeGamingQoS(inputs);
            Assert::IsTrue(result.low1Subscore.has_value());
            Assert::AreEqual(1., *result.low1Subscore, 0.0001);
        }

        TEST_METHOD(GradeBoundaries)
        {
            Assert::AreEqual(std::string("S"), GamingQoSGradeFromScore(99.));
            Assert::AreEqual(std::string("S"), GamingQoSGradeFromScore(96.));
            Assert::AreEqual(std::string("A"), GamingQoSGradeFromScore(90.));
            Assert::AreEqual(std::string("B"), GamingQoSGradeFromScore(80.));
            Assert::AreEqual(std::string("C"), GamingQoSGradeFromScore(70.));
            Assert::AreEqual(std::string("D"), GamingQoSGradeFromScore(60.));
            Assert::AreEqual(std::string("F"), GamingQoSGradeFromScore(59.9));
        }
    };
}
