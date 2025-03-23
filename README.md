## 3D First-Person shooter game
###  Made with raylib (https://github.com/raysan5/raylib)

---------------------

> [!WARNING]
> The game is not ready and __may__ run poorly because it is not yet optimized well.
> Also 'res/shaders/powerup_shop_bg.fs' shader can be really slow on older hardware(because of the raymarching algorithm). it is in the todo list to add option to disable it
> Try changing render settings in __src/terrain.h__ if you experience performance issues

---------------------


![image](https://github.com/331uw13/3D-Game/blob/main/screenshots/screenshot-32538.png?raw=true)

![image](https://github.com/331uw13/3D-Game/blob/main/screenshots/screenshot-47349.png?raw=true)

* Gain XP by shooting enemy robots and upgrade your powerups.  (Some powerups are not yet implemented)

* Enemies may drop metal pieces which can be used to fix armor.

* Apples spawn in the world naturally which can be used to heal the player.


### Controls
* 2: Open powerup shop menu.
* TAB: Open inventory
* ESC: Open menu.


### Compiling the game.
> [!NOTE]
> Raylib must be installed on your system.
>
> Follow the instructions from https://github.com/raysan5/raylib/wiki/Working-on-GNU-Linux
```bash
git clone https://github.com/331uw13/3D-Game.git
cd 3D-Game
chmod +x build.sh
./build.sh
```
