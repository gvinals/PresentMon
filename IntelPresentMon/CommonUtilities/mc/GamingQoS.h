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
        std::optional<double> pcLatencyMs;
        std::optional<double> aeP95Ms;
    };

    struct GamingQoSResult
    {
        double score = 0.;
        bool scoreValid = false;
        std::optional<double> low1Subscore;
        std::optional<double> low5Subscore;
        std::optional<double> latencySubscore;
        std::optional<double> animationErrorSubscore;
    };

    GamingQoSResult ComputeGamingQoS(const GamingQoSInputs& inputs);

    std::string GamingQoSGradeFromScore(double score, bool enableSPlus = true);
    std::wstring GamingQoSGradeFromScoreW(double score, bool enableSPlus = true);
}
