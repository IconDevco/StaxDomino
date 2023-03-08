#pragma once

#include <array>
#include <numeric>

#include <CryEntitySystem/IEntityComponent.h>
#include <CryMath/Cry_Camera.h>

#include <ICryMannequin.h>
#include <CrySchematyc/Utils/EnumFlags.h>

#include <DefaultComponents/Cameras/CameraComponent.h>
#include "DefaultComponents/Geometry/AdvancedAnimationComponent.h"
#include <DefaultComponents/Input/InputComponent.h>
#include <DefaultComponents/Audio/ListenerComponent.h>
#include "DefaultComponents/Physics/CharacterControllerComponent.h"
#include <CryAISystem/Components/IEntityNavigationComponent.h>


#include "CryInput/IInput.h"
#include "CryAction/IActionMapManager.h"


#include "PersistantDebug.h"

////////////////////////////////////////////////////////
// Represents a player participating in gameplay
////////////////////////////////////////////////////////

class CPlayerComponent final : public IEntityComponent
{

	enum class EInputFlagType
	{
		Hold = 0,
		Toggle
	};

	enum class EInputFlag : uint8
	{
		MoveLeft = 1 << 0,
		MoveRight = 1 << 1,
		MoveForward = 1 << 2,
		MoveBack = 1 << 3
	};

	static constexpr EEntityAspects InputAspect = eEA_GameClientD;

	template<typename T, size_t SAMPLES_COUNT>
	class MovingAverage
	{
		static_assert(SAMPLES_COUNT > 0, "SAMPLES_COUNT shall be larger than zero!");

	public:

		MovingAverage()
			: m_values()
			, m_cursor(SAMPLES_COUNT)
			, m_accumulator()
		{
		}

		MovingAverage& Push(const T& value)
		{
			if (m_cursor == SAMPLES_COUNT)
			{
				m_values.fill(value);
				m_cursor = 0;
				m_accumulator = std::accumulate(m_values.begin(), m_values.end(), T(0));
			}
			else
			{
				m_accumulator -= m_values[m_cursor];
				m_values[m_cursor] = value;
				m_accumulator += m_values[m_cursor];
				m_cursor = (m_cursor + 1) % SAMPLES_COUNT;
			}

			return *this;
		}

		T Get() const
		{
			return m_accumulator / T(SAMPLES_COUNT);
		}

		void Reset()
		{
			m_cursor = SAMPLES_COUNT;
		}

	private:

		std::array<T, SAMPLES_COUNT> m_values;
		size_t m_cursor;

		T m_accumulator;
	};



public:
	CPlayerComponent() = default;
	virtual ~CPlayerComponent() = default;

	// IEntityComponent
	virtual void Initialize() override;

	virtual Cry::Entity::EventFlags GetEventMask() const override;
	virtual void ProcessEvent(const SEntityEvent& event) override;
	// ~IEntityComponent

	// Reflect type to set a unique identifier for this component
	static void ReflectType(Schematyc::CTypeDesc<CPlayerComponent>& desc)
	{
		desc.SetGUID("{63F4C0C6-32AF-4ACB-8FB0-57D45DD14725}"_cry_guid);
	}

	void OnReadyForGameplayOnServer();
	bool IsLocalClient() const { return (m_pEntity->GetFlags() & ENTITY_FLAG_LOCAL_PLAYER) != 0; }

	void Revive(const Matrix34& transform);

	//ghost for domino placement;
	//void CreateGhostCursor();
void ShowGhostCursor();
//	void HideGhostCursor();

	void CreateFirstGhost(Vec3 p);
	void UpdateFirstGhost(float fTime);
	void DestroyFirstGhost();

protected:
	void HandleInputFlagChange(CEnumFlags<EInputFlag> flags, CEnumFlags<EActionActivationMode> activationMode, EInputFlagType type = EInputFlagType::Hold);

	void BindInputs();

	void InitializeLocalPlayer();

protected:


	/// Cryengine Shit ///

	struct RemoteReviveParams
	{
		void SerializeWith(TSerialize ser)
		{
			ser.Value("pos", position, 'wrld');
			ser.Value("rot", rotation, 'ori0');
		}

		Vec3 position;
		Quat rotation;
	};
	bool RemoteReviveOnClient(RemoteReviveParams&& params, INetChannel* pNetChannel);

	/// ~Cryengine Shit ///

protected:
	IPersistantDebug* debug;

	float waterLevel = 16;
	Cry::DefaultComponents::CCameraComponent* m_pCameraComponent = nullptr;
	Cry::DefaultComponents::CInputComponent* m_pInputComponent = nullptr;

protected:
	CEnumFlags<EInputFlag> m_inputFlags;
	Vec2 m_mouseDeltaRotation;

	MovingAverage<Vec2, 10> m_mouseDeltaSmoothingFilter;

	Quat m_lookOrientation = IDENTITY; //!< Should translate to head orientation in the future
	float m_horizontalAngularVelocity = 0;
	MovingAverage<float, 10> m_averagedHorizontalAngularVelocity;

	const char* m_playerName = "Moose";

	////////////////////// MODE /////////////////////////////
public:



	void ShowCursor();
	void HideCursor();

public:



protected:
	void UpdateZoom(float fTime);


	void UpdateCameraTargetGoal(float fTime);

	void UpdateTacticalViewDirection(float frameTime);

	void UpdateCamera(float frameTime);

	Vec3 GetTacticalCameraMovementInputDirection();
	Vec2 GetPilotMovementInputDirection();

	bool m_lookActive;
	const float m_rotationSensitivity = 0.002f;
	float m_scrollY;
	float m_scrollSpeedMultiplier = 0.2f;

	float m_desiredViewDistance = 30;
	float m_currViewDistance = 30;

	float m_zoomTension = 0.3f;//1.7f;

	//How close can a camera get
	float m_minViewDistance = 1;

	float m_minTacticalDistance = 1;
	float m_maxTacticalDistance = 12;
	//float MAX_MAP_DISTANCE = 200;

	Vec3 m_cameraCurrentGoalPosition;
	Vec3 m_cameraDesiredGoalPosition;

	float m_goalTension = 2.f;
	float m_goalSpeed = 2.f;
	
	Matrix34 m_desiredCameraTranform;

	float m_scrollSensitivity;
	float m_panSensitivity = 70;

	//ILINE ECameraMode GetCameraMode() const { return m_currentCameraMode; }

	Vec3 m_placementDesiredGoalPosition = Vec3(0);
	Vec3 m_placementCurrentGoalPosition = Vec3(0);

	void UpdatePlacementPosition(Vec3 o, float fTime);

	//if a fleet is selected, then you can select any vessel in the fleet and drive it
	void UpdateCursorPointer();
	Vec3 GetPositionFromPointer();

	bool m_placementActive = false;
	DynArray<IEntity*> Dominoes;
	DynArray<IEntity*> JustPlacedDominoes;
	IEntity* m_firstPlacedDomino = nullptr;
	IEntity* m_ghostFirstDomino = nullptr;
	IEntity* m_ghostCursorDomino = nullptr;
	Vec3 m_lastPlacedPosition = Vec3(0);
	Vec3 m_firstPlacedPosition = Vec3(0);

	void PlaceDomino(Vec3 pos, Quat rot = IDENTITY);

	bool m_firstPlaced = false;

	float m_placementDistance = .3f;
	int m_placedDominoes = 0;

	void BeginSimulation();
	void EndSimulation();
	void ResetDominoes();

	
	bool m_isSimulating = false;

	struct SHistorySet 
	{
		int m_index = 0;
		std::vector<IEntity*> Dominoes;
	};
	SHistorySet* m_ActiveHistory = nullptr;
	DynArray<SHistorySet*> History;

	void RemoveDomino(IEntity* Domino);

	void InsertHistorySet(SHistorySet* historySet);
	int m_historyStep= 0;

	int m_undoSteps = 0;
	void Undo();
	void Redo();
	void RestartHistory(int fromIndex);


};

