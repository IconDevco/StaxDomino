
#pragma once
#include "StdAfx.h"
#include "CryEntitySystem/IEntityComponent.h"
#include <DefaultComponents/Cameras/CameraComponent.h>
#include "DefaultComponents/Geometry/AdvancedAnimationComponent.h"
#include <DefaultComponents/Input/InputComponent.h>
#include <DefaultComponents/Audio/ListenerComponent.h>
#include "DefaultComponents/Physics/CharacterControllerComponent.h"
#include <CryAISystem/Components/IEntityNavigationComponent.h>
#include "CryPhysics/IPhysics.h"

#include "CryRenderer/ITexture.h"
#include "CryMath/Random.h"
//#include "CryRenderer/IShader.h"
//#include "CryRenderer/IShader_info.h"

class CDominoComponent final : public IEntityComponent
{
public:
	virtual ~CDominoComponent() {}

	// IEntityComponent
	virtual void Initialize() override
	{
		// Set the model
		const int geometrySlot = 0;
		GetEntity()->LoadGeometry(geometrySlot, "Objects/Dominoes.cgf");
		
		GetEntity()->LoadGeometry(geometrySlot +1, "Objects/Domino/1.cgf");
		GetEntity()->LoadGeometry(geometrySlot +2, "Objects/Domino/2.cgf");
		GetEntity()->LoadGeometry(geometrySlot + 3, "Objects/Domino/3.cgf");
		GetEntity()->LoadGeometry(geometrySlot + 4, "Objects/Domino/4.cgf");

		auto *pDominoMaterial = gEnv->p3DEngine->GetMaterialManager()->LoadMaterial("Objects/dominoes1");
		m_pEntity->SetMaterial(pDominoMaterial);
		
		for (int i = 1; i < 5; i++) {
			int r = cry_random(1, 6);
			string name("materials/domino/" + ToString(r));
			auto* pNumMaterial = gEnv->p3DEngine->GetMaterialManager()->LoadMaterial(name);
			
			IRenderAuxGeom* g = gEnv->pAuxGeomRenderer->GetAux();
			g->Draw2dLabel(10,10,3,Col_Yellow,false, name);

			m_pEntity->SetSlotMaterial(geometrySlot + i, pNumMaterial);
		}
		
		
		
		
		

		// Now create the physical representation of the entity
		SEntityPhysicalizeParams physParams;
		physParams.type = PE_RIGID;
		physParams.mass = 1.f;
		m_pEntity->Physicalize(physParams);


		// Make sure that bullets are always rendered regardless of distance
		// Ratio is 0 - 255, 255 being 100% visibility
		GetEntity()->SetViewDistRatio(255);
		GetEntity()->SetLodRatio(50);
		pe_action_awake awake; 
		awake.bAwake = 0;
		GetEntity()->GetPhysics()->Action(&awake);

		const unsigned int rayFlags = rwi_stop_at_pierceable | rwi_colltype_any;
		ray_hit hit;

		int hits = gEnv->pPhysicalWorld->RayWorldIntersection(GetEntity()->GetWorldPos(), Vec3(0, 0, -1) * INFINITE, ent_terrain, rayFlags, &hit, 1);

		if (hits > 0)
			GetEntity()->SetPos(hit.pt);

		m_position = GetEntity()->GetWorldPos();
		m_rotation = GetEntity()->GetWorldRotation();



	}
	Vec3 m_position = Vec3(0);
	Quat m_rotation = IDENTITY;
	// Reflect type to set a unique identifier for this component
	static void ReflectType(Schematyc::CTypeDesc<CDominoComponent>& desc)
	{
		desc.SetGUID("{B53A9A5F-F27A-42CB-82C7-B1E379C41A2A}"_cry_guid);
	}

	virtual Cry::Entity::EventFlags GetEventMask() const override { return ENTITY_EVENT_COLLISION; }
	virtual void ProcessEvent(const SEntityEvent& event) override
	{
		// Handle the OnCollision event, in order to have the entity removed on collision
		if (event.event == ENTITY_EVENT_COLLISION)
		{
			// Collision info can be retrieved using the event pointer
			//EventPhysCollision *physCollision = reinterpret_cast<EventPhysCollision *>(event.ptr);

			// Queue removal of this entity, unless it has already been done
			//gEnv->pEntitySystem->RemoveEntity(GetEntityId());
		}
	}
	// ~IEntityComponent

	
};