// Copyright (C) 2026 Intel Corporation
// SPDX-License-Identifier: MIT
#pragma once
#include "FlexElement.h"

namespace p2c::gfx::lay
{
	class TextElement;

	class QosReadoutElement : public FlexElement
	{
	public:
		QosReadoutElement(std::wstring label, std::wstring* pScoreText, std::vector<std::string> classes = {});
		QosReadoutElement(const QosReadoutElement&) = delete;
		QosReadoutElement& operator=(const QosReadoutElement&) = delete;
		~QosReadoutElement() override;
		static std::shared_ptr<Element> Make(std::wstring label, std::wstring* pScoreText, std::vector<std::string> classes = {});
	protected:
		void Draw_(Graphics& gfx) const override;
	private:
		std::shared_ptr<TextElement> pGrade;
		std::shared_ptr<TextElement> pScore;
		std::wstring* pScoreText;
		mutable std::wstring lastScoreText;
		mutable std::wstring lastGradeText;
	};
}
