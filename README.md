# SlowBeat

**This is in an early development stage.**

Single purpose speed shifting plugin. Originally free alternative to Gross Beat 1/2 speed effect and HalfTime plugin. Open source alternative to Cymatics DejaVu.

## Setup to use FST

FST (Free Studio Techology): https://git.iem.at/zmoelnig/FST

### Steps

Download the FST repository.

Make a folder directory, `fst\pluginterfaces\vst2.x`.

Copy all of the header files from the `fst` folder in the FST repository, and paste them in the `vst2.x` folder.

Open the `.jucer` file with Projucer, then click on `File -> Global Paths`.

Set the **VST (Legacy) SDK** directory to the `fst` folder that contains the `plugininterfaces\vst2.x` directory.

### Legality

TL;DR: if you plan to distribute builds as "VST" or "VST-compatible" (referring to VST2), you will need to have a VST2 licence. For all intents and purposes, this repository will have **FST** plugin builds.

The legality of using FST to release plugin builds is discussed in the FST repository's README.

Details about compiling and distributing plugins that interact with VST hosts (e.g. LMMS VeSTige) were discussed in various JUCE forum threads below.

https://forum.juce.com/t/using-bona-fida-reversed-engineered-headers-to-distribute-vst2-like-plugins/32795

https://forum.juce.com/t/hosting-vst2-plugins-under-vst3-sdk-license/37079

https://forum.juce.com/t/how-to-offer-vst2-plugins-now/39195
