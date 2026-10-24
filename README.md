# SlowBeat

**This is in early development stage.**

Single purpose speed shifting plugin. Originally free alternative to Gross Beat 1/2 speed effect and HalfTime plugin.

## Setup to use FST

FST (Free Studio Techology): https://git.iem.at/zmoelnig/FST

### Steps

Download the **FST** repository.

Make a folder directory, `fst\plugininterfaces\vst2.x`

Copy all header files from the `fst` folder in the FST repository, and paste them in the `vst2.x` folder.

When opening **Projucer**, click on `File -> Global Paths`.

Set the **VST (Legacy) SDK** directory to the `fst` folder that contains the `plugininterfaces\vst2.x` directory.

### Legality

I am not a lawyer.

Legality of using **FST** is discussed in the FST repository readme.

Details about compiling and distributing plugins that interact with **VST hosts** and alternatives (e.g. **LMMS VeSTige**) were discussed in various threads in JUCE forums.

https://forum.juce.com/t/using-bona-fida-reversed-engineered-headers-to-distribute-vst2-like-plugins/32795

https://forum.juce.com/t/hosting-vst2-plugins-under-vst3-sdk-license/37079

https://forum.juce.com/t/how-to-offer-vst2-plugins-now/39195
