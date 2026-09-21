# Growing Weed — Workshop Page Description

Grow, tend, and harvest cannabis in Arma Reforger. A full plant-to-harvest gameplay loop that plugs into any scenario — no economy hooks or roleplay dependencies required.

## What it adds

- **Two strains** — Sativa (tall, lanky, green buds) and Indica (shorter, bushier, purple-tinged buds), each with its own seed pack and harvest yield.
- **Full growing loop** — Place an empty pot → fill it with dirt using a shovel → plant a seed → wait for it to grow → harvest the mature buds → replant. Every step is a proper in-world hold-to-perform interaction, not a menu.
- **Live growth timer** — Plants advance through three visible stages driven by the game's in-game clock. Fast-forwarding time in Game Master makes plants grow instantly; a normal day/night cycle stretches the full cycle across in-game days.
- **Visual harvest cue** — Mature plants have 14 scattered cannabis buds hanging from their branches. You can tell at a glance whether a plant is ready to harvest.
- **Shovel-gated actions** — Filling and planting require any entrenching-tool-type item in hands (vanilla US ETool and Soviet MPL50 supported out of the box). Harvest is bare-handed.
- **Available in all arsenals** — Empty pots, both seed packs, and both cannabis top items are pre-wired into the USSR, US, FIA, and CIV faction entity catalogs. Any arsenal in the game lets players stock up.

## How to play

1. Grab an **Empty Plant Pot** from any arsenal (or via Game Master).
2. Grab a shovel — vanilla ETool, MPL50, or any mod-added spade works.
3. Drop the pot on the ground and use **Fill with Dirt** (hold, 3s) — the pot fills with soil.
4. Grab **Sativa Seeds** or **Indica Seeds** from an arsenal. With a shovel still in hand, use **Plant Sativa Seed** / **Plant Indica Seed** on the filled pot — one seed is consumed, a seedling appears.
5. Use **Check Growth** any time to see progress and remaining in-game hours.
6. When mature (Stage 3), use **Harvest Cannabis** — you get 2 to 4 species-specific cannabis tops in your inventory, and the plant is replaced by a filled pot ready for the next seed.
7. **Uproot Plant** at any stage to remove the plant entirely (destroys the pot too).

## Growth timings

Default in-game hours per stage:
- Seedling → Vegetative: **6 in-game hours**
- Vegetative → Mature: **8 in-game hours**

A typical Conflict server with accelerated time can grow a full plant in one game session; a real-time RP server plays out over multiple in-game days.

Growth timings are per-prefab attributes and can be tweaked without editing any code.

## For server admins and modders

- **Self-contained** — the mod's only dependency is the base game. No third-party mods required.
- **Server-authoritative** — all growth and harvest actions run on the server and replicate to clients.
- **Integration friendly** — every gameplay number is a prefab attribute, and there's a documented set of prefabs / components / actions you can reference from other mods (economy, criminality, drug processing, quests). See the integration guide on the mod repo.
- **Two components + eight actions** — the whole gameplay layer is a small, readable Enforce codebase.

## Credits

- Cannabis plant meshes and textures — "Cannabis Growth Stages Pack — Sativa and Indica" from CGTrader (royalty-free license).
- Terracotta clay pot — "Terracotta Pots" pack from Sketchfab.
- Cannabis bud mesh — "Weed Nug" (https://skfb.ly/pvnnT) by Skyler Young, licensed under Creative Commons Attribution 4.0 (http://creativecommons.org/licenses/by/4.0/).
- Scripts, integration, and packaging — Melsom.

## Support

Report issues, request features, or ask integration questions on the mod repo. Include your Arma Reforger version and any relevant server config when reporting bugs.
