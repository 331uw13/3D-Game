-----------------------------------------
## How to create new projectile modifier
> Projectile modifiers can change player's projectile behaviour
* `new-prjmod.sh` creates source files for empty projectile mod. It expects name for argument. Or create the files by yourself
* The source file could look something like this:
```c
#include "prjmod_newmod.h"
#include "../state.h"

void prjmod_newmod__init(struct state_t* gst, struct psystem_t* psys, struct particle_t* part) {
  // Called when the projectile is initialized.
}

void prjmod_newmod__update(struct state_t* gst, struct psystem_t* psys, struct particle_t* part) {
  // Called every frame when projectile receives an update.
}

void prjmod_newmod__env_hit(struct state_t* gst, struct psystem_t* psys, struct particle_t* part, Vector3 normal) {
  // Called when projectile hits the terrain.
}

int prjmod_newmod__enemy_hit(struct state_t* gst, struct psystem_t* psys, struct particle_t* part,
        struct enemy_t* ent, struct hitbox_t* hitbox, int* cancel_defdamage) {
  // 'cancel_defamage' can be set to positive number if you want to handle the damage from here.
  // 'enemy_damgae' function could be used see 'src/enemy.c'
    return 0; // If zero is returned the projectile will not be disabled after hit.
}
```
* Add the new projectile modifier id into `src/projectile_mod/prjmod.h`
  It should have an unique id between(max id that already exists) and `MAX_PRJMOD_INDICES` defined in `src/player.h`
* You can then activate the new projectile mod like this:
  ```c
    struct prjmod_t newmod = (struct prjmod_t) {
      .init_callback = prjmod_newmod__init,
      .update_callback = prjmod_newmod__update,
      .enemy_hit_callback = prjmod_newmod__enemy_hit,
      .env_hit_callback = prjmod_newmod__env_hit
    };
    add_prjmod(gst, &newmod, PRJMOD_NEWMOD_ID);
  ```
* It can also be disabled if needed:
  ```c
  rem_prjmod(gst, PRJMOD_NEWMOD_ID);
  ```
-----------------------------------------
## How to create new particle system
* Add the new particle system id into `src/state.h`. Remember to increase `MAX_PSYSTEMS` number
* The source file could look something like this:
```c
  #include "new_psys.h"
  #include "../state.h"
  #include "../util.h"
  #include <raymath.h>

  void new_psys_update(struct state_t* gst, struct psystem_t* psys, struct particle_t* part) {
    // ...
    part->position = Vector3Add(Vector3Scale(part->velocity, gst->dt));
    part->transform = MatrixTranslate(part->position.x, part->position.y, part->position.z);
  }

  void new_psys_init(
    struct state_t* gst,
    struct psystem_t* psys,
    struct particle_t* part,
    Vector3 origin,
    Vector3 velocity,
    Color part_color,
    void* extradata, int has_extradata
  ){
    part->lifetime = 0.0;
    part->max_lifetime = RSEEDRANDOMF(0.15, 2.0);
    part->color = (Color){ GetRandomValue(0, 255), GetRandomValue(0, 255), GetRandomValue(0, 255), 255 };
    part->velocity = (Vector3) {
      RSEEDRANDOMF(-20.0, 20.0),
      RSEEDRANDOMF(0.0, 20.0),
      RSEEDRANDOMF(-20.0, 20.0)
    };
  }
```
* Create the particle system (particle systems are created from `src/state.c` when the game starts)
* You can see available `PSYS_GROUPIDs` from `src/psystem.h` or create new one.
* Choose `time_setting`:
> `PSYS_ONESHOT`: Particle lifetime is increased overtime and they will "die" after `particle->max_lifetime` is reached
> `PSYS_CONTINUOUS`:  Ignores the lifetime and keeps updating the particles.
```c
  struct psystem_t* psystem = &gst->psystems[NEW_PSYS];
  create_psystem(
    gst,
    PSYS_GROUPID_ENV,
    PSYS_CONTINUOUS,
    psystem,
    64,  // Number of maximum particles.
    new_psys_update,
    new_psys_init,
    EXPLOSION_PSYS_SHADER // Shader index for the particle system. (index in 'gst->shaders' array)
  );
```
* You can enable `color vbo` if you need different colors for each particle
* particle color can be then changed just by editing `particle->color`
```c
  setup_psystem_color_vbo(gst, &gst->psystems[NEW_PSYS]);
```

* Add particles into the system:
```c
  add_particles(
    gst,
    &gst->psystems[NEW_PSYS],
    8, // Number of particles to add.
    (Vector3){ 0, 0, 0 }, // Origin.
    (Vector3){ 0, 0, 0 }, // Velocity, usually its updated from init_callback
    (Color){ 50, 255, 20, 255 }, // Color can be passed into init callback
    extradata_ptr,    // Pointer to extradata for all particles that are going to be added
    has_extradata,
    NO_IDB // idb here stands for "behaviour id". It can be used to identify particles that have different behaviour inside the same particle system.
  );
```
> `add_particles` will call `psystem->init_callback` for each particle.

> If `extradata_ptr` NULL then `has_extradata` should be set to `NO_EXTRADATA` or `HAS_EXTRADATA` if the pointer is used.

* Render the particle system from `src/state_render.c`.
* It should be rendered after `render_scene(gst, RENDERPASS_RESULT);`
```c
  // NOTE: Color here is not used if color vbo is enabled.
  render_psystem(gst, &gst->psystems[NEW_PSYS], (Color){ 0 });
```
* Update the particle system from `src/state.c` `state_update_frame()` function
```c
  update_psystem(gst, &gst->psystems[NEW_PSYS]);
```
-----------------------------------------
## How to add new enemies
(todo)
