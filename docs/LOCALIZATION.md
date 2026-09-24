# Localization

Draft string table for the Growing Weed addon. Spanish is **Latin American** wording, placed in
the `es_es` column (see note at the bottom).

Key shape follows vanilla's convention (`#AR-Item_Morphine_Name`), with `GW` as the mod tag.

## Items

| ID | en_us | es_es |
|---|---|---|
| `GW-Item_IndicaBud_Name` | Indica Bud | Cogollo Indica |
| `GW-Item_IndicaBud_Desc` | An Indica cannabis nugget. | Un cogollo de cannabis Indica. |
| `GW-Item_SativaBud_Name` | Sativa Bud | Cogollo Sativa |
| `GW-Item_SativaBud_Desc` | A Sativa cannabis nugget. | Un cogollo de cannabis Sativa. |
| `GW-Item_IndicaSeeds_Name` | Indica Seeds | Semillas Indica |
| `GW-Item_IndicaSeeds_Desc` | A pack of cannabis Indica seeds. | Un paquete de semillas de cannabis Indica. |
| `GW-Item_SativaSeeds_Name` | Sativa Seeds | Semillas Sativa |
| `GW-Item_SativaSeeds_Desc` | A pack of cannabis Sativa seeds. | Un paquete de semillas de cannabis Sativa. |
| `GW-Item_PotEmpty_Name` | Empty Plant Pot | Maceta vacía |
| `GW-Item_PotEmpty_Desc` | A pot for your plants. | Una maceta para tus plantas. |
| `GW-Item_PotFilled_Name` | Filled Plant Pot | Maceta con tierra |
| `GW-Item_PotFilled_Desc` | A dirt-filled pot ready for a cannabis seed. | Una maceta con tierra lista para una semilla de cannabis. |

## Plants

| ID | en_us | es_es |
|---|---|---|
| `GW-Plant_IndicaSeedling_Name` | Cannabis Seedling (Indica) | Plántula de cannabis (Indica) |
| `GW-Plant_IndicaSeedling_Desc` | A newly planted Indica seedling. | Una plántula Indica recién plantada. |
| `GW-Plant_SativaSeedling_Name` | Cannabis Seedling (Sativa) | Plántula de cannabis (Sativa) |
| `GW-Plant_SativaSeedling_Desc` | A newly planted Sativa seedling. | Una plántula Sativa recién plantada. |
| `GW-Plant_IndicaVeg_Name` | Cannabis Plant (Indica, Vegetative) | Planta de cannabis (Indica, vegetativa) |
| `GW-Plant_IndicaVeg_Desc` | An Indica cannabis plant in the vegetative growth stage. | Una planta de cannabis Indica en etapa de crecimiento vegetativo. |
| `GW-Plant_SativaVeg_Name` | Cannabis Plant (Sativa, Vegetative) | Planta de cannabis (Sativa, vegetativa) |
| `GW-Plant_SativaVeg_Desc` | A Sativa cannabis plant in the vegetative growth stage. | Una planta de cannabis Sativa en etapa de crecimiento vegetativo. |
| `GW-Plant_IndicaMature_Name` | Cannabis Plant (Indica, Mature) | Planta de cannabis (Indica, madura) |
| `GW-Plant_IndicaMature_Desc` | A mature Indica cannabis plant, ready to harvest. | Una planta de cannabis Indica madura, lista para cosechar. |
| `GW-Plant_SativaMature_Name` | Cannabis Plant (Sativa, Mature) | Planta de cannabis (Sativa, madura) |
| `GW-Plant_SativaMature_Desc` | A mature Sativa cannabis plant, ready to harvest. | Una planta de cannabis Sativa madura, lista para cosechar. |

## Interaction context labels

| ID | en_us | es_es |
|---|---|---|
| `GW-Context_CannabisPlant` | Cannabis Plant | Planta de cannabis |
| `GW-Context_ClayPot` | Clay Pot | Maceta de barro |
| `GW-Context_CannabisTopIndica` | Cannabis Top — Indica | Cogollo de cannabis — Indica |

## Actions

| ID | en_us | es_es |
|---|---|---|
| `GW-Action_FillWithDirt` | Fill with Dirt | Llenar con tierra |
| `GW-Action_PlantIndicaSeed` | Plant Indica Seed | Plantar semilla Indica |
| `GW-Action_PlantSativaSeed` | Plant Sativa Seed | Plantar semilla Sativa |
| `GW-Action_UprootPlant` | Uproot Plant | Arrancar planta |
| `GW-Action_HarvestCannabis` | Harvest Cannabis | Cosechar cannabis |
| `GW-Action_EmptyPot` | Empty Pot | Vaciar maceta |

## Script-built strings

Built at runtime in `Scripts/Game/GrowingWeed/GW_CheckGrowthAction.c`, so these need format
placeholders rather than plain lookups.

| ID | en_us | es_es |
|---|---|---|
| `GW-Growth_FullyGrown` | Growth: fully grown | Crecimiento: completamente crecida |
| `GW-Growth_Percent` | Growth: %1% | Crecimiento: %1 % |

## Notes

- **Dialect vs. column.** `es_es` is Reforger's European Spanish column; the wording above is
  Latin American by request. Chosen accordingly: *maceta* (not *tiesto*), *cogollo* for bud,
  *plántula* for seedling, *arrancar* for uproot. If a `es_la` / `es_419` column is available in
  the String Editor, that is the more accurate home for these.
- **Spanish spacing.** `Crecimiento: %1 %` keeps a space before the percent sign, which is
  correct in Spanish typography, unlike English.
- **Percent escaping.** Enter the percent sign as a single `%` in the String Editor, not `%%`.
  The doubling in the old code was `string.Format` escaping; the strings now go through
  `WidgetManager.Translate`, where `%1` is the parameter and a bare `%` is a literal.
- **The pot strings are NOT redundant — do not merge them.** Four look similar but mean
  different things, and each needs its own key:
  - `GW-Item_PotEmpty_Name` "Empty Plant Pot" — noun, the empty pot item.
  - `GW-Item_PotFilled_Name` "Filled Plant Pot" — noun, the filled pot item.
  - `GW-Context_ClayPot` "Clay Pot" — noun, the interaction context label on the filled pot.
  - `GW-Action_EmptyPot` "Empty Pot" — **a verb**: the `GW_UprootAction` that empties a filled
    pot back to an empty one. Translated "Vaciar maceta", not "Maceta vacía". Merging this with
    any of the nouns above would turn an action into a label.
- **English fixes folded in.** The current prefab text reads "A Indica Cannabis Nugget." —
  corrected to "An Indica cannabis nugget." above, with the mid-sentence capitals dropped for
  consistency with the other descriptions. Applying these means the English changes too.
- **Not covered here.** The Workshop listing (name, description, tags) and `README.md` are
  separate from the string table and are localized on the Workshop page itself.
- **Proofreading.** Worth a native speaker's pass before publishing, especially the cannabis
  terms, which carry strong regional connotations.
