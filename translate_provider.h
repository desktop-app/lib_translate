// This file is part of Desktop App Toolkit,
// a set of libraries for developing nice desktop applications.
//
// For license and copyright information please follow this link:
// https://github.com/desktop-app/legal/blob/master/LEGAL
//
#pragma once

#include "spellcheck/spellcheck_types.h"
#include "ui/text/text_entity.h"

namespace Ui {

enum class TranslateProviderError {
	None = 0,
	Unknown,
	LocalLanguagePackMissing,
};

struct TranslateProviderResult {
	std::optional<TextWithEntities> text;
	TranslateProviderError error = TranslateProviderError::None;
};

struct TranslateProviderRequest {
	using PeerId = int64;
	using MsgId = int64;

	PeerId peerId = 0;
	MsgId msgId = 0;
	TextWithEntities text;
};

class TranslateProvider {
public:
	virtual ~TranslateProvider() = default;
	[[nodiscard]] virtual bool supportsMessageId() const = 0;
	virtual void request(
		TranslateProviderRequest request,
		LanguageId to,
		Fn<void(TranslateProviderResult)> done) = 0;
	virtual void requestBatch(
			std::vector<TranslateProviderRequest> requests,
			const LanguageId &to,
			Fn<void(int, TranslateProviderResult)> doneOne,
			Fn<void()> doneAll) {
		if (requests.empty()) {
			doneAll();
			return;
		}
		auto remaining = std::make_shared<int>(requests.size());
		auto doneOneShared = std::make_shared<Fn<void(int, TranslateProviderResult)>>(
			std::move(doneOne));
		auto doneAllShared = std::make_shared<Fn<void()>>(std::move(doneAll));
		for (auto i = 0; i != requests.size(); ++i) {
			request(
				std::move(requests[i]),
				to,
				[=](TranslateProviderResult result) {
					(*doneOneShared)(i, std::move(result));
					if (!--*remaining) {
						(*doneAllShared)();
					}
				});
		}
	}
};

} // namespace Ui
