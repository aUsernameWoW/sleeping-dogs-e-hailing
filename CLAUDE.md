# SDTaxi — call a taxi from the phone in Sleeping Dogs: Definitive Edition

The user's idea (2026-09-28): an "E-Car Hailing" phone entry. Like the Car Valet bringing a car, but the car is a
taxi, the driver is a taxi driver and he stays in; Wei then gets in as a passenger and the game's own taxi ride
(map, fare, skip) takes over. Local repo only so far (no GitHub repo yet; ask the user before creating one).

Status (2026-09-28): the bridge works in game (compile errors with position, durational blocks finish, popups,
our `Debug.println` and the game's scripts' prints in the log). Test round 2 spawned the taxi (`625MHCTaxi01`,
friendly blip, driving role "Taxi") but no driver: `object-physical-character-drivers-taxi` is only a parent set
(`VehicleDriverTypeList`), the real one is `object-physical-character-TaxiDriverMale` (TwoPartPed, TrueCrowd,
resource tags Male/TaxiDriver/Adult/Asian; `reference\SDmodding\Files\GlobalProperties\*.xml` has the property sets).
A car without a driver is a locked parked car (needs hot-wiring). Round 3 used TaxiDriverMale: **works like an
ambient taxi** (user, 2026-09-28): it drives to the player (46 m in ~10 s, ~5 s of it standing still first), "HIRE
TAXI" (hold E), the fare is taken as Wei opens the door, the map, confirm → black screen → `WarpToDestination`
(logged track: (953,-147) → (292,884)) → ~50 m of driving → arrival. Like the original, E can't get out mid-ride.
Two earlier rides drove the whole way without the warp (first test: F11 pressed 5 times, 5 taxis, one crashed into
the player's; second: unclear, maybe the map was left without confirming); `core/ride.cc` (built, not yet
deployed) logs the transit calls to tell next time. The two differences the user still saw: the "friendly" blip
stays after the ride, and the ~5 s before the taxi moves. Round 4 (console file deployed 2026-09-28 evening)
removes the blip when the player gets in (or after 2 minutes), sets the driver back to
`PedSuspendOption_SuspendAllowed` after the ride, and logs the speed every 0.25 s on the way.

**Phone build verified (2026-09-28 19:08, user: "work as expected")**: contact "Taxi" → call screen, hang up
after 2 s → spawned 58 m away → within 12 m after 5 s → held (AI mode 0) → the player hired it → map →
`WarpToDestination: warped` → ~30 m of driving (mode 2, role 4, then 5, then 0 at arrival) → out → released,
contact back. Nothing committed yet; no GitHub repo (ask the user).

Round 4 (2026-09-28 19:00): the taxi started at once (speed 4 after 0.25 s; the ~5 s wait didn't recur), but
`_path_to_xform(player xform, true)` ended after 2 s 25 m from the player (the nearest road node to a player off
the road); after our single `stop()` (= AI driver mode 0), the car AI drove it on to the old destination as the
player walked up and then drove off. Round 5 (built; console file deployed, phone build not yet): re-aim every
5 s until within 12 m, then `stop()` every 0.5 s until the player sits in it, `wander(true, true)` if nobody got
in, and the taxi's AI driver state logged by `core/ride.cc` (`[SDTaxi:watch]`).

The hire prompt's conditions (`extract.ps1 act node 'InteractHandler/Queries/TaxiAvailable$'`): target
`eTARGET_TYPE_TRANSIT` valid and equal to the targeted vehicle and interactive prop, its driver alive and playing a
`Ride` node, ≤ 3 m (2D) and in front, the player in `Locomotion`. The fare: `TargetPurchaseItemTrack` at 0 ms of
`GlobalActions\SocialActions\Conversation\Initiator\Combined\PerformSocial\Taxi`. `Ride\Taxi\TaxiDriving` needs
the vehicle's `CarMode` GoTo and opens `GetOut\Taxi\TaxiArrival` / `Passenger` after 1.5 s.

Console lessons: `SkookumMgr::ConstructCodeBlockFromScript` wraps the text as `[<text>\n()\n]`, so a block's value
is lost and a lone literal fails ("no side effects" warning is an error); `Debug.println`'s `{Object}` is a group
parameter: write `Debug.println("a", b)` (braces nest a list, which the log now flattens anyway).

## Plan

1. **Spike** (done): run SkookumScript from the .asi (`core/skookum.cc`) and try the script API in game through
   the console (`core/console.cc`).
2. **Dispatch** (done through the console, round 3): a Skookum coroutine modeled on the Car Valet's.
3. **Phone contact** (`core/phone.cc` + `core/taxi.cc`, verified in game 2026-09-28).
4. Later: a root menu item next to Contacts/Messages (`Smartphone_AddMenuItem` + `UIHK_PDAWidget::handleMessage`),
   a taxi portrait, README/ADVANCED, GitHub repo (ask the user), CI.

## How the game does it (installed-build facts from the PDB/IDA)

- **Runtime Skookum**: `UFG::ScriptCache::GetScript(source, classScope, defaultScope, debugName, hash)` constructs
  an `SSParser` and `parse_code_block`s the text (`SkookumMgr::ConstructCodeBlockFromScript` wraps it), cached by
  `qStringHash32(source)` + class; `Script` = {qNodeRB, mRefCount +0x20, mpScriptCode +0x28, mpClassScope +0x30}.
  Used by `SkookumTrack`/`SkookumCondition` constructors (action tree tracks carry script text, e.g.
  `GameplayHelp.show("BUTTON_BUTTON2_HOLD", "$HUD_HIRE_TAXI", "Icon_EnterVehicle")`) and `InterestPoint::InitUsePOI`.
  `SkookumTask::Begin` runs it: `SkookumMgr::RunExternalCodeBlock(code, actor->i_class_p, actor, &finished,
  nullptr, &ASymbol_origin_embedded2)`, the actor being the sim object's TSActor or `SkookumScript::c_world_p`.
  It wraps the code in a temporary `SSMethod` + `SSIExternalMethodCallWrapper` (writes `*finished` when a
  durational block completes; returns null and sets it at once for an immediate one). `SkookumTask::End` only
  clears `i_finished_p`: the script keeps running.
- `SSDebug::mthdc_print`/`println` are folded to an empty function in this build: scripts can't print. Compile and
  runtime errors go through `SSDebug::print_error` → `ADebug::print(const AString&, bool)` (and `AgogErrorOutput::
  determine_choice` → both `ADebug::print`s), which in this build only calls the (empty) print funcs.
- Skookum identifiers are `qStringHash32` of the name (ASymbol ids); class files are `Class[<hash of the class
  name>].skoo-bin` in `Global.big`. Read them with `tools\extract.ps1 skoo decl|body|dis|dump` (workspace
  CLAUDE.md); `dump` puts every class as pseudo-source in `tools\extract\build\skoo\` (grep there for examples).
  Syntax as the game's scripts use it: temporaries declared first (`!a !b`), bound with `a: value`, assigned with
  `a := value`; `if cond [..] else [..]`, `loop [.. exit ..]`, `race [ [..] [..] ]`, casts `x<>Vehicle`, `%` calls
  on a maybe-nil, lists `{a, b}`, symbols `'name'`, `Transform!()` constructs, `.not()` negates.
- **Car Valet** = `Class[7fc2f9c2]`: `CarContact` (GameSlice root), `CCAmbient._gameslice_main` (phone call slice
  `E_CarValetPhoneCall` = `Class[3ccfbed8]`; the PDA contact and the trigger that starts the slice are not in the
  scripts). In short: loop `c_world._find_vehicle_spawn_xform(World.c_player.get_pos(), 40.0, max, true, 0.0001,
  found, spawn_xform, false)` with max 70 → 100 in steps of 10 and `_wait()`; none: `conv_CarValetNo<n>`,
  `PDA.end_phone_call()`, `_fail_gameslice()`; else `PDA.remove_contact('CarContact')`, `conv_CarValetOkay<n>` →
  move the valet spawn points there, `_wait(4.0)` → `carType: SnapshotData.get_id('ValetVehicle')` (default
  `'object-physical-vehicle-270DX01'`) → `c_world.spawn_object_at_xform(spawn_xform, carType,
  "CarContact-sp_carContactCar:spawn")<>Vehicle` → valet `SpawnPoint.spawn(...)`, `minimap_add_blip("friendly",
  false, , "$BLIP_CAPTION_CAR_CONTACT")` → race [dead] [`force_enter_vehicle(car, true)`, `follow(World.c_player,
  false, true)` (= `set_objective_and_actor("eAI_OBJECTIVE_BE_ALLY", player)`: the ally AI drives to the player),
  `set_can_wander(true)`, `enable_script_control(false)`, `car.set_driving_role("Ally")`,
  `World.c_player._wait_near_actor(contact, 2.0)`, `_exit_vehicle()`, `conv_CarValetDropOff<n>`, `_wander(true)`] →
  `complete_gameslice()`. Scripted phone contacts elsewhere: GunBackup (`Class[7f1f8632]`) `PDA.add_contact(...)`
  + `_wait_player_call_pda_contact({...}, result)` (polls `PDA.is_calling_contact`).
- Other scripts worth copying: `Tran._spawn_vehicle` spawns a taxi (`Car.create_at_xform(xform, 'object-physical-
  vehicle-625MHCTaxi01')`) with a driver (`Character.create_at_xform`, `enable_script_control(false)`,
  `set_suspend_option('PedSuspendOption_NoSuspend')`, `force_enter_vehicle(car, true, true)`, `car.wander(true,
  true)`); `S_NotATaxiDriver` (`Class[d70b4dc6]`) seats a ped in the rear of the player's taxi
  (`_enter_vehicle_closest_rear_seat`); `J_TTD_1` / `ScriptTests._test_wandering_car` put the player in as a
  passenger (`force_enter_vehicle(car, false, false)`); `Vehicle.set_scripted_driving_role("Taxi")`; ambulance
  response: `_path_to_xform(pos, true)` raced against `_wait_near_actor(x, 8.0)`, then `stop()`.
- Declarations used so far: `World._find_vehicle_spawn_xform(Vector3 origin, Real min_distance, Real max_distance,
  Boolean off_screen_only, Real timeout, Boolean result, Transform result_transform, Boolean filter_can_use_alley:
  false, Integer token: -1)` (result and result_transform are written into the objects passed);
  `World.spawn_object_at_xform(Transform xform, <PropertySet|Symbol> parent_properties, <String|Symbol|None> name:
  nil, <SceneLayer|Symbol|None> layer: nil) Object` (a Symbol name is used as is, a String made unique, so
  `find_instance` finds a Symbol-named spawn); `Character.create_at_xform(Transform, properties:
  'object-physical-character-ped1', name: nil, behave_type: nil, Boolean script_control: true)`;
  `Character.force_enter_vehicle(<Symbol|Vehicle>, Boolean as_driver, Boolean add_ai: true)`;
  `force_enter_vehicle_seat(vehicle, "eTARGET_TYPE_VEHICLE_PASSENGER[2|3]", add_ai)`; `Vehicle._path_to_xform(
  Transform, Boolean use_road_position: false)`; `Vehicle._stop_at(Transform, ...)`; `Vehicle.stop(Real
  deceleration: 0.0)`; `Actor._wait_near_actor(Actor, Real wait_distance: 1.0)`; `HintText.show_info_popup(String
  caption, Symbol type: 'default', Real duration: 5.0)`; `PDA.add_contact/remove_contact/is_calling_contact/
  end_phone_call/show_outgoing_phone_call(String contactName, Boolean auto: false)`.
- Bindings: `TSWorld::Coro_find_vehicle_spawn_xform` = `SimObjectUtility::FindSpawnVehicleTransformIterated`;
  `TSWorld::get_spawn_info` resolves the properties (PropertySet or `FindPropertySet(symbol)`), the name and the
  layer; `TSVehicle::Mthd_create_ai_driver` = `VehicleUtility::SetAIDriver(vehicle, false)`.
- **Taxis**: vehicles `object-physical-vehicle-625MHCTaxi01` (red), `625MHCTaxiGreen01` (New Territories),
  `...CNY01` variants; drivers `object-physical-character-drivers-taxi`, `...TaxiDriverMale`; property sets
  `Vehicles-TaxiProperties`, `Vehicles-AI_TaxiProperties`, personality `default-component-personality-drivertaxi`.
  The ride is the game's: `Player_behaviour` `InteractHandler\Prompts\Taxi` (hold to hire, `$HUD_HIRE_TAXI`; query
  `TaxiAvailable`, conditions not in the text dump) → `Vehicle\Interactions\Action\GetIn\Player\Taxi\Spawn\AsPassenger`
  → `Ride\Taxi\Passenger/TaxiDriving` → `UIScreenTask` / `TSCharacter::Mthd_force_enter_vehicle` call
  `UIHKScreenWorldMap::SetWorldMapFromVehicle` (class `Taxi` → transit view) → `TransitUtility::OnSelectDestination`
  (sets the AI driver's road destination) → confirm → fade → `PlaceTransitVehicle` (teleport near the destination)
  → `GetOut\Taxi\TaxiArrival`. CarAI: `Wander\Taxi\StopAtStimulus/StopImmediately`, `Stop\Taxi\TaxiWait`.
  `AiDriverComponent::IsTaxi` is read by `CarNoDestinationTask::Begin`.
- **Phone**: root menu built in `Data\UI\Screens\Hud.bin` AS2 `Smartphone_AnimateIntro` with
  `Smartphone_AddMenuItem(img, "$HUD_SMARTPHONE_...")`; `UIHK_PDAWidget::handleMessage` dispatches the selected item
  by its label string (Contacts / Messages / SocialHub / Bios; an unknown label does nothing but leaves the root menu
  in STATE_ENTER_SUBMENU). Contacts: `UIHK_PDAPhoneContactsWidget::PopulateList` = perk giver, trace contact, then
  each PDATrigger of `ProgressionTracker::mPDATriggerTracker` (`FindAndAddContact` / `AddContact(screen, symbol,
  name, portrait, ...)`); selecting one → `LaunchSubOption` → `LaunchCallMission` (activates the trigger's GameSlice,
  outgoing call UI via `SetCallerName` + `AnswerPhoneCall`). Portraits in the contacts texture pack
  (`PORTRAIT_SMARTPHONE_CAR_CONTACT` is the valet's).

## What the mod does

- `core/phone.cc` (`[Phone] Contact`, `Name`, `Info`, `Portrait`): hooks `UIHK_PDAPhoneContactsWidget::PopulateList`
  (called by `Flash_Activate` between `ContactList_Init` and `ContactList_Show`; its count decides "no contacts") to
  append a contact with `AddContact(widget, screen, &symbol, name, portrait, info)` (symbol `qStringHash32("SDTaxi")`;
  qSymbol arguments go by pointer) while `taxi::Available()`, and `LaunchSubOption` (after `ProcessInput` set
  `mSelectedIndex` +0x10; `mSymbolList` +0x100, items +0x108): for our symbol it does what `LaunchCallMission` does
  after finding a mission's PDATrigger — HUD `mInstance` → PDA (+0x200): `qString::Set(PDA+0x2D0 mPhoneContact)`,
  `qString::Set(mContactImage)`, `mOutgoingCall` +0x2F8 = 1, `mVoiceMail` +0x2FA = 0, `SetCallerName(PDA+0x1C0,
  ...)`, `AnswerPhoneCall(PDA)`, widget `mState` +0x8 = 6 (exit) — all found from `LaunchCallMission`'s own
  instructions (checked at fixed offsets) — then `taxi::Call`. `ProcessInput` adds the `PDATalk` action request
  (Wei holds the phone up) by itself. Why not `PDA.add_contact`: it adds a PDATrigger to the ProgressionTracker
  (`PDATriggerTracker::Add(symbol, no gameslice)`), which may be saved and outlive the mod as an "Err2" contact
  (`FindAndAddContact` falls back to "Err2"/Unknown/"PDA Error" for symbols not in
  `default-unlockables-contactList-list`). The scripted-contact pattern for reference: GunBackup
  (`PDA.add_contact`, `_wait_player_call_pda_contact` polling `PDA.is_calling_contact` = `IsTriggered`).
- `core/taxi.cc` (`[Taxi] Vehicle`, `Driver`): the dispatch script (embedded; `SDTaxi-dispatch.sk` next to the .asi
  replaces it, read at each call, for development): hang up after 2 s (`PDA.end_phone_call`), the valet's spot
  search 40-150 m, spawn (String names, made unique by the game), driver in, blip, `_path_to_xform` to the player's
  call position raced against `_wait_near_actor(taxi, 10.0)` and 2 minutes (speed logged every 0.25 s), `stop()`,
  wait up to 2 minutes for the player to get in (blip removed then), wait for them to get out, driver back to
  `SuspendAllowed`. One dispatch at a time: busy while the `skookum::Run` isn't finished; the call wrapper's
  destructor (`~SSIExternalMethodCallWrapper`) always sets the finished flag, also when a scene reset aborts it,
  so a load never leaves the contact hidden (and aborting via `abort_invoke` is safe while it's unset).

- `core/skookum.cc`: finds `GetScript`, `RunExternalCodeBlock`, `SkookumScript::update_delta` (hooked: the game-thread
  tick, `OnTick` callbacks run before the scripts update), both `ADebug::print`s (hooked: every line goes to the log
  as `skookum: ...`), and `c_world_p` / `ASymbol_origin_embedded2` from the instructions in `SkookumTask::Begin`
  (checked, and its call must reach RunExternalCodeBlock). `Start(name, source)` compiles in class `World` and runs
  on the world actor; a `Run` is never freed (the game writes its finished flag whenever the block ends).
  `Describe` prints a value (String/Integer/Real/Boolean/Symbol/Vector3/nil, else the class). Returned instances
  are not dereferenced (a small leak per console run).
- Script prints (`[Debug] ScriptPrints`, default on): once `c_world_p` is set, `SSBrain::get_class("Debug")`'s class
  methods (`SSClass` +0xD8 count / +0xE0 array; `SSMethodFunc` name +0x8, `i_atomic_f` +0x20) `print`, `println`
  and `break`, all the shared `ret 0` in this build, get our functions: the `{Object}` list argument (SSInvokedMethod
  `i_data` +0x58 count / +0x60 `SSData**`, `SSData::i_data_p` +0x8; a List instance's user data +0x20 points to its
  `SSList` {count +0x0, `SSInstance**` +0x8}, see `SSList::mthd_get_at`) is written to the log as `script: ...`, at
  most 100 lines a second. That covers the game's own scripts too (the valet's `[CAR VALET SYSTEM]` lines,
  gameslices starting; the first print comes from `ProgressionTracker::RestorePlayerLocation` right after loading).
  First version assumed the list's items inline at +0x20/+0x28 and crashed there on every load (2026-09-28); all
  reads of game memory in `skookum.cc` now go through `Read`/`Readable` (VirtualQuery, not `__try`, so the crash
  handler doesn't report caught faults).
- `core/console.cc` (`[Debug] Console`, default off; the user's ini has it on; `ConsoleKey` 0x7A = F11): runs
  `SDTaxi-console.sk` next to the .asi, split into blocks at `//---` lines. Presses less than 3 s apart count
  once. Running blocks aren't stopped (could be: `SSIExternalMethodCallWrapper::abort_invoke`, vtable +0x58,
  notify 1, child 1, while the run's finished flag is unset).
- `core/ride.cc` (`[Debug] RideLog`, default on): hooks `TransitUtility::OnSelectDestination` / `OnExitMap` /
  `PlaceTransitVehicle`, `AiDriverComponent::WarpToDestination` and `UIHKScreenWorldMap::SetWorldMapFromVehicle`
  (log only) and, once a second, logs the player's vehicle's AI driver state when it changes (mode, role,
  script-controlled, ambient, parked, drive-to coroutine, road-space destination). `GetPlayerVehicleAiDriver` is the
  call at `OnExitMap`+6. How the map skips the ride: confirm → `UIHKScreenWorldMap` states 6/7 (curtains) →
  `OnExitMap`: driving role 4, mode 2, `WarpToDestination` (fails only when the road-space destination is zero;
  kills traffic and parked cars within 25 m of the warp point, moves the car there) → `PlaceTransitVehicle` next
  vehicle-manager update; if the warp fails, mode 1 + attach to the road network: it drives there for real.
- `core/mem.hh`: `Readable` / `Read` for game memory whose layout comes from the disassembly.

## Testing

`tools\build.ps1 -Mod SDTaxi -Test -Deploy`. `load_test` loads the .asi outside the game (default ini, every
function reported missing).
