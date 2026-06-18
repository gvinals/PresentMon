#pragma once
#include <PresentMonAPI2/PresentMonAPI.h>
#include <PresentMonAPIWrapper/BlobContainer.h>
#include <PresentMonAPIWrapper/DynamicQuery.h>
#include <span>
#include <memory>
#include <vector>
#include <optional>
#include <cstddef>
#include "../kernel/OverlaySpec.h"

namespace pmapi
{
	class Session;
	class ProcessTracker;
}

namespace p2c::pmon
{
	class DynamicQuery
	{
	public:
		DynamicQuery(pmapi::Session& session, double winSizeMs, double metricOffsetMs, std::span<const kern::QualifiedMetric> qmet);
		void Poll(const pmapi::ProcessTracker& tracker);
		void PollWithTimestamp(const pmapi::ProcessTracker& tracker, uint64_t nowTimestamp);
		const uint8_t* GetBlobData() const;
		std::vector<PM_QUERY_ELEMENT> ExtractElements() const;
		std::optional<size_t> FindElementDataOffset(PM_METRIC metric, PM_STAT stat, uint32_t deviceId, uint32_t arrayIndex) const;
		std::optional<size_t> FindElementDataSize(PM_METRIC metric, PM_STAT stat, uint32_t deviceId, uint32_t arrayIndex) const;
	private:
		pmapi::DynamicQuery query;
		std::vector<PM_QUERY_ELEMENT> elements;
		pmapi::BlobContainer blobs;
	};
}
