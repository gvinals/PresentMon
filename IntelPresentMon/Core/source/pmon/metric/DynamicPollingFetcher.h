// Copyright (C) 2022 Intel Corporation
// SPDX-License-Identifier: MIT
#pragma once
#include <PresentMonAPI2/PresentMonAPI.h>
#include <PresentMonAPIWrapperCommon/EnumMap.h>
#include <Core/source/kernel/OverlaySpec.h>
#include "MetricFetcher.h"
#include "../DynamicQuery.h"
#include <CommonUtilities//str/String.h>
#include <algorithm>
#include <cmath>
#include <concepts>
#include <limits>
#include <string>


namespace p2c::pmon::met
{
    class DynamicPollingFetcher : public MetricFetcher
    {
    protected:
        // functions
        DynamicPollingFetcher(const PM_QUERY_ELEMENT& qel, const pmapi::intro::Root& introRoot,
            std::shared_ptr<DynamicQuery> pQuery);
        size_t ResolveOffset_() const
        {
            if (auto offset = pQuery_->FindElementDataOffset(metric_, stat_, deviceId_, arrayIndex_)) {
                return *offset;
            }
            if (offset_ != std::numeric_limits<uint32_t>::max()) {
                return offset_;
            }
            return 0;
        }

        bool HasResolvedBlobField_() const
        {
            return pQuery_->FindElementDataOffset(metric_, stat_, deviceId_, arrayIndex_).has_value() ||
                offset_ != std::numeric_limits<uint32_t>::max();
        }

        size_t ResolveStringMaxLen_() const
        {
            if (auto size = pQuery_->FindElementDataSize(metric_, stat_, deviceId_, arrayIndex_)) {
                return (size_t)std::min(*size, (size_t)PM_MAX_PATH);
            }
            if (dataSize_ > 0) {
                return (size_t)std::min(dataSize_, (uint64_t)PM_MAX_PATH);
            }
            return PM_MAX_PATH;
        }
        // data
        std::shared_ptr<DynamicQuery> pQuery_;
        PM_METRIC metric_;
        PM_STAT stat_;
        uint32_t deviceId_ = 0;
        uint32_t arrayIndex_ = 0;
        uint32_t offset_ = std::numeric_limits<uint32_t>::max();
        uint64_t dataSize_ = 0;
        float scale_ = 1.f;
    };

    template<typename T>
    class TypedDynamicPollingFetcher : public DynamicPollingFetcher
    {
    public:
        TypedDynamicPollingFetcher(const PM_QUERY_ELEMENT& qel, const pmapi::intro::Root& introRoot,
            std::shared_ptr<DynamicQuery> pQuery)
            :
            DynamicPollingFetcher{ qel, introRoot, std::move(pQuery) }
        {}
        std::optional<float> ReadValue() override
        {
            if constexpr (std::integral<T> || std::floating_point<T>) {
                if (auto pBlobBytes = pQuery_->GetBlobData()) {
                    const auto offset = ResolveOffset_();
                    const float scaled = scale_ * (float)*reinterpret_cast<const T*>(&pBlobBytes[offset]);
                    if (!std::isfinite(scaled)) {
                        return {};
                    }
                    return scaled;
                }
                return {};
            }
            if constexpr (std::same_as<T, const char*>) {
                pmlog_warn("Reading float value from string-typed metric");
            }
            else {
                pmlog_warn("Unknown type");
            }
            return {};
        }
        std::wstring ReadStringValue() override
        {
            if constexpr (std::integral<T> || std::floating_point<T>) {
                return MetricFetcher::ReadStringValue();
            }
            else if constexpr (std::same_as<T, const char*>) {
                if (!HasResolvedBlobField_()) {
                    return {};
                }
                if (auto pBlobBytes = pQuery_->GetBlobData()) {
                    const auto offset = ResolveOffset_();
                    const auto* pStr = reinterpret_cast<const char*>(&pBlobBytes[offset]);
                    const size_t maxLen = ResolveStringMaxLen_();
                    size_t len = 0;
                    while (len < maxLen && pStr[len] != '\0') {
                        ++len;
                    }
                    return ::pmon::util::str::ToWide(std::string(pStr, len));
                }
            }
            else {
                pmlog_warn("Unknown type");
            }
            return {};
        }
    };

    template<>
    class TypedDynamicPollingFetcher<PM_ENUM> : public DynamicPollingFetcher
    {
    public:
        TypedDynamicPollingFetcher(const PM_QUERY_ELEMENT& qel, const pmapi::intro::Root& introRoot,
            std::shared_ptr<DynamicQuery> pQuery, std::shared_ptr<const pmapi::EnumMap::KeyMap> pKeyMap);
        std::wstring ReadStringValue() override;
        std::optional<float> ReadValue() override;
    private:
        std::shared_ptr<const pmapi::EnumMap::KeyMap> pKeyMap_;
    };

    std::shared_ptr<DynamicPollingFetcher> MakeDynamicPollingFetcher(const PM_QUERY_ELEMENT& qel,
        const pmapi::intro::Root& introRoot, std::shared_ptr<DynamicQuery> pQuery);
}