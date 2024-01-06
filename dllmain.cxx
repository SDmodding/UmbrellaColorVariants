#define SDK_SD_MINIMAL_BUILD
#define SDK_SD_OPTIONAL_INCLUDES
#include <SDK/_Includes.hpp>
#include <SDK/Libs/MinHook.h>
#pragma comment(lib, "SDK/Libs/MinHook.lib")

//==================================================================================

namespace Helper
{
    __inline bool IsUmbrella(UFG::CSceneObjectProperties* p_SceneObject)
    {
        static constexpr UFG::qSymbol _UmbrellaNameUID = 0x949A4909;
        UFG::qPropertySet* _PropertySet = p_SceneObject->mpWritableProperties;
        if (!_PropertySet) 
        {
            _PropertySet = p_SceneObject->mpConstProperties;
            if (!_PropertySet) {
                return false;
            }
        }

        if (_PropertySet->mName != _UmbrellaNameUID)
        {
            if (_PropertySet->mNumParents)
            {
                _PropertySet = _PropertySet->GetParentFromIdx(0);
                if (_PropertySet->mName == _UmbrellaNameUID) {
                    return true;
                }
            }

            return false;
        }


        return true;
    }

    void ApplyColourOnObject(UFG::CSceneObjectProperties* p_SceneObject)
    {
        UFG::CCompositeDrawableComponent* _CompositeDrawable = p_SceneObject->m_pSimObject->GetCompositeDrawable();
        if (!_CompositeDrawable) {
            return; // If this happens then that isn't my issue
        }

        bool _RandSaturation = (rand() % 10 >= 8); // Prefer saturated color than randomized that will cause color to be fully white.
        float _MaxSaturation = 0.75f;

        UFG::qColour _RGBA; _RGBA.HSVToRGB({ UFG::qRandom(360.f), (_RandSaturation ? UFG::qRandom(_MaxSaturation) : _MaxSaturation), 1.f, 1.f });
        memcpy(_CompositeDrawable->mSceneryInstance.ColourTint, &_RGBA, sizeof(UFG::qColour));
    }
}

//==================================================================================

namespace UFG
{
    typedef void(__fastcall* Fn_ComponentFactoryPropertiesOnActivate)(CSceneObjectProperties*, qMatrix44*, qSymbol*, CTransformNodeComponent*);
    Fn_ComponentFactoryPropertiesOnActivate g_ComponentFactoryPropertiesOnActivate;

    void __fastcall ComponentFactoryPropertiesOnActivate(CSceneObjectProperties* p_SceneObject, qMatrix44* p_WorldMatrix, qSymbol* p_ObjectType, CTransformNodeComponent* p_ParentTransform)
    {
        g_ComponentFactoryPropertiesOnActivate(p_SceneObject, p_WorldMatrix, p_ObjectType, p_ParentTransform);

        if (*p_ObjectType != 0x8A5A95FC) { // Weapon
            return;
        }

        if (!Helper::IsUmbrella(p_SceneObject)) {
            return;
        }

        Helper::ApplyColourOnObject(p_SceneObject);
    }

    //===========================================

    typedef bool(__fastcall* Fn_CompositeDrawableComponentReset)(CCompositeDrawableComponent*);
    Fn_CompositeDrawableComponentReset g_CompositeDrawableComponentReset;

    void __fastcall CompositeDrawableComponentReset(CCompositeDrawableComponent* p_CompositeDrawable)
    {
        g_CompositeDrawableComponentReset(p_CompositeDrawable);

        if (!p_CompositeDrawable->m_pSimObject || !p_CompositeDrawable->m_pSimObject->m_pSceneObj) {
            return;
        }

        if (!Helper::IsUmbrella(p_CompositeDrawable->m_pSimObject->m_pSceneObj)) {
            return;
        }

        Helper::ApplyColourOnObject(p_CompositeDrawable->m_pSimObject->m_pSceneObj);
    }
}

//==================================================================================

int __stdcall DllMain(HMODULE p_Module, DWORD p_Reason, void* p_Reserved)
{
    if (p_Reason == DLL_PROCESS_ATTACH)
    {
        if (!SDK::Utils::IsValidExecutable())
        {
            MessageBoxA(0, "This is not valid executable.\nPlease visit: https://github.com/SDmodding \nAnd check README.md", "Umbrella Color Variants", MB_OK | MB_ICONERROR);
            return 0;
        }

        DisableThreadLibraryCalls(p_Module);

        MH_Initialize();
        MH_CreateHook(UFG_RVA_PTR(0x3DC0), UFG::CompositeDrawableComponentReset, (void**)&UFG::g_CompositeDrawableComponentReset);
        MH_CreateHook(UFG_RVA_PTR(0x588E20), UFG::ComponentFactoryPropertiesOnActivate, (void**)&UFG::g_ComponentFactoryPropertiesOnActivate);
    }

    return 1;
}
