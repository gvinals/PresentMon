// Copyright (C) 2026 Intel Corporation
// SPDX-License-Identifier: MIT
#include "../CommonUtilities/win/WinAPI.h"
#include "CppUnitTest.h"
#include "FirstFrameWait.h"
#include "Folders.h"
#include "TestProcess.h"
#include "../PresentMonAPIWrapper/PresentMonAPIWrapper.h"
#include "../Interprocess/source/SystemDeviceId.h"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstring>
#include <format>
#include <memory>
#include <thread>
#include <vector>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace GamingQoSQueryTests
{
    using namespace std::chrono_literals;

    size_t MaxQueryElementEnd_(const std::vector<PM_QUERY_ELEMENT>& elements)
    {
        size_t end = 0;
        for (const auto& element : elements) {
            end = std::max(end, (size_t)element.dataOffset + (size_t)element.dataSize);
        }
        return end;
    }

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
                PM_QUERY_ELEMENT{
                    .metric = PM_METRIC_GAMING_QOS_GRADE,
                    .stat = PM_STAT_NONE,
                    .deviceId = pmon::ipc::kUniversalDeviceId,
                    .arrayIndex = 0,
                    .dataOffset = 0,
                    .dataSize = 0,
                },
            };

            auto query = pSession->RegisterDynamicQuery(elements, 1000.0, 0.0);
            Assert::IsTrue(query.GetBlobSize() >= MaxQueryElementEnd_(elements),
                L"Registered blob size must cover all query element fields");
            auto blobs = query.MakeBlobContainer(1);

            bool gotFiniteScore = false;
            const auto deadline = std::chrono::steady_clock::now() + 5s;
            while (std::chrono::steady_clock::now() < deadline) {
                query.Poll(blobs);
                if (blobs.GetNumBlobsPopulated() > 0) {
                    const double score = *reinterpret_cast<const double*>(blobs[0] + (size_t)elements[0].dataOffset);
                    if (std::isfinite(score)) {
                        Assert::IsTrue(score >= 0. && score <= 100., L"Gaming QoS score out of range");
                        const char* grade = reinterpret_cast<const char*>(blobs[0] + (size_t)elements[1].dataOffset);
                        Assert::IsTrue(std::strlen(grade) > 0, L"Gaming QoS grade string empty");
                        gotFiniteScore = true;
                        break;
                    }
                }
                std::this_thread::sleep_for(50ms);
            }

            Assert::IsTrue(gotFiniteScore, L"Expected finite Gaming QoS score while presenter is running");
            tracker.Reset();
        }

        TEST_METHOD(MixedFrameAndQoSQueryBlobSizeCoversGradeString)
        {
            auto presenter = fixture_.LaunchPresenter({ "/FrameSleep=10"s });
            auto pSession = std::make_unique<pmapi::Session>(fixture_.GetCommonArgs().ctrlPipe);
            pSession->SetEtwFlushPeriod(8);
            auto tracker = pSession->TrackProcess(presenter.GetId());
            pmon::tests::WaitForFirstFrame(
                fixture_.GetCommonArgs().ctrlPipe,
                presenter.GetId(),
                "gaming-qos-mixed");

            std::vector<PM_QUERY_ELEMENT> elements{
                PM_QUERY_ELEMENT{
                    .metric = PM_METRIC_GAMING_QOS_GRADE,
                    .stat = PM_STAT_NONE,
                    .deviceId = pmon::ipc::kUniversalDeviceId,
                    .arrayIndex = 0,
                    .dataOffset = 0,
                    .dataSize = 0,
                },
                PM_QUERY_ELEMENT{
                    .metric = PM_METRIC_GAMING_QOS_SCORE,
                    .stat = PM_STAT_AVG,
                    .deviceId = pmon::ipc::kUniversalDeviceId,
                    .arrayIndex = 0,
                    .dataOffset = 0,
                    .dataSize = 0,
                },
                PM_QUERY_ELEMENT{
                    .metric = PM_METRIC_PRESENTED_FPS,
                    .stat = PM_STAT_AVG,
                    .deviceId = pmon::ipc::kUniversalDeviceId,
                    .arrayIndex = 0,
                    .dataOffset = 0,
                    .dataSize = 0,
                },
            };

            auto query = pSession->RegisterDynamicQuery(elements, 1000.0, 0.0);
            const auto registeredBlobSize = query.GetBlobSize();
            const auto naiveClientSize = (size_t)elements.back().dataOffset + (size_t)elements.back().dataSize;
            Assert::IsTrue(registeredBlobSize >= MaxQueryElementEnd_(elements),
                L"Blob size must cover all fields including grade string");
            Assert::IsTrue(registeredBlobSize > naiveClientSize,
                L"Authoritative blob size must exceed last-element extent heuristic");

            auto blobs = query.MakeBlobContainer(1);
            bool gotFps = false;
            const auto deadline = std::chrono::steady_clock::now() + 5s;
            while (std::chrono::steady_clock::now() < deadline) {
                query.Poll(blobs);
                if (blobs.GetNumBlobsPopulated() > 0) {
                    const double fps = *reinterpret_cast<const double*>(blobs[0] + (size_t)elements[2].dataOffset);
                    if (std::isfinite(fps) && fps > 0.) {
                        gotFps = true;
                        break;
                    }
                }
                std::this_thread::sleep_for(50ms);
            }

            Assert::IsTrue(gotFps, L"Presented FPS should remain valid in mixed QoS query");
            tracker.Reset();
        }
    };
}
