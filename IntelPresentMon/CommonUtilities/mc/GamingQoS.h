// Copyright (C) 2026 Intel Corporation
// SPDX-License-Identifier: MIT
#pragma once

#include <optional>
#include <string>

namespace pmon::util::metrics
{
    struct GamingQoSInputs
    {
        std::optional<double> avgFps;
        std::optional<double> low1Fps;
        std::optional<double> low5Fps;
        std::optional<double> animationErrorPercentAvg;
    };

    struct GamingQoSResult
    {
        double score = 0.;
        bool scoreValid = false;
        std::optional<double> low1Subscore;
        std::optional<double> low5Subscore;
        std::optional<double> animationErrorSubscore;
    };

    double AnimationErrorPercentOfFrame(double aeMs, double frameTimeMs);
    std::optional<double> AnimationErrorPercentSubscore(std::optional<double> aePercentAvg);

    GamingQoSResult ComputeGamingQoS(const GamingQoSInputs& inputs);

    std::string GamingQoSGradeFromScore(double score);
    std::wstring GamingQoSGradeFromScoreW(double score);
}
