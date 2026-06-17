// Copyright (C) 2026 Intel Corporation
// SPDX-License-Identifier: MIT
#include "../CommonUtilities/win/WinAPI.h"
#include "CppUnitTest.h"
#include "FirstFrameWait.h"
#include "Folders.h"
#include "TestProcess.h"
#include "../PresentMonAPIWrapper/PresentMonAPIWrapper.h"
#include "../Interprocess/source/SystemDeviceId.h"
#include <chrono>
#include <cmath>
#include <format>
#include <memory>
#include <thread>
#include <vector>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace GamingQoSQueryTests
{
    using namespace std::chrono_literals;

    class TestFixture : public CommonTestFixture
    {
    public:
        const CommonProcessArgs& GetCommonArgs() const override
        {
            static CommonProcessArgs args{
                .ctrlPipe = R"(\\.\pipe\pm-gamingqos-test-ctrl)",
                .shmNamePrefix = "pm_gamingqos_test",
                .logLevel = "verbose",
                .logVerboseModules = "middleware",
                .logFolder = logFolder_,
                .sampleClientMode = "NONE",
            };
            return args;
        }
    };

    TEST_CLASS(GamingQoSQueryTests)
    {
        TestFixture fixture_;

    public:
        TEST_METHOD_INITIALIZE(Setup)
        {
            fixture_.Setup({ "--etw-session-name"s, "GamingQoSSession"s });
        }

        TEST_METHOD_CLEANUP(Cleanup)
        {
            fixture_.Cleanup();
        }

        TEST_METHOD(GamingQoSDynamicQueryPollsFiniteScore)
        {
            auto presenter = fixture_.LaunchPresenter({ "/FrameSleep=10"s });
            auto pSession = std::make_unique<pmapi::Session>(fixture_.GetCommonArgs().ctrlPipe);
            pSession->SetEtwFlushPeriod(8);
            auto tracker = pSession->TrackProcess(presenter.GetId());
            pmon::tests::WaitForFirstFrame(
                fixture_.GetCommonArgs().ctrlPipe,
                presenter.GetId(),
                "gaming-qos");

            std::vector<PM_QUERY_ELEMENT> elements{
                PM_QUERY_ELEMENT{
                    .metric = PM_METRIC_GAMING_QOS_SCORE,
                    .stat = PM_STAT_AVG,
                    .deviceId = pmon::ipc::kUniversalDeviceId,
                    .arrayIndex = 0,
                    .dataOffset = 0,
                    .dataSize = 0,
                },
            };

            auto query = pSession->RegisterDynamicQuery(elements, 1000.0, 0.0);
            auto blobs = query.MakeBlobContainer(1);

            bool gotFiniteScore = false;
            const auto deadline = std::chrono::steady_clock::now() + 5s;
            while (std::chrono::steady_clock::now() < deadline) {
                query.Poll(blobs);
                if (blobs.GetNumBlobsPopulated() > 0) {
                    const double score = *reinterpret_cast<const double*>(blobs[0] + (size_t)elements[0].dataOffset);
                    if (std::isfinite(score)) {
                        Assert::IsTrue(score >= 0. && score <= 100., L"Gaming QoS score out of range");
                        gotFiniteScore = true;
                        break;
                    }
                }
                std::this_thread::sleep_for(50ms);
            }

            Assert::IsTrue(gotFiniteScore, L"Expected finite Gaming QoS score while presenter is running");
            tracker.Reset();
        }
    };
}
