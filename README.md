# PCBConverter
Util for converting PCB from gerber format to gcode for CNC machines. Tested with KiCAD gerber output. But should work with other software.

![](demo1.png?raw=true)
![](demo2.png?raw=true)

# Features
Util support 4 methods for DIY PCB making technics:
* Milling - engrave PCB by milling workpiece cooper layer with CNC machine and milling tool in it. Util will generate path around all elements of PCB. 
* Printing - if your CNC machine has pen with special protective ink, you can plot PCB on workpiece and then make real board with chemical methods.
* 3D Printing - the idea was to cover workpiece with plastic from 3D printer. It can be special electricity conductive material or just protective layer for chemical methods. Actually, it was never tested.
* Laser photoresist - cover workpiece with photoresist film and use 405nm laser to paint a PCB on it.

# Usage
For example, to engrave PCB board on your CNC engraving machinem you need to:  
* export PCB in gerber format (for KiCAD: open Pcbnew, "File"->"Plot", choose cooprom layers and generate drills too)
* open gerbers in util
* choose "Method" - "Milling" and other options
* click on generate to see preview
* click on "Write to file" to save it in gcode file
* run this file on your CNC machine

# Preview window usage
Left click - select element on screen  
Left hold and move - browse PCB  
Right click - change layer  
Middle button - show/hide generated route  
Wheel - zoom  
