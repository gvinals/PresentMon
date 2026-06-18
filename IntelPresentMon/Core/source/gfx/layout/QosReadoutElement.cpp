// Copyright (C) 2026 Intel Corporation
// SPDX-License-Identifier: MIT
#include "QosReadoutElement.h"
#include "TextElement.h"
#include <format>

namespace p2c::gfx::lay
{
	QosReadoutElement::QosReadoutElement(std::wstring label_, std::wstring* pGradeText_, std::wstring* pScoreText_, std::vector<std::string> classes)
		:
		FlexElement{ {}, [&classes] { classes.push_back("$qos-readout"); return std::move(classes); }() },
		pGradeText{ pGradeText_ },
		pScoreText{ pScoreText_ }
	{
		AddChild(TextElement::Make(std::move(label_), { "$text-large", "$label" }));
		AddChild(pGrade = TextElement::Make(L"NA", { "$text-large", "$grade-value" }));
		AddChild(pScore = TextElement::Make(L"--", { "$text-large", "$score-value" }));
	}

	void QosReadoutElement::Draw_(Graphics& gfx) const
	{
		if (pGradeText && *pGradeText != lastGradeText) {
			lastGradeText = *pGradeText;
			pGrade->SetText(lastGradeText.empty() ? L"NA" : lastGradeText);
		}
		if (pScoreText && *pScoreText != lastScoreText) {
			lastScoreText = *pScoreText;
			if (lastScoreText.empty() || lastScoreText == L"NA") {
				pScore->SetText(L"--");
			}
			else {
				pScore->SetText(std::format(L"{}%", lastScoreText));
			}
		}
		FlexElement::Draw_(gfx);
	}

	QosReadoutElement::~QosReadoutElement() {}

	std::shared_ptr<Element> QosReadoutElement::Make(std::wstring label, std::wstring* pGradeText, std::wstring* pScoreText, std::vector<std::string> classes)
	{
		return std::make_shared<QosReadoutElement>(std::move(label), pGradeText, pScoreText, std::move(classes));
	}
}
