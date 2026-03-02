// This file is part of Desktop App Toolkit,
// a set of libraries for developing nice desktop applications.
//
// For license and copyright information please follow this link:
// https://github.com/desktop-app/legal/blob/master/LEGAL
//
import Foundation
import NaturalLanguage
import Translation

typealias TranslateProviderMacSwiftCallback = @convention(c) (
	UnsafeMutableRawPointer?,
	UnsafePointer<CChar>?,
	UnsafePointer<CChar>?
) -> Void

private func duplicatedCString(_ value: String) -> UnsafePointer<CChar>? {
	guard let duplicated = strdup(value) else {
		return nil
	}
	return UnsafePointer(duplicated)
}

@available(macOS 15.0, *)
private func requestTranslation(
		_ text: String,
		_ targetLanguage: String) async throws -> String {
	guard let sourceLanguage
		= NLLanguageRecognizer.dominantLanguage(for: text) else {
			throw TranslationError.unableToIdentifyLanguage
		}
	if sourceLanguage.rawValue == targetLanguage {
		return text
	}
	let source = Locale.Language(identifier: sourceLanguage.rawValue)
	let target = Locale.Language(identifier: targetLanguage)
	let availability = LanguageAvailability()
	let status = await availability.status(from: source, to: target)
	if status != .installed {
		switch status {
		case .supported:
			if #available(macOS 26.0, *) {
				throw TranslationError.notInstalled
			}
			throw TranslationError.unsupportedLanguagePairing
		case .unsupported:
			throw TranslationError.unsupportedLanguagePairing
		case .installed:
			break
		@unknown default:
			throw TranslationError.unsupportedLanguagePairing
		}
	}
	let session = TranslationSession(installedSource: source, target: target)
	let response = try await session.translate(text)
	return response.targetText
}

@available(macOS 15.0, *)
private func translateErrorCode(_ error: Error) -> String {
	guard let translationError = error as? TranslationError else {
		return "unknown"
	}
	if #available(macOS 26.0, *), case .notInstalled = translationError {
		return "local-language-pack-missing"
	}
	switch translationError {
	case .unsupportedLanguagePairing:
		return "local-language-pack-missing"
	default:
		return "unknown"
	}
}

@_cdecl("TranslateProviderMacSwiftIsAvailable")
func TranslateProviderMacSwiftIsAvailable() -> Bool {
	if #available(macOS 15.0, *) {
		return true
	}
	return false
}

@_cdecl("TranslateProviderMacSwiftTranslate")
func TranslateProviderMacSwiftTranslate(
	_ sourceTextUtf8: UnsafePointer<CChar>?,
	_ targetLanguageCodeUtf8: UnsafePointer<CChar>?,
	_ context: UnsafeMutableRawPointer?,
	_ callback: TranslateProviderMacSwiftCallback?
) {
	guard let callback else {
		return
	}
	guard let sourceTextUtf8, let targetLanguageCodeUtf8 else {
		callback(context, nil, duplicatedCString("invalid-arguments"))
		return
	}
	let sourceText = String(cString: sourceTextUtf8)
	let targetLanguageCode = String(cString: targetLanguageCodeUtf8)
	Task.detached(priority: .utility) {
		if #available(macOS 15.0, *) {
			do {
				let translated = try await requestTranslation(
					sourceText,
					targetLanguageCode)
				callback(context, duplicatedCString(translated), nil)
			} catch {
				callback(
					context,
					nil,
					duplicatedCString(translateErrorCode(error)))
			}
		} else {
			callback(context, nil, duplicatedCString("unsupported-platform"))
		}
	}
}
