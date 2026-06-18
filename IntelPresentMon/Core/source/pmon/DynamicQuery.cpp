#pragma once
#include "DynamicQuery.h"
#include <PresentMonAPIWrapper/PresentMonAPIWrapper.h>
#include <ranges>


namespace p2c::pmon
{
	DynamicQuery::DynamicQuery(pmapi::Session& session, double winSizeMs, double metricOffsetMs, std::span<const kern::QualifiedMetric> qmets)
	{
		for (auto& qmet : qmets) {
			elements.push_back(PM_QUERY_ELEMENT{
				.metric = (PM_METRIC)qmet.metricId,
				.stat = (PM_STAT)qmet.statId,
				.deviceId = qmet.deviceId,
				.arrayIndex = qmet.arrayIndex,
			});
		}
		query = session.RegisterDynamicQuery(elements, winSizeMs, metricOffsetMs);
		blobs = query.MakeBlobContainer(1u);
	}

	std::vector<PM_QUERY_ELEMENT> DynamicQuery::ExtractElements() const
	{
		return elements;
	}

	std::optional<size_t> DynamicQuery::FindElementDataOffset(PM_METRIC metric, PM_STAT stat, uint32_t deviceId, uint32_t arrayIndex) const
	{
		for (const auto& element : elements) {
			if (element.metric == metric && element.stat == stat &&
				element.deviceId == deviceId && element.arrayIndex == arrayIndex) {
				return (size_t)element.dataOffset;
			}
		}
		return std::nullopt;
	}

	std::optional<size_t> DynamicQuery::FindElementDataSize(PM_METRIC metric, PM_STAT stat, uint32_t deviceId, uint32_t arrayIndex) const
	{
		for (const auto& element : elements) {
			if (element.metric == metric && element.stat == stat &&
				element.deviceId == deviceId && element.arrayIndex == arrayIndex) {
				return (size_t)element.dataSize;
			}
		}
		return std::nullopt;
	}

	void DynamicQuery::Poll(const pmapi::ProcessTracker& tracker)
	{
		if (query) {
			query.Poll(tracker, blobs);
		}
		else {
			pmlog_warn("Polling empty dynamic query");
		}
	}

	void DynamicQuery::PollWithTimestamp(const pmapi::ProcessTracker& tracker, uint64_t nowTimestamp)
	{
		if (query) {
			query.PollWithTimestamp(tracker, blobs, nowTimestamp);
		}
		else {
			pmlog_warn("Polling empty dynamic query");
		}
	}

	const uint8_t* DynamicQuery::GetBlobData() const
	{
		if (!query) {
			return nullptr;
		}
		return blobs.GetFirst();
	}
}
