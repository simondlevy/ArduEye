This repository contains Aruduino libraries for using the Stonyman2 vision chip from Centeye.
Most of this code was adapted from the [ArduEye repositories](https://github.com/ArduEye), following
current Arduino practices.

The <b>examples</b> folder contains the following sketches:
<ul>
<li> <b>Tester</b>: simple serial-monitor interaction, providing Matlab-formatted output
<li> <b>GUI</b>: works with the GUI program in <b>extras/processing</b> to display live streaming images
<li> <b>Flow</b>: works with the GUI program in <b>extras/processing</b> to display optical flow
</ul>

The <b>extras</b> folder contains:
<ul>
<li> <b>docs</b>: the most recent public documentation on the Stonyman and Hawksbill chips
<li> <b>processing</b>: a GUI for interacting with the Stonyman2 chip, written in 
<a href="https://processing.org/">Processing</a>
<li> <b>python</b> a Python program <b>snapshot.py</b> that works with the example in <b>examples/Tester</b>
<li> <b>standalone</b> standalone C++ programs (not requiring Stonyman) for optical flow and ASCII imaging
</ul>

