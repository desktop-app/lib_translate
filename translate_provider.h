// This file is part of Desktop App Toolkit,
// a set of libraries for developing nice desktop applications.
//
// For license and copyright information please follow this link:
// https://github.com/desktop-app/legal/blob/master/LEGAL
//
#pragma once

#include "spellcheck/platform/platform_language.h"
#include "ui/text/text_entity.h"

class PeerData;

namespace Ui {

struct TranslateProviderRequest {
	not_null<PeerData*> peer;
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
		Fn<void(std::optional<TextWithEntities>)> done) = 0;
};

} // namespace Ui
