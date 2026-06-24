// Copyright (C) 2026 Intel Corporation
// SPDX-License-Identifier: MIT
#include "GamingQoS.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace pmon::util::metrics
{
    namespace
    {
        constexpr double kAePercentWorst = 50.;
        constexpr double kPillarWeight = 0.25;

        double Clamp01(double value)
        {
            return std::max(0., std::min(1., value));
        }

        std::optional<double> FpsRatioSubscore(std::optional<double> lowFps, std::optional<double> avgFps)
        {
            if (!lowFps.has_value() || !avgFps.has_value() || *avgFps <= 0.) {
                return std::nullopt;
            }
            return Clamp01(*lowFps / *avgFps);
        }

        void AccumulatePillar_(double weight, std::optional<double> subscore, double& weightedSum, double& totalWeight)
        {
            if (!subscore.has_value()) {
                return;
            }
            weightedSum += weight * *subscore;
            totalWeight += weight;
        }
    }

    double AnimationErrorPercentOfFrame(double aeMs, double frameTimeMs)
    {
        if (frameTimeMs <= 0.) {
            return 0.;
        }
        return 100. * std::abs(aeMs) / frameTimeMs;
    }

    std::optional<double> AnimationErrorPercentSubscore(std::optional<double> aePercentAvg)
    {
        if (!aePercentAvg.has_value()) {
            return std::nullopt;
        }
        if (kAePercentWorst <= 0.) {
            return std::nullopt;
        }
        return Clamp01((kAePercentWorst - *aePercentAvg) / kAePercentWorst);
    }

    GamingQoSResult ComputeGamingQoS(const GamingQoSInputs& inputs)
    {
        GamingQoSResult result{};
        result.low1Subscore = FpsRatioSubscore(inputs.low1Fps, inputs.avgFps);
        result.low5Subscore = FpsRatioSubscore(inputs.low5Fps, inputs.avgFps);
        result.animationErrorSubscore = AnimationErrorPercentSubscore(inputs.animationErrorPercentAvg);

        double weightedSum = 0.;
        double totalWeight = 0.;
        AccumulatePillar_(kPillarWeight, result.low1Subscore, weightedSum, totalWeight);
        AccumulatePillar_(kPillarWeight, result.low5Subscore, weightedSum, totalWeight);
        AccumulatePillar_(kPillarWeight, result.animationErrorSubscore, weightedSum, totalWeight);

        if (totalWeight <= 0.) {
            result.score = std::numeric_limits<double>::quiet_NaN();
            result.scoreValid = false;
            return result;
        }

        result.score = 100. * (weightedSum / totalWeight);
        result.scoreValid = true;
        return result;
    }

    std::string GamingQoSGradeFromScore(double score)
    {
        if (!std::isfinite(score)) {
            return "NA";
        }
        if (score >= 96.) {
            return "S";
        }
        if (score >= 90.) {
            return "A";
        }
        if (score >= 80.) {
            return "B";
        }
        if (score >= 70.) {
            return "C";
        }
        if (score >= 60.) {
            return "D";
        }
        return "F";
    }

    std::wstring GamingQoSGradeFromScoreW(double score)
    {
        const auto grade = GamingQoSGradeFromScore(score);
        return std::wstring(grade.begin(), grade.end());
    }
}
