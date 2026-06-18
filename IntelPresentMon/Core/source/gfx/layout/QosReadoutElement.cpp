// Copyright (C) 2026 Intel Corporation
// SPDX-License-Identifier: MIT
#include "QosReadoutElement.h"
#include "TextElement.h"
#include <CommonUtilities/mc/GamingQoS.h>
#include <cmath>
#include <cstdlib>

namespace p2c::gfx::lay
{
	namespace
	{
		std::optional<double> TryParseScoreText_(const std::wstring& text)
		{
			if (text.empty() || text == L"NA") {
				return std::nullopt;
			}
			wchar_t* pEnd = nullptr;
			const double value = wcstod(text.c_str(), &pEnd);
			if (pEnd == text.c_str() || !std::isfinite(value)) {
				return std::nullopt;
			}
			return value;
		}
	}

	QosReadoutElement::QosReadoutElement(std::wstring label_, std::wstring* pScoreText_, std::vector<std::string> classes)
		:
		FlexElement{ {}, [&classes] { classes.push_back("$qos-readout"); return std::move(classes); }() },
		pScoreText{ pScoreText_ }
	{
		AddChild(TextElement::Make(std::move(label_), { "$text-large", "$label" }));
		AddChild(pGrade = TextElement::Make(L"NA", { "$text-large", "$grade-value" }));
	}

	void QosReadoutElement::Draw_(Graphics& gfx) const
	{
		if (pScoreText && *pScoreText != lastScoreText) {
			lastScoreText = *pScoreText;
			if (lastScoreText.empty() || lastScoreText == L"NA") {
				lastGradeText = L"NA";
				pGrade->SetText(L"NA");
			}
			else if (const auto score = TryParseScoreText_(lastScoreText)) {
				const auto grade = ::pmon::util::metrics::GamingQoSGradeFromScoreW(*score);
				if (grade != lastGradeText) {
					lastGradeText = grade;
					pGrade->SetText(lastGradeText);
				}
			}
			else {
				lastGradeText = L"NA";
				pGrade->SetText(L"NA");
			}
		}
		FlexElement::Draw_(gfx);
	}

	QosReadoutElement::~QosReadoutElement() {}

	std::shared_ptr<Element> QosReadoutElement::Make(std::wstring label, std::wstring* pScoreText, std::vector<std::string> classes)
	{
		return std::make_shared<QosReadoutElement>(std::move(label), pScoreText, std::move(classes));
	}
}
