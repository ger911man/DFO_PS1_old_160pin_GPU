# 🎮 PS1 Dual-Frequency Oscillator (DFO) Mod
### for 160-Pin GPU Revisions (IC203 - Sony CXD8514Q)
### and 208-Pin GPU Revisions (IC203 - Sony CXD8561*)
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
* 🎛️ **Audio desync** and minor gameplay stuttering because the hardware clock doesn't match the game's intended speed.
---
### The Solution: Dual-Frequency Oscillator
A **DFO mod** solves this permanently at the hardware level. 
It acts as a smart, dynamic clock generator. By monitoring the console's video mode pin (GPU VSYNC pin), the microcontroller automatically switches the master clock frequency on the fly:
* Running an **NTSC game**? The DFO instantly injects a perfect **53.693MHz** signal.
* Running a **PAL game**? The DFO instantly switches to a perfect **53.222MHz** signal.

With a DFO installed, your console outputs a **100% video-standard accurate signal** for both regions, giving you perfect color, frame rates, and compatibility with modern displays!

---

# 🛠️ 🎮 Supported Motherboards & Models 

| PS1 Model | Mainboard Revision | RGB Video Encoder Chip | Notes / DFO Context |
| :--- | :--- | :--- | :--- |
| **SCPH-1000** | PU-7 | **Sony CXA1645M** (IC501) | 🔴 $\color{red}{\textsf{Hard DFO Required}}$ for composite/S-Video cross-region color. |
| **SCPH-1001 / 1002** | PU-8 (Early) | **Sony CXA1645M** (IC501) | 🔴 $\color{red}{\textsf{Hard DFO Required}}$ for composite color. |
| **SCPH-3000 / 3500 / 5000** | PU-8 (Late) | **Sony CXA1645M** (IC501) | 🔴 $\color{red}{\textsf{Hard DFO Required}}$ for composite color. |
| **SCPH-5500 / 5501 / 5502** | PU-18 | **Sony CXA1645M** (IC501) | 🔴 $\color{red}{\textsf{Hard DFO Required}}$ for composite/S-Video color. |
| **SCPH-7000 / 7001 / 7002** | PU-20 | **Sony S_RGB** (CXA2106R) | 🟢 $\color{green}{\textsf{DFO optional.}}$ Decoupled subcarrier clock (outputs PAL60 color fine without a DFO). |
| **SCPH-7500 / 7501 / 7502** | PU-22 | **Sony S_RGB** (CXA2106R) | 🟢 $\color{green}{\textsf{DFO optional.}}$ Decoupled subcarrier clock. DFO only needed for perfect sync on upscalers. |
| **SCPH-9000 / 9001 / 9002** | PU-23 | **Sony S_RGB** or **Rohm BH7236F** | 🟢 $\color{green}{\textsf{DFO optional.}}$ Decoupled subcarrier clock. |
| **SCPH-100 / 101 / 102** | PM-41 / PM-41 (2) | **Sony S_RGB** or **Rohm BH7240AKV** | 🟢 $\color{green}{\textsf{DFO optional.}}$ Decoupled subcarrier clock. |


### Why Do Only Some Models Require a Hard DFO Mod?

The distinction between a mandatory modification and an optional one comes down to how Sony engineered the video encoding circuit across different motherboard generations:

* **The Problem (PU-7, PU-8, PU-18):** 
  Because these early revisions utilize the **CXA1645M** (designated as **IC501**), they tie the color subcarrier frequency directly to the GPU's fixed master clock.
 When you boot an out-of-region game on these stock boards, the GPU successfully changes its display mode (e.g., from PAL to NTSC),
 but the master clock remains wrong for that new mode. The CXA1645M cannot generate the correct color frequency from that mismatched clock signal.
 This is exactly why these specific revisions yield a **Black & White** display over standard AV cables unless you feed them the correct frequency via a DFO mod.

* **The Solution (PU-20 and Later):** 
  Starting with the **PU-20** motherboard (`SCPH-700x` series), Sony replaced the CXA1645M with a newer video encoder layout, such as the integrated proprietary **S_RGB** chip or later A-Video chip setups.
 These later revisions decoupled the color subcarrier from the main master clock entirely. This architectural shift allows them to cleanly output a hybrid "PAL60" color signal,
 completely bypassing the need for a DFO mod just to see color.

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
---
---


> [!NOTE]
> This block will have a blue accent.

> [!IMPORTANT]
> This block will have a purple accent.

> [!WARNING]
> This block will have a red/amber accent line.




This repository contains the firmware and documentation for installing a dual-frequency oscillator (DFO) mod on older revisions of the Sony PlayStation 1 / PSX. 

This specific modification targets consoles equipped with the older **160-pin GPU (IC203 / Sony CXD8514Q)**.
## 🛠️ Supported Motherboards & Models

The 160-pin Sony GPU is found exclusively on **PU-7** and early **PU-8** motherboard revisions. You can typically find these boards inside the following early console models:

* 🇯🇵 **SCPH-1000**
* 🌍 **SCPH-3000**
* 🇺🇸/🇪🇺 **SCPH-5000**

> [!TIP]
> Always open your console and verify your motherboard revision (`PU-7` or `PU-8` printed on the board) and GPU chip number before flashing or soldering, as Sony mixed and matched components during these early production runs.
