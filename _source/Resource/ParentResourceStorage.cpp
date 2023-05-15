#include "Resource\ParentResourceStorage.h"

namespace process::resource {

    namespace{
        using ResourceBase = wasp::resource::ResourceBase;
    }

	using ResourceType = Resource<ChildList>;

	ChildListResource::~ChildListResource() {
		//erase children first
		for (auto& childPointer : *dataPointer) {
			childPointer->setParentPointer(nullptr);
		}
		dataPointer->clear();

		//remove self from parent in IResource destructor
	}

	void ChildListResource::removeChild(ResourceBase* child) {
		auto found{
            std::find(dataPointer->begin(), dataPointer->end(), child)
        };
		if (found != dataPointer->end()) {
			dataPointer->erase(found);
		}
		else {
			throw std::runtime_error("Error removeChild called on non child");
		}
	}

	static void unloadChildren(ResourceType& resource) {
		ChildList& childList{ *resource.getDataPointerCopy() };

		for (ResourceBase* childPointer : childList) {
			if (childPointer->isLoaded()) {
				const std::wstring& childID{ childPointer->getID() };
				childPointer->getStoragePointer()->unload(childID);
			}
		}
	}

	void ParentResourceStorage::unload(const std::wstring& id) {
		auto found{ resourceMap.find(id) };
		if (found != resourceMap.end()) {
			Resource<ChildList>& resource{
				*(std::get<1>(*found))
			};

			if (resource.isLoaded()) {
				unloadChildren(resource);
				resource.unloadData();
			}
		}
	}

	static void removeChildren(ResourceType& resource) {
		ChildList& childList{ *resource.getDataPointerCopy() };

		for (ResourceBase* childPointer : childList) {
			if (childPointer->isLoaded()) {
				const std::wstring& childID{ childPointer->getID() };
				childPointer->getStoragePointer()->remove(childID);
			}
		}
	}

	void ParentResourceStorage::remove(const std::wstring& id) {
		auto found{ resourceMap.find(id) };
		if (found != resourceMap.end()) {
			Resource<ChildList>& resource{
				*(std::get<1>(*found))
			};

			if (resource.isLoaded()) {
				removeChildren(resource);
				resourceMap.erase(found);
			}
		}
	}
}