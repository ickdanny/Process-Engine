#pragma once

#include "Resource\ResourceStorage.h"
#include "Resource\ResourceBase.h"
#include "Lexer.h"
#include "Parser.h"

#pragma warning(disable : 4250) //suppress inherit via dominance

namespace process::game::resources {
	class ScriptStorage
		: public resource::ResourceStorage<darkness::AstNode>,
			public resource::FileLoadable,
			public resource::ManifestLoadable {
	public:
		//typedefs
		using Script = darkness::AstNode;
		using ResourceType = resource::Resource<Script>;
		using ResourceBase = wasp::resource::ResourceBase;
		
	private:
		//fields
		darkness::Lexer lexer{};
		darkness::Parser parser{};
		
	public:
		ScriptStorage()
			: FileLoadable { { L"dk" } }, ManifestLoadable { { L"dkScript" } } {
		}
		
		void reload(const std::wstring& id) override;
		
		ResourceBase* loadFromFile(
			const resource::FileOrigin& fileOrigin,
			const resource::ResourceLoader& resourceLoader
		) override;
		
		ResourceBase* loadFromManifest(
			const resource::ManifestOrigin& manifestOrigin,
			const resource::ResourceLoader& resourceLoader
		) override;
		
	private:
		Script parseScriptFile(const std::wstring& fileName);
	};
}