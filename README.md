Name: Jack Fraley
Date: 10/23/24

  This application is my attempt at coding minecraft in c and c++.


Date: 10/30/24

  I have been able to render 9 full cubes in openGl. I used a texture map to allow me to render all the textures from the same file to reduce rendering speeds. As of right now my code is rendering inside faces as well as repeated vertices. This will significantly effect performance so I plan to fix this as soon as possible. I am going to use indices rather than individual vertices to prevent reused vertices from needing to be rendered. I will then run a test to see if a face that is being rendered is next to another face, and if so I will clear that face to prevent it from rendering. By using these strategies I will be able to lower the total number of rendered vertices for this layout from 324 to 89 vertices which is a 72.5% improvement in rendering time!!
  ![First Successful Render](9x9GridRender.png)

  This image shows the render in wireframe mode which shows all the vertices that dont actually need to be rendered.
  ![Wireframe render showing uneccesary faces and vertices](9x9WireframeRender.png)

Date: 12/18/24

  A lot as changed since my last update. The world now is able to generate infinitely by rendering 25 chunks in a 5x5 grid around the player. As the player moves it changes which chunks it generates by selecting the nearby chunks from a quadrant chunk array. Each chunk is 16x16x32 and has a random height map to create some terrain generation. I will need to improve this terrain generation in the future as well as fixing some world generation bugs. I have also added the ability to place and delete a block. I do this by using a Moller-Trumbore ray marching algorithm to calculate which block the user is looking at. This was a bit more difficult than anticiapted but I have it completely working now. Lastly, I have added the ability to place different types of blocks. Each block has its own row in the texture map and I just select the different textures by multiplying its block type number by 0.25 for the lower bounds and then adding 0.25 to that to get the upper bounds. Right now I have dirt on the fourth row (index 3) and stone on the third row (index 2). I also have the world generating three blocks of dirt with the remaining blocks being stone to create somewhat realistic terrain. I will follow up with some terrain generation improvements shortly. 
