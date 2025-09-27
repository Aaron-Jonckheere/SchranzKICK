# SchranzKick (VST3)

Een open-source VST3 instrument voor **hard techno/schranz kicks**. Gebouwd met [JUCE], met een snelle pitch-envelope, klikgenerator, distortion chain, 2-bands EQ en eenvoudige limiter. 

> **Belangrijk**: je hebt JUCE 7+ nodig om dit te bouwen. Zie *Build-instructies* hieronder.

## Features
- Monofone kick-synth: body (sine + harmonics) met **exponentiële pitch-envelope**
- **Click layer** (noise burst) met HPF en ultra-korte decay
- **Dubbele distortion** (tanh + hard clip) + optionele **bitcrush**
- **2× oversampling** voor de drive-sectie
- **Low-shelf + peaking EQ** (juce::dsp)
- **Soft limiter** op de uitgang
- **Tuned**: MIDI note of vaste basisfrequentie, selecteerbaar
- Gebruiksvriendelijke UI met essentiële knoppen/sliders

## Parameters (kort)
- *Tune mode* (MIDI / Fixed), *Base Hz*, *Start Ratio*, *Pitch Decay (ms)*, *Pitch Curve*
- *Length (ms)*, *Amp Decay (ms)*, *Harmonics*, *Shape*
- *Click Level*, *Click Length (ms)*, *Click HPF (Hz)*
- *Drive 1*, *Drive 2*, *BitDepth*
- *EQ LowShelf Freq/Gain*, *EQ Peak Freq/Q/Gain*
- *Oversampling* (Off/2x), *Limiter*, *Output*

## Build-instructies (CMake)
1. **Installeer JUCE 7+** en zorg dat `JUCE_DIR` wijst naar de map met `JUCEConfig.cmake`.
2. Configureer & build:

```bash
# Windows (MSVC)
cmake -S . -B build -DJUCE_DIR="C:/SDKs/JUCE" -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release

# macOS (Xcode)
cmake -S . -B build -G Xcode -DJUCE_DIR=~/SDKs/JUCE -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

3. **Installeer de VST3**: kopieer het gebouwde `SchranzKick.vst3` naar je VST3-map.
   - Windows: `C:\\Program Files\\Common Files\\VST3`
   - macOS: `~/Library/Audio/Plug-Ins/VST3`
4. **FL Studio**: Open **Plugin Manager** → **Find plugins**. Voeg hem toe en markeer als favoriet.

## Licentie
MIT (zie LICENSE). JUCE valt onder de JUCE-licentie.

