// Copyright (C) 2026 Intel Corporation
// SPDX-License-Identifier: MIT
#include "QosReadoutElement.h"
#include "TextElement.h"
#include <CommonUtilities/mc/GamingQoS.h>
#include <cwchar>
#include <format>

namespace p2c::gfx::lay
{
	QosReadoutElement::QosReadoutElement(std::wstring label_, std::wstring* pScoreText_, std::vector<std::string> classes)
		:
		FlexElement{ {}, [&classes] { classes.push_back("$qos-readout"); return std::move(classes); }() },
		pScoreText{ pScoreText_ }
	{
		AddChild(TextElement::Make(std::move(label_), { "$text-large", "$label" }));
		AddChild(pGrade = TextElement::Make(L"NA", { "$text-large", "$grade-value" }));
		AddChild(pScore = TextElement::Make(L"--", { "$text-large", "$score-value" }));
	}

	void QosReadoutElement::Draw_(Graphics& gfx) const
	{
		if (*pScoreText != lastScoreText) {
			lastScoreText = *pScoreText;
			double score = 0.;
			const bool parsed = !lastScoreText.empty() && lastScoreText != L"NA" &&
				(std::swscanf(lastScoreText.c_str(), L"%lf", &score) == 1);
			if (parsed && std::isfinite(score)) {
				lastGradeText = ::pmon::util::metrics::GamingQoSGradeFromScoreW(score);
				pScore->SetText(std::format(L"{:.0f}", score));
			}
			else {
				lastGradeText = L"NA";
				pScore->SetText(L"--");
			}
			pGrade->SetText(lastGradeText);
		}
		FlexElement::Draw_(gfx);
	}

	QosReadoutElement::~QosReadoutElement() {}

	std::shared_ptr<Element> QosReadoutElement::Make(std::wstring label, std::wstring* pScoreText, std::vector<std::string> classes)
	{
		return std::make_shared<QosReadoutElement>(std::move(label), pScoreText, std::move(classes));
	}
}
