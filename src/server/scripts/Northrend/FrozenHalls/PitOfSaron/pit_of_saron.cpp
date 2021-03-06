/*
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ScriptPCH.h"
#include "pit_of_saron.h"
#include "Vehicle.h"

enum eSpells
{
    SPELL_FIREBALL              = 69583, //Ymirjar Flamebearer
    SPELL_HELLFIRE              = 69586,
    SPELL_TACTICAL_BLINK        = 69584,
    SPELL_FROST_BREATH          = 69527, //Iceborn Proto-Drake
    SPELL_BLINDING_DIRT         = 70302, //Wrathbone Laborer
    SPELL_PUNCTURE_WOUND        = 70278,
    SPELL_SHOVELLED             = 69572,
    SPELL_LEAPING_FACE_MAUL     = 69504, // Geist Ambusher
};

enum eEvents
{
    // Ymirjar Flamebearer
    EVENT_FIREBALL              = 1,
    EVENT_TACTICAL_BLINK        = 2,

    //Wrathbone Laborer
    EVENT_BLINDING_DIRT         = 3,
    EVENT_PUNCTURE_WOUND        = 4,
    EVENT_SHOVELLED             = 5,
};

const Position spawnPoints1[4] =
{
    {877.666199f, 61.523361f, 527.974243f, 3.624145f},
    {885.715027f, 65.515602f, 533.431030f, 3.857180f},
    {912.531189f, 63.785076f, 547.925720f, 3.235373f},
    {909.602722f, 83.248398f, 551.600403f, 3.251082f},
};
const Position spawnPoints2[4] =
{    
    {879.286743f, 41.535030f, 521.009338f, 3.486701f},
    {889.974548f, 45.270569f, 527.154236f, 3.612364f},
    {919.853027f, 88.957771f, 558.705993f, 3.592729f},
    {921.595520f, 69.234627f, 557.946716f, 3.145053f},    
};
const Position spawnPointsYmirjar[2] =
{
    {883.150024f, 54.626401f, 528.262024f, 3.678787f},
    {915.107971f, 75.316299f, 553.531006f, 3.678787f},
};
const Position spawnPointsFallenWarrior[8] =
{
    {937.606506f, 0.776727f, 578.888000f, 1.090893f},
    {928.419006f, 8.786335f, 577.693970f, 1.122307f},
    {924.478699f, -7.662051f, 582.044983f, 1.489874f},
    {935.244568f, -10.427516f, 583.265503f, 1.358702f},
    {935.098694f, -24.272480f, 588.035400f, 1.653226f},
    {921.272644f, -22.194103f, 585.452576f, 1.331212f},
    {930.109009f, -56.889900f, 591.848999f, 2.353980f},
    {924.945984f, -60.164799f, 591.879028f, 2.237270f},
};

class mob_ymirjar_flamebearer : public CreatureScript
{
    public:
        mob_ymirjar_flamebearer() : CreatureScript("mob_ymirjar_flamebearer") { }

        struct mob_ymirjar_flamebearerAI: public ScriptedAI
        {
            mob_ymirjar_flamebearerAI(Creature* creature) : ScriptedAI(creature)
            {
            }

            void Reset()
            {
                _events.Reset();
            }

            void EnterCombat(Unit* /*who*/)
            {
                _events.ScheduleEvent(EVENT_FIREBALL, 4000);
                _events.ScheduleEvent(EVENT_TACTICAL_BLINK, 15000);
            }

            void UpdateAI(const uint32 diff)
            {
                if (!UpdateVictim())
                    return;

                _events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = _events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_FIREBALL:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0))
                                DoCast(target, SPELL_FIREBALL);
                            _events.RescheduleEvent(EVENT_FIREBALL, 5000);
                            break;
                        case EVENT_TACTICAL_BLINK:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0))
                                DoCast(target, SPELL_TACTICAL_BLINK);
                            DoCast(me, SPELL_HELLFIRE);
                            _events.RescheduleEvent(EVENT_TACTICAL_BLINK, 12000);
                            break;
                        default:
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }

        private:
            EventMap _events;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new mob_ymirjar_flamebearerAI(creature);
        }
};

class mob_iceborn_protodrake : public CreatureScript
{
    public:
        mob_iceborn_protodrake() : CreatureScript("mob_iceborn_protodrake") { }

        struct mob_iceborn_protodrakeAI: public ScriptedAI
        {
            mob_iceborn_protodrakeAI(Creature* creature) : ScriptedAI(creature), _vehicle(creature->GetVehicleKit())
            {
                ASSERT(_vehicle);
            }

            void Reset()
            {
                _frostBreathCooldown = 5000;
            }

            void EnterCombat(Unit* /*who*/)
            {
                _vehicle->RemoveAllPassengers();
            }

            void UpdateAI(const uint32 diff)
            {
                if (!UpdateVictim())
                    return;

                if (_frostBreathCooldown < diff)
                {
                    DoCastVictim(SPELL_FROST_BREATH);
                    _frostBreathCooldown = 10000;
                }
                else
                    _frostBreathCooldown -= diff;

                DoMeleeAttackIfReady();
            }

        private:
            Vehicle* _vehicle;
            uint32 _frostBreathCooldown;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new mob_iceborn_protodrakeAI(creature);
        }
};

class mob_wrathbone_laborer : public CreatureScript
{
    public:
        mob_wrathbone_laborer() : CreatureScript("mob_wrathbone_laborer") { }

        struct mob_wrathbone_laborerAI: public ScriptedAI
        {
            mob_wrathbone_laborerAI(Creature* creature) : ScriptedAI(creature)
            {
            }

            void Reset()
            {
                _events.Reset();
            }

            void EnterCombat(Unit* /*who*/)
            {
                _events.ScheduleEvent(EVENT_BLINDING_DIRT, 8000);
                _events.ScheduleEvent(EVENT_PUNCTURE_WOUND, 9000);
                _events.ScheduleEvent(EVENT_SHOVELLED, 5000);
            }

            void UpdateAI(const uint32 diff)
            {
                if (!UpdateVictim())
                    return;

                _events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = _events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_BLINDING_DIRT:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1, 10.0f, true))
                                DoCast(target, SPELL_BLINDING_DIRT);
                            _events.RescheduleEvent(EVENT_BLINDING_DIRT, 10000);
                            return;
                        case EVENT_PUNCTURE_WOUND:
                            DoCastVictim(SPELL_PUNCTURE_WOUND);
                            _events.RescheduleEvent(EVENT_PUNCTURE_WOUND, 9000);
                            return;
                        case EVENT_SHOVELLED:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1, -5.0f))
                                DoCast(target, SPELL_SHOVELLED);
                            _events.RescheduleEvent(EVENT_SHOVELLED, 7000);
                            return;
                    }
                }

                DoMeleeAttackIfReady();
            }

        private:
            EventMap _events;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new mob_wrathbone_laborerAI(creature);
        }
};

class mob_geist_ambusher : public CreatureScript
{
    public:
        mob_geist_ambusher() : CreatureScript("mob_geist_ambusher") { }

        struct mob_geist_ambusherAI: public ScriptedAI
        {
            mob_geist_ambusherAI(Creature* creature) : ScriptedAI(creature)
            {
            }

            void Reset()
            {
                _leapingFaceMaulCooldown = 9000;
            }

            void MoveInLineOfSight(Unit* who)
            {
                if (who->GetTypeId() != TYPEID_PLAYER)
                    return;

                if (me->IsWithinDistInMap(who, 30.0f))
                    DoCast(who, SPELL_LEAPING_FACE_MAUL);
            }

            void UpdateAI(const uint32 diff)
            {
                if (!UpdateVictim())
                    return;

                if (_leapingFaceMaulCooldown < diff)
                {
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 5.0f, true))
                        DoCast(target, SPELL_LEAPING_FACE_MAUL);
                    _leapingFaceMaulCooldown = urand(9000, 14000);
                }
                else
                    _leapingFaceMaulCooldown -= diff;

                DoMeleeAttackIfReady();
            }

        private:
            uint32 _leapingFaceMaulCooldown;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new mob_geist_ambusherAI(creature);
        }
};

class spell_trash_mob_glacial_strike : public SpellScriptLoader
{
    public:
        spell_trash_mob_glacial_strike() : SpellScriptLoader("spell_trash_mob_glacial_strike") { }

        class spell_trash_mob_glacial_strike_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_trash_mob_glacial_strike_AuraScript);

            void PeriodicTick(AuraEffect const* /*aurEff*/)
            {
                if (GetTarget()->IsFullHealth())
                {
                    GetTarget()->RemoveAura(GetId(), 0, 0, AURA_REMOVE_BY_ENEMY_SPELL);
                    PreventDefaultAction();
                }
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_trash_mob_glacial_strike_AuraScript::PeriodicTick, EFFECT_2, SPELL_AURA_PERIODIC_DAMAGE_PERCENT);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_trash_mob_glacial_strike_AuraScript();
        }
};

/*
Script Complete: 100%
Name: Start Pit Of Saron
Author: Cemak
*/
enum SAYTyrranusEvent
{
    SAY_TYRRANUS_1                 = -1658074,
    SAY_TYRRANUS_2                 = -1658075,
    SAY_TYRRANUS_3                 = -1658078,
    SAY_TYRRANUS_4                 = -1658082,
    
    SAY_SYLVANAS_1                 = -1658076,
    SAY_SYLVANAS_2                 = -1658080,
    SAY_SYLVANAS_3                 = -1658085,
    SAY_SYLVANAS_4                 = -1658087,
    
    SAY_JAINA_1                    = -1658077,
    SAY_JAINA_2                    = -1658081,
    SAY_JAINA_3                    = -1658084,
    SAY_JAINA_4                    = -1658086,
    
    SAY_SOUND                      = 16750,
};

enum SPELLSTyrranusEvent
{
    SPELL_NECROMANTIC_POWER         = 69347,
    SPELL_DEATH_ARMY_VISUAL         = 67693,
    SPELL_BLIZZARD_JAINA            = 70132,
    SPELL_ICE_LANCE_JAINA           = 70464,
    SPELL_SHADOW_BOLT_SYLVANAS      = 70512,
    SPELL_MULTI_SHOT_SYLVANAS       = 70513,
};

struct startPosition
{
    uint32 entry[2];
    Position movePosition;
} startPositions[] =
{
    { { NPC_CHAMPION_1_ALLIANCE, NPC_CHAMPION_1_HORDE }, { 445.286f, 214.251f, 528.71f, 0.375996f } },
    { { NPC_CHAMPION_1_ALLIANCE, NPC_CHAMPION_1_HORDE }, { 447.246f, 208.189f, 528.71f, 0.0264938f } },
    { { NPC_CHAMPION_1_ALLIANCE, NPC_CHAMPION_1_HORDE }, { 437.81f,  231.115f, 528.708f, 0.525224f } },
    { { NPC_CHAMPION_1_ALLIANCE, NPC_CHAMPION_1_HORDE }, { 437.444f, 225.798f, 528.712f, 0.36029f } },
    { { NPC_CHAMPION_1_ALLIANCE, NPC_CHAMPION_1_HORDE }, { 438.941f, 221.784f, 528.708f, 0.30924f } },
    { { NPC_CHAMPION_1_ALLIANCE, NPC_CHAMPION_1_HORDE }, { 440.384f, 216.785f, 528.71f, 0.203211f } },
    { { NPC_CHAMPION_1_ALLIANCE, NPC_CHAMPION_1_HORDE }, { 441.479f, 211.472f, 528.709f,0.203211f } },
    { { NPC_CHAMPION_1_ALLIANCE, NPC_CHAMPION_1_HORDE }, { 443.986f, 222.103f, 528.709f,6.27373f } },
    { { NPC_CHAMPION_1_ALLIANCE, NPC_CHAMPION_1_HORDE }, { 448.944f, 204.945f, 528.709f,0.0258984f } },
    { { NPC_CHAMPION_2_ALLIANCE, NPC_CHAMPION_2_HORDE }, { 446.613f, 211.37f,  528.709f,0.360472f } },
    { { NPC_CHAMPION_2_ALLIANCE, NPC_CHAMPION_2_HORDE }, { 442.647f, 204.635f, 528.707f,0.0887308f } },
    { { NPC_CHAMPION_2_ALLIANCE, NPC_CHAMPION_2_HORDE }, { 435.054f, 207.61f,  528.707f,0.173361f } },
    { { NPC_CHAMPION_2_ALLIANCE, NPC_CHAMPION_2_HORDE }, { 434.568f, 210.389f, 528.707f,0.173361f } },
    { { NPC_CHAMPION_2_ALLIANCE, NPC_CHAMPION_2_HORDE }, { 443.458f, 226.623f, 528.709f,0.232266f } },
    { { NPC_CHAMPION_2_ALLIANCE, NPC_CHAMPION_2_HORDE }, { 435.593f, 204.535f, 528.728f,0.173361f } },
    { { NPC_CHAMPION_2_ALLIANCE, NPC_CHAMPION_3_HORDE }, { 442.47f,  208.153f, 528.706f,0.259756f } },
    { { NPC_CHAMPION_2_ALLIANCE, NPC_CHAMPION_3_HORDE }, { 441.333f, 233.866f, 528.709f,0.255828f } },
    { { NPC_CHAMPION_2_ALLIANCE, NPC_CHAMPION_3_HORDE }, { 442.124f, 230.78f,  528.709f,0.244047f } },

    { { NPC_KORLAEN, NPC_KORALEN }, { 432.386f, 212.979f, 528.709f, 0.237168f } },
    { { NPC_ELANDRA, NPC_KILARA }, { 431.65f, 216.976f, 528.709f, 0.20104f } },
    { { NPC_JAINA_PART1, NPC_SYLVANAS_PART1 }, { 438.84f, 213.502f, 528.709f, 0.1900f } },

    { { 0, 0 }, { 0.0f, 0.0f, 0.0f, 0.0f } }
};

class pitofsaron_start : public CreatureScript
{
public:
    pitofsaron_start() : CreatureScript("pitofsaron_start") { }

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new pitofsaron_startAI(pCreature);
    }

    struct pitofsaron_startAI : public ScriptedAI
    {
        pitofsaron_startAI(Creature *c) : ScriptedAI(c)
        {
            instance = c->GetInstanceScript();
            Reset();
        }

        void Reset()
        {
            StartPhase = 0;
            StartTimer = 5000;
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            me->SetReactState(REACT_PASSIVE);
            event = true;
        }

        void UpdateAI(const uint32 diff)
        {
            if(instance->GetData(DATA_TYRANNUS_START) == IN_PROGRESS && event)
            {
                if (StartTimer <= diff)
                {
                    switch (StartPhase)
                    {
                        case 0:
                            if (Creature* sTyrannus = me->FindNearestCreature(NPC_TYRANNUS_EVENTS, 50.0f, true))
                            {
                                sTyrannus->SetCanFly(true);
                                DoScriptText(SAY_TYRRANUS_1, sTyrannus);
                                
                                int32 entryIndex;
                               if (instance->GetData(DATA_TEAM_IN_INSTANCE) == ALLIANCE)
                                     entryIndex = 0;
                                  else
                                     entryIndex = 1;
                                     
                               Position spawnPoint = {427.36f, 212.636f, 529.47f, 6.2564f};
                               
                              for (int8 i = 0; startPositions[i].entry[entryIndex] != 0; ++i)
                              {
                                 if (Creature* summon = me->SummonCreature(startPositions[i].entry[entryIndex], spawnPoint, TEMPSUMMON_DEAD_DESPAWN))
                                  summon->GetMotionMaster()->MovePoint(0, startPositions[i].movePosition);
                              }
                            }
                            ++StartPhase;
                            StartTimer = 5000;
                            break;
                        case 1:
                            if (Creature* sTyrannus = me->FindNearestCreature(NPC_TYRANNUS_EVENTS, 50.0f, true))
                                DoScriptText(SAY_TYRRANUS_2, sTyrannus);
                            ++StartPhase;
                            StartTimer = 15000;
                            break;
                        case 2:
                            if(me->GetEntry() == NPC_SYLVANAS_PART1)
                                DoScriptText(SAY_SYLVANAS_1, me);
                            else if(me->GetEntry() == NPC_JAINA_PART1)
                                DoScriptText(SAY_JAINA_1, me);

                            if (instance->GetData(DATA_TEAM_IN_INSTANCE) == ALLIANCE)
                            {
                                GetCreatureListWithEntryInGrid(Champion1, me, NPC_CHAMPION_1_ALLIANCE, 150.0f);
                                for(std::list<Creature*>::iterator itr = Champion1.begin(); itr != Champion1.end(); ++itr)
                                {
                                    Creature *sChampions = *itr;
                                    if (!sChampions)
                                        continue;

                                    if (sChampions->isAlive())
                                        if (Creature* sNecrolyte = me->FindNearestCreature(367880, 150.0f, true))
                                        {
                                            sChampions->Attack(sNecrolyte, true);
                                            sChampions->GetMotionMaster()->MoveChase(sNecrolyte);
                                        }
                                }

                                GetCreatureListWithEntryInGrid(Champion2, me, NPC_CHAMPION_2_ALLIANCE, 150.0f);
                                for(std::list<Creature*>::iterator itr = Champion2.begin(); itr != Champion2.end(); ++itr)
                                {
                                    Creature *sChampions = *itr;
                                    if (!sChampions)
                                        continue;

                                    if (sChampions->isAlive())
                                        if (Creature* sNecrolyte = me->FindNearestCreature(36788, 150.0f, true))
                                        {
                                            sChampions->Attack(sNecrolyte, true);
                                            sChampions->GetMotionMaster()->MoveChase(sNecrolyte);
                                        }
                                }
                            }
                            else
                            {
                                GetCreatureListWithEntryInGrid(Champion1, me, NPC_CHAMPION_1_HORDE, 150.0f);
                                for(std::list<Creature*>::iterator itr = Champion1.begin(); itr != Champion1.end(); ++itr)
                                {
                                    Creature *sChampions = *itr;
                                    if (!sChampions)
                                        continue;

                                    if (sChampions->isAlive())
                                        if (Creature* sNecrolyte = me->FindNearestCreature(367880, 150.0f, true))
                                        {
                                            sChampions->Attack(sNecrolyte, true);
                                            sChampions->GetMotionMaster()->MoveChase(sNecrolyte);
                                        }
                                }

                                GetCreatureListWithEntryInGrid(Champion2, me, NPC_CHAMPION_2_HORDE, 150.0f);
                                for(std::list<Creature*>::iterator itr = Champion2.begin(); itr != Champion2.end(); ++itr)
                                {
                                    Creature *sChampions = *itr;
                                    if (!sChampions)
                                        continue;

                                    if (sChampions->isAlive())
                                        if (Creature* sNecrolyte = me->FindNearestCreature(36788, 150.0f, true))
                                        {
                                            sChampions->Attack(sNecrolyte, true);
                                            sChampions->GetMotionMaster()->MoveChase(sNecrolyte);
                                        }
                                }

                                GetCreatureListWithEntryInGrid(Champion3, me, NPC_CHAMPION_3_HORDE, 150.0f);
                                for(std::list<Creature*>::iterator itr = Champion3.begin(); itr != Champion3.end(); ++itr)
                                {
                                    Creature *sChampions = *itr;
                                    if (!sChampions)
                                        continue;

                                    if (sChampions->isAlive())
                                        if (Creature* sNecrolyte = me->FindNearestCreature(36788, 150.0f, true))
                                        {
                                            sChampions->Attack(sNecrolyte, true);
                                            sChampions->GetMotionMaster()->MoveChase(sNecrolyte);
                                        }
                                }
                            }
                            ++StartPhase;
                            StartTimer = 1000;
                            break;
                        case 3:
                            if (Creature* sTyrannus = me->FindNearestCreature(NPC_TYRANNUS_EVENTS, 150.0f, true))
                                DoScriptText(SAY_TYRRANUS_3, sTyrannus);
                            ++StartPhase;
                            StartTimer = 2000;
                            break;
                        case 4:
                            if (Creature* sTyrannus = me->FindNearestCreature(NPC_TYRANNUS_EVENTS, 150.0f, true))
                                DoPlaySoundToSet(sTyrannus, SAY_SOUND);

                            for(std::list<Creature*>::iterator itr = Champion1.begin(); itr != Champion1.end(); ++itr)
                            {
                                Creature *sChampions = *itr;
                                if (!sChampions)
                                    continue;

                                if (sChampions->isAlive())
                                {
                                    sChampions->AttackStop();
                                    sChampions->GetMotionMaster()->MoveFall();
                                }
                            }

                            for(std::list<Creature*>::iterator itr = Champion2.begin(); itr != Champion2.end(); ++itr)
                            {
                                Creature *sChampions = *itr;
                                if (!sChampions)
                                    continue;

                                if (sChampions->isAlive())
                                {
                                    sChampions->AttackStop();
                                    sChampions->GetMotionMaster()->MoveFall();
                                }
                            }

                            for(std::list<Creature*>::iterator itr = Champion3.begin(); itr != Champion3.end(); ++itr)
                            {
                                Creature *sChampions = *itr;
                                if (!sChampions)
                                    continue;

                                if (sChampions->isAlive())
                                {
                                    sChampions->AttackStop();
                                    sChampions->GetMotionMaster()->MoveFall();
                                }
                            }
                            ++StartPhase;
                            StartTimer = 5000;
                            break;
                        case 5:
                            if (Creature* sTyrannus = me->FindNearestCreature(NPC_TYRANNUS_EVENTS, 50.0f, true))
                                sTyrannus->CastSpell(sTyrannus, SPELL_NECROMANTIC_POWER, false);

                            for(std::list<Creature*>::iterator itr = Champion1.begin(); itr != Champion1.end(); ++itr)
                            {
                                Creature *sChampions = *itr;
                                if (!sChampions)
                                    continue;

                                if (sChampions->isAlive())
                                    sChampions->Kill(sChampions, false);
                            }

                            for(std::list<Creature*>::iterator itr = Champion2.begin(); itr != Champion2.end(); ++itr)
                            {
                                Creature *sChampions = *itr;
                                if (!sChampions)
                                    continue;

                                if (sChampions->isAlive())
                                    sChampions->Kill(sChampions, false);
                            }

                            for(std::list<Creature*>::iterator itr = Champion3.begin(); itr != Champion3.end(); ++itr)
                            {
                                Creature *sChampions = *itr;
                                if (!sChampions)
                                    continue;

                                if (sChampions->isAlive())
                                    sChampions->Kill(sChampions, false);
                            }

                            if(me->GetEntry() == NPC_SYLVANAS_PART1)
                                DoScriptText(SAY_SYLVANAS_2, me);
                            else if(me->GetEntry() == NPC_JAINA_PART1)
                                DoScriptText(SAY_JAINA_2, me);
                            ++StartPhase;
                            StartTimer = 5000;
                            break;
                        case 6:
                            for(std::list<Creature*>::iterator itr = Champion1.begin(); itr != Champion1.end(); ++itr)
                            {
                                Creature *sChampions = *itr;
                                if (!sChampions)
                                    continue;

                                sChampions->Respawn();
                                sChampions->CastSpell(sChampions, SPELL_DEATH_ARMY_VISUAL, true);
                                if(me->GetEntry() == NPC_SYLVANAS_PART1)
                                    sChampions->UpdateEntry(36796, ALLIANCE);
                                else
                                    sChampions->UpdateEntry(36796, HORDE);
                                sChampions->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                                sChampions->SetReactState(REACT_PASSIVE);
                            }

                            for(std::list<Creature*>::iterator itr = Champion2.begin(); itr != Champion2.end(); ++itr)
                            {
                                Creature *sChampions = *itr;
                                if (!sChampions)
                                    continue;

                                sChampions->Respawn();
                                sChampions->CastSpell(sChampions, SPELL_DEATH_ARMY_VISUAL, true);
                                if(me->GetEntry() == NPC_SYLVANAS_PART1)
                                    sChampions->UpdateEntry(36796, ALLIANCE);
                                else
                                    sChampions->UpdateEntry(36796, HORDE);
                                sChampions->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                                sChampions->SetReactState(REACT_PASSIVE);
                            }

                            for(std::list<Creature*>::iterator itr = Champion3.begin(); itr != Champion3.end(); ++itr)
                            {
                                Creature *sChampions = *itr;
                                if (!sChampions)
                                    continue;

                                sChampions->Respawn();
                                sChampions->CastSpell(sChampions, SPELL_DEATH_ARMY_VISUAL, true);
                                if(me->GetEntry() == NPC_SYLVANAS_PART1)
                                    sChampions->UpdateEntry(36796, ALLIANCE);
                                else
                                    sChampions->UpdateEntry(36796, HORDE);
                                sChampions->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                                sChampions->SetReactState(REACT_PASSIVE);
                            }

                            if (Creature* sTyrannus = me->FindNearestCreature(NPC_TYRANNUS_EVENTS, 50.0f, true))
                                DoScriptText(SAY_TYRRANUS_4, sTyrannus);

                            for(std::list<Creature*>::iterator itr = Champion1.begin(); itr != Champion1.end(); ++itr)
                            {
                                Creature *sChampions = *itr;
                                if (!sChampions)
                                    continue;

                                if (Creature* pHelper = me->FindNearestCreature(NPC_KORALEN, 150.0f, true))
                                {
                                    sChampions->Attack(pHelper, true);
                                    sChampions->GetMotionMaster()->MoveChase(pHelper);
                                }
                                else if (Creature* pHelper = me->FindNearestCreature(NPC_KORLAEN, 150.0f, true))
                                {
                                    sChampions->Attack(pHelper, true);
                                    sChampions->GetMotionMaster()->MoveChase(pHelper);
                                }
                            }

                            for(std::list<Creature*>::iterator itr = Champion2.begin(); itr != Champion2.end(); ++itr)
                            {
                                Creature *sChampions = *itr;
                                if (!sChampions)
                                    continue;

                                if (Creature* pHelper = me->FindNearestCreature(NPC_KORALEN, 150.0f, true))
                                {
                                    sChampions->Attack(pHelper, true);
                                    sChampions->GetMotionMaster()->MoveChase(pHelper);
                                }
                                else if (Creature* pHelper = me->FindNearestCreature(NPC_KORLAEN, 150.0f, true))
                                {
                                    sChampions->Attack(pHelper, true);
                                    sChampions->GetMotionMaster()->MoveChase(pHelper);
                                }
                            }

                            for(std::list<Creature*>::iterator itr = Champion3.begin(); itr != Champion3.end(); ++itr)
                            {
                                Creature *sChampions = *itr;
                                if (!sChampions)
                                    continue;

                                if (Creature* pHelper = me->FindNearestCreature(NPC_KORALEN, 150.0f, true))
                                {
                                    sChampions->Attack(pHelper, true);
                                    sChampions->GetMotionMaster()->MoveChase(pHelper);
                                }
                                else if (Creature* pHelper = me->FindNearestCreature(NPC_KORLAEN, 150.0f, true))
                                {
                                    sChampions->Attack(pHelper, true);
                                    sChampions->GetMotionMaster()->MoveChase(pHelper);
                                }
                            }
                            ++StartPhase;
                            StartTimer = 3000;
                            break;
                        case 7:
                            for(std::list<Creature*>::iterator itr = Champion1.begin(); itr != Champion1.end(); ++itr)
                            {
                                Creature *sChampions = *itr;
                                if (!sChampions)
                                    continue;

                                if(me->GetEntry() == NPC_SYLVANAS_PART1)
                                    me->CastSpell(sChampions, SPELL_MULTI_SHOT_SYLVANAS, true);
                                else
                                    me->CastSpell(sChampions, SPELL_BLIZZARD_JAINA, true);
                            }

                            for(std::list<Creature*>::iterator itr = Champion2.begin(); itr != Champion2.end(); ++itr)
                            {
                                Creature *sChampions = *itr;
                                if (!sChampions)
                                    continue;

                                if(me->GetEntry() == NPC_SYLVANAS_PART1)
                                    me->CastSpell(sChampions, SPELL_MULTI_SHOT_SYLVANAS, true);
                                else
                                    me->CastSpell(sChampions, SPELL_BLIZZARD_JAINA, true);
                            }

                            for(std::list<Creature*>::iterator itr = Champion3.begin(); itr != Champion3.end(); ++itr)
                            {
                                Creature *sChampions = *itr;
                                if (!sChampions)
                                    continue;

                                if(me->GetEntry() == NPC_SYLVANAS_PART1)
                                    me->CastSpell(sChampions, SPELL_MULTI_SHOT_SYLVANAS, true);
                                else
                                    me->CastSpell(sChampions, SPELL_BLIZZARD_JAINA, true);
                            }
                            ++StartPhase;
                            StartTimer = 500;
                            break;
                        case 8:
                            for(std::list<Creature*>::iterator itr = Champion1.begin(); itr != Champion1.end(); ++itr)
                            {
                                Creature *sChampions = *itr;
                                if (!sChampions)
                                    continue;

                                if(me->GetEntry() == NPC_SYLVANAS_PART1)
                                    me->CastSpell(sChampions, SPELL_SHADOW_BOLT_SYLVANAS, true);
                                else
                                    me->CastSpell(sChampions, SPELL_ICE_LANCE_JAINA, true);
                            }

                            for(std::list<Creature*>::iterator itr = Champion2.begin(); itr != Champion2.end(); ++itr)
                            {
                                Creature *sChampions = *itr;
                                if (!sChampions)
                                    continue;

                                if(me->GetEntry() == NPC_SYLVANAS_PART1)
                                    me->CastSpell(sChampions, SPELL_SHADOW_BOLT_SYLVANAS, true);
                                else
                                    me->CastSpell(sChampions, SPELL_ICE_LANCE_JAINA, true);
                            }

                            for(std::list<Creature*>::iterator itr = Champion3.begin(); itr != Champion3.end(); ++itr)
                            {
                                Creature *sChampions = *itr;
                                if (!sChampions)
                                    continue;

                                if(me->GetEntry() == NPC_SYLVANAS_PART1)
                                    me->CastSpell(sChampions, SPELL_SHADOW_BOLT_SYLVANAS, true);
                                else
                                    me->CastSpell(sChampions, SPELL_ICE_LANCE_JAINA, true);
                            }
                            ++StartPhase;
                            StartTimer = 500;
                            break;
                        case 9:
                            for(std::list<Creature*>::iterator itr = Champion1.begin(); itr != Champion1.end(); ++itr)
                            {
                                Creature *sChampions = *itr;
                                if (!sChampions)
                                    continue;
                                me->Kill(sChampions, false);
                            }

                            for(std::list<Creature*>::iterator itr = Champion2.begin(); itr != Champion2.end(); ++itr)
                            {
                                Creature *sChampions = *itr;
                                if (!sChampions)
                                    continue;

                                me->Kill(sChampions, false);
                            }

                            for(std::list<Creature*>::iterator itr = Champion3.begin(); itr != Champion3.end(); ++itr)
                            {
                                Creature *sChampions = *itr;
                                if (!sChampions)
                                    continue;

                                me->Kill(sChampions, false);
                            }
                            ++StartPhase;
                            StartTimer = 3000;
                            break;
                        case 10:
                            if(me->GetEntry() == NPC_JAINA_PART1)
                                DoScriptText(SAY_JAINA_3, me);
                            else 
                                DoScriptText(SAY_SYLVANAS_3, me);
                            ++StartPhase;
                            StartTimer = 10000;
                            break;
                        case 11:
                            if(me->GetEntry() == NPC_JAINA_PART1)
                            DoScriptText(SAY_JAINA_4, me);
                            else 
                                DoScriptText(SAY_SYLVANAS_4, me);
                            ++StartPhase;
                            StartTimer = 10000;
                            break;
                        case 12:
                            if (Creature* sTyrannus = me->FindNearestCreature(NPC_TYRANNUS_EVENTS, 150.0f, true))
                                sTyrannus->GetMotionMaster()->MovePoint(0, 938.125f, 93.928f, 585.39f);
                            instance->SetData(DATA_TYRANNUS_START, DONE);
                            event = false;
                            ++StartPhase;
                            StartTimer = 10000;
                            break;
                        return;					
                    }
                } else StartTimer -= diff;
            }
        }

    private:
        InstanceScript* instance;
        std::list<Creature*> Champion1;
        std::list<Creature*> Champion2;
        std::list<Creature*> Champion3;
        uint32 StartTimer;
        uint8 StartPhase;
        bool event;
    };
};
class at_pit_of_saron_start : public AreaTriggerScript
{
    public:
        at_pit_of_saron_start() : AreaTriggerScript("at_pit_of_saron_start") { }

        bool OnTrigger(Player* player, AreaTriggerEntry const* areaTrigger)
        {
            InstanceScript* instance = player->GetInstanceScript();
            if(instance->GetData(DATA_TYRANNUS_START) == IN_PROGRESS || instance->GetData(DATA_TYRANNUS_START) == DONE || player->isGameMaster() || !instance)
                      return false;
                      
               instance->SetData(DATA_TYRANNUS_START, IN_PROGRESS);
              
            return false;
        }
};

enum sTyrannus
{
     SAY_TYRANNUS_AMBUSH_1                      = -1658050,
     SAY_TYRANNUS_AMBUSH_2                      = -1658051,
     SAY_GAUNTLET_START                         = -1658052,
};

class at_ymirjar_flamebearer_pos : public AreaTriggerScript
{
    public:
        at_ymirjar_flamebearer_pos() : AreaTriggerScript("at_ymirjar_flamebearer_pos") { }

        bool OnTrigger(Player* player, AreaTriggerEntry const* areaTrigger)
        {
            InstanceScript* instance = player->GetInstanceScript();
               if(instance->GetData(DATA_AREA_TRIGGER_YMIRJAR) == DONE || player->isGameMaster() || !instance)
                      return false;
                      
                if (instance->GetBossState(DATA_GARFROST) == DONE && instance->GetBossState(DATA_ICK) == DONE)
                {
                   instance->SetData(DATA_AREA_TRIGGER_YMIRJAR, DONE);
                   if(Creature *pTyrannus = player->SummonCreature(NPC_TYRANNUS_EVENTS, 940.076355f, 91.047089f, 576.178040f, 3.596342f, TEMPSUMMON_DEAD_DESPAWN, 0))
                   {
                     DoScriptText(SAY_TYRANNUS_AMBUSH_1, pTyrannus);
                     pTyrannus->GetMotionMaster()->MovePoint(0, 898.798340f, -242.198914f, 695.351074f);
                   }
                   for (uint8 i = 0; i < 4; i++)
                   {
                      player->SummonCreature(NPC_YMIRJAR_FLAMEBEARER, spawnPoints1[i], TEMPSUMMON_DEAD_DESPAWN, 0);
                      player->SummonCreature(NPC_YMIRJAR_WRATBRINGER, spawnPoints2[i], TEMPSUMMON_DEAD_DESPAWN, 0);
                   }
                   for(uint8 k = 0; k < 2; k++)
                   {
                      player->SummonCreature(NPC_YMIRJAR_DEATHBRINGER, spawnPointsYmirjar[k], TEMPSUMMON_DEAD_DESPAWN, 0);
                   }
                   
                   return false;
                }
                return false;
        }
};
class at_fallen_warrior_pos : public AreaTriggerScript
{
    public:
        at_fallen_warrior_pos() : AreaTriggerScript("at_fallen_warrior_pos") { }

        bool OnTrigger(Player* player, AreaTriggerEntry const* areaTrigger)
        {
            InstanceScript* instance = player->GetInstanceScript();
            if(instance->GetData(DATA_AREA_TRIGGER_FALLEN) == DONE || player->isGameMaster() || !instance)
                      return false;
                      
              if (instance->GetData(DATA_AREA_TRIGGER_YMIRJAR) == DONE)  
                {
                   instance->SetData(DATA_AREA_TRIGGER_FALLEN, DONE);
                   if(Creature *pTyrannus = player->SummonCreature(NPC_TYRANNUS_EVENTS, 916.282104f, -71.079742f, 606.430359f, 1.326541f, TEMPSUMMON_DEAD_DESPAWN, 0))
                   {
                     DoScriptText(SAY_TYRANNUS_AMBUSH_2, pTyrannus);
                   }
                   for (uint8 i = 0; i < 8; i++)
                   {
                      player->SummonCreature(NPC_FALLEN_WARRIOR, spawnPointsFallenWarrior[i], TEMPSUMMON_DEAD_DESPAWN, 0);
                   }
                   return false;
                }
                
            return false;
        }
};
void AddSC_pit_of_saron()
{
    new mob_ymirjar_flamebearer();
    new mob_wrathbone_laborer();
    new mob_iceborn_protodrake();
    new mob_geist_ambusher();
    new spell_trash_mob_glacial_strike();
    new pitofsaron_start();
    new at_pit_of_saron_start();
    new at_ymirjar_flamebearer_pos();
    new at_fallen_warrior_pos();
}