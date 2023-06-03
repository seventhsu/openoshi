# open-oshi
A fully open-source, DIY idol concert lightstick. (v1.0)

Whether in the midst of the crowd at Belluna Dome or sitting in the dark in your bedroom, cheer on your favorite idols with this fully customizable lightstick. Designed in the image of the King Blade X10, ubiquitous in Japan, this lightstick runs off of a single lithium-ion battery to shine for hours of your favorite songs.

OpenOshi is fully DIY: it consists of a 3D-printable handle and screw-in translucent blade, a central PCB with an RGBW LED, and a simple board programming interface (AVR 6-pin ICSP) to add your favorite idols' colors. OpenOshi is open-source, too; all are free to modify it to their liking or submit improvements.

**Building tips**
- For the handle and cap, I suggest a neutral color of PLA filament, and for the tube, transparent PETG printed at 100% rectilinear infill.
- The PCB is made to OSH Park specifications and tolerances as of June 2023. Modify as needed to meet your preferred PCB supplier's capabilities.
- Control software written in AVR C is provided, and can be flashed onto the board using avrdude.
- The handle was originally designed with a single lithium-ion battery as a power source in mind, so there is a cutout in the bottom of the handle for a common TP4056 USB-C lithium-ion charger. However, the board can also be powered by three alkaline batteries (such as AAAs) in series for 4.5V, so the cutout can be patched up if deemed unnecessary.

![image](https://github.com/seventhsu/open-oshi/assets/63973217/be688b74-c05e-496e-a8b8-6f73533c6004)
![image](https://github.com/seventhsu/open-oshi/assets/63973217/5dab5763-5ceb-48ac-a18f-744e79fa537c)
