// This file is part of Desktop App Toolkit,
// a set of libraries for developing nice desktop applications.
//
// For license and copyright information please follow this link:
// https://github.com/desktop-app/legal/blob/master/LEGAL
//
#pragma once

#include "ui/text/text_entity.h"

struct LanguageId;

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
};

} // namespace Ui
