#include "pch-il2cpp.h"
#include "host_tab.h"
#include "utility.h"
#include "game.h"
#include "state.hpp"
#include "gui-helpers.hpp"

namespace HostTab {
	enum Groups {
		Utils,
		Settings,
		Extra
	};

	static bool openUtils = true; //default to utils tab group
	static bool openSettings = false;
	static bool openExtra = false;

	void CloseOtherGroups(Groups group) {
		openUtils = group == Groups::Utils;
		openSettings = group == Groups::Settings;
		openExtra = group == Groups::Extra;
	}

	static void SetRoleAmount(RoleTypes__Enum type, int amount) {
		auto&& options = GameOptions().GetRoleOptions();
		auto maxCount = options.GetNumPerGame(type);
		if (amount > maxCount)
			options.SetRoleRate(type, amount, 100);
		else if (amount > 0)
			options.SetRoleRate(type, maxCount, 100);
	}

	void SyncAllSettings() {
		if (IsInGame()) State.rpcQueue.push(new RpcSyncSettings());
		if (IsInLobby()) State.lobbyRpcQueue.push(new RpcSyncSettings());
	}

	const ptrdiff_t GetRoleCount(RoleType role)
	{
		return std::count_if(State.assignedRoles.cbegin(), State.assignedRoles.cend(), [role](RoleType i) {return i == role; });
	}

	void Render() {
		if (IsHost()) {
			ImGui::SameLine(100 * State.dpiScale);
			ImGui::BeginChild("###Host", ImVec2(500 * State.dpiScale, 0), true, ImGuiWindowFlags_NoBackground);
			if (TabGroup("Utils", openUtils)) {
				CloseOtherGroups(Groups::Utils);
			}
			if ((IsInGame() || IsInLobby()) && GameOptions().HasOptions()) {
				ImGui::SameLine();
				if (TabGroup("Settings", openSettings)) {
					CloseOtherGroups(Groups::Settings);
				}
			}
			if ((IsInGame() || IsInLobby()) && GameOptions().HasOptions()) {
				ImGui::SameLine();
				if (TabGroup("Extra", openExtra)) {
					CloseOtherGroups(Groups::Extra);
				}
			}
			GameOptions options;
			if (openUtils) {
				if (IsInLobby()) {
					ImGui::BeginChild("host#list", ImVec2(200, 0) * State.dpiScale, true, ImGuiWindowFlags_NoBackground);
					bool shouldEndListBox = ImGui::ListBoxHeader("Choose Roles", ImVec2(200, 290) * State.dpiScale);
					auto allPlayers = GetAllPlayerData();
					auto playerAmount = allPlayers.size();
					auto maxImpostorAmount = GetMaxImpostorAmount((int)playerAmount);
					for (size_t index = 0; index < playerAmount; index++) {
						auto playerData = allPlayers[index];
						if (playerData == nullptr) continue;
						PlayerControl* playerCtrl = GetPlayerControlById(playerData->fields.PlayerId);
						if (playerCtrl == nullptr) continue;
						State.assignedRolesPlayer[index] = playerCtrl;
						if (State.assignedRolesPlayer[index] == nullptr)
							continue;

						auto outfit = GetPlayerOutfit(playerData);
						if (outfit == NULL) continue;
						const std::string& playerName = convert_from_string(outfit->fields.PlayerName);
						//player colors in host tab by gdjkhp (https://github.com/GDjkhp/AmongUsMenu/commit/53b017183bac503c546f198e2bc03539a338462c)
						if (CustomListBoxInt(playerName.c_str(), reinterpret_cast<int*>(&State.assignedRoles[index]), ROLE_NAMES, 80 * State.dpiScale, AmongUsColorToImVec4(GetPlayerColor(outfit->fields.ColorId))))
						{
							State.engineers_amount = (int)GetRoleCount(RoleType::Engineer);
							State.scientists_amount = (int)GetRoleCount(RoleType::Scientist);
							State.trackers_amount = (int)GetRoleCount(RoleType::Tracker);
							State.noisemakers_amount = (int)GetRoleCount(RoleType::Noisemaker);
							State.shapeshifters_amount = (int)GetRoleCount(RoleType::Shapeshifter);
							State.phantoms_amount = (int)GetRoleCount(RoleType::Phantom);
							State.impostors_amount = (int)GetRoleCount(RoleType::Impostor);
							if (State.impostors_amount + State.shapeshifters_amount + State.phantoms_amount > maxImpostorAmount)
							{
								if (State.assignedRoles[index] == RoleType::Impostor)
									State.assignedRoles[index] = RoleType::Random;
								else if (State.assignedRoles[index] == RoleType::Shapeshifter)
									State.assignedRoles[index] = RoleType::Random;
								else if (State.assignedRoles[index] == RoleType::Tracker)
									State.assignedRoles[index] = RoleType::Random;
								State.shapeshifters_amount = (int)GetRoleCount(RoleType::Shapeshifter);
								State.impostors_amount = (int)GetRoleCount(RoleType::Impostor);
								State.crewmates_amount = (int)GetRoleCount(RoleType::Crewmate);
							}

							if (State.assignedRoles[index] == RoleType::Engineer || State.assignedRoles[index] == RoleType::Scientist ||
								State.assignedRoles[index] == RoleType::Tracker || State.assignedRoles[index] == RoleType::Noisemaker ||
								State.assignedRoles[index] == RoleType::Crewmate) {
								if (State.engineers_amount + State.scientists_amount + State.trackers_amount + State.noisemakers_amount + State.crewmates_amount >= (int)playerAmount)
									State.assignedRoles[index] = RoleType::Random;
							} //Some may set all players to non imps. This hangs the game on beginning. Leave space to Random so we have imps.

							if (options.GetGameMode() == GameModes__Enum::HideNSeek)
							{
								if (State.assignedRoles[index] == RoleType::Shapeshifter)
									State.assignedRoles[index] = RoleType::Impostor;
								else if (State.assignedRoles[index] == RoleType::Phantom)
									State.assignedRoles[index] = RoleType::Impostor;
								else if (State.assignedRoles[index] == RoleType::Tracker)
									State.assignedRoles[index] = RoleType::Engineer;
								else if (State.assignedRoles[index] == RoleType::Noisemaker)
									State.assignedRoles[index] = RoleType::Engineer;
								else if (State.assignedRoles[index] == RoleType::Crewmate)
									State.assignedRoles[index] = RoleType::Engineer;
							} //Assign other roles in hidenseek causes game bug.
							//These are organized. Do not change the order unless you find it necessary.

							if (!IsInGame()) {
								if (options.GetGameMode() == GameModes__Enum::HideNSeek)
									SetRoleAmount(RoleTypes__Enum::Engineer, 15);
								else
									SetRoleAmount(RoleTypes__Enum::Engineer, State.engineers_amount);
								SetRoleAmount(RoleTypes__Enum::Scientist, State.scientists_amount);
								SetRoleAmount(RoleTypes__Enum::Tracker, State.trackers_amount);
								SetRoleAmount(RoleTypes__Enum::Noisemaker, State.noisemakers_amount);
								SetRoleAmount(RoleTypes__Enum::Shapeshifter, State.shapeshifters_amount);
								SetRoleAmount(RoleTypes__Enum::Phantom, State.phantoms_amount);
								if (options.GetNumImpostors() <= State.impostors_amount + State.shapeshifters_amount + State.phantoms_amount)
									options.SetInt(app::Int32OptionNames__Enum::NumImpostors, State.impostors_amount + State.shapeshifters_amount + State.phantoms_amount);
							}
						}
					}
					if (shouldEndListBox)
						ImGui::ListBoxFooter();
					ImGui::EndChild();
				}
				if (IsInLobby()) ImGui::SameLine();
				ImGui::BeginChild("host#actions", ImVec2(300, 0) * State.dpiScale, true, ImGuiWindowFlags_NoBackground);

				if (IsInLobby() && ToggleButton("Custom Impostor Amount", &State.CustomImpostorAmount))
					State.Save();
				State.ImpostorCount = std::clamp(State.ImpostorCount, 0, int(Game::MAX_PLAYERS));
				if (IsInLobby() && State.CustomImpostorAmount && ImGui::InputInt("Impostor Count", &State.ImpostorCount))
					State.Save();



				ImGui::Dummy(ImVec2(14, 14) * State.dpiScale);
				if (IsInLobby() && ImGui::Button("Force Start of Game"))
				{
					app::InnerNetClient_SendStartGame((InnerNetClient*)(*Game::pAmongUsClient), NULL);
				}


				ImGui::Dummy(ImVec2(10, 10) * State.dpiScale);
				if (ToggleButton("Disable Meetings", &State.DisableMeetings))
					State.Save();
				if (ToggleButton("Disable Sabotages", &State.DisableSabotages))
					State.Save();
				if (ToggleButton("Disable Kills", &State.DisableKills))
					State.Save();
				if (State.DisableKills) ImGui::Text("Note: Cheaters can still bypass this feature!");
				if (IsHost) { //lobby isn't possible in freeplay
					if (ToggleButton("Disable Game Ending", &State.NoGameEnd)) {
						State.Save();
					}


					if ((State.mapType == Settings::MapType::Airship) && IsInGame() && ImGui::Button("Switch Moving Platform Side"))
					{
						State.rpcQueue.push(new RpcUsePlatform());
					}

					if (State.InMeeting && ImGui::Button("End Meeting")) {
						State.rpcQueue.push(new RpcEndMeeting());
						State.InMeeting = false;
					}


					if (IsInGame()) {
						CustomListBoxInt("Reason", &State.SelectedGameEndReasonId, GAMEENDREASON, 120.0f * State.dpiScale);

						ImGui::SameLine();

						if (ImGui::Button("End Game")) {
							State.rpcQueue.push(new RpcEndGame(GameOverReason__Enum(std::clamp(State.SelectedGameEndReasonId, 0, 8))));
						}
					}
				}

				CustomListBoxInt(" ?", &State.HostSelectedColorId, HOSTCOLORS, 85.0f * State.dpiScale);

				if (ToggleButton("Force Color for Everyone", &State.ForceColorForEveryone)) {
					State.Save();
				}

				if (ToggleButton("Force Name for Everyone", &State.ForceNameForEveryone)) {
					State.Save();
				}
				if (InputString("Username", &State.hostUserName)) {
					State.Save();
				}

				if (IsHost() && GetPlayerData(*Game::pLocalPlayer)->fields.IsDead && ImGui::Button("Revive Yourself"))
				{
					if (IsInGame()) State.rpcQueue.push(new RpcRevive(*Game::pLocalPlayer));
					if (IsInLobby()) State.lobbyRpcQueue.push(new RpcRevive(*Game::pLocalPlayer));
				}

				ImGui::EndChild();
			}

			if (openSettings) {
				// AU v2022.8.24 has been able to change maps in lobby.
				State.mapHostChoice = options.GetByte(app::ByteOptionNames__Enum::MapId);
				if (State.mapHostChoice > 3)
					State.mapHostChoice--;
				State.mapHostChoice = std::clamp(State.mapHostChoice, 0, (int)MAP_NAMES.size() - 1);
				if (IsInLobby() && CustomListBoxInt("Map", &State.mapHostChoice, MAP_NAMES, 75 * State.dpiScale)) {
					if (!IsInGame()) {
						// disable flip
						if (State.mapHostChoice == 3) {
							options.SetByte(app::ByteOptionNames__Enum::MapId, 0);
							State.FlipSkeld = true;
						}
						else {
							options.SetByte(app::ByteOptionNames__Enum::MapId, State.mapHostChoice);
							State.FlipSkeld = false;
						}
						auto id = State.mapHostChoice;
						if (id >= 3) id++;
						options.SetByte(app::ByteOptionNames__Enum::MapId, id);
						SyncAllSettings();
					}
				}
				auto gamemode = options.GetGameMode();

				auto MakeBool = [&](const char* str, bool& v, BoolOptionNames__Enum opt) {
					if (ToggleButton(str, &v)) {
						options.SetBool(opt, v);
						SyncAllSettings();
					}
					else v = options.GetBool(opt);
					};

				auto MakeInt = [&](const char* str, int& v, Int32OptionNames__Enum opt) {
					if (ImGui::InputInt(str, &v)) {
						options.SetInt(opt, v);
						SyncAllSettings();
					}
					else v = options.GetInt(opt);
					};

				auto MakeFloat = [&](const char* str, float& v, FloatOptionNames__Enum opt) {
					if (ImGui::InputFloat(str, &v)) {
						options.SetFloat(opt, v);
						SyncAllSettings();
					}
					else v = options.GetFloat(opt);
					};

				if (gamemode == GameModes__Enum::Normal || gamemode == GameModes__Enum::NormalFools) {
					static int emergencyMeetings = 1, emergencyCooldown = 1, discussionTime = 1,
						votingTime = 1, killDistance = 1, commonTasks = 1, shortTasks = 1, longTasks = 1, taskBarMode = 1;

					static float playerSpeed = 1.f, crewVision = 1.f, impVision = 1.f, killCooldown = 1.f;

					static bool ejects = false, anonVotes = false, visualTasks = false;

#pragma region General
					MakeBool("Confirm Ejects", ejects, BoolOptionNames__Enum::ConfirmImpostor);
					MakeInt("# Emergency Meetings", emergencyMeetings, Int32OptionNames__Enum::NumEmergencyMeetings);
					MakeBool("Anonymous Votes", anonVotes, BoolOptionNames__Enum::AnonymousVotes);
					MakeInt("Emergency Cooldown", emergencyCooldown, Int32OptionNames__Enum::EmergencyCooldown);
					MakeInt("Discussion Time", discussionTime, Int32OptionNames__Enum::DiscussionTime);
					MakeInt("Voting Time", votingTime, Int32OptionNames__Enum::VotingTime);
					MakeFloat("Player Speed", playerSpeed, FloatOptionNames__Enum::PlayerSpeedMod);
					MakeInt("Task Bar Updates", taskBarMode, Int32OptionNames__Enum::TaskBarMode);
					MakeBool("Visual Tasks", visualTasks, BoolOptionNames__Enum::VisualTasks);
					MakeFloat("Crewmate Vision", crewVision, FloatOptionNames__Enum::CrewLightMod);
					MakeFloat("Impostor Vision", impVision, FloatOptionNames__Enum::ImpostorLightMod);
					MakeFloat("Kill Cooldown", killCooldown, FloatOptionNames__Enum::KillCooldown);
					MakeInt("Kill Distance", killDistance, Int32OptionNames__Enum::KillDistance);
					MakeInt("# Short Tasks", shortTasks, Int32OptionNames__Enum::NumShortTasks);
					MakeInt("# Common Tasks", commonTasks, Int32OptionNames__Enum::NumCommonTasks);
					MakeInt("# Long Tasks", longTasks, Int32OptionNames__Enum::NumLongTasks);
#pragma endregion
#pragma region Scientist
					ImGui::Text("Scientist");
					static float vitalsCooldown = 1.f, batteryDuration = 1.f;

					MakeFloat("Vitals Display Cooldown", vitalsCooldown, FloatOptionNames__Enum::ScientistCooldown);
					MakeFloat("Battery Duration", batteryDuration, FloatOptionNames__Enum::ScientistBatteryCharge);
#pragma endregion
#pragma region Engineer
					ImGui::Text("Engineer");
					static float ventCooldown = 1.f, ventDuration = 1.f;

					MakeFloat("Vent Use Cooldown", ventCooldown, FloatOptionNames__Enum::EngineerCooldown);
					MakeFloat("Max Time in Vents", ventDuration, FloatOptionNames__Enum::EngineerInVentMaxTime);
#pragma endregion
#pragma region Guardian Angel
					ImGui::Text("Guardian Angel");
					static float protectCooldown = 1.f, protectDuration = 1.f;
					static bool protectVisible = false;

					MakeFloat("Protect Cooldown", protectCooldown, FloatOptionNames__Enum::GuardianAngelCooldown);
					MakeFloat("Protection Duration", protectCooldown, FloatOptionNames__Enum::ProtectionDurationSeconds);
					MakeBool("Protect Visible to Impostors", protectVisible, BoolOptionNames__Enum::ImpostorsCanSeeProtect);
#pragma endregion
#pragma region Shapeshifter
					ImGui::Text("Shapeshifter");
					static float shapeshiftDuration = 1.f, shapeshiftCooldown = 1.f;
					static bool shapeshiftEvidence = false;

					MakeFloat("Shapeshift Duration", shapeshiftDuration, FloatOptionNames__Enum::ShapeshifterDuration);
					MakeFloat("Shapeshift Cooldown", shapeshiftCooldown, FloatOptionNames__Enum::ShapeshifterCooldown);
					MakeBool("Leave Shapeshifting Evidence", shapeshiftEvidence, BoolOptionNames__Enum::ShapeshifterLeaveSkin);
#pragma endregion
#pragma region Noisemaker
					ImGui::Text("Noisemaker");
					static float alertDuration = 1.f;

					MakeFloat("Alert Duration", alertDuration, FloatOptionNames__Enum::NoisemakerAlertDuration);
#pragma endregion
#pragma region Tracker
					ImGui::Text("Tracker");
					static float trackerDuration = 1.f, trackerCooldown = 1.f, trackerDelay = 1.f;

					MakeFloat("Tracker Duration", trackerDuration, FloatOptionNames__Enum::TrackerDuration);
					MakeFloat("Tracker Cooldown", trackerCooldown, FloatOptionNames__Enum::TrackerCooldown);
					MakeFloat("Tracker Delay", trackerDelay, FloatOptionNames__Enum::TrackerDelay);
#pragma endregion
#pragma region Phantom
					ImGui::Text("Phantom");
					static float phantomDuration = 1.f, phantomCooldown = 1.f;

					MakeFloat("Phantom Duration", phantomDuration, FloatOptionNames__Enum::PhantomDuration);
					MakeFloat("Phantom Cooldown", phantomCooldown, FloatOptionNames__Enum::PhantomCooldown);
#pragma endregion
				}
#pragma region Hide and Seek
				if (gamemode == GameModes__Enum::HideNSeek || gamemode == GameModes__Enum::SeekFools) {
					static int killDistance = 1, commonTasks = 1, shortTasks = 1, longTasks = 1, maxVents = 1;

					static float playerSpeed = 1.f, crewVision = 1.f, impVision = 1.f, killCooldown = 1.f,
						hidingTime = 1.f, finalHideTime = 1.f, ventTime = 1.f, crewLight = 1.f, impLight = 1.f,
						finalImpSpeed = 1.f, pingInterval = 1.f;

					static bool flashlight = false, seekMap = false, hidePings = false, showNames = false;

					MakeFloat("Crewmate Vision", crewVision, FloatOptionNames__Enum::CrewLightMod);
					MakeFloat("Impostor Vision", impVision, FloatOptionNames__Enum::ImpostorLightMod);
					MakeFloat("Kill Cooldown", killCooldown, FloatOptionNames__Enum::KillCooldown);
					MakeInt("Kill Distance", killDistance, Int32OptionNames__Enum::KillDistance);
					MakeInt("# Short Tasks", shortTasks, Int32OptionNames__Enum::NumShortTasks);
					MakeInt("# Common Tasks", commonTasks, Int32OptionNames__Enum::NumCommonTasks);
					MakeInt("# Long Tasks", longTasks, Int32OptionNames__Enum::NumLongTasks);
					MakeFloat("Player Speed", playerSpeed, FloatOptionNames__Enum::PlayerSpeedMod);
					MakeFloat("Hiding Time", hidingTime, FloatOptionNames__Enum::EscapeTime);
					MakeFloat("Final Hide Time", finalHideTime, FloatOptionNames__Enum::FinalEscapeTime);
					MakeInt("Max Vent Uses", maxVents, Int32OptionNames__Enum::CrewmateVentUses);
					MakeFloat("Max Time In Vent", ventTime, FloatOptionNames__Enum::CrewmateTimeInVent);
					MakeBool("Flashlight Mode", flashlight, BoolOptionNames__Enum::UseFlashlight);
					MakeFloat("Crewmate Flashlight Size", crewLight, FloatOptionNames__Enum::CrewmateFlashlightSize);
					MakeFloat("Impostor Flashlight Size", impLight, FloatOptionNames__Enum::ImpostorFlashlightSize);
					MakeFloat("Final Hide Impostor Speed", finalImpSpeed, FloatOptionNames__Enum::SeekerFinalSpeed);
					MakeBool("Final Hide Seek Map", seekMap, BoolOptionNames__Enum::SeekerFinalMap);
					MakeBool("Final Hide Pings", hidePings, BoolOptionNames__Enum::SeekerPings);
					MakeFloat("Ping Interval", pingInterval, FloatOptionNames__Enum::MaxPingTime);
					MakeBool("Show Names", showNames, BoolOptionNames__Enum::ShowCrewmateNames);
				}
			}
			if (openExtra) {
				ImGui::Dummy(ImVec2(5, 5)* State.dpiScale);
				if (IsHost() && ToggleButton("Flip Skeld", &State.FlipSkeld)) State.Save();
					ImGui::SameLine();
				ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), ("Soon"));
				State.Save(); //Soon fix


				ImGui::Dummy(ImVec2(15, 15)* State.dpiScale);
				int32_t maxPlayers = options.GetMaxPlayers();
				maxPlayers = std::clamp(maxPlayers, 0, int(Game::MAX_PLAYERS));
				ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), ("\nWorks With Modded Regions!"));
				if (IsHost() && ImGui::InputInt("Players Count", &maxPlayers))
					GameOptions().SetInt(app::Int32OptionNames__Enum::MaxPlayers, maxPlayers); //support for more than 15 players
				State.Save();


				ImGui::Dummy(ImVec2(15, 15) * State.dpiScale);
				if (ToggleButton("Disable Specific RPC Call ID", &State.DisableCallId)) State.Save();
					State.Save();
				int callId = State.ToDisableCallId;
				if (ImGui::InputInt("ID to Disable", &callId)) {
					State.ToDisableCallId = (uint8_t)callId;
					State.Save();
					}
				}
			}
#pragma endregion
			ImGui::EndChild();
		}
	}