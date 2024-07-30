# gravity-simulation

This project produces a real-time simulation of four bodies interacting with eacother via the gravitational force. The inital conditions are set up to demonstrate the Lagrange points where low-mass objects and two high-mass objects may all remain stationary in the rotating frame of the center of mass of the system. The simulation demonstrates that some Lagrange points are stable while others are instable.

## Usage

An easy way to run this project is to add all of the files to a Visual Studio project that has been given proper access to the SFML animation library and run the project in Visual Studio.

## Animation

See the video file Pendulum_Anim_Square.mp4 for a demonstratoin of the animation produced. 

## Potential Improvements

-could alter to use the Barnes-Hut algorithm to allow for better framerate with large numbers of particles [reduce time complexity from O(n^2) to O(nlogn)]
-could alter to handle masses colliding
