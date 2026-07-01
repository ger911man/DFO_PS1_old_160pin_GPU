# 🎮 PSX Dual-Frequency Oscillator (DFO) Mod
## Project Overview
This project was specifically created to fix a critical hardware conflict encountered when installing both a **DFO (Dual Frequency Oscillator)** and a **PicoStation ODE (Optical Drive Emulator)** on early Sony PlayStation 1 motherboards: **PU-7 and early PU-8 revisions** equipped with the older **160-pin GPU**. 
While born out of a need to resolve issues on these specific early revisions, this DFO mod approach is fully applicable to all other revisions of the PS1. In fact, it is **highly recommended** if your goal is to achieve a truly region-free, multi-regional console with perfect video sync and color encoding across both PAL and NTSC regions.

---
# ❓ Why is a DFO Mod Needed?
The original PlayStation console was designed to run at different speeds depending on the region it was sold in:
* **NTSC Regions (US / Japan):** Runs at **60Hz** video refresh rate (utilizing a **53.693MHz** master clock).
* **PAL Regions (Europe):** Runs at **50Hz** video refresh rate (utilizing a **53.222MHz** master clock).
### The Problem: Region-Unlocking Isn't Enough
When you install a standard modchip or use software to force a PAL console to play an NTSC game (or vice versa), the console's video chip changes the software refresh rate to 60Hz. 
However, because the motherboard only has **one single crystal oscillator fixed to that region's native frequency**, the video signal output becomes slightly off-spec ($59.2Hz$ instead of true $59.94Hz$). 

**This results in:**
* 📺 **Black and white video** on many CRT televisions.
* 🚫 **No signal or "Unsupported Format"** errors on modern LCD/OLED TVs and upscalers (like the RetroTINK or OSSC).
* 🎵 **Audio desync** and minor gameplay stuttering because the hardware clock doesn't match the game's intended speed.
---
### The Solution: Dual-Frequency Oscillator
A **DFO mod** solves this permanently at the hardware level. 
It acts as a smart, dynamic clock generator. By monitoring the console's video mode pin (GPU VSYNC pin), the microcontroller automatically switches the master clock frequency on the fly:
* Running an **NTSC game**? The DFO instantly injects a perfect **53.693MHz** signal.
* Running a **PAL game**? The DFO instantly switches to a perfect **53.222MHz** signal.

With a DFO installed, your console outputs a **100% video-standard accurate signal** for both regions, giving you perfect color, frame rates, and compatibility with modern displays!

---

# Why this matters for your DFO installation:
Because these specific revisions use the CXA1645M as IC501, they tie the color subcarrier frequency directly to the GPU's fixed master clock.
When you boot an out-of-region game on these boards, the GPU changes its display mode (e.g., from PAL to NTSC), but the master clock remains wrong for that new mode. 
The CXA1645M cannot generate the correct color frequency from that mismatched clock signal, which is exactly why these specific revisions display out-of-region games in black and white over standard AV cables unless you feed them the correct frequency via a DFO mod

Starting with the PU-20 motherboard (SCPH-700x series), Sony replaced the CXA1645M with a newer video encoder layout (such as the integrated S_RGB chip or the later A-Video chip setups). These later revisions decoupled the color subcarrier from the main master clock, allowing them to cleanly output a hybrid "PAL60" color signal without requiring a DFO just to see color. 

> ⚠️ **If you decide to skip installing a DFO** on a PU-20 or newer motherboard revision, you won't suffer from a black-and-white screen, but you will still have to live with a few technical compromises.
> Here is the list of consequences when running out-of-region games without a DFO on newer revisions:
> 1. Frame-Pacing Micro-Stutter (Judder)
> Because the console's main master clock remains locked to its native hardware frequency, running an out-of-region game forces the GPU to output video at an off-spec refresh rate.
> For example, running an NTSC game on a stock PAL console outputs at roughly 59.27 Hz instead of the native NTSC standard 59.94 Hz.
> This slight mismatch causes a frame-pacing error, manifesting as subtle micro-stuttering or judder, which is most noticeable during smooth, continuous camera panning (like in side-scrollers, racing games, or RPGs).
>
> 2. Signal Drops and Sync Issues on Picky Upscalers
> Modern flat-screen TVs and video upscalers (like the OSSC, RetroTink, or Framemeister) are highly sensitive to video signal timing.
> Because a non-DFO console outputs a slightly out-of-spec signal, picky upscalers can struggle to maintain a stable sync lock.
> This can result in intermittent black screens (signal dropouts), image tearing, or the upscaler refusing to display the image altogether when switching between native and out-of-region titles.
>
> 3. Audio Pitch and Speed Deviations
> The console's audio processing unit (SPU) relies on timings derived from the central clock system. When the GPU runs a game at an off-spec refresh rate to compensate for the missing dual oscillator, 
> the audio playback speed and pitch can drift slightly out of synchronization with the video, or run minutely faster/slower than originally intended by the developers.
> 4. Severe Compatibility Issues with CRT Displays
> If you ever play on a vintage CRT monitor or television, many models (especially older European PAL TVs) cannot properly process a hybrid "PAL60" or non-standard NTSC signal.
> Without a DFO to normalize the master clock, you may encounter rolling screens, vertical sync collapse, or a complete loss of color on analog tubes.

---

# 🛠️ 🎮 Supported Motherboards & Models 

| PS1 Model | Mainboard Revision | RGB Video Encoder Chip | Notes / DFO Context |
| :--- | :--- | :--- | :--- |
| **SCPH-1000** | PU-7 | **Sony CXA1645M** (IC501) | 🔴 DFO is recommended for correct composite/S-Video cross-region color. |
| **SCPH-1001 / 1002** | PU-8 (Early) | **Sony CXA1645M** (IC501) | 🔴 DFO is recommended.  -\\- |
| **SCPH-3000 / 3500 / 5000** | PU-8 (Late) | **Sony CXA1645M** (IC501) | 🔴 DFO is recommended.  -\\- |
| **SCPH-5500 / 5501 / 5502** | PU-18 | **Sony CXA1645M** (IC501) | 🔴 DFO is recommended.  -\\- |
| **SCPH-7000 / 7001 / 7002** | PU-20 | **Sony S_RGB** (CXA2106R) | 🟢 The DFO is optional (with a decoupled subcarrier clock, it outputs PAL60 color fine without a DFO), but skipping it may result in the issues listed above. |
| **SCPH-7500 / 7501 / 7502** | PU-22 | **Sony S_RGB** (CXA2106R) | 🟢 The DFO is optional. -\\- |
| **SCPH-9000 / 9001 / 9002** | PU-23 | **Sony S_RGB** or **Rohm BH7236F** | 🟢 The DFO is optional. -\\- |
| **SCPH-100 / 101 / 102** | PM-41 / PM-41 (2) | **Sony S_RGB** or **Rohm BH7240AKV** | 🟢 The DFO is optional. -\\- |

---

# 🛠️ Required Hardware

* **Si5351:** An I2C-configurable clock generator.
* **ATtiny13A:** An AVR microcontroller, preferably in a SOIC package for compact "dead-bug" soldering.
* **220Ω resistor:** An SMD version is preferred to keep the setup compact.
* **AVR Programmer:** Required to flash the firmware onto the ATtiny13A. For a budget setup, use an **Arduino configured as an ISP** or a **USBasp**.
* **Soldering tools & accessories:** fine-tip soldering iron, solder, flux, and thin wrapping wire.

---
  
# Setup example early PU-8 
![Project Screenshot](/images/diagram_DFO_PU-8_early.jpg)


---
---
---


> [!NOTE]
> This block will have a blue accent.

> [!IMPORTANT]
> This block will have a purple accent.

> [!WARNING]
> This block will have a red/amber accent line.




* 🇯🇵 **SCPH-1000**
* 🌍 **SCPH-3000**
* 🇺🇸/🇪🇺 **SCPH-5000**

> [!TIP]
> Always open your console and verify your motherboard revision (`PU-7` or `PU-8` printed on the board) and GPU chip number before flashing or soldering, as Sony mixed and matched components during these early production runs.
