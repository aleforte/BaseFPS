# BaseFPS (Prototype)

## Plugins
- CommonUI
- CommonUser
- CommonGame (requires the following dependencies)
    - ModularGameplay
    - ModularGameplayActors
- EnhancedInput
- GameSettings
- AudioModulation
- GameSubtitles

### Modified CommonUI (Important Note)

Needed to set all modules to the "PreDefault" loading phase. Changes are here:
```
Engine\Plugins\Experimental\CommonUI\CommonUI.uplugin
```
Seems like directly referencing CommonUI files/assets in C++ code causes an assertion error on project load. This is the only known fix...for now


### Modified Project Settings (Outline)
- Project -> Maps & Modes
    - Game Instance Class: `BaseFPSGameInstance` (extends `CommonGameInstance`)
- Engine -> General Settings
    - Game Viewport Client Class: `CommonGameViewportClient`
    - Local Player Class: `BaseFPSLocalPlayer`
    - Game User Settings Class: `BaseFPSSettingsLocal`
- Game -> Common Input Settings
    - Input Data: `B_CommonInputData` (enables Back button on Menus)
- Plugins -> Common UI Editor
    - Template Text Style: `TestStyle-Base`
    - Template Button Style: `ButtonStyle-Clear`
- Plugins -> Common UI Input Settings
    - Added **Action**
        - Action Tag: `UI.Action.Escape`
        - Default DisplayName: `Back`
        - Key Mappings, add two: `Escape` and `Gamepad Special Right`
- Plugins -> Common UI Framework
    - Default Rich Text Data Class: `CommonUIRichTextData`
    - Platform Traits, added...
        - `Platform.Trait.SupportsWindowedMode`
        - `Platform.Trait.SupportsChangingAudioOutputDevice`
        - `Platform.Trait.NeedsBrightnessAdjustment`