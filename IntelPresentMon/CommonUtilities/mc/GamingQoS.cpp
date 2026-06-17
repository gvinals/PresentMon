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
        constexpr double kLatencyBadMs = 60.;
        constexpr double kLatencyGoodMs = 20.;
        constexpr double kAeBadMs = 2.;
        constexpr double kAeGoodMs = 0.5;
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

        std::optional<double> LatencySubscore(std::optional<double> pcLatencyMs)
        {
            if (!pcLatencyMs.has_value()) {
                return std::nullopt;
            }
            const double span = kLatencyBadMs - kLatencyGoodMs;
            if (span <= 0.) {
                return std::nullopt;
            }
            return Clamp01((kLatencyBadMs - *pcLatencyMs) / span);
        }

        std::optional<double> AnimationErrorSubscore(std::optional<double> aeP95Ms)
        {
            if (!aeP95Ms.has_value()) {
                return std::nullopt;
            }
            const double span = kAeBadMs - kAeGoodMs;
            if (span <= 0.) {
                return std::nullopt;
            }
            return Clamp01((kAeBadMs - *aeP95Ms) / span);
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

    GamingQoSResult ComputeGamingQoS(const GamingQoSInputs& inputs)
    {
        GamingQoSResult result{};
        result.low1Subscore = FpsRatioSubscore(inputs.low1Fps, inputs.avgFps);
        result.low5Subscore = FpsRatioSubscore(inputs.low5Fps, inputs.avgFps);
        result.latencySubscore = LatencySubscore(inputs.pcLatencyMs);
        result.animationErrorSubscore = AnimationErrorSubscore(inputs.aeP95Ms);

        double weightedSum = 0.;
        double totalWeight = 0.;
        AccumulatePillar_(kPillarWeight, result.low1Subscore, weightedSum, totalWeight);
        AccumulatePillar_(kPillarWeight, result.low5Subscore, weightedSum, totalWeight);
        AccumulatePillar_(kPillarWeight, result.latencySubscore, weightedSum, totalWeight);
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

    std::string GamingQoSGradeFromScore(double score, bool enableSPlus)
    {
        if (!std::isfinite(score)) {
            return "NA";
        }
        if (enableSPlus && score >= 99.) {
            return "S+";
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

    std::wstring GamingQoSGradeFromScoreW(double score, bool enableSPlus)
    {
        const auto grade = GamingQoSGradeFromScore(score, enableSPlus);
        return std::wstring(grade.begin(), grade.end());
    }
}
