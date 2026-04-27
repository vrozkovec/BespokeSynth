# BespokeSynth

A live-patchable software modular synthesizer. C++17, built on JUCE (audio/UI framework) and NanoVG (vector rendering). Created by Ryan Challinor. Licensed under GPL v3.

Cross-platform: Windows, macOS (10.13+), Linux.

## Build Commands

First-time setup:
```bash
git submodule update --init --recursive
```

Using the `just` command runner (recommended):
```bash
just configure Release    # configure CMake (Debug is default if omitted)
just build Release        # build (auto-configures if needed)
just run Release          # run (auto-builds if needed)
just clean                # remove build directory
```

Using raw CMake:
```bash
cmake -Bignore/build -GNinja -DCMAKE_BUILD_TYPE=Release
cmake --build ignore/build --parallel $(nproc)
```

Executable output: `ignore/build/Source/BespokeSynth_artefacts/{Release|Debug}/BespokeSynth`

Optional CMake flags:
- `-DBESPOKE_VST2_SDK_LOCATION=/path` - enable VST2 hosting (non-FOSS)
- `-DBESPOKE_ASIO_SDK_LOCATION=/path` - ASIO support (Windows only)
- `-DBESPOKE_SPACEMOUSE_SDK_LOCATION=/path` - SpaceMouse support (Windows only)
- `-DBESPOKE_PYTHON_ROOT=/path` - override Python detection
- `-DBESPOKE_USE_ASAN=ON` - build with AddressSanitizer
- `-DBESPOKE_PORTABLE=ON` - self-contained build with bundled dependencies
- `-GXcode` - generate Xcode project (macOS)

There is no unit test framework. Testing is done manually by running the application. CI runs clang-format checks via the `code-quality-pipeline-checks` CMake target.

## Code Style

Enforced by `.clang-format` (WebKit-based) and `.editorconfig`.

- **Indentation**: 3 spaces
- **Braces**: Allman style (open brace on new line for functions, classes, control statements)
- **Member variables**: `mCamelCase` prefix (e.g., `mGain`, `mGainSlider`)
- **Static variables**: `sCamelCase` prefix (e.g., `sPythonInitialized`)
- **Constants/macros**: `UPPER_CASE` (e.g., `MAX_BUFFER_SIZE`, `kNumVoices`)
- **Enum values**: `kEnumName_Value` prefix (e.g., `kModuleCategory_Audio`)
- **Struct fields**: no prefix
- **Header guards**: `#pragma once`
- **License header**: GPL v3 block on every source file
- **Includes**: `#include` order is not sorted (SortIncludes: false)

Commit messages: lowercase imperative, backticks around code identifiers, PR number suffix `(#123)`. Example: `` fix LFO display being broken by #2002 when a slider's range is not 0-1 (#2009) ``. Merge strategy: squash and merge.

## Architecture

### Entry Point and Main Loop

`Source/Main.cpp` - JUCE application (`BespokeApplication : JUCEApplication`). Handles CLI args, loads `.bsk`/`.bskt` project files.

`Source/ModularSynth.h/.cpp` - central controller. Manages module creation, audio/MIDI I/O, UI rendering, patching, canvas navigation. Accessed globally as `TheSynth`.

`Source/SynthGlobals.h/.cpp` - global state: `gSampleRate`, `gBufferSize`, `gTime`, work buffers (`gWorkBuffer[kWorkBufferSize]`), NanoVG render contexts.

### Module System

All modules inherit from `IDrawableModule` (`Source/IDrawableModule.h`), which provides rendering, save/load, minimization, patch cable management, and child module support.

Module categories: `kModuleCategory_Note`, `kModuleCategory_Synth`, `kModuleCategory_Audio`, `kModuleCategory_Instrument`, `kModuleCategory_Processor`, `kModuleCategory_Modulator`, `kModuleCategory_Pulse`, `kModuleCategory_Other`.

### Signal Flow Interfaces

**Audio chain** (`Source/IAudioSource.h`, `IAudioReceiver.h`, `IAudioProcessor.h`, `IAudioEffect.h`):
- `IAudioSource` - produces audio via `Process(double time)`
- `IAudioReceiver` - consumes audio, provides `GetBuffer()` (ChannelBuffer)
- `IAudioProcessor` - both source and receiver (`SyncBuffers()` helper)
- `IAudioEffect` - inline effect via `ProcessAudio(double time, ChannelBuffer* buffer)`

**Note/MIDI chain** (`Source/INoteSource.h`, `INoteReceiver.h`, `NoteEffectBase.h`):
- `INoteSource` - emits notes via `PlayNoteOutput(NoteMessage)`
- `INoteReceiver` - receives via `PlayNote(NoteMessage)` and `SendCC()`
- `NoteEffectBase` - pass-through base for note processors

**Pulse/clock chain** (`Source/IPulseSource.h`, `IPulseReceiver.h`):
- `IPulseSource` - dispatches clock/trigger via `DispatchPulse()`
- `IPulseReceiver` - receives via `OnPulse(double time, float velocity, int flags)`

**Modulation** (`Source/IModulator.h`):
- `IModulator` - outputs value via `Value(int samplesIn)`, targets UI controls through patch cables

### Patching

`Source/IPatchable.h` - interface for objects that can be patched. `PatchCableSource` (`Source/PatchCableSource.h`) manages output connections. `PatchCable` (`Source/PatchCable.h`) represents visual cable rendering. Cable types: audio, note, pulse, modulation.

### UI Controls

`Source/IUIControl.h` - base class for all widgets. Supports MIDI CC mapping, modulation targeting, save/load.

Concrete controls: `FloatSlider`, `IntSlider` (`Source/Slider.h`), `Checkbox` (`Source/Checkbox.h`), `ClickButton` (`Source/ClickButton.h`), `DropdownList` (`Source/DropdownList.h`), `TextEntry` (`Source/TextEntry.h`), `RadioButton` (`Source/RadioButton.h`), `UIGrid` (`Source/UIGrid.h`).

Listener pattern: `IFloatSliderListener`, `IIntSliderListener`, `IButtonListener`, `ITextEntryListener`, etc.

### Module Factory

`Source/ModuleFactory.h/.cpp` - factory for instantiating modules. Uses registration macros:
```cpp
REGISTER(ClassName, name, kModuleCategory_*)           // normal module
REGISTER_HIDDEN(ClassName, name, kModuleCategory_*)    // hidden from menu
REGISTER_EXPERIMENTAL(ClassName, name, kModuleCategory_*)  // marked experimental
```

`Source/EffectFactory.h/.cpp` - separate factory for `IAudioEffect` subclasses.

### Rendering

NanoVG-based 2D vector graphics. OpenFrameworks compatibility layer in `Source/OpenFrameworksPort.h` (defines `ofColor`, `ofVec2f`, `ofRectangle`). Multiple render contexts (main, font bounds, Push2 screen, screenshot).

## Directory Structure

```
Source/              # all C++ source (~387 .h, ~370 .cpp) - flat layout, no subdirectories
  Source/CFMessaging/  # Ableton controller messaging protocol
  Source/cmake/        # CMake helper files
libs/                # vendored third-party libraries
  JUCE/              # audio/UI framework (git submodule)
  pybind11/          # Python C++ bindings (git submodule)
  json/jsoncpp/      # JSON serialization
  nanovg/            # vector graphics rendering
  ableton-link/      # tempo synchronization
  exprtk/            # math expression parsing
  tuning-library/    # microtonal tuning support
  readerwriterqueue/ # lock-free queue
  freeverb/          # reverb algorithm
  push2/             # Ableton Push 2 integration
  xwax/              # vinyl scratching emulation
  oddsound-mts/      # MTS-ESP tuning
  psmove/            # PlayStation Move controller
resource/
  userdata_original/ # default user data (scripts, savestates, controllers, drums, scales)
scripts/             # build and installer scripts
ignore/              # build output (gitignored)
```

## Adding a New Module

1. Create `Source/MyModule.h` and `Source/MyModule.cpp`
2. Inherit from `IDrawableModule` plus the appropriate signal interfaces
3. Implement required static methods and overrides:

```cpp
// MyModule.h
class MyModule : public IAudioProcessor, public IDrawableModule, public IFloatSliderListener
{
public:
   MyModule();
   virtual ~MyModule();
   static IDrawableModule* Create() { return new MyModule(); }
   static bool AcceptsAudio() { return true; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;

   // IAudioSource
   void Process(double time) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   // IFloatSliderListener
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override {}

   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   bool IsEnabled() const override { return mEnabled; }

private:
   void DrawModule() override;

   float mGain{ 1 };
   FloatSlider* mGainSlider{ nullptr };
};
```

4. In `Source/ModuleFactory.cpp`:
   - Add `#include "MyModule.h"` to the include block
   - Add `REGISTER(MyModule, mymodule, kModuleCategory_Audio);` in the constructor

## File Formats

- `.bsk` - binary project save file (faster I/O)
- `.bskt` - text/JSON project save file (human-readable, version-control friendly)
- Controller mappings: JSON files in `resource/userdata_original/controllers/`
- Python scripts: `.py` files in `resource/userdata_original/scripts/`
- Savestates: example patches in `resource/userdata_original/savestate/`

## Platform Macros

- `BESPOKE_MAC` - macOS
- `BESPOKE_WINDOWS` - Windows
- `BESPOKE_LINUX` - Linux

## Key Dependencies

| Library | Location | Purpose |
|---------|----------|---------|
| JUCE | `libs/JUCE/` | Audio engine, MIDI, plugin hosting, GUI windowing |
| pybind11 | `libs/pybind11/` | Python livecoding support (`ScriptModule`) |
| NanoVG | `libs/nanovg/` | 2D vector graphics rendering |
| jsoncpp | `libs/json/jsoncpp/` | JSON serialization for save files |
| Ableton Link | `libs/ableton-link/` | Cross-app tempo synchronization |
| ExprTk | `libs/exprtk/` | Mathematical expression evaluation |
| tuning-library | `libs/tuning-library/` | Microtonal/alternate tuning support |
| readerwriterqueue | `libs/readerwriterqueue/` | Lock-free single-producer/single-consumer queue |
| Python 3.6+ | system | Required at build time and runtime for scripting |

## Resources

- Documentation: https://www.bespokesynth.com/docs/
- Community wiki: https://github.com/BespokeSynth/BespokeSynthDocs/wiki
- Discord: https://discord.gg/YdTMkvvpZZ
- CI: Azure Pipelines (`azure-pipelines.yml`) - builds macOS, Windows, Linux + format checks
