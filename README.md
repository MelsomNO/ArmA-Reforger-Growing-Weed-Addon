# Growing Weed

A self-contained cannabis growing system for Arma Reforger. Plant seeds, tend the plant through visible growth stages, harvest the buds — no economy hooks, no roleplay dependencies, just a clean growing loop that fits into any scenario.

## Features

- **Two species with distinct visuals** — Sativa (tall, lanky, green buds) and Indica (shorter, bushier, purple-tinged buds), each with its own seed item.
- **Full plant-to-harvest loop** — Empty pot → fill with dirt → plant a seed → wait through growth stages → harvest mature buds → replant. Each step is a proper in-world action, not a menu.
- **In-game time growth** — Plants advance through three stages using the game's day/night clock. Fast-forwarding time in Game Master makes plants grow instantly; slowing the day stretches the cycle.
- **Live progress check** — Look at any growing plant to see its exact growth percentage; a "Check Growth" action pops a hint with remaining in-game hours.
- **Uproot with shovel** — Any stage can be removed with the shovel demolition animation; the plant is destroyed.
- **Shovel-gated actions** — Filling the pot and planting seeds require an entrenching-tool-type item in hands (default patterns match the vanilla US ETool and Soviet MPL50). Configurable per prefab if you use a shovel mod with different naming.
- **Bud visuals on mature plants** — Stage 3 plants have 14 scattered buds hanging from branches, visually distinct from the vegetative stage so players can tell at a glance when a plant is ready.
- **Available in every arsenal** — Empty pots, both seed packs, and both cannabis top items are pre-wired into the USSR, US, FIA, and CIV faction entity catalogs, so players can pull them from any vanilla arsenal without extra setup.

## How to play

1. Pick up an **Empty Plant Pot** from any arsenal (or spawn one via Game Master).
2. Drop the pot on the ground, equip a shovel, and use **Fill with Dirt** — pot becomes a filled planter.
3. Have **Sativa Seeds** or **Indica Seeds** in your inventory. With a shovel still in hands, use **Plant Sativa Seed** / **Plant Indica Seed** on the filled pot — one seed is consumed, a seedling appears.
4. Check progress at any time with **Check Growth**.
5. When mature, **Harvest Cannabis** yields 2–4 species-specific cannabis tops into your inventory; the plant is replaced by a fresh filled pot ready for the next seed.
6. **Uproot Plant** at any stage removes the plant entirely (destroys the pot too).

## Tuning

Everything is exposed as prefab attributes — no code edits needed:

- **Growth duration** — `m_fGrowInGameHours` on each stage's `GW_GrowthComponent` (defaults: 6 h → Stage 2, 8 h → Stage 3).
- **Harvest yield range** — `m_iYieldMin` / `m_iYieldMax` on the Stage 3 prefab's `GW_HarvestAction` (defaults: 2–4).
- **Species-specific yields** — `m_rYieldPrefab` on each Stage 3 `GW_HarvestAction` points to the matching `GW_CannabisTop_*` item.
- **Shovel matching** — `m_sShovelNamePatterns` on plant/uproot/fill actions; semicolon-separated substring list (default `shovel;etool;entrench;spade`).
- **Action durations** — `Duration` on each user action (defaults: 3 s for shovel actions, 2 s for harvest).

## Dependencies

- **Arma Reforger Data** (base game).

Fully self-contained — no other mods required.

## Credits

- **Cannabis plant meshes and textures (all growth stages, both species)** — "Cannabis Growth Stages Pack — Sativa and Indica" from CGTrader (Royalty Free license).
- **Terracotta clay pot mesh and textures** — from a "Terracotta Pots" pack on Sketchfab.
- **Cannabis bud / nugget mesh and textures** — "Weed Nug" (https://skfb.ly/pvnnT) by Skyler Young, licensed under Creative Commons Attribution 4.0 (http://creativecommons.org/licenses/by/4.0/).
- **Scripts and integration** — Melsom.
