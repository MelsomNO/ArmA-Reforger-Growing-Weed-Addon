# Growing Weed

A self-contained cannabis growing system for Arma Reforger, built on top of the Cannabis Sativa asset pack. Plant, tend, and harvest — no economy hooks, no roleplay dependencies, just a clean growing loop that fits into any scenario.

## Features

- **Pickupable empty pot** — Carry it in a backpack, drop it anywhere, plant it with a shovel (vanilla dig animation).
- **In-game time growth** — Plants advance through three visual stages using the game's actual day/night clock. Fast-forwarding time in Game Master makes plants grow immediately; slowing the day extends the cycle.
- **Live progress check** — Look at any growing plant to see its exact growth percentage; a "Check Growth" action pops a hint with remaining in-game hours.
- **Harvest with auto-replant** — Mature plants yield cannabis-tops and immediately reset to a fresh seedling, keeping the cycle running.
- **Uproot with shovel** — Any stage can be removed with the shovel demolition animation; the bucket returns to your inventory as an empty pot.
- **Shovel-gated actions** — Both plant and uproot require any entrenching-tool-type item in hands (default patterns match the vanilla US ETool and Soviet MPL50). Configurable per-prefab if you use a shovel mod with different naming.

## How to play

1. Spawn or pick up an **Empty Plant Pot** (via Game Master).
2. Equip a shovel (ETool / entrenching tool).
3. Drop the pot on the ground, walk up to it, hold **Plant with Shovel** — character digs, seedling appears, growth begins.
4. Check on it any time with **Check Growth**.
5. When mature, **Harvest Plant** yields three cannabis-tops and the pot immediately restarts the cycle.
6. **Uproot Plant** at any stage to remove the plant and get the empty pot back.

## Tuning

Everything is exposed as prefab attributes — no code edits needed:

- **Growth duration** — `m_fGrowInGameHours` on each stage's `GW_GrowthComponent` (defaults: 6 h → Stage 2, 8 h → Stage 3).
- **Harvest yield** — `m_iYield` on the mature plant's `GW_HarvestAction` (default 3).
- **Shovel matching** — `m_sShovelNamePatterns` on plant/uproot actions; semicolon-separated substring list (default `shovel;etool;entrench;spade`).
- **Action durations** — `Duration` on each user action.

## Dependencies

- **Cannabis Sativa** by kuka- — provides the bucket, plant, and cannabis-top models.
- **Arma Reforger Data** (base game).

Both are pulled in automatically; no configuration required.

## Credits

- Models & textures: kuka- ("Cannabis Sativa" workshop mod)
- Original weed-growing concept inspiration: KryBoxGaming ("Weed System")
- Everything else in this mod: rebuilt from scratch with no economy dependencies

## License

Arma Public License (APL). Do what you want with it, credit the original asset authors.
