#pragma once

#include "ECS/Component/ComponentStorage.h"
#include "ECS/Entity/EntityMetadataStorage.h"
#include "ECS/Entity/EntityID.h"

namespace wasp::ecs {
    class DataStorage {
    private:
        //typedefs
        using EntityMetadataStorage = entity::EntityMetadataStorage;
        using EntityID = entity::EntityID;
        using EntityHandle = entity::EntityHandle;
        using EntityMetadata = entity::EntityMetadata;
        using ComponentStorage = component::ComponentStorage;
        using ComponentSet = component::ComponentSet;
        using Group = component::Group;

        //fields (not initialized!)
        EntityMetadataStorage entityMetadataStorage;
        ComponentStorage componentStorage;

    public:

        //constructs a DataStorage with the specified initial entity capacity
        //and initial component capacity
        DataStorage(std::size_t initEntityCapacity, std::size_t initComponentCapacity)
            : entityMetadataStorage{ initEntityCapacity }
            , componentStorage{ initEntityCapacity, initComponentCapacity } {
        }

        void recreate() {
            entityMetadataStorage.clear();
            componentStorage.recreate();
        }

        //data query functions
        //the overloads that take an EntityHandle check for generation matching

        //returns a pointer to the group specified by the template parameters
        template <typename... Ts>
        Group* getGroupPointer() {
            return componentStorage.getGroupPointer<Ts...>();
        }

        //checks if the given entityID is alive and matches the generation
        bool isAlive(EntityHandle entityHandle) const {
            return entityMetadataStorage.isAlive(entityHandle);
        }
        //checks if the given entityID is dead or doesn't match the generation
        bool isDead(EntityHandle entityHandle) const {
            return entityMetadataStorage.isDead(entityHandle);
        }

        //checks if the given entityID is alive ignoring generation
        bool isAlive(EntityID entityID) const {
            return entityMetadataStorage.isAlive(entityID);
        }
        //checks if the given entityID is dead ignoring generation
        bool isDead(EntityID entityID) const {
            return entityMetadataStorage.isDead(entityID);
        }

        //returns true if the given entity handle contains the component,
        //returns false otherwise, including the case where the entity is dead
        template <typename T>
        bool containsComponent(EntityHandle entityHandle) const {
            if (isAlive(entityHandle)) {
                return getComponentSetPointer(entityHandle.entityID)
                    ->containsComponent<T>();
            }
            return false;
        }

        //returns true if the given entityID contains the component ignoring generation,
        //returns false otherwise
        template <typename T>
        bool containsComponent(EntityID entityID) const {
            if (isAlive(entityID)) {
                return getComponentSetPointer(entityID)
                    ->containsComponent<T>();
            }
            return false;
        }

        //returns true if the given entity handle contains all specified components,
        //returns false otherwise, including the case where the entity is dead
        template <typename... Ts>
        bool containsAllComponents(EntityHandle entityHandle) const {
            if (isAlive(entityHandle)) {
                return getComponentSetPointer(entityHandle.entityID)
                    ->containsAllComponents<Ts...>();
            }
            return false;
        }

        //returns true if the given entityID contains all specified components
        //ignoring generation, returns false otherwise
        template <typename... Ts>
        bool containsAllComponents(EntityID entityID) const {
            if (isAlive(entityID)) {
                return getComponentSetPointer(entityID)
                    ->containsAllComponents<Ts...>();
            }
            return false;
        }

        //returns true if the given entity handle contains any of the specified
        //components, returns false othwerise, including the case where the entity is
        //dead
        template <typename... Ts>
        bool containsAnyComponent(EntityHandle entityHandle) const {
            if (isAlive(entityHandle)) {
                return getComponentSetPointer(entityHandle.entityID)
                    ->containsAnyComponent<Ts...>();
            }
            return false;
        }

        //returns true if the given entityID contains any of the specified components
        //ignoring generation, returns false othwerise, including the case where the 
        //entity is dead
        template <typename... Ts>
        bool containsAnyComponent(EntityID entityID) const {
            if (isAlive(entityID)) {
                return getComponentSetPointer(entityID)
                    ->containsAnyComponent<Ts...>();
            }
            return false;
        }

        //retrieves the specified component for the given entity handle, throwing
        //if the entity either does not have that component or is dead
        template <typename T>
        T& getComponent(EntityHandle entityHandle) {
            if (isAlive(entityHandle)) {
                if (containsComponent<T>(entityHandle)) {
                    return componentStorage.getComponent<T>(
                        entityHandle.entityID,
                        *getComponentSetPointer(entityHandle.entityID)
                    );
                }
                else {
                    throw std::runtime_error{ "entity doesn't contain component!" };
                }
            }
            throw std::runtime_error{ "tried to get component of dead entity!" };
        }
        //const version
        template <typename T>
        const T& getComponent(EntityHandle entityHandle) const {
            if (isAlive(entityHandle)) {
                if (containsComponent<T>(entityHandle)) {
                    return componentStorage.getComponent<T>(
                        entityHandle.entityID,
                        *getComponentSetPointer(entityHandle.entityID)
                    );
                }
                else {
                    throw std::runtime_error{ "entity doesn't contain component!" };
                }
            }
            throw std::runtime_error{ "tried to get component of dead entity!" };
        }

        //retrieves the specified component for the given entityID ignoring generation,
        //throwing if the entity either does not have that component or is dead
        template <typename T>
        T& getComponent(EntityID entityID) {
            if (isAlive(entityID)) {
                if (containsComponent<T>(entityID)) {
                    return componentStorage.getComponent<T>(
                        entityID,
                        *getComponentSetPointer(entityID)
                    );
                }
                else {
                    throw std::runtime_error{ "entity doesn't contain component!" };
                }
            }
            throw std::runtime_error{ "tried to get component of dead entity!" };
        }
        //const version
        template <typename T>
        const T& getComponent(EntityID entityID) const {
            if (isAlive(entityID)) {
                if (containsComponent<T>(entityID)) {
                    return componentStorage.getComponent<T>(
                        entityID,
                        *getComponentSetPointer(entityID)
                        );
                }
                else {
                    throw std::runtime_error{ "entity doesn't contain component!" };
                }
            }
            throw std::runtime_error{ "tried to get component of dead entity!" };
        }

        //Returns an entity handle for the entity with the specified entityID of the
        //current generation. Throws runtime_error if there is no such alive entity.
        EntityHandle makeHandle(EntityID entityID) const;

        //data modification functions

        //returns true if successfully added component, false otherwise
        template <typename T>
        bool addComponent(AddComponentOrder<T> addComponentOrder) {
            if (isAlive(addComponentOrder.entityHandle)) {
                EntityID entityID{ addComponentOrder.entityHandle.entityID };

                const ComponentSet* oldComponentSetPointer{
                    getComponentSetPointer(entityID)
                };

                const ComponentSet* newComponentSetPointer{ 
                    componentStorage.addComponent(
                        addComponentOrder,
                        *oldComponentSetPointer
                    )
                };

                if (newComponentSetPointer) {
                    //successfully added component
                    setComponentSetPointer(entityID, newComponentSetPointer);
                    return true;
                }
            }
            return false;
        }

        //returns true if successfully set component, false otherwise
        template <typename T>
        bool setComponent(SetComponentOrder<T> setComponentOrder) {
            if (isAlive(setComponentOrder.entityHandle)) {
                EntityID entityID{ setComponentOrder.entityHandle.entityID };

                const ComponentSet* oldComponentSetPointer{
                    getComponentSetPointer(entityID)
                };

                const ComponentSet* newComponentSetPointer{ 
                    componentStorage.setComponent(
                        setComponentOrder,
                        *oldComponentSetPointer
                    ) 
                };

                if (newComponentSetPointer != oldComponentSetPointer) {
                    setComponentSetPointer(entityID, newComponentSetPointer);
                }
                return true;
            }
            return false;
        }

        //returns true if successfully removed component, false otherwise
        //entities with no components will still be "alive"
        template <typename T>
        bool removeComponent(RemoveComponentOrder<T> removeComponentOrder) {
            if (isAlive(removeComponentOrder.entityHandle)) {
                EntityID entityID{ removeComponentOrder.entityHandle.entityID };

                const ComponentSet* oldComponentSetPointer{
                    getComponentSetPointer(entityID)
                };

                const ComponentSet* newComponentSetPointer{
                    componentStorage.removeComponent(
                        removeComponentOrder,
                        *oldComponentSetPointer
                    )
                };

                if (newComponentSetPointer != oldComponentSetPointer) {
                    //successfully removed component
                    setComponentSetPointer(entityID, newComponentSetPointer);
                    return true;
                }
            }
            return false;
        }

        //returns entityID
        template <typename... Ts>
        EntityHandle addEntity(const AddEntityOrder<Ts...>& addEntityOrder) {
            EntityHandle entityHandle{ entityMetadataStorage.createEntity() };

            const ComponentSet* componentSetPointer{
                componentStorage.addEntity(addEntityOrder, entityHandle.entityID)
            };

            setComponentSetPointer(entityHandle.entityID, componentSetPointer);
            return entityHandle;
        }

        //returns entityIDs in order
        template <typename... Ts, typename... Us>
        std::vector<EntityHandle> addEntities(
            const AddEntityOrder<Ts...>& addEntityOrder, 
            const Us&... args
        ) {
            std::vector<EntityHandle> entityHandleVector{};
            entityHandleVector.push_back(addEntity(addEntityOrder));
            if constexpr (sizeof...(args) > 0) {
                addEntities(entityHandleVector, args...);
            }
            return entityHandleVector;
        }

        //returns true if successfully removed entity, false otherwise
        bool removeEntity(RemoveEntityOrder removeEntityOrder);

    private:
        //helper functions
        const ComponentSet* getComponentSetPointer(EntityID entityID) const {
            return getMetadata(entityID).getComponentSetPointer();
        }

        //the non-const version of getMetadata() returns by reference
        EntityMetadata& getMetadata(EntityID entityID) {
            return entityMetadataStorage.getMetadata(entityID);
        }

        //the const version of getMetadata() returns by value
        EntityMetadata getMetadata(EntityID entityID) const {
            return entityMetadataStorage.getMetadata(entityID);
        }

        void setComponentSetPointer(
            EntityID entityID,
            const ComponentSet* componentSetPointer
        );

        template <typename... Ts, typename... Us>
        void addEntities(
            std::vector<EntityHandle>& entityHandleVector,
            const AddEntityOrder<Ts...>& addEntityOrder,
            const Us&... args
        ) {
            entityHandleVector.push_back(addEntity(addEntityOrder));
            if constexpr (sizeof...(args) > 0) {
                addEntities(entityHandleVector, args...);
            }
        }
    };
}