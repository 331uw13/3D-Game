## 3D First-Person shooter game
###  Made with raylib (https://github.com/raysan5/raylib)

---------------------

> [!WARNING]
> The game is not ready and __may__ run poorly because it is not yet optimized well.
> Also 'res/shaders/powerup_shop_bg.fs' shader can be really slow on older hardware(because of the raymarching algorithm). It is in the todo list to optimize it
> Try changing render settings from the menu

---------------------


![image](https://github.com/331uw13/3D-Game/blob/main/screenshots/screenshot-51819.png?raw=true)
![image](https://github.com/331uw13/3D-Game/blob/main/screenshots/screenshot-14380.png?raw=true)


* Gain XP by shooting enemy robots and upgrade your powerups.  (Some powerups are not yet implemented)

* Enemies may drop metal pieces which can be used to fix armor.

* Apples spawn in the world naturally which can be used to heal the player.

-------------
### Controls
* 2: Open powerup shop menu.
* ESC: Open menu.
-------------
### Currently Working Powerups
* Accuracy boost
* Faster firerate
* Max health boost
* Max armor boost
* Movement boost
* Damage boost
* Projectile speed boost
* Bigger projectiles
* FMJ Projectile ability (projectiles go trough enemies)
* Gravity projectiles (player also takes damage when fired)
-------------
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
