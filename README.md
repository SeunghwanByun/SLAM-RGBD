# Astra-Drone-SLAM

This repository will include source code about drone with indoor SLAM algorithm using astra-depth-camera.

This code flow will be like under explanation.

1. First, sensor module will be connect with variable sensors. And send data buffer to the logging module. So, thread will be executed in infinite while loop.

2. Second, logging module receive that data buffer and toss it algorithm module.

3. Third, after algorithm will be exectued, then send some sort of result of algorithms like slam, etc.. to the viewer module.

4. Finally, the viewer module will be send the result including raw data. And the viewer module will visualize that results in GUI using OpenGL.