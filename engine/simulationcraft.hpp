// ==========================================================================
// Dedmonwakeen's Raid DPS/TPS Simulator.
// Send questions to natehieter@gmail.com
// ==========================================================================
#ifndef SIMULATIONCRAFT_H
#define SIMULATIONCRAFT_H

#define SC_MAJOR_VERSION "623"
#define SC_MINOR_VERSION "03"
#define SC_VERSION ( SC_MAJOR_VERSION "-" SC_MINOR_VERSION )
#define SC_BETA 0
#if SC_BETA
  static const char* SC_BETA_STR = "legion";
#endif
#define SC_USE_STAT_CACHE

// Platform, compiler and general configuration
#include "config.hpp"
#include <algorithm>
#include <cassert>
#include <cctype>
#include <cfloat>
#include <cmath>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <limits>
#include <list>
#include <map>
#include <memory>
#include <numeric>
#include <queue>
#include <sstream>
#include <stack>
#include <string>
#include <typeinfo>
#include <vector>
#include <bitset>
#include <array>
#include <functional>
#include <memory>
#include <type_traits>
#include <unordered_map>
#include <atomic>
#include <random>
#if defined( SC_OSX )
#include <Availability.h>
#endif

// Forward Declarations =====================================================

struct absorb_buff_t;
struct action_callback_t;
struct action_priority_t;
struct action_priority_list_t;
struct action_state_t;
struct action_t;
struct actor_t;
struct alias_t;
struct attack_t;
struct benefit_t;
struct buff_t;
struct callback_t;
struct cooldown_t;
struct cost_reduction_buff_t;
class dbc_t;
struct debuff_t;
struct dot_t;
struct event_t;
struct expr_t;
struct gain_t;
struct haste_buff_t;
struct heal_t;
struct item_t;
struct instant_absorb_t;
struct module_t;
struct pet_t;
struct player_t;
struct plot_t;
struct proc_t;
struct reforge_plot_t;
struct scaling_t;
struct sim_t;
struct spell_data_t;
struct spell_id_t;
struct spelleffect_data_t;
struct spell_t;
struct stats_t;
struct stat_buff_t;
struct stat_pair_t;
struct travel_event_t;
struct xml_node_t;
struct action_cost_tick_event_t;
class xml_writer_t;
namespace highchart {
  struct chart_t;
}


#include "dbc/data_enums.hh"
#include "dbc/data_definitions.hh"
#include "util/utf8.h"

// Time class representing ingame time
#include "sc_timespan.hpp"

// Generic programming tools
#include "util/generic.hpp"

// Sample Data
#include "util/sample_data.hpp"

// Timeline
#include "util/timeline.hpp"

// Random Number Generators
#include "util/rng.hpp"

// String Utilities
#include "util/str.hpp"

// mutex, thread
#include "util/concurrency.hpp"

#include "sc_enums.hpp"

/* SimulationCraft timeline:
 * - data_type is double
 * - timespan_t add helper function
 */
struct sc_timeline_t : public timeline_t
{
  typedef timeline_t base_t;
  using timeline_t::add;
  double bin_size;

  sc_timeline_t() : timeline_t(), bin_size( 1.0 ) {}

  // methods to modify/retrieve the bin size
  void set_bin_size( double bin )
  {
    bin_size = bin;
  }
  double get_bin_size() const
  {
    return bin_size;
  }

  // Add 'value' at the corresponding time
  void add( timespan_t current_time, double value )
  { base_t::add( static_cast<size_t>( current_time.total_millis() / 1000 / bin_size ), value ); }

  // Add 'value' at corresponding time, replacing existing entry if new value is larger
  void add_max( timespan_t current_time, double new_value )
  {
    size_t index = static_cast<size_t>( current_time.total_millis() / 1000 / bin_size );
    if ( data().size() == 0 || data().size() <= index )
      add( current_time, new_value );
    else if ( new_value > data().at( index ) )
    {
      add( current_time, new_value - data().at( index ) );
    }
  }

  void adjust( sim_t& sim );

  void build_derivative_timeline( sc_timeline_t& out ) const
  { base_t::build_sliding_average_timeline( out, 20 ); }

private:
  static std::vector<double> build_divisor_timeline( const extended_sample_data_t& simulation_length, double bin_size );
};

// Cache Control ============================================================
#include "util/cache.hpp"

// Talent Translation =======================================================

const int MAX_TALENT_ROWS = 7;
const int MAX_TALENT_COLS = 3;
const int MAX_TALENT_SLOTS = MAX_TALENT_ROWS * MAX_TALENT_COLS;


// Utilities ================================================================

#if defined ( SC_VS ) && SC_VS < 13 // VS 2015 adds in support for a C99-compliant snprintf
// C99-compliant snprintf - MSVC _snprintf is NOT the same.

#undef vsnprintf
int vsnprintf_simc( char* buf, size_t size, const char* fmt, va_list ap );
#define vsnprintf vsnprintf_simc

#undef snprintf
inline int snprintf( char* buf, size_t size, const char* fmt, ... )
{
  va_list ap;
  va_start( ap, fmt );
  int rval = vsnprintf( buf, size, fmt, ap );
  va_end( ap );
  return rval;
}
#endif

#include "sc_util.hpp"

#include "util/stopwatch.hpp"
#include "sim/sc_option.hpp"

// Data Access ==============================================================
const int MAX_LEVEL = 100;
const int MAX_SCALING_LEVEL = 105;
const int MAX_ILEVEL = 1000;

// Include DBC Module
#include "dbc/dbc.hpp"

// Include IO Module
#include "util/io.hpp"

// Report ===================================================================

#include "report/sc_report.hpp"

// Spell information struct, holding static functions to output spell data in a human readable form

namespace spell_info
{
std::string to_str( const dbc_t& dbc, const spell_data_t* spell, int level = MAX_LEVEL );
void        to_xml( const dbc_t& dbc, const spell_data_t* spell, xml_node_t* parent, int level = MAX_LEVEL );
//static std::string to_str( sim_t* sim, uint32_t spell_id, int level = MAX_LEVEL );
std::string talent_to_str( const dbc_t& dbc, const talent_data_t* talent, int level = MAX_LEVEL );
std::string set_bonus_to_str( const dbc_t& dbc, const item_set_bonus_t* set_bonus, int level = MAX_LEVEL );
void        talent_to_xml( const dbc_t& dbc, const talent_data_t* talent, xml_node_t* parent, int level = MAX_LEVEL );
void        set_bonus_to_xml( const dbc_t& dbc, const item_set_bonus_t* talent, xml_node_t* parent, int level = MAX_LEVEL );
std::ostringstream& effect_to_str( const dbc_t& dbc, const spell_data_t* spell, const spelleffect_data_t* effect, std::ostringstream& s, int level = MAX_LEVEL );
void                effect_to_xml( const dbc_t& dbc, const spell_data_t* spell, const spelleffect_data_t* effect, xml_node_t*    parent, int level = MAX_LEVEL );
}

/* Luxurious sample data container with automatic merge/analyze,
 * intended to be used in class modules for custom reporting.
 * Iteration based sampling
 */
struct luxurious_sample_data_t : public extended_sample_data_t, private noncopyable
{
  luxurious_sample_data_t( player_t& p, std::string n );

  void add( double x )
  { buffer_value += x; }

  void datacollection_begin()
  {
    reset_buffer();
  }
  void datacollection_end()
  {
    write_buffer_as_sample();
  }
  player_t& player;
private:
  double buffer_value;
  void write_buffer_as_sample()
  {
    extended_sample_data_t::add( buffer_value );
    reset_buffer();
  }
  void reset_buffer()
  {
    buffer_value = 0.0;
  }
};

// Raid Event

struct raid_event_t : private noncopyable
{
  sim_t* sim;
  std::string name_str;
  int64_t num_starts;
  timespan_t first, last, next;
  timespan_t cooldown;
  timespan_t cooldown_stddev;
  timespan_t cooldown_min;
  timespan_t cooldown_max;
  timespan_t duration;
  timespan_t duration_stddev;
  timespan_t duration_min;
  timespan_t duration_max;
  std::string first_str, last_str;

  // Player filter options
  double     distance_min; // Minimal player distance
  double     distance_max; // Maximal player distance
  bool players_only; // Don't affect pets
  double player_chance; // Chance for individual player to be affected by raid event

  std::string affected_role_str;
  role_e     affected_role;

  timespan_t saved_duration;
  std::vector<player_t*> affected_players;
  std::vector<std::unique_ptr<option_t>> options;

  raid_event_t( sim_t*, const std::string& );
private:
  virtual void _start() = 0;
  virtual void _finish() = 0;
public:
  virtual ~raid_event_t() {}

  virtual bool filter_player( const player_t* );

  void add_option( std::unique_ptr<option_t> new_option )
  { options.insert( options.begin(), std::move(new_option) ); }
  timespan_t cooldown_time();
  timespan_t duration_time();
  timespan_t next_time() { return next; }
  double distance() { return distance_max; }
  double min_distance() { return distance_min; }
  double max_distance() { return distance_max; }
  void schedule();
  void reset();
  void start();
  void finish();
  void set_next( timespan_t t ) { next = t; }
  void parse_options( const std::string& options_str );
  static std::unique_ptr<raid_event_t> create( sim_t* sim, const std::string& name, const std::string& options_str );
  static void init( sim_t* );
  static void reset( sim_t* );
  static void combat_begin( sim_t* );
  static void combat_end( sim_t* ) {}
  const char* name() const { return name_str.c_str(); }
  static double evaluate_raid_event_expression(sim_t* s, std::string& type, std::string& filter );
};

// Gear Stats ===============================================================

struct gear_stats_t
{
  std::array<double, ATTRIBUTE_MAX> attribute;
  std::array<double, RESOURCE_MAX> resource;
  double spell_power;
  double attack_power;
  double expertise_rating;
  double expertise_rating2;
  double hit_rating;
  double hit_rating2;
  double crit_rating;
  double haste_rating;
  double weapon_dps;
  double weapon_speed;
  double weapon_offhand_dps;
  double weapon_offhand_speed;
  double armor;
  double bonus_armor;
  double dodge_rating;
  double parry_rating;
  double block_rating;
  double mastery_rating;
  double resilience_rating;
  double pvp_power;
  double multistrike_rating;
  double readiness_rating;
  double versatility_rating;
  double leech_rating;
  double speed_rating;
  double avoidance_rating;

  gear_stats_t() :
    attribute(), resource(),
    spell_power( 0.0 ), attack_power( 0.0 ), expertise_rating( 0.0 ), expertise_rating2( 0.0 ),
    hit_rating( 0.0 ), hit_rating2( 0.0 ), crit_rating( 0.0 ), haste_rating( 0.0 ), weapon_dps( 0.0 ), weapon_speed( 0.0 ),
    weapon_offhand_dps( 0.0 ), weapon_offhand_speed( 0.0 ), armor( 0.0 ), bonus_armor( 0.0 ), dodge_rating( 0.0 ),
    parry_rating( 0.0 ), block_rating( 0.0 ), mastery_rating( 0.0 ), resilience_rating( 0.0 ), pvp_power( 0.0 ),
    multistrike_rating( 0.0 ), readiness_rating( 0.0 ), versatility_rating( 0.0 ), leech_rating( 0.0 ), speed_rating( 0.0 ),
    avoidance_rating( 0.0 )
  { }

  friend gear_stats_t operator+( const gear_stats_t& left, const gear_stats_t& right )
  {
    gear_stats_t a = gear_stats_t( left );
    a += right;
    return a;
  }

  gear_stats_t& operator+=( const gear_stats_t& right )
  {
    spell_power += right.spell_power;
    attack_power += right.attack_power;
    expertise_rating += right.expertise_rating;
    expertise_rating2 += right.expertise_rating2;
    hit_rating += right.hit_rating;
    hit_rating2 += right.hit_rating2;
    crit_rating += right.crit_rating;
    haste_rating += right.haste_rating;
    weapon_dps += right.weapon_dps;
    weapon_speed += right.weapon_speed;
    weapon_offhand_dps += right.weapon_offhand_dps;
    weapon_offhand_speed += right.weapon_offhand_speed;
    armor += right.armor;
    bonus_armor += right.bonus_armor;
    dodge_rating += right.dodge_rating;
    parry_rating += right.parry_rating;
    block_rating += right.block_rating;
    mastery_rating += right.mastery_rating;
    resilience_rating += right.resilience_rating;
    pvp_power += right.pvp_power;
    multistrike_rating += right.multistrike_rating;
    readiness_rating += right.readiness_rating;
    versatility_rating += right.versatility_rating;
    leech_rating += right.leech_rating;
    speed_rating += right.speed_rating;
    avoidance_rating += right.avoidance_rating;
    range::transform ( attribute, right.attribute, attribute.begin(), std::plus<double>() );
    range::transform ( resource, right.resource, resource.begin(), std::plus<int>() );
    return *this;
  }

  void   add_stat( stat_e stat, double value );
  void   set_stat( stat_e stat, double value );
  double get_stat( stat_e stat ) const;
  std::string to_string();
  static double stat_mod( stat_e stat );
};

// Actor Pair ===============================================================

struct actor_pair_t
{
  player_t* target;
  player_t* source;

  actor_pair_t( player_t* target, player_t* source )
    : target( target ), source( source )
  {}

  actor_pair_t( player_t* p = nullptr )
    : target( p ), source( p )
  {}
};

struct actor_target_data_t : public actor_pair_t, private noncopyable
{
  struct atd_debuff_t
  {
    debuff_t* mark_of_doom;
    debuff_t* fel_burn;
  } debuff;

  struct atd_dot_t
  {
  } dot;

  actor_target_data_t( player_t* target, player_t* source );
};

// Uptime ==================================================================

struct uptime_common_t
{
private:
  timespan_t last_start;
  timespan_t iteration_uptime_sum;
public:
  simple_sample_data_t uptime_sum;

  uptime_common_t() :
    last_start( timespan_t::min() ),
    iteration_uptime_sum( timespan_t::zero() ),
    uptime_sum()
  { }
  void update( bool is_up, timespan_t current_time )
  {
    if ( is_up )
    {
      if ( last_start < timespan_t::zero() )
        last_start = current_time;
    }
    else if ( last_start >= timespan_t::zero() )
    {
      iteration_uptime_sum += current_time - last_start;
      reset();
    }
  }
  void datacollection_begin()
  { iteration_uptime_sum = timespan_t::zero(); }
  void datacollection_end( timespan_t t )
  { uptime_sum.add( t != timespan_t::zero() ? iteration_uptime_sum / t : 0.0 ); }
  void reset() { last_start = timespan_t::min(); }
  void merge( const uptime_common_t& other )
  { uptime_sum.merge( other.uptime_sum ); }
};

struct uptime_t : public uptime_common_t
{
  std::string name_str;

  uptime_t( const std::string& n ) :
    uptime_common_t(), name_str( n )
  {}

  const char* name() const
  { return name_str.c_str(); }
};

struct buff_uptime_t : public uptime_common_t
{
  buff_uptime_t() :
    uptime_common_t() {}
};

// Buff Creation ====================================================================
namespace buff_creation {

// This is the base buff creator class containing data to create a buff_t
struct buff_creator_basics_t
{
protected:
  actor_pair_t _player;
  sim_t* _sim;
  std::string _name;
  const spell_data_t* s_data;
  const item_t* item;
  double _chance;
  double _default_value;
  int _max_stack;
  timespan_t _duration, _cooldown, _period;
  int _quiet, _reverse, _activated, _can_cancel;
  int _affects_regen;
  buff_tick_behavior_e _behavior;
  buff_refresh_behavior_e _refresh_behavior;
  std::function<void(buff_t*, int, int)> _tick_callback;
  std::function<timespan_t(const buff_t*, const timespan_t&)> _refresh_duration_callback;
  std::vector<cache_e> _invalidate_list;
  friend struct ::buff_t;
  friend struct ::debuff_t;
private:
  void init();
public:
  buff_creator_basics_t( actor_pair_t, const std::string& name, const spell_data_t* = spell_data_t::nil() );
  buff_creator_basics_t( actor_pair_t, uint32_t id, const std::string& name );
  buff_creator_basics_t( sim_t*, const std::string& name, const spell_data_t* = spell_data_t::nil() );

  buff_creator_basics_t( actor_pair_t, const std::string& name, const spell_data_t* = spell_data_t::nil(), const item_t* item = nullptr );
  buff_creator_basics_t( actor_pair_t, uint32_t id, const std::string& name, const item_t* item = nullptr );
  buff_creator_basics_t( sim_t*, const std::string& name, const spell_data_t* = spell_data_t::nil(), const item_t* item = nullptr );
};

// This helper template is necessary so that reference functions of the classes inheriting from it return the type of the derived class.
// eg. buff_creator_helper_t<stat_buff_creator_t>::chance() will return a reference of type stat_buff_creator_t
template <typename T>
struct buff_creator_helper_t : public buff_creator_basics_t
{
  typedef T bufftype;
  typedef buff_creator_helper_t base_t;

public:
  buff_creator_helper_t( actor_pair_t q, const std::string& name, const spell_data_t* s = spell_data_t::nil(), const item_t* item = nullptr ) :
    buff_creator_basics_t( q, name, s, item ) {}
  buff_creator_helper_t( actor_pair_t q, uint32_t id, const std::string& name, const item_t* item = nullptr ) :
    buff_creator_basics_t( q, id, name, item ) {}
  buff_creator_helper_t( sim_t* sim, const std::string& name, const spell_data_t* s = spell_data_t::nil(), const item_t* item = nullptr ) :
    buff_creator_basics_t( sim, name, s, item ) {}

  bufftype& actors( actor_pair_t q )
  { _player = q; return *( static_cast<bufftype*>( this ) ); }
  bufftype& duration( timespan_t d )
  { _duration = d; return *( static_cast<bufftype*>( this ) ); }
  bufftype& period( timespan_t d )
  { _period = d; return *( static_cast<bufftype*>( this ) ); }
  bufftype& default_value( double v )
  { _default_value = v; return *( static_cast<bufftype*>( this ) ); }
  bufftype& chance( double c )
  { _chance = c; return *( static_cast<bufftype*>( this ) ); }
  bufftype& can_cancel( bool cc )
  { _can_cancel = cc; return *( static_cast<bufftype*>( this ) );  }
  bufftype& max_stack( unsigned ms )
  { _max_stack = ms; return *( static_cast<bufftype*>( this ) ); }
  bufftype& cd( timespan_t t )
  { _cooldown = t; return *( static_cast<bufftype*>( this ) ); }
  bufftype& reverse( bool r )
  { _reverse = r; return *( static_cast<bufftype*>( this ) ); }
  bufftype& quiet( bool q )
  { _quiet = q; return *( static_cast<bufftype*>( this ) ); }
  bufftype& activated( bool a )
  { _activated = a; return *( static_cast<bufftype*>( this ) ); }
  bufftype& spell( const spell_data_t* s )
  { s_data = s; return *( static_cast<bufftype*>( this ) ); }
  bufftype& add_invalidate( cache_e c )
  { _invalidate_list.push_back( c ); return *( static_cast<bufftype*>( this ) ); }
  bufftype& tick_behavior( buff_tick_behavior_e b )
  { _behavior = b; return *( static_cast<bufftype*>( this ) ); }
  bufftype& tick_callback( std::function<void(buff_t*, int, int)> cb )
  { _tick_callback = cb; return *( static_cast<bufftype*>( this ) ); }
  bufftype& affects_regen( bool state )
  { _affects_regen = state; return *( static_cast<bufftype*>( this ) ); }
  bufftype& refresh_behavior( buff_refresh_behavior_e b )
  { _refresh_behavior = b; return *( static_cast<bufftype*>( this ) ); }
  bufftype& refresh_duration_callback( std::function<timespan_t(const buff_t*, const timespan_t&)> cb )
  { _refresh_duration_callback = cb; return *( static_cast<bufftype*>( this ) ); }
};

struct buff_creator_t : public buff_creator_helper_t<buff_creator_t>
{
public:
  buff_creator_t( actor_pair_t q, const std::string& name, const spell_data_t* s = spell_data_t::nil(), const item_t* item = nullptr ) :
    base_t( q, name, s, item ) {}
  buff_creator_t( actor_pair_t q, uint32_t id, const std::string& name, const item_t* item = nullptr ) :
    base_t( q, id, name, item ) {}
  buff_creator_t( sim_t* sim, const std::string& name, const spell_data_t* s = spell_data_t::nil(), const item_t* item = nullptr ) :
    base_t( sim, name, s, item ) {}

  operator buff_t* () const;
  operator debuff_t* () const;
};

struct stat_buff_creator_t : public buff_creator_helper_t<stat_buff_creator_t>
{
private:

  struct buff_stat_t
  {
    stat_e stat;
    double amount;
    std::function<bool(const stat_buff_t&)> check_func;

    buff_stat_t( stat_e s, double a, std::function<bool(const stat_buff_t&)> c = std::function<bool(const stat_buff_t&)>() ) :
      stat( s ), amount( a ), check_func( c ) {}
  };

  std::vector<buff_stat_t> stats;

  friend struct ::stat_buff_t;
public:
  stat_buff_creator_t( actor_pair_t q, const std::string& name, const spell_data_t* s = spell_data_t::nil(), const item_t* item = nullptr ) :
    base_t( q, name, s, item ) {}
  stat_buff_creator_t( sim_t* sim, const std::string& name, const spell_data_t* s = spell_data_t::nil(), const item_t* item = nullptr ) :
    base_t( sim, name, s, item ) {}

  bufftype& add_stat( stat_e s, double a, std::function<bool(const stat_buff_t&)> c = std::function<bool(const stat_buff_t&)>() )
  { stats.push_back( buff_stat_t( s, a, c ) ); return *this; }

  operator stat_buff_t* () const;
};

struct absorb_buff_creator_t : public buff_creator_helper_t<absorb_buff_creator_t>
{
private:
  school_e _absorb_school;
  stats_t* _absorb_source;
  gain_t*  _absorb_gain;
  bool     _high_priority; // For tank absorbs that should explicitly "go first"
  std::function< bool( const action_state_t* ) > _eligibility; // A custom function whose result determines if the attack is eligible to be absorbed.
  friend struct ::absorb_buff_t;
public:
  absorb_buff_creator_t( actor_pair_t q, const std::string& name, const spell_data_t* s = spell_data_t::nil(), const item_t* i = nullptr ) :
    base_t( q, name, s, i ),
    _absorb_school( SCHOOL_CHAOS ), _absorb_source( nullptr ), _absorb_gain( nullptr ), _high_priority( false )
  { }

  absorb_buff_creator_t( sim_t* sim, const std::string& name, const spell_data_t* s = spell_data_t::nil(), const item_t* i = nullptr ) :
    base_t( sim, name, s, i ),
    _absorb_school( SCHOOL_CHAOS ), _absorb_source( nullptr ), _absorb_gain( nullptr ), _high_priority( false )
  { }

  bufftype& source( stats_t* s )
  { _absorb_source = s; return *this; }

  bufftype& school( school_e school )
  { _absorb_school = school; return *this; }

  bufftype& gain( gain_t* g )
  { _absorb_gain = g; return *this; }

  bufftype& high_priority( bool h )
  { _high_priority = h; return *this; }

  bufftype& eligibility( std::function<bool(const action_state_t*)> e )
  { _eligibility = e; return *this; }

  operator absorb_buff_t* () const;
};

struct cost_reduction_buff_creator_t : public buff_creator_helper_t<cost_reduction_buff_creator_t>
{
private:
  double _amount;
  school_e _school;
  friend struct ::cost_reduction_buff_t;
public:
  cost_reduction_buff_creator_t( actor_pair_t q, const std::string& name, const spell_data_t* s = spell_data_t::nil(), const item_t* i = nullptr ) :
    base_t( q, name, s, i ),
    _amount( 0 ), _school( SCHOOL_NONE )
  {}

  cost_reduction_buff_creator_t( sim_t* sim, const std::string& name, const spell_data_t* s = spell_data_t::nil(), const item_t* i = nullptr ) :
    base_t( sim, name, s, i ),
    _amount( 0 ), _school( SCHOOL_NONE )
  {}

  bufftype& amount( double a )
  { _amount = a; return *this; }
  bufftype& school( school_e s )
  { _school = s; return *this; }

  operator cost_reduction_buff_t* () const;
};

struct haste_buff_creator_t : public buff_creator_helper_t<haste_buff_creator_t>
{
private:
  friend struct ::haste_buff_t;
public:
  haste_buff_creator_t( actor_pair_t q, const std::string& name, const spell_data_t* s = spell_data_t::nil(), const item_t* i = nullptr ) :
    base_t( q, name, s, i )
  { }

  operator haste_buff_t* () const;
};

} // END NAMESPACE buff_creation

using namespace buff_creation;

// Buffs ====================================================================

struct buff_t : private noncopyable
{
public:
  sim_t* const sim;
  player_t* const player;
  const item_t* const item;
  const std::string name_str;
  const spell_data_t* s_data;
  player_t* const source;
  event_t* expiration;
  event_t* delay;
  event_t* expiration_delay;
  cooldown_t* cooldown;
  sc_timeline_t uptime_array;

  // static values
private: // private because changing max_stacks requires resizing some stack-dependant vectors
  int _max_stack;
  std::vector<cache_e> invalidate_list;
public:
  double default_value;
  bool activated, reactable;
  bool reverse, constant, quiet, overridden, can_cancel;
  bool requires_invalidation;

  // dynamic values
  double current_value;
  int current_stack;
  timespan_t buff_duration;
  double default_chance;
  std::vector<timespan_t> stack_occurrence, stack_react_time;
  std::vector<event_t*> stack_react_ready_triggers;

  buff_refresh_behavior_e refresh_behavior;
  std::function<timespan_t(const buff_t*, const timespan_t&)> refresh_duration_callback;

  // Ticking buff values
  timespan_t buff_period;
  buff_tick_behavior_e tick_behavior;
  event_t* tick_event;
  std::function<void(buff_t*, int, int)> tick_callback;

  // tmp data collection
protected:
  timespan_t last_start;
  timespan_t last_trigger;
  timespan_t iteration_uptime_sum;
  timespan_t last_benefite_update;
  unsigned int up_count, down_count, start_count, refresh_count;
  unsigned int overflow_count, overflow_total;
  int trigger_attempts, trigger_successes;
  int simulation_max_stack;

  // report data
public:
  simple_sample_data_t benefit_pct, trigger_pct;
  simple_sample_data_t avg_start, avg_refresh;
  simple_sample_data_t avg_overflow_count, avg_overflow_total;
  simple_sample_data_t uptime_pct, start_intervals, trigger_intervals;
  std::vector<buff_uptime_t> stack_uptime;

  virtual ~buff_t() {}

protected:
  buff_t( const buff_creator_basics_t& params );
  friend struct buff_creation::buff_creator_t;
public:
  const spell_data_t& data() const { return *s_data; }

  /**
   * Get current number of stacks, no benefit tracking.
   * Use check() inside of ready() and cost() methods to prevent skewing of "benefit" calculations.
   * Use up() where the presence of the buff affects the action mechanics.
   */
  int check() const
  {
    return current_stack;
  }

  /**
   * Get current number of stacks + benefit tracking.
   * Use check() inside of ready() and cost() methods to prevent skewing of "benefit" calculations.
   * This will only modify the benefit counters once per sim time unit. 
   * Use up()/down where the presence of the buff affects the action mechanics.
   */
  int stack();

  /**
   * Check if buff is up
   * Use check() inside of ready() and cost() methods to prevent skewing of "benefit" calculations.
   * Use up()/down where the presence of the buff affects the action mechanics.
   */
  bool up()
  {
    return stack() > 0;
  }

  /**
   * Get current buff value + benefit tracking.
   */
  virtual double value()
  {
    stack();
    return current_value;
  }

  /**
   * Get current buff value  multiplied by current stacks + benefit tracking.
   */
  double stack_value()
  {
    return current_stack * value();
  }

  /**
   * Get current buff value + NO benefit tracking.
   */
  double check_value()
  {
    return current_value;
  }

  /**
   * Get current buff value  multiplied by current stacks + NO benefit tracking.
   */
  double check_stack_value()
  {
    return current_stack * check_value();
  }

  timespan_t remains() const;
  timespan_t elapsed( const timespan_t& t ) const { return t - last_start; }
  bool   remains_gt( timespan_t time ) const;
  bool   remains_lt( timespan_t time ) const;
  bool   trigger  ( action_t*, int stacks = 1, double value = DEFAULT_VALUE(), timespan_t duration = timespan_t::min() );
  virtual bool   trigger  ( int stacks = 1, double value = DEFAULT_VALUE(), double chance = -1.0, timespan_t duration = timespan_t::min() );
  virtual void   execute ( int stacks = 1, double value = DEFAULT_VALUE(), timespan_t duration = timespan_t::min() );
  virtual void   increment( int stacks = 1, double value = DEFAULT_VALUE(), timespan_t duration = timespan_t::min() );
  virtual void   decrement( int stacks = 1, double value = DEFAULT_VALUE() );
  virtual void   extend_duration( player_t* p, timespan_t seconds );

  virtual void start    ( int stacks = 1, double value = DEFAULT_VALUE(), timespan_t duration = timespan_t::min() );
  virtual void refresh  ( int stacks = 0, double value = DEFAULT_VALUE(), timespan_t duration = timespan_t::min() );
  virtual void bump     ( int stacks = 1, double value = DEFAULT_VALUE() );
  virtual void override_buff ( int stacks = 1, double value = DEFAULT_VALUE() );
  virtual bool may_react( int stacks = 1 );
  virtual int stack_react();
  void expire( timespan_t delay = timespan_t::zero() );

  // Called only if previously active buff expires
  virtual void expire_override( int /* expiration_stacks */, timespan_t /* remaining_duration */ ) {}
  virtual void predict();
  virtual void reset();
  virtual void aura_gain();
  virtual void aura_loss();
  virtual void merge( const buff_t& other_buff );
  virtual void analyze();
  virtual void datacollection_begin();
  virtual void datacollection_end();

  virtual timespan_t refresh_duration( const timespan_t& new_duration ) const;

  void add_invalidate( cache_e );
#if defined(SC_USE_STAT_CACHE)
  virtual void invalidate_cache();
#else
  void invalidate_cache() {}
#endif

  virtual int total_stack();

  static expr_t* create_expression( std::string buff_name,
                                    action_t* action,
                                    const std::string& type,
                                    buff_t* static_buff = nullptr );
  std::string to_str() const;

  static double DEFAULT_VALUE() { return -std::numeric_limits< double >::min(); }
  static buff_t* find( const std::vector<buff_t*>&, const std::string& name, player_t* source = nullptr );
  static buff_t* find(    sim_t*, const std::string& name );
  static buff_t* find( player_t*, const std::string& name, player_t* source = nullptr );
  static buff_t* find_expressable( const std::vector<buff_t*>&, const std::string& name, player_t* source = nullptr );

  const char* name() const { return name_str.c_str(); }
  std::string source_name() const;
  int max_stack() const { return _max_stack; }

  rng::rng_t& rng();

  bool change_regen_rate;
};

struct stat_buff_t : public buff_t
{
  struct buff_stat_t
  {
    stat_e stat;
    double amount;
    double current_value;
    std::function<bool(const stat_buff_t&)> check_func;

    buff_stat_t( stat_e s, double a, std::function<bool(const stat_buff_t&)> c = std::function<bool(const stat_buff_t&)>() ) :
      stat( s ), amount( a ), current_value( 0 ), check_func( c ) {}
  };
  std::vector<buff_stat_t> stats;
  gain_t* stat_gain;

  virtual void bump     ( int stacks = 1, double value = -1.0 ) override;
  virtual void decrement( int stacks = 1, double value = -1.0 ) override;
  virtual void expire_override( int expiration_stacks, timespan_t remaining_duration ) override;
  virtual double value() override{ stack(); return stats[ 0 ].current_value; }

protected:
  stat_buff_t( const stat_buff_creator_t& params );
  friend struct buff_creation::stat_buff_creator_t;
};

struct absorb_buff_t : public buff_t
{
  school_e absorb_school;
  stats_t* absorb_source;
  gain_t*  absorb_gain;
  bool     high_priority; // For tank absorbs that should explicitly "go first"
  std::function< bool( const action_state_t* ) > eligibility; // A custom function whose result determines if the attack is eligible to be absorbed.

protected:
  absorb_buff_t( const absorb_buff_creator_t& params );
  friend struct buff_creation::absorb_buff_creator_t;

  // Hook for derived classes to recieve notification when some of the absorb is consumed.
  // Called after the adjustment to current_value.
  virtual void absorb_used( double /* amount */ ) {}

public:
  virtual void start( int stacks = 1, double value = DEFAULT_VALUE(), timespan_t duration = timespan_t::min() ) override;
  virtual void expire_override( int expiration_stacks, timespan_t remaining_duration ) override;

  double consume( double amount );
};

struct cost_reduction_buff_t : public buff_t
{
  double amount;
  school_e school;

protected:
  cost_reduction_buff_t( const cost_reduction_buff_creator_t& params );
  friend struct buff_creation::cost_reduction_buff_creator_t;
public:
  virtual void bump     ( int stacks = 1, double value = -1.0 ) override;
  virtual void decrement( int stacks = 1, double value = -1.0 ) override;
  virtual void expire_override( int expiration_stacks, timespan_t remaining_duration ) override;
};

struct haste_buff_t : public buff_t
{
protected:
  haste_buff_t( const haste_buff_creator_t& params );
  friend struct buff_creation::haste_buff_creator_t;
public:
  virtual void execute( int stacks = 1, double value = -1.0, timespan_t duration = timespan_t::min() ) override;
  virtual void expire_override( int expiration_stacks, timespan_t remaining_duration ) override;
};

struct debuff_t : public buff_t
{
protected:
  debuff_t( const buff_creator_basics_t& params );
  friend struct buff_creation::buff_creator_t;
};

typedef struct buff_t aura_t;

// Expressions ==============================================================

enum token_e
{
  TOK_UNKNOWN = 0,
  TOK_PLUS,
  TOK_MINUS,
  TOK_MULT,
  TOK_DIV,
  TOK_ADD,
  TOK_SUB,
  TOK_AND,
  TOK_OR,
  TOK_XOR,
  TOK_NOT,
  TOK_EQ,
  TOK_NOTEQ,
  TOK_LT,
  TOK_LTEQ,
  TOK_GT,
  TOK_GTEQ,
  TOK_LPAR,
  TOK_RPAR,
  TOK_IN,
  TOK_NOTIN,
  TOK_NUM,
  TOK_STR,
  TOK_ABS,
  TOK_SPELL_LIST,
  TOK_FLOOR,
  TOK_CEIL
};

struct expr_token_t
{
  token_e type;
  std::string label;
};

struct expression_t
{
  static int precedence( token_e );
  static bool is_unary( token_e );
  static bool is_binary( token_e );
  static token_e next_token( action_t* action, const std::string& expr_str, int& current_index,
                             std::string& token_str, token_e prev_token );
  static std::vector<expr_token_t> parse_tokens( action_t* action, const std::string& expr_str );
  static void print_tokens( std::vector<expr_token_t>& tokens, sim_t* sim );
  static void convert_to_unary( std::vector<expr_token_t>& tokens );
  static bool convert_to_rpn( std::vector<expr_token_t>& tokens );
};

// Action expression types ==================================================

struct expr_t
{
  expr_t( const std::string&, token_e op = TOK_UNKNOWN )
    : op_( op )
#if !defined( NDEBUG )
      ,
      id_( get_global_id() )
#endif
  {
  }
  virtual ~expr_t()
  {
  }

  virtual const char* name() const
  { return "anonymous expression"; }
  int id() const
  {
#if !defined( NDEBUG )
    return id_;
#else
    return -1;
#endif
  }

  double eval() { return evaluate(); }
  bool success() { return eval() != 0; }

  static expr_t* parse( action_t*, const std::string& expr_str, bool optimize=false );
  static expr_t* create_constant( const std::string& name, double value );

  template <typename T> static double coerce( T t ) { return static_cast<double>( t ); }
  static double coerce( timespan_t t ) { return t.total_seconds(); }

  virtual expr_t* optimize( int /* spacing */ = 0 ) { /* spacing = 0; */ return this; }
  virtual double evaluate() = 0;

  virtual bool is_constant( double* /*return_value*/ ) { return false; }
  bool always_true()  { double v; return is_constant( &v ) && v != 0.0; }
  bool always_false() { double v; return is_constant( &v ) && v == 0.0; }

  token_e op_;
private:
#if !defined( NDEBUG )
  int id_;

  int get_global_id()
  {
    return ++unique_id;
  }

  static std::atomic<int> unique_id;
#endif
};

// Reference Expression - ref_expr_t
// Class Template to create a expression with a reference ( ref ) of arbitrary type T, and evaluate that reference
template <typename T>
struct ref_expr_t : public expr_t
{
public:
  ref_expr_t( const std::string& name, const T& t_ ) :
    expr_t( name ), t( t_ )
  {}
private:
  const T& t;
  virtual double evaluate() override { return coerce( t ); }

};

// Template to return a reference expression
template <typename T>
inline expr_t* make_ref_expr( const std::string& name, T& t )
{ return new ref_expr_t<T>( name, const_cast<const T&>( t ) ); }

// Function Expression - fn_expr_t
// Class Template to create a function ( fn ) expression with arbitrary functor f, which gets evaluated
template <typename F>
struct fn_expr_t : public expr_t
{
public:
  fn_expr_t( const std::string& name, F f_ ) :
    expr_t( name ), f( f_ ) {}
private:
  F f;

  virtual double evaluate() override { return coerce( f() ); }

};

struct target_wrapper_expr_t : public expr_t
{
  action_t& action;
  std::vector<expr_t*> proxy_expr;
  std::string suffix_expr_str;

  // Inlined
  target_wrapper_expr_t( action_t& a, const std::string& name_str, const std::string& expr_str );
  virtual player_t* target() const;
  double evaluate() override;

  ~target_wrapper_expr_t()
  { range::dispose( proxy_expr ); }
};

// Template to return a function expression
template <typename F>
inline expr_t* make_fn_expr( const std::string& name, F f )
{ return new fn_expr_t<F>( name, f ); }

// Make member function expression - make_mem_fn_expr
// Template to return function expression that calls a member ( mem ) function ( fn ) f on reference t.
template <typename F, typename T>
inline expr_t* make_mem_fn_expr( const std::string& name, T& t, F f )
{ return make_fn_expr( name, std::bind( std::mem_fn( f ), &t ) ); }

// Spell query expression types =============================================

enum expr_data_e
{
  DATA_SPELL = 0,
  DATA_TALENT,
  DATA_EFFECT,
  DATA_TALENT_SPELL,
  DATA_CLASS_SPELL,
  DATA_RACIAL_SPELL,
  DATA_MASTERY_SPELL,
  DATA_SPECIALIZATION_SPELL,
  DATA_GLYPH_SPELL,
  DATA_SET_BONUS_SPELL,
  DATA_PERK_SPELL,
  DATA_SET_BONUS,
};

struct spell_data_expr_t
{
  std::string name_str;
  sim_t* sim;
  expr_data_e data_type;
  bool effect_query;

  token_e result_tok;
  double result_num;
  std::vector<uint32_t> result_spell_list;
  std::string result_str;

  spell_data_expr_t( sim_t* sim, const std::string& n, expr_data_e dt = DATA_SPELL, bool eq = false, token_e t = TOK_UNKNOWN )
    : name_str( n ), sim( sim ), data_type( dt ),         effect_query( eq ),  result_tok( t ),            result_num( 0 ),            result_spell_list(),               result_str( "" ) {}
  spell_data_expr_t( sim_t* sim, const std::string& n, double       constant_value )
    : name_str( n ), sim( sim ), data_type( DATA_SPELL ), effect_query( false ), result_tok( TOK_NUM ),        result_num( constant_value ), result_spell_list(),               result_str( "" ) {}
  spell_data_expr_t( sim_t* sim, const std::string& n, const std::string& constant_value )
    : name_str( n ), sim( sim ), data_type( DATA_SPELL ), effect_query( false ), result_tok( TOK_STR ),        result_num( 0.0 ),            result_spell_list(),               result_str( constant_value ) {}
  spell_data_expr_t( sim_t* sim, const std::string& n, const std::vector<uint32_t>& constant_value )
    : name_str( n ), sim( sim ), data_type( DATA_SPELL ), effect_query( false ), result_tok( TOK_SPELL_LIST ), result_num( 0.0 ),            result_spell_list( constant_value ), result_str( "" ) {}
  virtual ~spell_data_expr_t() {}
  virtual int evaluate() { return result_tok; }
  const char* name() { return name_str.c_str(); }

  virtual std::vector<uint32_t> operator|( const spell_data_expr_t& /* other */ ) { return std::vector<uint32_t>(); }
  virtual std::vector<uint32_t> operator&( const spell_data_expr_t& /* other */ ) { return std::vector<uint32_t>(); }
  virtual std::vector<uint32_t> operator-( const spell_data_expr_t& /* other */ ) { return std::vector<uint32_t>(); }

  virtual std::vector<uint32_t> operator<( const spell_data_expr_t& /* other */ ) { return std::vector<uint32_t>(); }
  virtual std::vector<uint32_t> operator>( const spell_data_expr_t& /* other */ ) { return std::vector<uint32_t>(); }
  virtual std::vector<uint32_t> operator<=( const spell_data_expr_t& /* other */ ) { return std::vector<uint32_t>(); }
  virtual std::vector<uint32_t> operator>=( const spell_data_expr_t& /* other */ ) { return std::vector<uint32_t>(); }
  virtual std::vector<uint32_t> operator==( const spell_data_expr_t& /* other */ ) { return std::vector<uint32_t>(); }
  virtual std::vector<uint32_t> operator!=( const spell_data_expr_t& /* other */ ) { return std::vector<uint32_t>(); }

  virtual std::vector<uint32_t> in( const spell_data_expr_t& /* other */ ) { return std::vector<uint32_t>(); }
  virtual std::vector<uint32_t> not_in( const spell_data_expr_t& /* other */ ) { return std::vector<uint32_t>(); }

  static spell_data_expr_t* parse( sim_t* sim, const std::string& expr_str );
  static spell_data_expr_t* create_spell_expression( sim_t* sim, const std::string& name_str );
};

// Iteration data entry for replayability
struct iteration_data_entry_t
{
  double   metric;
  uint64_t seed;
  std::vector <uint64_t> target_health;

  iteration_data_entry_t( double m, uint64_t s, uint64_t h ) :
    metric( m ), seed( s )
  { target_health.push_back( h ); }

  iteration_data_entry_t( double m, uint64_t s ) :
    metric( m ), seed( s )
  { }

  void add_health( uint64_t h )
  { target_health.push_back( h ); }
};

// Simulation Setup =========================================================

struct player_description_t
{
  // Add just enough to describe a player

  // name, class, talents, glyphs, gear, professions, actions explicitly stored
  std::string name;
  // etc

  // flesh out API, these functions cannot depend upon sim_t
  // ideally they remain static, but if not then move to sim_control_t
  static void load_bcp    ( player_description_t& /*etc*/ );
  static void load_wowhead( player_description_t& /*etc*/ );
};

struct combat_description_t
{
  std::string name;
  int target_seconds;
  std::string raid_events;
  // etc
};

struct sim_control_t
{
  combat_description_t combat;
  std::vector<player_description_t> players;
  option_db_t options;
};

// Progress Bar =============================================================

struct progress_bar_t
{
  sim_t& sim;
  int steps, updates, interval;
  double start_time;
  std::string status;

  progress_bar_t( sim_t& s );
  void init();
  bool update( bool finished = false );
};

/* Encapsulated Vector
 * const read access
 * Modifying the vector triggers registered callbacks
 */
template <typename T>
struct vector_with_callback
{
private:
  std::vector<T> _data;
  std::vector<std::function<void(T)> > _callbacks ;
public:
  /* Register your custom callback, which will be called when the vector is modified
   */
  void register_callback( std::function<void(T)> c )
  {
    if ( c )
      _callbacks.push_back( c );
  }

  typename std::vector<T>::iterator begin()
  { return _data.begin(); }
  typename std::vector<T>::const_iterator begin() const
  { return _data.begin(); }
  typename std::vector<T>::iterator end()
  { return _data.end(); }
  typename std::vector<T>::const_iterator end() const
  { return _data.end(); }

  typedef typename std::vector<T>::iterator iterator;

  void trigger_callbacks(T v) const
  {
    for ( size_t i = 0; i < _callbacks.size(); ++i )
      _callbacks[i](v);
  }

  void push_back( T x )
  { _data.push_back( x ); trigger_callbacks( x ); }

  void find_and_erase( T x )
  {
    typename std::vector<T>::iterator it = range::find( _data, x );
    if ( it != _data.end() )
      erase( it );
  }

  void find_and_erase_unordered( T x )
  {
    typename std::vector<T>::iterator it = range::find( _data, x );
    if ( it != _data.end() )
      erase_unordered( it );
  }

  // Warning: If you directly modify the vector, you need to trigger callbacks manually!
  std::vector<T>& data()
  { return _data; }

  const std::vector<T>& data() const
  { return _data; }

  player_t* operator[]( size_t i ) const
  { return _data[ i ]; }

  size_t size() const
  { return _data.size(); }

  bool empty() const
  { return _data.empty(); }

private:
  void erase_unordered( typename std::vector<T>::iterator it )
  {
    T _v = *it;
    ::erase_unordered( _data, it );
    trigger_callbacks( _v );
  }

  void erase( typename std::vector<T>::iterator it )
  {
    T _v = *it;
    _data.erase( it );
    trigger_callbacks( _v );
  }
};

/* Unformatted SimC output class.
 */
struct sc_raw_ostream_t {
  template <class T>
  sc_raw_ostream_t & operator<< (T const& rhs)
  { (*_stream) << rhs; return *this; }
  sc_raw_ostream_t& printf( const char* format, ... );
  sc_raw_ostream_t( std::shared_ptr<std::ostream> os ) :
    _stream( os ) {}
  const sc_raw_ostream_t operator=( std::shared_ptr<std::ostream> os )
  { _stream = os; return *this; }
  std::ostream* get_stream()
  { return _stream.get(); }
private:
  std::shared_ptr<std::ostream> _stream;
};

/* Formatted SimC output class.
 */
struct sim_ostream_t
{
  struct no_close {};

  explicit sim_ostream_t( sim_t& s, std::shared_ptr<std::ostream> os ) :
      sim(s),
    _raw( os )
  {
  }
  sim_ostream_t( sim_t& s, std::ostream* os, no_close ) :
      sim(s),
    _raw( std::shared_ptr<std::ostream>( os, dont_close ) )
  {}
  const sim_ostream_t operator=( std::shared_ptr<std::ostream> os )
  { _raw = os; return *this; }

  sc_raw_ostream_t& raw()
  { return _raw; }

  std::ostream* get_stream()
  { return _raw.get_stream(); }
  template <class T>
  sim_ostream_t & operator<< (T const& rhs);
  sim_ostream_t& printf( const char* format, ... );
private:
  static void dont_close( std::ostream* ) {}
  sim_t& sim;
  sc_raw_ostream_t _raw;
};

struct sim_report_information_t
{
  bool charts_generated;
  std::vector<std::string> dps_charts, priority_dps_charts, hps_charts, dtps_charts, tmi_charts, dpet_charts, apm_charts;
  std::string timeline_chart, downtime_chart;
  sim_report_information_t() { charts_generated = false; }
};

#ifndef NDEBUG
#define ACTOR_EVENT_BOOKKEEPING 1
#else
#define ACTOR_EVENT_BOOKKEEPING 0
#endif

// Event Manager ============================================================

struct event_manager_t
{
  sim_t* sim;
  timespan_t current_time;
  uint64_t events_remaining;
  uint64_t events_processed;
  uint64_t total_events_processed;
  uint64_t max_events_remaining;
  unsigned timing_slice, global_event_id;
  std::vector<event_t*> timing_wheel;
  event_t* recycled_event_list;
  int    wheel_seconds, wheel_size, wheel_mask, wheel_shift;
  double wheel_granularity;
  timespan_t wheel_time;
  std::vector<event_t*> allocated_events;

  stopwatch_t event_stopwatch;
  bool monitor_cpu;
  bool canceled;
#ifdef EVENT_QUEUE_DEBUG
  unsigned max_queue_depth, n_allocated_events, n_end_insert, n_requested_events;
  uint64_t events_traversed, events_added;
  std::vector<std::pair<unsigned, unsigned> > event_queue_depth_samples;
  std::vector<unsigned> event_requested_size_count;
#endif /* EVENT_QUEUE_DEBUG */

  event_manager_t( sim_t* );
 ~event_manager_t();
  void* allocate_event( std::size_t size );
  void recycle_event( event_t* );
  void add_event( event_t*, timespan_t delta_time );
  void reschedule_event( event_t* );
  event_t* next_event();
  bool execute();
  void cancel();
  void flush();
  void init();
  void reset();
  void merge( event_manager_t& other );
};

// Simulation Engine ========================================================

struct sim_t : private sc_thread_t
{
  event_manager_t event_mgr;

  // Output
  sim_ostream_t out_std;
  sim_ostream_t out_log;
  sim_ostream_t out_debug;
  bool debug;

  // Iteration Controls
  timespan_t max_time, expected_iteration_time;
  double vary_combat_length;
  int current_iteration, iterations;
  bool canceled;
  double target_error;
  double current_error;
  double current_mean;
  int analyze_error_interval;

  sim_control_t* control;
  sim_t*      parent;
  bool initialized;
  player_t*   target;
  player_t*   heal_target;
  vector_with_callback<player_t*> target_list;
  vector_with_callback<player_t*> target_non_sleeping_list;
  vector_with_callback<player_t*> player_list;
  vector_with_callback<player_t*> player_no_pet_list;
  vector_with_callback<player_t*> player_non_sleeping_list;
  vector_with_callback<player_t*> healing_no_pet_list;
  vector_with_callback<player_t*> healing_pet_list;
  player_t*   active_player;
  int         num_players;
  int         num_enemies;
  int         num_tanks;
  int         enemy_targets;
  int         healing; // Creates healing targets. Useful for ferals, I guess.
  int global_spawn_index;
  int         max_player_level;
  timespan_t  queue_lag, queue_lag_stddev;
  timespan_t  gcd_lag, gcd_lag_stddev;
  timespan_t  channel_lag, channel_lag_stddev;
  timespan_t  queue_gcd_reduction;
  int         strict_gcd_queue;
  double      confidence, confidence_estimator;
  // Latency
  timespan_t  world_lag, world_lag_stddev;
  double      travel_variance, default_skill;
  timespan_t  reaction_time, regen_periodicity;
  timespan_t  ignite_sampling_delta;
  bool        fixed_time, optimize_expressions;
  int         current_slot;
  int         optimal_raid, log, debug_each;
  int         save_profiles, default_actions;
  stat_e      normalized_stat;
  std::string current_name, default_region_str, default_server_str, save_prefix_str, save_suffix_str;
  int         save_talent_str;
  talent_format_e talent_format;
  auto_dispose< std::vector<player_t*> > actor_list;
  std::string main_target_str;
  int         auto_ready_trigger;
  int         stat_cache;
  int         max_aoe_enemies;
  bool        show_etmi;
  double      tmi_window_global;
  double      tmi_bin_size;
  bool        requires_regen_event;

  // Target options
  double      enemy_death_pct;
  int         rel_target_level, target_level;
  std::string target_race;
  int         target_adds;
  std::string sim_phase_str;
  int         desired_targets; // desired number of targets
  bool        enable_taunts;

  // Data access
  dbc_t       dbc;

  // Default stat enchants
  gear_stats_t enchant;

  bool challenge_mode; // if active, players will get scaled down to 620 and set bonuses are deactivated
  int timewalk;
  int scale_to_itemlevel; //itemlevel to scale to. if -1, we don't scale down
  bool scale_itemlevel_down_only; // Items below the value of scale_to_itemlevel will not be scaled up.
  bool disable_set_bonuses; // Disables all set bonuses.
  unsigned int disable_2_set; // Disables all 2 set bonuses for this tier/integer that this is set as
  unsigned int disable_4_set; // Disables all 4 set bonuses for this tier/integer that this is set as
  unsigned int enable_2_set;// Enables all 2 set bonuses for the tier/integer that this is set as
  unsigned int enable_4_set; // Enables all 4 set bonuses for the tier/integer that this is set as
  bool pvp_crit; // Sets critical strike damage to 150% instead of 200%, and limits multistrike to one roll.

  // Actor tracking
  int active_enemies;
  int active_allies;

  std::unordered_map<std::string, std::string> var_map;
  std::vector<std::unique_ptr<option_t>> options;
  std::vector<std::string> party_encoding;
  std::vector<std::string> item_db_sources;

  // Random Number Generation
  std::unique_ptr<rng::rng_t> _rng;
  std::string rng_str;
  uint64_t seed;
  int deterministic;
  int average_range, average_gauss;
  int convergence_scale;

  // Raid Events
  std::vector<std::unique_ptr<raid_event_t>> raid_events;
  std::string raid_events_str;
  std::string fight_style;

  // Buffs and Debuffs Overrides
  struct overrides_t
  {
    // Buff overrides
    int attack_power_multiplier;
    int critical_strike;
    int mastery;
    int haste;
    int multistrike;
    int spell_power_multiplier;
    int stamina;
    int str_agi_int;
    int versatility;

    // Debuff overrides
    int mortal_wounds;
    int bleeding;

    // Misc stuff needs resolving
    int    bloodlust;
    std::vector<uint64_t> target_health;
  } overrides;

  // Auras
  struct auras_t
  {
    // Raid-wide auras from various classes
    aura_t* attack_power_multiplier;
    aura_t* critical_strike;
    aura_t* mastery;
    aura_t* haste;
    aura_t* multistrike;
    aura_t* spell_power_multiplier;
    aura_t* stamina;
    aura_t* str_agi_int;
    aura_t* versatility;
  } auras;

  // Auras and De-Buffs
  auto_dispose< std::vector<buff_t*> > buff_list;

  // Global aura related delay
  timespan_t default_aura_delay;
  timespan_t default_aura_delay_stddev;

  auto_dispose< std::vector<cooldown_t*> > cooldown_list;

  // Reporting
  progress_bar_t progress_bar;
  std::unique_ptr<scaling_t> scaling;
  std::unique_ptr<plot_t> plot;
  std::unique_ptr<reforge_plot_t> reforge_plot;
  double elapsed_cpu;
  double elapsed_time;
  double     iteration_dmg, priority_iteration_dmg,  iteration_heal, iteration_absorb;
  simple_sample_data_t raid_dps, total_dmg, raid_hps, total_heal, total_absorb, raid_aps;
  extended_sample_data_t simulation_length;
  // Deterministic simulation iteration data collectors for specific iteration
  // replayability
  std::vector<iteration_data_entry_t> iteration_data, low_iteration_data, high_iteration_data;
  // Report percent (how many% of lowest/highest iterations reported, default 2.5%)
  double     report_iteration_data;
  // Minimum number of low/high iterations reported (default 5 of each)
  int        min_report_iteration_data;
  int        report_progress;
  int        bloodlust_percent;
  timespan_t bloodlust_time;
  std::string reference_player_str;
  std::vector<player_t*> players_by_dps;
  std::vector<player_t*> players_by_priority_dps;
  std::vector<player_t*> players_by_hps;
  std::vector<player_t*> players_by_hps_plus_aps;
  std::vector<player_t*> players_by_dtps;
  std::vector<player_t*> players_by_tmi;
  std::vector<player_t*> players_by_name;
  std::vector<player_t*> players_by_apm;
  std::vector<player_t*> players_by_variance;
  std::vector<player_t*> targets_by_name;
  std::vector<std::string> id_dictionary;
  std::map<double, std::vector<double> > divisor_timeline_cache;
  std::string output_file_str, html_file_str, json_file_str;
  std::string xml_file_str, xml_stylesheet_file_str;
  std::string reforge_plot_output_file_str;
  std::vector<std::string> error_list;
  int report_precision;
  int report_pets_separately;
  int report_targets;
  int report_details;
  int report_raw_abilities;
  int report_rng;
  int hosted_html;
  int save_raid_summary;
  int save_gear_comments;
  int statistics_level;
  int separate_stats_by_actions;
  int report_raid_summary;
  int buff_uptime_timeline;
  int decorated_tooltips;

  int allow_potions;
  int allow_food;
  int allow_flasks;
  int solo_raid;
  int global_item_upgrade_level;
  bool maximize_reporting;
  std::string apikey;
  bool ilevel_raid_report;
  bool distance_targeting_enabled;
  bool enable_dps_healing;
  double scaling_normalized;

  sim_report_information_t report_information;

  // Multi-Threading
  mutex_t merge_mutex;
  int threads;
  std::vector<sim_t*> children; // Manual delete!
  int thread_index;
  computer_process::priority_e process_priority;
  struct sim_progress_t
  {
    int current_iterations;
    int total_iterations;
    double pct() const
    { return current_iterations / static_cast<double>(total_iterations); }
  };
  struct work_queue_t
  {
    private:
    mutex_t m;
    public:
    int total_work, projected_work, work;
    work_queue_t() : total_work( 0 ), projected_work( 0 ), work( 0 ) {}
    void init( int w )    { AUTO_LOCK(m); total_work = projected_work = w; }
    void flush()          { AUTO_LOCK(m); total_work = projected_work = work; }
    void project( int w ) { AUTO_LOCK(m); projected_work = w; assert(w>=work); }
    int  size()           { AUTO_LOCK(m); return total_work; }
    bool pop()         
    { 
      AUTO_LOCK(m); 
      if( work >= total_work ) return false; 
      if( ++work == total_work ) projected_work = work;
      return work < total_work;
    }
    sim_progress_t progress()
    {
      AUTO_LOCK(m);
      return sim_progress_t{work, projected_work };
    }
  };
  std::shared_ptr<work_queue_t> work_queue;

  // Related Simulations
  mutex_t relatives_mutex;
  std::vector<sim_t*> relatives;

  // Spell database access
  std::unique_ptr<spell_data_expr_t> spell_query;
  unsigned           spell_query_level;
  std::string        spell_query_xml_output_file_str;

  mutex_t* pause_mutex; // External pause mutex, instantiated an external entity (in our case the GUI).
  bool paused;

  // Highcharts stuff

  // Vector of on-ready charts. These receive individual jQuery handlers in the HTML report (at the
  // end of the report) to load the highcharts into the target div.
  std::vector<std::string> on_ready_chart_data;

  // A map of highcharts data, added as a json object into the HTML report. JQuery installs handlers
  // to correct elements (toggled elements in the HTML report) based on the data.
  std::map<std::string, std::vector<std::string> > chart_data;

  bool enable_highcharts;
  bool output_relative_difference;
  double boxplot_percentile;

  // List of callbacks to call when an actor_target_data_t object is created. Currently used to
  // initialize the generic targetdata debuffs/dots we have.
  std::vector<std::function<void(actor_target_data_t*)> > target_data_initializer;

  bool display_hotfixes, disable_hotfixes;
  bool display_bonus_ids;

  sim_t( sim_t* parent = nullptr, int thread_index = 0 );
  virtual ~sim_t();

  virtual void run() override;
  int       main( const std::vector<std::string>& args );
  double    iteration_time_adjust() const;
  double    expected_max_time() const;
  bool      is_canceled() const;
  void      cancel_iteration();
  void      cancel();
  void      interrupt();
  void      add_relative( sim_t* cousin );
  void      remove_relative( sim_t* cousin );
  sim_progress_t progress(std::string* phase = nullptr );
  double    progress( std::string& phase, std::string* detailed = nullptr );
  void      detailed_progress( std::string*, int current_iterations, int total_iterations );
  void      datacollection_begin();
  void      datacollection_end();
  void      reset();
  bool      check_actors();
  bool      init_parties();
  bool      init_actors();
  bool      init_actor( player_t* );
  bool      init_actor_pets();
  bool      init();
  void      analyze();
  void      merge( sim_t& other_sim );
  void      merge();
  bool      iterate();
  void      partition();
  bool      execute();
  void      analyze_error();
  void      analyze_iteration_data();
  void      print_options();
  void      add_option( std::unique_ptr<option_t> opt );
  void      create_options();
  bool      parse_option( const std::string& name, const std::string& value );
  void      setup( sim_control_t* );
  bool      time_to_think( timespan_t proc_time );
  timespan_t total_reaction_time ();
  player_t* find_player( const std::string& name ) const;
  player_t* find_player( int index ) const;
  cooldown_t* get_cooldown( const std::string& name );
  void      use_optimal_buffs_and_debuffs( int value );
  expr_t*   create_expression( action_t*, const std::string& name );
  void      errorf( const char* format, ... ) PRINTF_ATTRIBUTE(2, 3);
  void abort();
  void combat();
  void combat_begin();
  void combat_end();
  void add_chart_data( const highchart::chart_t& chart );

  timespan_t current_time() const
  { return event_mgr.current_time; }
  static double distribution_mean_error( const sim_t& s, const extended_sample_data_t& sd )
  { return s.confidence_estimator * sd.mean_std_dev; }
  void register_target_data_initializer(std::function<void(actor_target_data_t*)> cb)
  { target_data_initializer.push_back( cb ); }
  rng::rng_t& rng() const
  { return *_rng; }
  double averaged_range( double min, double max )
  {
    if ( average_range ) return ( min + max ) / 2.0;
    return rng().range( min, max );
  }
private:
  void do_pause();
  void print_spell_query();
};

// Module ===================================================================

struct module_t
{
  player_e type;

  module_t( player_e t ) :
    type( t ) {}

  virtual ~module_t() {}
  virtual player_t* create_player( sim_t* sim, const std::string& name, race_e r = RACE_NONE ) const = 0;
  virtual bool valid() const = 0;
  virtual void init( player_t* ) const = 0;
  virtual void static_init() const { }
  virtual void register_hotfixes() const { }
  virtual void combat_begin( sim_t* ) const = 0;
  virtual void combat_end( sim_t* ) const = 0;

  static const module_t* death_knight();
  static const module_t* druid();
  static const module_t* hunter();
  static const module_t* mage();
  static const module_t* monk();
  static const module_t* paladin();
  static const module_t* priest();
  static const module_t* rogue();
  static const module_t* shaman();
  static const module_t* warlock();
  static const module_t* warrior();
  static const module_t* enemy();
  static const module_t* tmi_enemy();
  static const module_t* tank_dummy_enemy();
  static const module_t* heal_enemy();

  static const module_t* get( player_e t )
  {
    switch ( t )
    {
      case DEATH_KNIGHT: return death_knight();
      case DRUID:        return druid();
      case HUNTER:       return hunter();
      case MAGE:         return mage();
      case MONK:         return monk();
      case PALADIN:      return paladin();
      case PRIEST:       return priest();
      case ROGUE:        return rogue();
      case SHAMAN:       return shaman();
      case WARLOCK:      return warlock();
      case WARRIOR:      return warrior();
      case ENEMY:        return enemy();
      case TMI_BOSS:     return tmi_enemy();
      case TANK_DUMMY:   return tank_dummy_enemy();
      default: break;
    }
    return nullptr;
  }
  static const module_t* get( const std::string& n )
  {
    return get( util::parse_player_type( n ) );
  }
  static void init()
  {
    for ( player_e i = PLAYER_NONE; i < PLAYER_MAX; i++ )
    {
      const module_t* m = get( i );
      if ( m )
      {
        m -> static_init();
        m -> register_hotfixes();
      }
    }
  }
};

// Scaling ==================================================================

struct scaling_t
{
  mutex_t mutex;
  sim_t* sim;
  sim_t* baseline_sim;
  sim_t* ref_sim;
  sim_t* delta_sim;
  sim_t* ref_sim2;
  sim_t* delta_sim2;
  stat_e scale_stat;
  double scale_value;
  double scale_delta_multiplier;
  int    calculate_scale_factors;
  int    center_scale_delta;
  int    positive_scale_delta;
  int    scale_lag;
  double scale_factor_noise;
  int    normalize_scale_factors;
  int    debug_scale_factors;
  std::string scale_only_str;
  stat_e current_scaling_stat;
  int num_scaling_stats, remaining_scaling_stats;
  std::string scale_over;
  scale_metric_e scaling_metric;
  std::string scale_over_player;

  // Gear delta for determining scale factors
  gear_stats_t stats;

  scaling_t( sim_t* s );

  void init_deltas();
  void analyze();
  void analyze_stats();
  void analyze_ability_stats( stat_e, double, player_t*, player_t*, player_t* );
  void analyze_lag();
  void normalize();
  void derive();
  double progress( std::string& phase, std::string* detailed = nullptr );
  void create_options();
  bool has_scale_factors() const;
};

// Plot =====================================================================

struct plot_t
{
public:
  sim_t* sim;
  std::string dps_plot_stat_str;
  double dps_plot_step;
  int    dps_plot_points;
  int    dps_plot_iterations;
  double dps_plot_target_error;
  int    dps_plot_debug;
  stat_e current_plot_stat;
  int    num_plot_stats, remaining_plot_stats, remaining_plot_points;
  bool   dps_plot_positive, dps_plot_negative;

  plot_t( sim_t* s );
  void analyze();
  double progress( std::string& phase, std::string* detailed = nullptr );
private:
  void analyze_stats();
  void write_output_file();
  void create_options();
};

// Reforge Plot =============================================================
class reforge_plot_run_t
{
public:
  std::vector<stat_e> reforge_plot_stat_indices;
};
struct reforge_plot_t
{

  sim_t* sim;
  sim_t* current_reforge_sim;
  std::string reforge_plot_stat_str;
  std::vector<reforge_plot_run_t> reforge_plots;
  int    reforge_plot_step;
  int    reforge_plot_amount;
  int    reforge_plot_iterations;
  double reforge_plot_target_error;
  int    reforge_plot_debug;
  int    current_stat_combo;
  int    num_stat_combos;

  reforge_plot_t( sim_t* s );

  void start();
  void run_plots();
  double progress( std::string& phase, std::string* detailed = nullptr );
private:
  void generate_stat_mods( std::vector<std::vector<int> > &stat_mods,
                           const std::vector<stat_e> &stat_indices,
                           int cur_mod_stat,
                           std::vector<int> cur_stat_mods );
  void write_output_file();
  void create_options();
  void run_reforge_plot( const reforge_plot_run_t& );
  void debug_plot(  const reforge_plot_run_t&, const std::vector<std::vector<int>>& stat_mods);
};

struct plot_data_t
{
  double plot_step;
  double value;
  double error;
};

// Event ====================================================================
//
// core_event_t is designed to be a very simple light-weight event transporter and
// as such there are rules of use that must be honored:
//
// (1) The pure virtual execute() method MUST be implemented in the sub-class
// (2) There is 1 * sizeof( event_t ) space available to extend the sub-class
// (3) sim_t is responsible for deleting the memory associated with allocated events

struct event_t : private noncopyable
{
  sim_t& _sim;
  event_t*    next;
  timespan_t  time;
  timespan_t  reschedule_time;
  uint32_t    id;
  bool        canceled;
  bool        recycled;
#if ACTOR_EVENT_BOOKKEEPING
  actor_t*    actor;
#endif
  event_t( sim_t& s, actor_t* p = nullptr);
  event_t( actor_t& p );

  timespan_t occurs()  { return ( reschedule_time != timespan_t::zero() ) ? reschedule_time : time; }
  timespan_t remains() { return occurs() - _sim.event_mgr.current_time; }

  void reschedule( timespan_t new_time );
  void add_event( timespan_t delta_time );
  sim_t& sim()
  { return _sim; }
  const sim_t& sim() const
  { return _sim; }
  rng::rng_t& rng() { return sim().rng(); }
  rng::rng_t& rng() const { return sim().rng(); }

  virtual void execute() = 0; // MUST BE IMPLEMENTED IN SUB-CLASS!
  virtual const char* name() const
  { return "core_event_t"; }

  virtual ~event_t() {}

  template<class T>
  static void cancel( T& e )
  {
    event_t* ref = static_cast<event_t*>(e);
    event_t::cancel( ref );
    e = nullptr;
  }
  static void cancel( event_t*& e );

  static void* operator new( std::size_t size, sim_t& sim ) { return sim.event_mgr.allocate_event( size ); }

  // DO NOT USE ANY OF THE FOLLOWING!
  static void  operator delete( void*, sim_t& ) { std::terminate(); }
  static void  operator delete( void* ) { std::terminate(); }  // DO NOT USE!
  static void* operator new( std::size_t ) = delete;
};

// Gear Rating Conversions ==================================================

enum rating_e
{
  RATING_DODGE = 0,
  RATING_PARRY,
  RATING_BLOCK,
  RATING_MELEE_HIT,
  RATING_RANGED_HIT,
  RATING_SPELL_HIT,
  RATING_MELEE_CRIT,
  RATING_RANGED_CRIT,
  RATING_SPELL_CRIT,
  RATING_MULTISTRIKE,
  RATING_READINESS,
  RATING_PVP_RESILIENCE,
  RATING_LEECH,
  RATING_MELEE_HASTE,
  RATING_RANGED_HASTE,
  RATING_SPELL_HASTE,
  RATING_EXPERTISE,
  RATING_MASTERY,
  RATING_PVP_POWER,
  RATING_DAMAGE_VERSATILITY,
  RATING_HEAL_VERSATILITY,
  RATING_MITIGATION_VERSATILITY,
  RATING_SPEED,
  RATING_AVOIDANCE,
  RATING_MAX
};

inline cache_e cache_from_rating( rating_e r )
{
  switch ( r )
  {
    case RATING_SPELL_HASTE: return CACHE_SPELL_HASTE;
    case RATING_SPELL_HIT: return CACHE_SPELL_HIT;
    case RATING_SPELL_CRIT: return CACHE_SPELL_CRIT;
    case RATING_MELEE_HASTE: return CACHE_ATTACK_HASTE;
    case RATING_MELEE_HIT: return CACHE_ATTACK_HIT;
    case RATING_MELEE_CRIT: return CACHE_ATTACK_CRIT;
    case RATING_RANGED_HASTE: return CACHE_ATTACK_HASTE;
    case RATING_RANGED_HIT: return CACHE_ATTACK_HIT;
    case RATING_RANGED_CRIT: return CACHE_ATTACK_CRIT;
    case RATING_EXPERTISE: return CACHE_EXP;
    case RATING_DODGE: return CACHE_DODGE;
    case RATING_PARRY: return CACHE_PARRY;
    case RATING_BLOCK: return CACHE_BLOCK;
    case RATING_MASTERY: return CACHE_MASTERY;
    case RATING_PVP_POWER: return CACHE_NONE;
    case RATING_PVP_RESILIENCE: return CACHE_NONE;
    case RATING_MULTISTRIKE: return CACHE_MULTISTRIKE;
    case RATING_READINESS: return CACHE_READINESS;
    case RATING_DAMAGE_VERSATILITY: return CACHE_DAMAGE_VERSATILITY;
    case RATING_HEAL_VERSATILITY: return CACHE_HEAL_VERSATILITY;
    case RATING_MITIGATION_VERSATILITY: return CACHE_MITIGATION_VERSATILITY;
    case RATING_LEECH: return CACHE_LEECH;
    case RATING_SPEED: return CACHE_RUN_SPEED;
    case RATING_AVOIDANCE: return CACHE_AVOIDANCE;
    default: break;
  }
  assert( false ); return CACHE_NONE;
}

struct rating_t
{
  double  spell_haste,  spell_hit,  spell_crit;
  double attack_haste, attack_hit, attack_crit;
  double ranged_haste, ranged_hit, ranged_crit;
  double expertise;
  double dodge, parry, block;
  double mastery;
  double pvp_resilience, pvp_power;
  double multistrike;
  double readiness;
  double damage_versatility, heal_versatility, mitigation_versatility;
  double leech, speed, avoidance;

  double& get( rating_e r )
  {
    switch ( r )
    {
      case RATING_SPELL_HASTE: return spell_haste;
      case RATING_SPELL_HIT: return spell_hit;
      case RATING_SPELL_CRIT: return spell_crit;
      case RATING_MELEE_HASTE: return attack_haste;
      case RATING_MELEE_HIT: return attack_hit;
      case RATING_MELEE_CRIT: return attack_crit;
      case RATING_RANGED_HASTE: return ranged_haste;
      case RATING_RANGED_HIT: return ranged_hit;
      case RATING_RANGED_CRIT: return ranged_crit;
      case RATING_EXPERTISE: return expertise;
      case RATING_DODGE: return dodge;
      case RATING_PARRY: return parry;
      case RATING_BLOCK: return block;
      case RATING_MASTERY: return mastery;
      case RATING_PVP_POWER: return pvp_power;
      case RATING_PVP_RESILIENCE: return pvp_resilience;
      case RATING_MULTISTRIKE: return multistrike;
      case RATING_READINESS: return readiness;
      case RATING_DAMAGE_VERSATILITY: return damage_versatility;
      case RATING_HEAL_VERSATILITY: return heal_versatility;
      case RATING_MITIGATION_VERSATILITY: return mitigation_versatility;
      case RATING_LEECH: return leech;
      case RATING_SPEED: return speed;
      case RATING_AVOIDANCE: return avoidance;
      default: break;
    }
    assert( false ); return mastery;
  }

  rating_t()
  {
    // Initialize all ratings to a very high number
    double max = +1.0E+50;
    for ( rating_e i = static_cast<rating_e>( 0 ); i < RATING_MAX; ++i )
    {
      get( i ) = max;
    }
  }

  void init( dbc_t& dbc, int level )
  {
    // Read ratings from DBC
    for ( rating_e i = static_cast<rating_e>( 0 ); i < RATING_MAX; ++i )
    {
      get( i ) = dbc.combat_rating( i,  level );
      if ( i == RATING_MASTERY )
        get( i ) /= 100.0;
    }
  }

  std::string to_string()
  {
    std::ostringstream s;
    for ( rating_e i = static_cast<rating_e>( 0 ); i < RATING_MAX; ++i )
    {
      if ( i > 0 ) s << " ";
      s << util::cache_type_string( cache_from_rating( i ) ) << "=" << get( i ); // hacky
    }
    return s.str();
  }
};

// Weapon ===================================================================

struct weapon_t
{
  weapon_e type;
  school_e school;
  double damage, dps;
  double min_dmg, max_dmg;
  timespan_t swing_time;
  slot_e slot;
  int    buff_type;
  double buff_value;
  double bonus_dmg;

  weapon_t( weapon_e t    = WEAPON_NONE,
            double d      = 0.0,
            timespan_t st = timespan_t::from_seconds( 2.0 ),
            school_e s    = SCHOOL_PHYSICAL ) :
    type( t ),
    school( s ),
    damage( d ),
    dps( d / st.total_seconds() ),
    min_dmg( d ),
    max_dmg( d ),
    swing_time( st ),
    slot( SLOT_INVALID ),
    buff_type( 0 ),
    buff_value( 0.0 ),
    bonus_dmg( 0.0 )
  {}

  weapon_e group() const
  {
    if ( type <= WEAPON_SMALL )
      return WEAPON_SMALL;

    if ( type <= WEAPON_1H )
      return WEAPON_1H;

    if ( type <= WEAPON_2H )
      return WEAPON_2H;

    if ( type <= WEAPON_RANGED )
      return WEAPON_RANGED;

    return WEAPON_NONE;
  }

  /**
   * Normalized weapon speed for weapon damage calculations
   *
   * * WEAPON_SMALL: 1.7s
   * * WEAPON_1H: 2.4s
   * * WEAPON_2h: 3.3s
   * * WEAPON_RANGED: 2.8s
   */
  timespan_t get_normalized_speed()
  {
    weapon_e g = group();

    if ( g == WEAPON_SMALL  ) return timespan_t::from_seconds( 1.7 );
    if ( g == WEAPON_1H     ) return timespan_t::from_seconds( 2.4 );
    if ( g == WEAPON_2H     ) return timespan_t::from_seconds( 3.3 );
    if ( g == WEAPON_RANGED ) return timespan_t::from_seconds( 2.8 );

    assert( false );
    return timespan_t::zero();
  }

  double proc_chance_on_swing( double PPM, timespan_t adjusted_swing_time = timespan_t::zero() )
  {
    if ( adjusted_swing_time == timespan_t::zero() ) adjusted_swing_time = swing_time;

    timespan_t time_to_proc = timespan_t::from_seconds( 60.0 ) / PPM;
    double proc_chance = adjusted_swing_time / time_to_proc;

    return proc_chance;
  }
};

// Special Effect ===========================================================

struct special_effect_t
{
  const item_t* item;
  player_t* player;

  special_effect_e type;
  special_effect_source_e source;
  std::string name_str, trigger_str, encoding_str;
  unsigned proc_flags_; /* Proc-by */
  unsigned proc_flags2_; /* Proc-on (hit/damage/...) */
  stat_e stat;
  school_e school;
  int max_stacks;
  double stat_amount, discharge_amount, discharge_scaling;
  double proc_chance_;
  double ppm_;
  rppm_scale_e rppm_scale_;
  timespan_t duration_, cooldown_, tick;
  bool cost_reduction;
  int refresh;
  bool chance_to_discharge;
  unsigned int override_result_es_mask;
  unsigned result_es_mask;
  bool reverse;
  int aoe;
  bool proc_delay;
  bool unique;
  bool weapon_proc;
  unsigned spell_id, trigger_spell_id;
  action_t* execute_action; // Allows custom action to be executed on use
  buff_t* custom_buff; // Allows custom action
  void (*custom_init)(special_effect_t& );


  special_effect_t( player_t* p ) :
    item( nullptr ), player( p ),
    name_str()
  { reset(); }

  special_effect_t( const item_t* item );

  void reset();
  std::string to_string() const;
  bool active() { return stat != STAT_NONE || school != SCHOOL_NONE || execute_action; }

  const spell_data_t* driver() const;
  const spell_data_t* trigger() const;
  std::string name() const;
  std::string cooldown_name() const;

  // On-use item cooldown handling accessories
  int cooldown_group() const;
  std::string cooldown_group_name() const;
  timespan_t cooldown_group_duration() const;

  // Buff related functionality
  buff_t* create_buff() const;
  special_effect_buff_e buff_type() const;
  int max_stack() const;

  bool is_stat_buff() const;
  stat_e stat_type() const;
  stat_buff_t* initialize_stat_buff() const;

  bool is_absorb_buff() const;
  absorb_buff_t* initialize_absorb_buff() const;

  // Action related functionality
  action_t* create_action() const;
  special_effect_action_e action_type() const;
  bool is_offensive_spell_action() const;
  spell_t* initialize_offensive_spell_action() const;
  bool is_heal_action() const;
  heal_t* initialize_heal_action() const;
  bool is_attack_action() const;
  attack_t* initialize_attack_action() const;
  bool is_resource_action() const;
  spell_t* initialize_resource_action() const;

  /* Accessors for driver specific features of the proc; some are also used for on-use effects */
  unsigned proc_flags() const;
  unsigned proc_flags2() const;
  double ppm() const;
  double rppm() const;
  rppm_scale_e rppm_scale() const;
  double rppm_modifier() const;
  double proc_chance() const;
  timespan_t cooldown() const;

  /* Accessors for buff specific features of the proc. */
  timespan_t duration() const;
  timespan_t tick_time() const;
};

// Item =====================================================================

struct stat_pair_t
{
  stat_e stat;
  int value;

  stat_pair_t() : stat( STAT_NONE ), value( 0 )
  { }

  stat_pair_t( stat_e s, int v ) : stat( s ), value( v )
  { }
};

struct item_t
{
  sim_t* sim;
  player_t* player;
  slot_e slot;
  bool unique, unique_addon, is_ptr;

  // Structure contains the "parsed form" of this specific item, be the data
  // from user options, or a data source such as the Blizzard API, or Wowhead
  struct parsed_input_t
  {
    unsigned                 item_level;
    int                      upgrade_level;
    int                      suffix_id;
    unsigned                 enchant_id;
    unsigned                 addon_id;
    int                      armor;
    std::array<int, 3>       gem_id;
    std::array<int, 3>       gem_color;
    std::vector<int>         bonus_id;
    std::vector<stat_pair_t> gem_stats, meta_gem_stats, socket_bonus_stats;
    std::string              encoded_enchant;
    std::vector<stat_pair_t> enchant_stats;
    std::string              encoded_addon;
    std::vector<stat_pair_t> addon_stats;
    std::vector<stat_pair_t> suffix_stats;
    item_data_t              data;
    auto_dispose< std::vector<special_effect_t*> > special_effects;
    std::vector<std::string> source_list;
    timespan_t               initial_cd;

    parsed_input_t() :
      item_level( 0 ), upgrade_level( 0 ), suffix_id( 0 ), enchant_id( 0 ), addon_id( 0 ),
      armor( 0 ), data(), initial_cd( timespan_t::zero() )
    {
      range::fill( data.stat_type_e, -1 );
      range::fill( data.stat_val, 0 );
      range::fill( gem_id, 0 );
      range::fill( bonus_id, 0 );
      range::fill( gem_color, SOCKET_COLOR_NONE );
    }
  } parsed;

  std::shared_ptr<xml_node_t> xml;

  std::string name_str;
  std::string icon_str;

  std::string source_str;
  std::string options_str;

  // Option Data
  std::string option_name_str;
  std::string option_stats_str;
  std::string option_gems_str;
  std::string option_enchant_str;
  std::string option_addon_str;
  std::string option_equip_str;
  std::string option_use_str;
  std::string option_weapon_str;
  std::string option_lfr_str;
  std::string option_warforged_str;
  std::string option_heroic_str;
  std::string option_mythic_str;
  std::string option_armor_type_str;
  std::string option_ilevel_str;
  std::string option_quality_str;
  std::string option_data_source_str;
  std::string option_enchant_id_str;
  std::string option_addon_id_str;
  std::string option_gem_id_str;
  std::string option_bonus_id_str;
  double option_initial_cd;

  // Extracted data
  gear_stats_t base_stats, stats;

  mutable int cached_upgrade_item_level;

  item_t() : sim( nullptr ), player( nullptr ), slot( SLOT_INVALID ), unique( false ),
    unique_addon( false ), is_ptr( false ),
    parsed(), xml(),
    cached_upgrade_item_level( -1 ){ }
  item_t( player_t*, const std::string& options_str );

  bool active() const;
  const char* name() const;
  std::string full_name() const;
  const char* slot_name() const;
  weapon_t* weapon() const;
  bool init();
  bool parse_options();
  inventory_type inv_type() const;

  bool is_matching_type() const;
  bool is_valid_type() const;
  bool socket_color_match() const;

  unsigned item_level() const;
  unsigned upgrade_level() const;
  unsigned upgrade_item_level() const;
  unsigned base_item_level() const;
  stat_e stat( size_t idx ) const;
  int stat_value( size_t idx ) const;
  bool has_item_stat( stat_e stat ) const;

  std::string encoded_item() const;
  void encoded_item( xml_writer_t& writer ) const;
  std::string encoded_comment();

  std::string encoded_stats() const;
  std::string encoded_weapon() const;
  std::string encoded_gems() const;
  std::string encoded_enchant() const;
  std::string encoded_addon() const;
  std::string encoded_upgrade_level() const;
  std::string encoded_random_suffix_id() const;

  bool decode_stats();
  bool decode_gems();
  bool decode_enchant();
  bool decode_addon();
  bool decode_weapon();
  bool decode_warforged();
  bool decode_lfr();
  bool decode_heroic();
  bool decode_mythic();
  bool decode_armor_type();
  bool decode_random_suffix();
  bool decode_upgrade_level();
  bool decode_ilevel();
  bool decode_quality();
  bool decode_data_source();
  bool decode_equip_effect();
  bool decode_use_effect();

  bool decode_proc_spell( special_effect_t& effect );

  static bool download_slot( item_t& item );
  static bool download_item( item_t& );
  static bool download_glyph( player_t* player, std::string& glyph_name, const std::string& glyph_id );

  static std::vector<stat_pair_t> str_to_stat_pair( const std::string& stat_str );
  static std::string stat_pairs_to_str( const std::vector<stat_pair_t>& stat_pairs );

  std::string to_string() const;
  std::string item_stats_str() const;
  std::string weapon_stats_str() const;
  std::string suffix_stats_str() const;
  std::string gem_stats_str() const;
  std::string socket_bonus_stats_str() const;
  std::string enchant_stats_str() const;
  bool has_stats() const;
  bool has_special_effect( special_effect_source_e source = SPECIAL_EFFECT_SOURCE_NONE, special_effect_e type = SPECIAL_EFFECT_NONE ) const;
  bool has_use_special_effect() const
  { return has_special_effect( SPECIAL_EFFECT_SOURCE_NONE, SPECIAL_EFFECT_USE ); }
  bool has_scaling_stat_bonus_id() const;

  const special_effect_t& special_effect( special_effect_source_e source = SPECIAL_EFFECT_SOURCE_NONE, special_effect_e type = SPECIAL_EFFECT_NONE ) const;
};


// Benefit ==================================================================

struct benefit_t : private noncopyable
{
private:
  int up, down;
public:
  simple_sample_data_t ratio;
  const std::string name_str;

  explicit benefit_t( const std::string& n ) :
    up( 0 ), down( 0 ),
    ratio(), name_str( n ) {}

  void update( bool is_up )
  { if ( is_up ) up++; else down++; }
  void datacollection_begin()
  { up = down = 0; }
  void datacollection_end()
  { ratio.add( up != 0 ? 100.0 * up / ( down + up ) : 0.0 ); }
  void merge( const benefit_t& other )
  { ratio.merge( other.ratio ); }

  const char* name() const
  { return name_str.c_str(); }
};

// Proc =====================================================================

struct proc_t : private noncopyable
{
private:
  sim_t& sim;
  size_t iteration_count; // track number of procs during the current iteration
  timespan_t last_proc; // track time of the last proc
public:
  const std::string name_str;
  simple_sample_data_t interval_sum;
  simple_sample_data_t count;

  proc_t( sim_t& s, const std::string& n ) :
    sim( s ),
    iteration_count(),
    last_proc( timespan_t::min() ),
    name_str( n ),
    interval_sum(),
    count()
  {}

  void occur()
  {
    iteration_count++;
    if ( last_proc >= timespan_t::zero() && last_proc < sim.current_time() )
    {
      interval_sum.add( ( sim.current_time() - last_proc ).total_seconds() );
      reset();
    }
    if ( sim.debug )
      sim.out_debug.printf( "[PROC] %s: iteration_count=%u count.sum=%u last_proc=%f",
                  name(),
                  static_cast<unsigned>( iteration_count ),
                  static_cast<unsigned>( count.sum() ),
                  last_proc.total_seconds() );

    last_proc = sim.current_time();
  }

  void reset()
  { last_proc = timespan_t::min(); }

  void merge( const proc_t& other )
  {
    count.merge( other.count );
    interval_sum.merge( other.interval_sum );
  }

  void datacollection_begin()
  { iteration_count = 0; }
  void datacollection_end()
  { count.add( static_cast<double>( iteration_count ) ); }

  const char* name() const
  { return name_str.c_str(); }
};

// Set Bonus ================================================================

struct set_bonus_t
{
  // Some magic constants
  static const unsigned N_BONUSES = 2;       // Number of set bonuses in tier gear

  struct set_bonus_data_t
  {
    const spell_data_t* spell;
    const item_set_bonus_t* bonus;
    int overridden;
    bool enabled;

    set_bonus_data_t() :
      spell( spell_data_t::not_found() ), bonus( nullptr ), overridden( -1 ), enabled( false )
    { }
  };

  // Data structure definitions
  typedef std::vector<set_bonus_data_t> bonus_t;
  typedef std::vector<bonus_t> bonus_type_t;
  typedef std::vector<bonus_type_t> set_bonus_type_t;

  typedef std::vector<unsigned> bonus_count_t;
  typedef std::vector<bonus_count_t> set_bonus_count_t;

  player_t* actor;

  // Set bonus data structure
  set_bonus_type_t set_bonus_spec_data;
  // Set item counts
  set_bonus_count_t set_bonus_spec_count;

  set_bonus_t( player_t* p );

  // Collect item information about set bonuses, fully DBC driven
  void initialize_items();

  // Initialize set bonuses in earnest
  void initialize();

  expr_t* create_expression( const player_t*, const std::string& type );

  std::vector<const item_set_bonus_t*> enabled_set_bonus_data() const;

  // Fast accessor to a set bonus spell, returns the spell, or spell_data_t::not_found()
  const spell_data_t* set( specialization_e spec, set_bonus_type_e set_bonus, set_bonus_e bonus ) const
  {
#ifdef NDEBUG
    switch ( set_bonus )
    {
      case PVP:
      case T17LFR:
      case T17:
      case T18:
        break;
      default:
        assert( 0 && "Attempt to access role-based set bonus through specialization." );
    }
#endif
    return set_bonus_spec_data[ set_bonus ][ specdata::spec_idx( spec ) ][ bonus ].spell;
  }

  const spell_data_t* set( set_role_e role, set_bonus_type_e set_bonus, set_bonus_e bonus ) const
  {
#ifndef NDEBUG
    switch ( set_bonus )
    {
      case T13:
      case T14:
      case T15:
      case T16:
        break;
      default:
        assert( 0 && "Attempt to access spec-based set bonus through role." );
    }
#endif
    return set_bonus_spec_data[ set_bonus ][ role ][ bonus ].spell;
  }

  // Fast accessor for checking whether a set bonus is enabled
  bool has_set_bonus( specialization_e spec, set_bonus_type_e set_bonus, set_bonus_e bonus ) const
  { return set_bonus_spec_data[ set_bonus ][ specdata::spec_idx( spec ) ][ bonus ].enabled; }

  // Fast accessor for checking whether a set bonus is enabled
  bool has_set_bonus( set_role_e role, set_bonus_type_e set_bonus, set_bonus_e bonus ) const
  { return set_bonus_spec_data[ set_bonus ][ role ][ bonus ].enabled; }

  bool parse_set_bonus_option( const std::string& opt_str, set_bonus_type_e& set_bonus, set_role_e& role, set_bonus_e& bonus );
  std::string to_string() const;
  std::string to_profile_string( const std::string& = "\n" ) const;
  std::string generate_set_bonus_options() const;

  static set_role_e translate_set_bonus_role_str( const std::string& name );
  static const char* translate_set_bonus_role( set_role_e );
  static std::string set_bonus_type_str( set_bonus_e );

  static bool role_set_bonus( set_bonus_type_e set_bonus )
  {
    switch ( set_bonus )
    {
      case T13:
      case T14:
      case T15:
      case T16:
        return true;
      default:
        return false;
    }
  }

  static bool role_set_bonus( size_t set_bonus )
  { return role_set_bonus( static_cast<set_bonus_type_e>( set_bonus ) ); }
};

// Cooldown =================================================================

struct cooldown_t
{
  sim_t& sim;
  player_t* player;
  std::string name_str;
  timespan_t duration;
  timespan_t ready;
  timespan_t reset_react;
  int charges;
  int current_charge;
  event_t* recharge_event;
  event_t* ready_trigger_event;
  timespan_t last_start, last_charged;

  cooldown_t( const std::string& name, player_t& );
  cooldown_t( const std::string& name, sim_t& );

  // Adjust the CD. If "requires_reaction" is true (or not provided), then the CD change is something
  // the user would react to rather than plan ahead for.
  void adjust( timespan_t, bool requires_reaction = true );
  void reset( bool require_reaction );
  void start( action_t* action, timespan_t override = timespan_t::min(), timespan_t delay = timespan_t::zero() );
  void start( timespan_t override = timespan_t::min(), timespan_t delay = timespan_t::zero() );

  void reset_init();

  timespan_t remains() const
  { return std::max( timespan_t::zero(), ready - sim.current_time() ); }

  // return true if the cooldown is done (i.e., the associated ability is ready)
  bool up() const
  { return ready <= sim.current_time(); }

  // Return true if the cooldown is currently ticking down
  bool down() const
  { return ready > sim.current_time(); }

  const char* name() const
  { return name_str.c_str(); }

  timespan_t reduced_cooldown() const
  { return ready - last_start; }

  double get_recharge_multiplier() const
  { return recharge_multiplier; }

  void set_recharge_multiplier( double );

  expr_t* create_expression( action_t* a, const std::string& name_str );

  static timespan_t ready_init()
  { return timespan_t::from_seconds( -60 * 60 ); }

  static timespan_t cooldown_duration( const cooldown_t* cd, const timespan_t& override_duration = timespan_t::min(), const action_t* cooldown_action = nullptr );

private:
  double recharge_multiplier;
};

// Player Callbacks
struct player_callbacks_t
{
  std::vector<action_callback_t*> all_callbacks;
  // New proc system

  // Callbacks (procs) stored in a vector
  typedef std::vector<action_callback_t*> proc_list_t;
  // .. an array of callbacks, for each proc_type2 enum (procced by hit/crit, etc...)
  typedef std::array<proc_list_t, PROC2_TYPE_MAX> proc_on_array_t;
  // .. an array of procced by arrays, for each proc_type enum (procced on aoe, heal, tick, etc...)
  typedef std::array<proc_on_array_t, PROC1_TYPE_MAX> proc_array_t;

  proc_array_t procs;

  virtual ~player_callbacks_t()
  { range::sort( all_callbacks ); dispose( all_callbacks.begin(), range::unique( all_callbacks ) ); }

  void reset();

  void register_callback( unsigned proc_flags, unsigned proc_flags2, action_callback_t* cb );
private:
  void add_proc_callback( proc_types type, unsigned flags, action_callback_t* cb );
};

// Stat Cache

/* The Cache system increases simulation performance by moving the calculation point
 * from call-time to modification-time of a stat. Because a stat is called much more
 * often than it is changed, this reduces costly and unnecessary repetition of floating-point
 * operations.
 *
 * When a stat is accessed, its 'valid'-state is checked:
 *   If its true, the cached value is accessed.
 *   If it is false, the stat value is recalculated and written to the cache.
 *
 * To indicate when a stat gets modified, it needs to be 'invalidated'. Every time a stat
 * is invalidated, its 'valid'-state gets set to false.
 */

/* - To invalidate a stat, use player_t::invalidate_cache( cache_e )
 * - using player_t::stat_gain/loss automatically invalidates the corresponding cache
 * - Same goes for stat_buff_t, which works through player_t::stat_gain/loss
 * - Buffs with effects in a composite_ function need invalidates added to their buff_creator
 *
 * To create invalidation chains ( eg. Priest: Spirit invalidates Hit ) override the
 * virtual player_t::invalidate_cache( cache_e ) function.
 *
 * Attention: player_t::invalidate_cache( cache_e ) is recursive and may call itself again.
 */
struct player_stat_cache_t
{
  const player_t* player;
  mutable std::array<bool, CACHE_MAX> valid;
  mutable std::array < bool, SCHOOL_MAX + 1 > spell_power_valid, player_mult_valid, player_heal_mult_valid;
  // 'valid'-states
private:
  // cached values
  mutable double _strength, _agility, _stamina, _intellect, _spirit;
  mutable double _spell_power[SCHOOL_MAX + 1], _attack_power;
  mutable double _attack_expertise;
  mutable double _attack_hit, _spell_hit;
  mutable double _attack_crit, _spell_crit;
  mutable double _attack_haste, _spell_haste;
  mutable double _attack_speed, _spell_speed;
  mutable double _dodge, _parry, _block, _crit_block, _armor, _bonus_armor;
  mutable double _mastery, _mastery_value, _crit_avoidance, _miss, _multistrike, _readiness;
  mutable double _player_mult[SCHOOL_MAX + 1], _player_heal_mult[SCHOOL_MAX + 1];
  mutable double _damage_versatility, _heal_versatility, _mitigation_versatility;
  mutable double _leech, _run_speed, _avoidance;
public:
  bool active; // runtime active-flag
  void invalidate_all();
  void invalidate( cache_e );
  double get_attribute( attribute_e ) const;
  player_stat_cache_t( const player_t* p ) : player( p ), active( false ) { invalidate_all(); }
#if defined(SC_USE_STAT_CACHE)
  // Cache stat functions
  double strength() const;
  double agility() const;
  double stamina() const;
  double intellect() const;
  double spirit() const;
  double spell_power( school_e ) const;
  double attack_power() const;
  double attack_expertise() const;
  double attack_hit() const;
  double attack_crit() const;
  double attack_haste() const;
  double attack_speed() const;
  double spell_hit() const;
  double spell_crit() const;
  double spell_haste() const;
  double spell_speed() const;
  double dodge() const;
  double parry() const;
  double block() const;
  double crit_block() const;
  double crit_avoidance() const;
  double miss() const;
  double armor() const;
  double mastery() const;
  double mastery_value() const;
  double multistrike() const;
  double readiness() const;
  double bonus_armor() const;
  double player_multiplier( school_e ) const;
  double player_heal_multiplier( const action_state_t* ) const;
  double damage_versatility() const;
  double heal_versatility() const;
  double mitigation_versatility() const;
  double leech() const;
  double run_speed() const;
  double avoidance() const;
#else
  // Passthrough cache stat functions for inactive cache
  double strength() const  { return _player -> strength();  }
  double agility() const   { return _player -> agility();   }
  double stamina() const   { return _player -> stamina();   }
  double intellect() const { return _player -> intellect(); }
  double spirit() const    { return _player -> spirit();    }
  double spell_power( school_e s ) const { return _player -> composite_spell_power( s ); }
  double attack_power() const            { return _player -> composite_melee_attack_power();   }
  double attack_expertise() const { return _player -> composite_melee_expertise(); }
  double attack_hit() const       { return _player -> composite_melee_hit();       }
  double attack_crit() const      { return _player -> composite_melee_crit();      }
  double attack_haste() const     { return _player -> composite_melee_haste();     }
  double attack_speed() const     { return _player -> composite_melee_speed();     }
  double spell_hit() const        { return _player -> composite_spell_hit();       }
  double spell_crit() const       { return _player -> composite_spell_crit();      }
  double spell_haste() const      { return _player -> composite_spell_haste();     }
  double spell_speed() const      { return _player -> composite_spell_speed();     }
  double dodge() const            { return _player -> composite_dodge();      }
  double parry() const            { return _player -> composite_parry();      }
  double block() const            { return _player -> composite_block();      }
  double crit_block() const       { return _player -> composite_crit_block(); }
  double crit_avoidance() const   { return _player -> composite_crit_avoidance();       }
  double miss() const             { return _player -> composite_miss();       }
  double armor() const            { return _player -> composite_armor();           }
  double mastery() const          { return _player -> composite_mastery();   }
  double mastery_value() const    { return _player -> composite_mastery_value();   }
  double multistrike() const      { return _player -> composite_multistrike(); }
  double readiness() const        { return _player -> composite_readiness(); }
  double damage_versatility() const { return _player -> composite_damage_versatility(); }
  double heal_versatility() const { return _player -> composite_heal_versatility(); }
  double mitigation_versatility() const { return _player -> composite_mitigation_versatility(); }
  double leech() const { return _player -> composite_leech(); }
  double run_speed() const { return _player -> composite_run_speed(); }
  double avoidance() const { return _player -> composite_avoidance(); }
#endif
};

struct player_processed_report_information_t
{
  bool charts_generated, buff_lists_generated;
  std::string action_dpet_chart, action_dmg_chart, time_spent_chart;
  std::array<std::string, RESOURCE_MAX> timeline_resource_chart, gains_chart;
  std::array<std::string, STAT_MAX> timeline_stat_chart;
  std::string timeline_dps_chart, timeline_dps_error_chart, timeline_resource_health_chart;
  std::string distribution_dps_chart, scaling_dps_chart, scale_factors_chart;
  std::vector<std::pair<unsigned,std::string>> reforge_dps_charts;
  std::string dps_error_chart;
  std::string distribution_deaths_chart;
  std::string health_change_chart, health_change_sliding_chart;
  std::array<std::string, SCALE_METRIC_MAX> gear_weights_lootrank_link, gear_weights_wowhead_std_link, gear_weights_pawn_string, gear_weights_askmrrobot_link;
  std::string save_str;
  std::string save_gear_str;
  std::string save_talents_str;
  std::string save_actions_str;
  std::string comment_str;
  std::string thumbnail_url;
  std::string html_profile_str;
  std::vector<buff_t*> buff_list, dynamic_buffs, constant_buffs;

  player_processed_report_information_t() : charts_generated(), buff_lists_generated() {}
};

/* Contains any data collected during / at the end of combat
 * Mostly statistical data collection, represented as sample data containers
 */
struct player_collected_data_t
{
  extended_sample_data_t fight_length;
  extended_sample_data_t waiting_time, executed_foreground_actions;

  // DMG
  extended_sample_data_t dmg;
  extended_sample_data_t compound_dmg;
  extended_sample_data_t prioritydps;
  extended_sample_data_t dps;
  extended_sample_data_t dpse;
  extended_sample_data_t dtps;
  extended_sample_data_t dmg_taken;
  sc_timeline_t timeline_dmg;
  sc_timeline_t timeline_dmg_taken;
  // Heal
  extended_sample_data_t heal;
  extended_sample_data_t compound_heal;
  extended_sample_data_t hps;
  extended_sample_data_t hpse;
  extended_sample_data_t htps;
  extended_sample_data_t heal_taken;
  sc_timeline_t timeline_healing_taken;
  // Absorb
  extended_sample_data_t absorb;
  extended_sample_data_t compound_absorb;
  extended_sample_data_t aps;
  extended_sample_data_t atps;
  extended_sample_data_t absorb_taken;
  // Tank
  extended_sample_data_t deaths;
  extended_sample_data_t theck_meloree_index;
  extended_sample_data_t effective_theck_meloree_index;
  extended_sample_data_t max_spike_amount;

  // Metric used to end simulations early
  extended_sample_data_t target_metric;
  mutex_t target_metric_mutex;

  std::array<simple_sample_data_t,RESOURCE_MAX> resource_lost, resource_gained;
  struct resource_timeline_t
  {
    resource_e type;
    sc_timeline_t timeline;

    resource_timeline_t( resource_e t = RESOURCE_NONE ) : type( t ) {}
  };
  // Druid requires 4 resource timelines health/mana/energy/rage
  std::vector<resource_timeline_t> resource_timelines;

  std::vector<simple_sample_data_with_min_max_t > combat_end_resource;

  struct stat_timeline_t
  {
    stat_e type;
    sc_timeline_t timeline;

    stat_timeline_t( stat_e t = STAT_NONE ) : type( t ) {}
  };

  std::vector<stat_timeline_t> stat_timelines;

  // hooked up in resource timeline collection event
  struct health_changes_timeline_t
  {
    double previous_loss_level, previous_gain_level;
    sc_timeline_t timeline; // keeps only data per iteration
    sc_timeline_t timeline_normalized; // same as above, but normalized to current player health
    sc_timeline_t merged_timeline;
    bool collect; // whether we collect all this or not.
    health_changes_timeline_t() : previous_loss_level( 0.0 ), previous_gain_level( 0.0 ), collect( false ) {}

    void set_bin_size( double bin )
    {
      timeline.set_bin_size( bin );
      timeline_normalized.set_bin_size( bin );
      merged_timeline.set_bin_size( bin );
    }

    double get_bin_size() const
    {
      if ( timeline.get_bin_size() != timeline_normalized.get_bin_size() || timeline.get_bin_size() != merged_timeline.get_bin_size() )
      {
        assert( false );
        return 0.0;
      }
      else
        return timeline.get_bin_size();
    }
  };

  health_changes_timeline_t health_changes;     //records all health changes
  health_changes_timeline_t health_changes_tmi; //records only health changes due to damage and self-healng/self-absorb

  struct resolve_timeline_t
  {
    sc_timeline_t iteration_timeline;
    sc_timeline_t merged_timeline;

    resolve_timeline_t() {}

    // Add 'value' at corresponding time, replacing existing entry if new value is larger
    void add_max( timespan_t current_time, double new_value )
    {
      size_t index = static_cast< size_t >( current_time.total_millis() / 1000 / iteration_timeline.bin_size );

      // if data doesn't exist in this element, add it
      if ( iteration_timeline.data().size() == 0 || iteration_timeline.data().size() <= index )
        iteration_timeline.add( current_time, new_value );
      // otherwise store only the maximum value
      else if ( new_value > iteration_timeline.data().at( index ) )
      {
        iteration_timeline.add( current_time, new_value - iteration_timeline.data().at( index ) );
      }
    }
  };

  resolve_timeline_t resolve_timeline;

  struct action_sequence_data_t
  {
    const action_t* action;
    const player_t* target;
    const timespan_t time;
    timespan_t wait_time;
    std::vector<std::pair<buff_t*, int> > buff_list;
    std::array<double, RESOURCE_MAX> resource_snapshot;
    std::array<double, RESOURCE_MAX> resource_max_snapshot;

    action_sequence_data_t( const timespan_t& ts, const timespan_t& wait, const player_t* p );
    action_sequence_data_t( const action_t* a, const player_t* t, const timespan_t& ts, const player_t* p );
  };
  auto_dispose< std::vector<action_sequence_data_t*> > action_sequence;
  auto_dispose< std::vector<action_sequence_data_t*> > action_sequence_precombat;

  // Buffed snapshot_stats (for reporting)
  struct buffed_stats_t
  {
    std::array< double, ATTRIBUTE_MAX > attribute;
    std::array< double, RESOURCE_MAX > resource;

    double spell_power, spell_hit, spell_crit, manareg_per_second;
    double attack_power,  attack_hit,  mh_attack_expertise,  oh_attack_expertise, attack_crit;
    double armor, miss, crit, dodge, parry, block, bonus_armor;
    double spell_haste, spell_speed, attack_haste, attack_speed;
    double mastery_value, multistrike, readiness;
    double damage_versatility, heal_versatility, mitigation_versatility;
    double leech, run_speed, avoidance;
  } buffed_stats_snapshot;

  player_collected_data_t( const std::string& player_name, sim_t& );
  void reserve_memory( const player_t& );
  void merge( const player_collected_data_t& );
  void analyze( const player_t& );
  void collect_data( const player_t& );
  void print_tmi_debug_csv( const sc_timeline_t* nma, const std::vector<double>& weighted_value, const player_t& p );
  double calculate_tmi( const health_changes_timeline_t& tl, int window, double f_length, const player_t& p );
  double calculate_max_spike_damage( const health_changes_timeline_t& tl, int window );
  std::ostream& data_str( std::ostream& s ) const;
};

struct player_talent_points_t
{
public:
  player_talent_points_t() { clear(); }

  int choice( int row ) const
  {
    row_check( row );
    return choices[ row ];
  }

  void clear( int row )
  {
    row_check( row );
    choices[ row ] = -1;
  }

  bool has_row_col( int row, int col ) const
  { return choice( row ) == col; }

  void select_row_col( int row, int col )
  {
    row_col_check( row, col );
    choices[ row ] = col;
  }

  void clear();
  std::string to_string() const;

  friend std::ostream& operator << ( std::ostream& os, const player_talent_points_t& tp )
  { os << tp.to_string(); return os; }
private:
  std::array<int, MAX_TALENT_ROWS> choices;

  static void row_check( int row )
  { assert( row >= 0 && row < MAX_TALENT_ROWS ); ( void )row; }

  static void column_check( int col )
  { assert( col >= 0 && col < MAX_TALENT_COLS ); ( void )col; }

  static void row_col_check( int row, int col )
  { row_check( row ); column_check( col ); }

};

// Actor
/* actor_t is a lightweight representation of an actor belonging to a simulation,
 * having a name and some event-related helper functionality.
 */

struct actor_t : private noncopyable
{
  sim_t* sim; // owner
  std::string name_str;
  int event_counter; // safety counter. Shall never be less than zero

#if defined(ACTOR_EVENT_BOOKKEEPING)
  /// Measure cpu time for actor-specific events.
  stopwatch_t event_stopwatch;
#endif // ACTOR_EVENT_BOOKKEEPING

  actor_t( sim_t* s, const std::string& name ) :
    sim( s ), name_str( name ),
    event_counter( 0 ),
    event_stopwatch( STOPWATCH_THREAD )
  {

  }
  virtual ~ actor_t() { }
  virtual const char* name() const
  { return name_str.c_str(); }
};

namespace resolve {
/* Encapsulations of Resolve interface so we can keep most things out of player_t (bloated enough)
 */
struct manager_t
{
  manager_t( player_t& );
  void init();
  void start();
  void stop();
  void update();
  bool is_started() const
  { return _started; }
  bool is_init() const
  { return _init; }
  void add_diminishing_return_entry( const player_t* actor, double raw_dps, timespan_t current_time );
  int get_diminsihing_return_rank( int actor_spawn_index );
  void add_damage_event( double amount, timespan_t current_time );
  const spell_data_t* resolve;
private:
  struct update_event_t;
  struct diminishing_returns_list_t;
  struct damage_event_list_t;
  player_t& _player;
  event_t* _update_event;
  bool _init;
  bool _started;
  std::unique_ptr<damage_event_list_t >_damage_list;
  std::unique_ptr<diminishing_returns_list_t> _diminishing_return_list;
};

} // resolve

/* Player Report Extension
 * Allows class modules to write extension to the report sections
 * based on the dynamic class of the player
 */

struct player_report_extension_t
{
public:
  virtual ~player_report_extension_t()
  {

  }
  virtual void html_customsection( report::sc_html_stream& )
  {

  }
};

// Player ===================================================================

enum action_var_e
{
  OPERATION_NONE = -1,
  OPERATION_SET,
  OPERATION_PRINT,
  OPERATION_RESET,
  OPERATION_ADD,
  OPERATION_SUB
};

struct action_variable_t
{
  std::string name_;
  double current_value_, default_;

  action_variable_t( const std::string& name, double def = 0 ) :
    name_( name ), current_value_( def ), default_( def )
  { }

  double value() const
  { return current_value_; }

  void reset()
  { current_value_ = default_; }
};

struct scaling_metric_data_t {
  std::string name;
  double value, stddev;
  scale_metric_e metric;
  scaling_metric_data_t( scale_metric_e m, const std::string& n, double v, double dev ) :
    name( n ), value( v ), stddev( dev ), metric( m ) {}
  scaling_metric_data_t( scale_metric_e m, const extended_sample_data_t& sd ) :
    name( sd.name_str ), value( sd.mean() ), stddev( sd.mean_std_dev ), metric( m ) {}
  scaling_metric_data_t( scale_metric_e m, const sc_timeline_t& tl, const std::string& name ) :
    name( name ), value( tl.mean() ), stddev( tl.mean_stddev() ), metric( m ) {}
};

struct player_t : public actor_t
{
  static const int default_level = 100;

  // static values
  player_e type;
  player_t* parent; // corresponding player in main thread
  int index;
  size_t actor_index;
  int actor_spawn_index; // a unique identifier for each arise() of the actor
  // (static) attributes - things which should not change during combat
  race_e       race;
  role_e       role;
  int          true_level; /* The character's true level. If the outcome would change when the character's level is
                           scaled down (such as when timewalking) then use the level() method instead. */
  int          party;
  int          ready_type;
  specialization_e  _spec;
  bool         bugs; // If true, include known InGame mechanics which are probably the cause of a bug and not inteded
  int          disable_hotfixes;
  bool scale_player;
  double death_pct; // Player will die if he has equal or less than this value as health-pct
  double size; // Actor size, only used for enemies. Affects the travel distance calculation for spells.
  int          timewalk;

  // dynamic attributes - things which change during combat
  player_t*   target;
  bool        initialized;
  bool        potion_used;

  std::string talents_str, glyphs_str, id_str, target_str;
  std::string region_str, server_str, origin_str;
  std::string race_str, professions_str, position_str;
  enum timeofday_e { NIGHT_TIME, DAY_TIME, } timeofday; // Specify InGame time of day to determine Night Elf racial
  timespan_t  gcd_ready, base_gcd, started_waiting;
  std::vector<pet_t*> pet_list;
  std::vector<pet_t*> active_pets;
  std::vector<absorb_buff_t*> absorb_buff_list;
  std::map<unsigned,instant_absorb_t*> instant_absorb_list;
  resolve::manager_t resolve_manager;

  int         invert_scaling;

  // Reaction
  timespan_t  reaction_offset, reaction_mean, reaction_stddev, reaction_nu;
  // Latency
  timespan_t  world_lag, world_lag_stddev;
  timespan_t  brain_lag, brain_lag_stddev;
  bool        world_lag_override, world_lag_stddev_override;

  // Data access
  dbc_t       dbc;

  // Option Parsing
  std::vector<std::unique_ptr<option_t>> options;

  // Stat Timelines to Display
  std::vector<stat_e> stat_timelines;

  // Talent Parsing
  player_talent_points_t talent_points;
  std::string talent_overrides_str;

  // Glyph Parsing
  std::vector<const spell_data_t*> glyph_list;

  // Profs
  std::array<int, PROFESSION_MAX> profession;

  virtual ~player_t();

  // TODO: FIXME, these stats should not be increased by scale factor deltas
  struct base_initial_current_t
  {
    base_initial_current_t();
    std::string to_string();
    gear_stats_t stats;

    double mana_regen_per_second;

    double spell_power_per_intellect, spell_crit_per_intellect;
    double attack_power_per_strength, attack_power_per_agility, attack_crit_per_agility;
    double dodge_per_agility, parry_per_strength;
    double mana_regen_per_spirit, mana_regen_from_spirit_multiplier, health_per_stamina;
    std::array<double, SCHOOL_MAX> resource_reduction;
    double miss, dodge, parry, block;
    double hit, expertise;
    double spell_crit, attack_crit, block_reduction, mastery;
    double skill, skill_debuff, distance;
    double distance_to_move;
    double moving_away;
    movement_direction_e movement_direction;
    double armor_coeff;
  private:
    friend struct player_t;
    bool sleeping;
    rating_t rating;
  public:

    std::array<double, ATTRIBUTE_MAX> attribute_multiplier;
    double spell_power_multiplier, attack_power_multiplier, armor_multiplier;
    position_e position;
  }
  base, // Base values, from some database or overridden by user
  initial, // Base + Gear + Global Enchants
  current; // Current values, reset to initial before every iteration

  const rating_t& current_rating() const
  { return current.rating; }
  const rating_t& initial_rating() const
  { return initial.rating; }

  // Spell Mechanics
  double base_energy_regen_per_second;
  double base_focus_regen_per_second;
  double base_chi_regen_per_second;
  timespan_t last_cast;

  // Defense Mechanics
  struct diminishing_returns_constants_t
  {
    double horizontal_shift;
    double vertical_stretch;
    double dodge_factor;
    double parry_factor;
    double miss_factor;
    double block_factor;

    diminishing_returns_constants_t()
    {
      horizontal_shift = vertical_stretch = 0.0;
      dodge_factor = parry_factor = miss_factor = block_factor = 1.0;
    }
  } def_dr;

  // Weapons
  weapon_t main_hand_weapon;
  weapon_t off_hand_weapon;

  // Main, offhand, and ranged attacks
  attack_t* main_hand_attack;
  attack_t*  off_hand_attack;

  // Resources
  struct resources_t
  {
    std::array<double, RESOURCE_MAX> base, initial, max, current, temporary,
        base_multiplier, initial_multiplier;
    std::array<int, RESOURCE_MAX> infinite_resource;

    resources_t()
    {
      range::fill( base, 0.0 );
      range::fill( initial, 0.0 );
      range::fill( max, 0.0 );
      range::fill( current, 0.0 );
      range::fill( temporary, 0.0 );
      range::fill( base_multiplier, 1.0 );
      range::fill( initial_multiplier, 1.0 );
      range::fill( infinite_resource, 0 );
    }

    double pct( resource_e rt ) const
    { return current[ rt ] / max[ rt ]; }

    bool is_infinite( resource_e rt ) const
    { return infinite_resource[ rt ] != 0; }
  } resources;

  struct consumables_t {
    stat_buff_t* flask;
    stat_buff_t* guardian_elixir;
    stat_buff_t* battle_elixir;
    stat_buff_t* food;
    stat_buff_t* augmentation;
  } consumables;

  // Events
  action_t* executing;
  action_t* channeling;
  action_t* strict_sequence; // Strict sequence of actions currently being executed
  event_t* readying;
  event_t* off_gcd;
  bool in_combat;
  bool action_queued;
  bool first_cast;
  action_t* last_foreground_action;
  action_t* last_gcd_action;
  std::vector<action_t*> off_gcdactions; // Returns all off gcd abilities used since the last gcd.

  // Delay time used by "cast_delay" expression to determine when an action
  // can be used at minimum after a spell cast has finished, including GCD
  timespan_t cast_delay_reaction;
  timespan_t cast_delay_occurred;

  // Callbacks
  player_callbacks_t callbacks;
  auto_dispose< std::vector<special_effect_t*> > special_effects;
  std::vector<std::function<void(void)> > callbacks_on_demise;

  // Action Priority List
  auto_dispose< std::vector<action_t*> > action_list;
  std::string action_list_str;
  std::string choose_action_list;
  std::string action_list_skip;
  std::string modify_action;
  std::string use_apl;
  bool use_default_action_list;
  auto_dispose< std::vector<dot_t*> > dot_list;
  auto_dispose< std::vector<action_priority_list_t*> > action_priority_list;
  std::vector<action_t*> precombat_action_list;
  action_priority_list_t* active_action_list;
  action_priority_list_t* active_off_gcd_list;
  action_priority_list_t* restore_action_list;
  std::unordered_map<std::string, std::string> alist_map;
  std::string action_list_information; // comment displayed in profile
  bool no_action_list_provided;

  bool quiet;
  // Reporting
  std::unique_ptr<player_report_extension_t> report_extension;
  timespan_t iteration_fight_length, arise_time;
  timespan_t iteration_waiting_time;
  int iteration_executed_foreground_actions;
  std::array< double, RESOURCE_MAX > iteration_resource_lost, iteration_resource_gained;
  double rps_gain, rps_loss;
  std::string tmi_debug_file_str;
  double tmi_window;

  auto_dispose< std::vector<buff_t*> > buff_list;
  auto_dispose< std::vector<proc_t*> > proc_list;
  auto_dispose< std::vector<gain_t*> > gain_list;
  auto_dispose< std::vector<stats_t*> > stats_list;
  auto_dispose< std::vector<benefit_t*> > benefit_list;
  auto_dispose< std::vector<uptime_t*> > uptime_list;
  auto_dispose< std::vector<cooldown_t*> > cooldown_list;
  std::array< std::vector<plot_data_t>, STAT_MAX > dps_plot_data;
  std::unordered_map<const reforge_plot_run_t*, std::vector<std::vector<plot_data_t>>> reforge_plot_data;
  auto_dispose< std::vector<luxurious_sample_data_t*> > sample_data_list;

  // All Data collected during / end of combat
  player_collected_data_t collected_data;

  // Damage
  double iteration_dmg, priority_iteration_dmg, iteration_dmg_taken; // temporary accumulators
  double dpr;
  std::vector<std::pair<timespan_t, double> > incoming_damage; // for tank active mitigation conditionals

  std::vector<double> dps_convergence_error;
  double dps_convergence;

  // Heal
  double iteration_heal, iteration_heal_taken, iteration_absorb, iteration_absorb_taken; // temporary accumulators
  double hpr;
  std::vector<unsigned> absorb_priority; // for strict sequence absorbs

  player_processed_report_information_t report_information;

  void sequence_add( const action_t* a, const player_t* target, const timespan_t& ts );
  void sequence_add_wait( const timespan_t& amount, const timespan_t& ts );

  // Gear
  std::string items_str, meta_gem_str;
  std::vector<item_t> items;
  gear_stats_t gear, enchant;
  gear_stats_t total_gear; // composite of gear, enchant and for non-pets sim -> enchant
  set_bonus_t sets;
  meta_gem_e meta_gem;
  bool matching_gear;
  cooldown_t item_cooldown;
  cooldown_t* legendary_tank_cloak_cd; // non-Null if item available

  // Warlord's Unseeing Eye (6.2 Trinket)
  double warlords_unseeing_eye;
  stats_t* warlords_unseeing_eye_stats;

  // Scale Factors
  std::array<gear_stats_t, SCALE_METRIC_MAX> scaling;
  std::array<gear_stats_t, SCALE_METRIC_MAX> scaling_normalized;
  std::array<gear_stats_t, SCALE_METRIC_MAX> scaling_error;
  std::array<gear_stats_t, SCALE_METRIC_MAX> scaling_delta_dps;
  std::array<gear_stats_t, SCALE_METRIC_MAX> scaling_compare_error;
  std::array<double, SCALE_METRIC_MAX> scaling_lag, scaling_lag_error;
  std::array<bool, STAT_MAX> scales_with;
  std::array<double, STAT_MAX> over_cap;
  std::array<std::vector<stat_e>, SCALE_METRIC_MAX> scaling_stats; // sorting vector

  // Movement & Position
  double base_movement_speed;
  double passive_modifier; // _PASSIVE_ movement speed modifiers
  double x_position, y_position;

  struct buffs_t
  {
    buff_t* angelic_feather;
    buff_t* aspect_of_the_fox;
    buff_t* aspect_of_the_pack;
    buff_t* beacon_of_light;
    buff_t* blood_fury;
    buff_t* body_and_soul;
    buff_t* darkflight;
    buff_t* devotion_aura;
    buff_t* earth_shield;
    buff_t* exhaustion;
    buff_t* guardian_spirit;
    buff_t* hand_of_sacrifice;
    buff_t* mongoose_mh;
    buff_t* mongoose_oh;
    buff_t* nitro_boosts;
    buff_t* pain_supression;
    buff_t* raid_movement;
    buff_t* self_movement;
    buff_t* stampeding_roar;
    buff_t* shadowmeld;
    buff_t* fierce_tiger_movement_aura;
    buff_t* stoneform;
    buff_t* stunned;
    buff_t* weakened_soul;
    buff_t* resolve;

    haste_buff_t* berserking;
    haste_buff_t* bloodlust;

    buff_t* cooldown_reduction;
    buff_t* amplification;
    buff_t* amplification_2;

    // Legendary meta stuff
    buff_t* courageous_primal_diamond_lucidity;
    buff_t* tempus_repit;
    buff_t* fortitude;

    buff_t* archmages_greater_incandescence_str;
    buff_t* archmages_greater_incandescence_agi;
    buff_t* archmages_greater_incandescence_int;
    buff_t* archmages_incandescence_str;
    buff_t* archmages_incandescence_agi;
    buff_t* archmages_incandescence_int;
    buff_t* legendary_aoe_ring; // Legendary ring buff.
    buff_t* legendary_tank_buff;

    // T17 LFR stuff
    buff_t* surge_of_energy;
    buff_t* natures_fury;
    buff_t* brute_strength;

    // 6.2 trinket proxy buffs
    buff_t* naarus_discipline; // Priest-Discipline Boss 13 T18 trinket
    buff_t* spirit_shift; // Soul Capacitor trinket
    buff_t* tyrants_immortality; // Tyrant's Decree trinket proc
    buff_t* tyrants_decree_driver; // Tyrant's Decree trinket driver

    haste_buff_t* fel_winds; // T18 LFR Plate Melee Attack Speed buff
    buff_t* demon_damage_buff; // 6.2.3 Heirloom trinket demon damage buff
  } buffs;

  struct debuffs_t
  {
    debuff_t* bleeding;
    debuff_t* casting;
    debuff_t* flying;
    debuff_t* forbearance;
    debuff_t* invulnerable;
    debuff_t* vulnerable;
    debuff_t* dazed;
    debuff_t* damage_taken;

    // WoD debuffs
    debuff_t* mortal_wounds;
  } debuffs;

  struct gains_t
  {
    gain_t* arcane_torrent;
    gain_t* endurance_of_niuzao;
    gain_t* energy_regen;
    gain_t* focus_regen;
    gain_t* health;
    gain_t* mana_potion;
    gain_t* mp5_regen;
    gain_t* restore_mana;
    gain_t* touch_of_the_grave;
    gain_t* vampiric_embrace;
    gain_t* warlords_unseeing_eye;

    gain_t* leech;
  } gains;

  struct spells_t
  {
    action_t* leech;
  } spell;

  struct procs_t
  {
    proc_t* hat_donor;
    proc_t* parry_haste;
  } procs;

  struct uptimes_t
  {
    uptime_t* primary_resource_cap;
  } uptimes;

  struct racials_t
  {
    const spell_data_t* quickness;
    const spell_data_t* command;
    const spell_data_t* arcane_acuity;
    const spell_data_t* heroic_presence;
    const spell_data_t* might_of_the_mountain;
    const spell_data_t* expansive_mind;
    const spell_data_t* nimble_fingers;
    const spell_data_t* time_is_money;
    const spell_data_t* the_human_spirit;
    const spell_data_t* touch_of_elune;
    const spell_data_t* brawn;
    const spell_data_t* endurance;
    const spell_data_t* viciousness;
  } racials;

  struct passives_t
  {
    double amplification_1;
    double amplification_2;
  } passive_values;

  bool active_during_iteration;
  const spelleffect_data_t* _mastery; // = find_mastery_spell( specialization() ) -> effectN( 1 );

  player_t( sim_t* sim, player_e type, const std::string& name, race_e race_e );

  virtual const char* name() const override { return name_str.c_str(); }

  virtual void init();
  virtual void override_talent( std::string override_str );
  virtual void init_meta_gem( gear_stats_t& );
  virtual void init_resources( bool force = false );
  virtual std::string init_use_item_actions( const std::string& append = std::string() );
  virtual std::string init_use_profession_actions( const std::string& append = std::string() );
  virtual std::string init_use_racial_actions( const std::string& append = std::string() );
  virtual std::vector<std::string> get_item_actions( const std::string& options = std::string() );
  virtual std::vector<std::string> get_profession_actions();
  virtual std::vector<std::string> get_racial_actions();
  bool add_action( std::string action, std::string options = "", std::string alist = "default" );
  bool add_action( const spell_data_t* s, std::string options = "", std::string alist = "default" );
  std::string include_default_on_use_items( player_t&, const std::string& exclude_effects );
  std::string include_specific_on_use_item( player_t&, const std::string& effect_names, const std::string& options );

  virtual void init_target();
  void init_character_properties();
  virtual void init_race();
  virtual void init_talents();
  virtual void init_glyphs();
  virtual void replace_spells();
  virtual void init_position();
  virtual void init_professions();
  virtual void init_spells();
  virtual bool init_items();
  virtual void init_weapon( weapon_t& );
  virtual void init_base_stats();
  virtual void init_initial_stats();

  virtual void init_defense();
  virtual void create_buffs();
  virtual void init_special_effects();
  virtual void init_scaling();
  virtual void init_action_list() {}
  virtual void init_gains();
  virtual void init_procs();
  virtual void init_uptimes();
  virtual void init_benefits();
  virtual void init_rng();
  virtual void init_stats();
  virtual void init_distance_targeting();
  virtual void init_absorb_priority();
  virtual void register_callbacks();
  // Class specific hook for first-phase initializing special effects. Returns true if the class-specific hook initialized something, false otherwise.
  virtual bool init_special_effect( special_effect_t& /* effect */, unsigned /* spell_id */ ) { return false; }

  virtual bool init_actions();
  virtual bool init_finished();

  virtual void reset();
  virtual void combat_begin();
  virtual void combat_end();
  virtual void merge( player_t& other );

  virtual void datacollection_begin();
  virtual void datacollection_end();

  virtual int level() const;

  virtual double energy_regen_per_second() const;
  virtual double focus_regen_per_second() const;
  virtual double mana_regen_per_second() const;

  virtual double composite_melee_haste() const;
  virtual double composite_melee_speed() const;
  virtual double composite_melee_attack_power() const;
  virtual double composite_melee_hit() const;
  virtual double composite_melee_crit() const;
  virtual double composite_melee_crit_multiplier() const { return 1.0; }
  virtual double composite_melee_expertise( const weapon_t* w = nullptr ) const;

  virtual double composite_spell_haste() const; //This is the subset of the old_spell_haste that applies to RPPM
  virtual double composite_spell_speed() const; //This is the old spell_haste and incorporates everything that buffs cast speed
  virtual double composite_spell_power( school_e school ) const;
  virtual double composite_spell_crit() const;
  virtual double composite_spell_crit_multiplier() const { return 1.0; }
  virtual double composite_spell_hit() const;
  virtual double composite_mastery() const;
  virtual double composite_mastery_value() const;
  virtual double composite_multistrike() const;
  virtual double composite_readiness() const;
  virtual double composite_bonus_armor() const;

  virtual double composite_damage_versatility() const;
  virtual double composite_heal_versatility() const;
  virtual double composite_mitigation_versatility() const;

  virtual double composite_leech() const;
  virtual double composite_run_speed() const;
  virtual double composite_avoidance() const;

  virtual double composite_armor() const;
  virtual double composite_armor_multiplier() const;
  virtual double composite_miss() const;
  virtual double composite_dodge() const;
  virtual double composite_parry() const;
  virtual double composite_block() const;
          double composite_block_dr( double extra_block ) const;
  virtual double composite_block_reduction() const;
  virtual double composite_crit_block() const;
  virtual double composite_crit_avoidance() const;

  virtual double composite_attack_power_multiplier() const;
  virtual double composite_spell_power_multiplier() const;

  virtual double matching_gear_multiplier( attribute_e /* attr */ ) const { return 0; }

  virtual double composite_player_multiplier   ( school_e ) const;
  virtual double composite_player_dd_multiplier( school_e,  const action_t* /* a */ = nullptr ) const { return 1; }
  virtual double composite_player_td_multiplier( school_e,  const action_t* a = nullptr ) const;
  // Persistent multipliers that are snapshot at the beginning of the spell application/execution
  virtual double composite_persistent_multiplier( school_e ) const
  { return 1.0; }

  virtual double composite_player_heal_multiplier( const action_state_t* s ) const;
  virtual double composite_player_dh_multiplier( school_e /* school */ ) const { return 1; }
  virtual double composite_player_th_multiplier( school_e school ) const;

  virtual double composite_player_absorb_multiplier( const action_state_t* s ) const;

  virtual double composite_player_critical_damage_multiplier() const;
  virtual double composite_player_critical_healing_multiplier() const;
  virtual double composite_player_multistrike_damage_multiplier() const 
  { return 0.30; };
  virtual double composite_player_multistrike_healing_multiplier() const 
  { return 0.30; };

  virtual double composite_mitigation_multiplier( school_e ) const;

  virtual double temporary_movement_modifier() const;
  virtual double passive_movement_modifier() const;
  virtual double composite_movement_speed() const;

  virtual double composite_attribute( attribute_e attr ) const;
  virtual double composite_attribute_multiplier( attribute_e attr ) const;

  virtual double composite_rating_multiplier( rating_e /* rating */ ) const;
  virtual double composite_rating( rating_e rating ) const;

  virtual double composite_spell_hit_rating() const
  { return composite_rating( RATING_SPELL_HIT ); }
  virtual double composite_spell_crit_rating() const
  { return composite_rating( RATING_SPELL_CRIT ); }
  virtual double composite_spell_haste_rating() const
  { return composite_rating( RATING_SPELL_HASTE ); }

  virtual double composite_melee_hit_rating() const
  { return composite_rating( RATING_MELEE_HIT ); }
  virtual double composite_melee_crit_rating() const
  { return composite_rating( RATING_MELEE_CRIT ); }
  virtual double composite_melee_haste_rating() const
  { return composite_rating( RATING_MELEE_HASTE ); }

  virtual double composite_ranged_hit_rating() const
  { return composite_rating( RATING_RANGED_HIT ); }
  virtual double composite_ranged_crit_rating() const
  { return composite_rating( RATING_RANGED_CRIT ); }
  virtual double composite_ranged_haste_rating() const
  { return composite_rating( RATING_RANGED_HASTE ); }

  virtual double composite_mastery_rating() const
  { return composite_rating( RATING_MASTERY ); }
  virtual double composite_expertise_rating() const
  { return composite_rating( RATING_EXPERTISE ); }

  virtual double composite_dodge_rating() const
  { return composite_rating( RATING_DODGE ); }
  virtual double composite_parry_rating() const
  { return composite_rating( RATING_PARRY ); }
  virtual double composite_block_rating() const
  { return composite_rating( RATING_BLOCK ); }

  virtual double composite_multistrike_rating() const
  { return composite_rating( RATING_MULTISTRIKE ); }

  virtual double composite_readiness_rating() const
  { return composite_rating( RATING_READINESS ); }

  virtual double composite_damage_versatility_rating() const
  { return composite_rating( RATING_DAMAGE_VERSATILITY ); }
  virtual double composite_heal_versatility_rating() const
  { return composite_rating( RATING_HEAL_VERSATILITY ); }
  virtual double composite_mitigation_versatility_rating() const
  { return composite_rating( RATING_MITIGATION_VERSATILITY ); }

  virtual double composite_leech_rating() const
  { return composite_rating( RATING_LEECH ); }

  virtual double composite_speed_rating() const
  { return composite_rating( RATING_SPEED ); }

  virtual double composite_avoidance_rating() const
  { return composite_rating( RATING_AVOIDANCE ); }

  double get_attribute( attribute_e a ) const
  { return util::floor( composite_attribute( a ) * composite_attribute_multiplier( a ) ); }

  double strength() const  { return get_attribute( ATTR_STRENGTH ); }
  double agility() const   { return get_attribute( ATTR_AGILITY ); }
  double stamina() const   { return get_attribute( ATTR_STAMINA ); }
  double intellect() const { return get_attribute( ATTR_INTELLECT ); }
  double spirit() const    { return get_attribute( ATTR_SPIRIT ); }
  double mastery_coefficient() const { return _mastery -> mastery_value(); }

  // Stat Caching
  player_stat_cache_t cache;
#if defined(SC_USE_STAT_CACHE)
  virtual void invalidate_cache( cache_e c );
#else
  void invalidate_cache( cache_e ) {}
#endif

  virtual void interrupt();
  virtual void halt();
  virtual void moving();
  virtual void finish_moving() { }
  virtual void stun();
  virtual void clear_debuffs();
  virtual void trigger_ready();
  virtual void schedule_ready( timespan_t delta_time = timespan_t::zero(), bool waiting = false );
  virtual void arise();
  virtual void demise();
  virtual timespan_t available() const { return timespan_t::from_seconds( 0.1 ); }
  virtual action_t* select_action( const action_priority_list_t& );
  virtual action_t* execute_action();

  virtual void   regen( timespan_t periodicity = timespan_t::from_seconds( 0.25 ) );
  virtual double resource_gain( resource_e resource_type, double amount, gain_t* g = nullptr, action_t* a = nullptr );
  virtual double resource_loss( resource_e resource_type, double amount, gain_t* g = nullptr, action_t* a = nullptr );
  virtual void   recalculate_resource_max( resource_e resource_type );
  virtual bool   resource_available( resource_e resource_type, double cost ) const;
  void collect_resource_timeline_information();
  virtual resource_e primary_resource() const { return RESOURCE_NONE; }
  virtual role_e   primary_role() const;
  virtual stat_e convert_hybrid_stat( stat_e s ) const { return s; }
  specialization_e specialization() const { return _spec; }
  const char* primary_tree_name() const;
  virtual stat_e normalize_by() const;

  virtual double health_percentage() const;
  virtual double max_health() const;
  virtual double current_health() const;
  virtual timespan_t time_to_percent(double percent) const;
  timespan_t total_reaction_time();

  void stat_gain( stat_e stat, double amount, gain_t* g = nullptr, action_t* a = nullptr, bool temporary = false );
  void stat_loss( stat_e stat, double amount, gain_t* g = nullptr, action_t* a = nullptr, bool temporary = false );

  void modify_current_rating( rating_e stat, double amount );

  virtual void cost_reduction_gain( school_e school, double amount, gain_t* g = nullptr, action_t* a = nullptr );
  virtual void cost_reduction_loss( school_e school, double amount, action_t* a = nullptr );

  virtual double get_raw_dps( action_state_t* );
  virtual void assess_damage( school_e, dmg_e, action_state_t* );
  virtual void target_mitigation( school_e, dmg_e, action_state_t* );
  virtual void assess_damage_imminent_pre_absorb( school_e, dmg_e, action_state_t* );
  virtual void assess_damage_imminent( school_e, dmg_e, action_state_t* );
  double       compute_incoming_damage( timespan_t = timespan_t::from_seconds( 5 ) );
  double       calculate_time_to_bloodlust();

  virtual void assess_heal( school_e, dmg_e, action_state_t* );

  virtual bool taunt( player_t* /* source */ ) { return false; }

  virtual void  summon_pet( const std::string& name, timespan_t duration = timespan_t::zero() );
  virtual void dismiss_pet( const std::string& name );

  bool is_moving() const { return buffs.raid_movement -> check() || buffs.self_movement -> check(); }

  bool parse_talents_numbers( const std::string& talent_string );
  bool parse_talents_armory( const std::string& talent_string );
  bool parse_talents_wowhead( const std::string& talent_string );

  void create_talents_numbers();
  void create_talents_armory();
  void create_talents_wowhead();

  const spell_data_t* find_glyph( const std::string& name ) const;
  const spell_data_t* find_racial_spell( const std::string& name, const std::string& token = std::string(), race_e s = RACE_NONE ) const;
  const spell_data_t* find_class_spell( const std::string& name, const std::string& token = std::string(), specialization_e s = SPEC_NONE ) const;
  const spell_data_t* find_pet_spell( const std::string& name, const std::string& token = std::string() ) const;
  const spell_data_t* find_talent_spell( const std::string& name, const std::string& token = std::string(), specialization_e s = SPEC_NONE, bool name_tokenized = false, bool check_validity = true ) const;
  const spell_data_t* find_glyph_spell( const std::string& name, const std::string& token = std::string() ) const;
  const spell_data_t* find_specialization_spell( const std::string& name, const std::string& token = std::string(), specialization_e s = SPEC_NONE ) const;
  const spell_data_t* find_perk_spell( const std::string& name, specialization_e s = SPEC_NONE ) const;
  const spell_data_t* find_perk_spell( size_t idx, specialization_e s = SPEC_NONE ) const;
  const spell_data_t* find_mastery_spell( specialization_e s, const std::string& token = std::string(), uint32_t idx = 0 ) const;
  const spell_data_t* find_spell( const std::string& name, const std::string& token = std::string(), specialization_e s = SPEC_NONE ) const;
  const spell_data_t* find_spell( const unsigned int id, const std::string& token = std::string() ) const;

  virtual expr_t* create_expression( action_t*, const std::string& name );
  expr_t* create_resource_expression( const std::string& name );

  virtual void create_options();
  void add_option( std::unique_ptr<option_t> o )
  {
    // Push_front so derived classes (eg. enemy_t) can override existing options
    // (eg. target_level)
    options.insert( options.begin(), std::move( o ) );
  }
  void recreate_talent_str( talent_format_e format = TALENT_FORMAT_NUMBERS );
  virtual std::string create_profile( save_e = SAVE_ALL );

  virtual void copy_from( player_t* source );

  virtual action_t* create_action( const std::string& name, const std::string& options );
  virtual void      create_pets() { }
  virtual pet_t*    create_pet( const std::string& /* name*/,  const std::string& /* type */ = std::string() ) { return nullptr; }

  virtual void armory_extensions( const std::string& /* region */, const std::string& /* server */, const std::string& /* character */,
                                  cache::behavior_e /* behavior */ = cache::players() )
  {}

  // Class-Specific Methods
  static player_t* create( sim_t* sim, const player_description_t& );

  // Raid-wide aura/buff/debuff maintenance
  static bool init ( sim_t* sim );

  bool is_pet() const { return type == PLAYER_PET || type == PLAYER_GUARDIAN || type == ENEMY_ADD; }
  bool is_enemy() const { return _is_enemy( type ); }
  static bool _is_enemy( player_e t ) { return t == ENEMY || t == ENEMY_ADD || t == TMI_BOSS || t == TANK_DUMMY; }
  bool is_add() const { return type == ENEMY_ADD; }
  static bool _is_sleeping( const player_t* t ) { return t -> current.sleeping; }
  bool is_sleeping() const { return _is_sleeping( this ); }

  pet_t* cast_pet() { return debug_cast<pet_t*>( this ); }
  const pet_t* cast_pet() const { return debug_cast<const pet_t*>( this ); }
  bool is_my_pet( player_t* t ) const;

  /**
   * Returns owner if available, otherwise the player itself.
   */
  virtual const player_t* get_owner_or_self() const
  { return this; }

  player_t* get_owner_or_self()
  { return const_cast<player_t*>(static_cast<const player_t*>(this) -> get_owner_or_self()); }

  bool      in_gcd() const { return gcd_ready > sim -> current_time(); }
  bool      recent_cast();
  bool      dual_wield() const { return main_hand_weapon.type != WEAPON_NONE && off_hand_weapon.type != WEAPON_NONE; }
  bool      has_shield_equipped() const
  { return  items[ SLOT_OFF_HAND ].parsed.data.item_subclass == ITEM_SUBCLASS_ARMOR_SHIELD; }

  // T18 Hellfire Citadel class trinket detection
  virtual bool has_t18_class_trinket() const;

  action_priority_list_t* find_action_priority_list( const std::string& name ) const;
  void                    clear_action_priority_lists() const;
  void                    copy_action_priority_list( const std::string& old_list, const std::string& new_list );

  pet_t*    find_pet( const std::string& name ) const;
  item_t*     find_item( const std::string& );
  action_t*   find_action( const std::string& ) const;
  cooldown_t* find_cooldown( const std::string& name ) const;
  dot_t*      find_dot     ( const std::string& name, player_t* source ) const;
  stats_t*    find_stats   ( const std::string& name ) const;
  gain_t*     find_gain    ( const std::string& name ) const;
  proc_t*     find_proc    ( const std::string& name ) const;
  benefit_t*  find_benefit ( const std::string& name ) const;
  uptime_t*   find_uptime  ( const std::string& name ) const;
  luxurious_sample_data_t* find_sample_data( const std::string& name ) const;

  cooldown_t* get_cooldown( const std::string& name );
  dot_t*      get_dot     ( const std::string& name, player_t* source );
  gain_t*     get_gain    ( const std::string& name );
  proc_t*     get_proc    ( const std::string& name );
  stats_t*    get_stats   ( const std::string& name, action_t* action = nullptr );
  benefit_t*  get_benefit ( const std::string& name );
  uptime_t*   get_uptime  ( const std::string& name );
  luxurious_sample_data_t* get_sample_data( const std::string& name );
  double      get_player_distance( player_t& );
  double      get_ground_aoe_distance( action_state_t& );
  double      get_position_distance( double m = 0, double v = 0 );
  double avg_item_level() const;
  action_priority_list_t* get_action_priority_list( const std::string& name, const std::string& comment = std::string() );

  // Targetdata stuff
  virtual actor_target_data_t* get_target_data( player_t* /* target */ ) const
  { return nullptr; }

  // Opportunity to perform any stat fixups before analysis
  virtual void pre_analyze_hook() {}

  /* New stuff */
  virtual double composite_player_vulnerability( school_e ) const;

  virtual void activate_action_list( action_priority_list_t* a, bool off_gcd = false );

  virtual void analyze( sim_t& );

  scaling_metric_data_t scaling_for_metric( scale_metric_e metric ) const;

  void change_position( position_e );
  position_e position() const
  { return current.position; }

  virtual action_t* create_proc_action( const std::string& /* name */, const special_effect_t& /* effect */ )
  { return nullptr; }
  virtual bool requires_data_collection() const
  { return active_during_iteration; }

  rng::rng_t& rng() { return sim -> rng(); }
  rng::rng_t& rng() const { return sim -> rng(); }
  std::vector<action_variable_t> variables;
  // Add 1ms of time to ensure that we finish this run. This is necessary due
  // to the millisecond accuracy in our timing system.
  virtual timespan_t time_to_move() const
  {
    if ( current.distance_to_move > 0 || current.moving_away > 0 )
      return timespan_t::from_seconds( ( current.distance_to_move + current.moving_away ) / composite_movement_speed() + 0.001 );
    else
      return timespan_t::zero();
  }

  virtual void trigger_movement( double distance, movement_direction_e direction )
  {
    // Distance of 0 disables movement
    if ( distance == 0 )
      do_update_movement( 9999 );
    else
    {
      if ( direction == MOVEMENT_BOOMERANG )
        current.moving_away = distance;
      else
        current.distance_to_move = distance;
      current.movement_direction = direction;
      buffs.raid_movement -> trigger();
    }
  }

  virtual void update_movement( timespan_t duration )
  {
    // Presume stunned players don't move
    if ( buffs.stunned -> check() )
      return;

    double yards = duration.total_seconds() * composite_movement_speed();
    do_update_movement( yards );

    if ( sim -> debug )
      sim -> out_debug.printf( "Player %s movement, direction=%s speed=%f distance_covered=%f to_go=%f duration=%f",
          name(),
          util::movement_direction_string( movement_direction() ),
          composite_movement_speed(),
          yards,
          current.distance_to_move,
          duration.total_seconds() );
  }

  // Instant teleport. No overshooting support for now.
  virtual void teleport( double yards, timespan_t duration = timespan_t::zero() )
  {
    do_update_movement( yards );

    if ( sim -> debug )
      sim -> out_debug.printf( "Player %s warp, direction=%s speed=LIGHTSPEED! distance_covered=%f to_go=%f",
          name(),
          util::movement_direction_string( movement_direction() ),
          yards,
          current.distance_to_move );
    (void) duration;
  }

  virtual movement_direction_e movement_direction() const
  { return current.movement_direction; }

  std::vector<std::string> action_map;

  size_t get_action_id( const std::string& name )
  {
    for ( size_t i = 0; i < action_map.size(); ++i )
    {
      if ( util::str_compare_ci( name, action_map[ i ] ) )
        return i;
    }

    action_map.push_back( name );
    return action_map.size() - 1;
  }

  int find_action_id( const std::string& name )
  {
    for ( size_t i = 0; i < action_map.size(); i++ )
    {
      if ( util::str_compare_ci( name, action_map[ i ] ) )
        return static_cast<int>(i);
    }

    return -1;
  }
private:
  std::vector<unsigned> active_dots;
public:
  void add_active_dot( unsigned action_id )
  {
    if ( active_dots.size() < action_id + 1 )
      active_dots.resize( action_id + 1 );

    active_dots[ action_id ]++;
    if ( sim -> debug )
      sim -> out_debug.printf( "%s Increasing %s dot count to %u", name(), action_map[ action_id ].c_str(), active_dots[ action_id ] );
  }

  void remove_active_dot( unsigned action_id )
  {
    assert( active_dots.size() > action_id );
    assert( active_dots[ action_id ] > 0 );

    active_dots[ action_id ]--;
    if ( sim -> debug )
      sim -> out_debug.printf( "%s Decreasing %s dot count to %u", name(), action_map[ action_id ].c_str(), active_dots[ action_id ] );
  }

  unsigned get_active_dots( unsigned action_id ) const
  {
    if ( active_dots.size() <= action_id )
      return 0;

    return active_dots[ action_id ];
  }

private:
  // Update movement data, and also control the buff
  void do_update_movement( double yards )
  {
    if ( ( yards >= current.distance_to_move ) && current.moving_away <= 0 )
    {
      //x_position += current.distance_to_move; Maybe in wonderland we can track this type of player movement. 
      current.distance_to_move = 0;
      current.movement_direction = MOVEMENT_NONE;
      buffs.raid_movement -> expire();
    }
    else
    {
      if ( current.moving_away > 0 )
      {
        //x_position -= yards;
        current.moving_away -= yards;
        current.distance_to_move += yards;
      }
      else
      {
        //x_position += yards;
        current.moving_away = 0;
        current.distance_to_move -= yards;
      }
    }
  }
public:

  // Static (default), Dynamic, Disabled
  regen_type_e regen_type;

  // Last iteration time regenration occurred. Set at player_t::arise()
  timespan_t last_regen;

  // A list of CACHE_x enumerations (stats) that affect the resource
  // regeneration of the actor.
  std::vector<bool> regen_caches;

  // Flag to indicate if any pets require dynamic regneration. Initialized in
  // player_t::init().
  bool dynamic_regen_pets;

  // Perform dynamic resource regeneration
  virtual void do_dynamic_regen();

  // Visited action lists, needed for call_action_list support. Reset by
  // player_t::execute_action().
  uint64_t visited_apls_;

  // Internal counter for action priority lists, used to set
  // action_priority_list_t::internal_id for lists.
  unsigned action_list_id_;

  // Figure out another actor, by name. Prioritizes pets > harmful targets >
  // other players. Used by "actor.<name>" expression currently.
  virtual player_t* actor_by_name_str( const std::string& ) const;

  // Wicked resource threshold trigger-ready stuff .. work in progress
  event_t* resource_threshold_trigger;
  std::vector<double> resource_thresholds;
  void min_threshold_trigger();

  // Figure out if healing should be recorded
  bool record_healing() const
  { return role == ROLE_TANK || role == ROLE_HEAL || sim -> enable_dps_healing; }
};

// Target Specific ==========================================================

template < class T >
struct target_specific_t
{
  bool owner_;
public:
  target_specific_t( bool owner = true ) : owner_( owner )
  { }

  T*& operator[](  const player_t* target ) const
  {
    assert( target );
    if ( data.empty() )
    {
      data.resize( target -> sim -> actor_list.size() );
    }
    return data[ target -> actor_index ];
  }
  ~target_specific_t()
  {
    if ( owner_ )
      range::dispose( data );
  }
private:
  mutable std::vector<T*> data;
};

struct player_event_t : public event_t
{
  player_t* _player;
  player_event_t( player_t& p ) :
    event_t( p ),
    _player( &p ){}
  player_t* p()
  { return player(); }
  player_t* player()
  { return _player; }
  virtual const char* name() const override
  { return "event_t"; }
};

/* Event which will demise the player
 * - Reason for it are that we need to finish the current action ( eg. a dot tick ) without
 * killing off target dependent things ( eg. dot state ).
 */
struct player_demise_event_t : public player_event_t
{
  player_demise_event_t( player_t& p, timespan_t delta_time = timespan_t::zero() /* Instantly kill the player */ ) :
     player_event_t( p )
  {
    if ( sim().debug )
      sim().out_debug.printf( "New Player-Demise Event: %s", p.name() );

    add_event( delta_time );
  }
  virtual const char* name() const override
  { return "Player-Demise"; }
  virtual void execute() override
  {
    p() -> demise();
  }
};

// Pet ======================================================================

struct pet_t : public player_t
{
  typedef player_t base_t;

  std::string full_name_str;
  player_t* const owner;
  double stamina_per_owner;
  double intellect_per_owner;
  bool summoned;
  bool dynamic;
  pet_e pet_type;
  event_t* expiration;
  timespan_t duration;
  bool affects_wod_legendary_ring;

  struct owner_coefficients_t
  {
    double armor, health, ap_from_ap, ap_from_sp, sp_from_ap, sp_from_sp;
    owner_coefficients_t();
  } owner_coeff;

private:
  void init_pet_t_();
public:
  pet_t( sim_t* sim, player_t* owner, const std::string& name, bool guardian = false, bool dynamic = false );
  pet_t( sim_t* sim, player_t* owner, const std::string& name, pet_e pt, bool guardian = false, bool dynamic = false );

  virtual void init() override;
  virtual void init_base_stats() override;
  virtual void init_target() override;
  virtual void reset() override;
  virtual void summon( timespan_t duration = timespan_t::zero() );
  virtual void dismiss( bool expired = false );
  virtual void assess_damage( school_e, dmg_e, action_state_t* s ) override;
  virtual void combat_begin() override;

  virtual const char* name() const override { return full_name_str.c_str(); }
  virtual const player_t* get_owner_or_self() const override
  { return owner; }

  const spell_data_t* find_pet_spell( const std::string& name, const std::string& token = std::string() );

  virtual double composite_attribute( attribute_e attr ) const override;
  virtual double composite_player_multiplier( school_e ) const override;

  // new pet scaling by Ghostcrawler, see http://us.battle.net/wow/en/forum/topic/5889309137?page=49#977
  // http://us.battle.net/wow/en/forum/topic/5889309137?page=58#1143

  double hit_exp() const;
  
  virtual double composite_player_multistrike_damage_multiplier() const override
  { return owner -> composite_player_multistrike_damage_multiplier(); }
  virtual double composite_player_multistrike_healing_multiplier() const override
  { return owner -> composite_player_multistrike_healing_multiplier(); }

  virtual double composite_movement_speed() const override
  { return owner -> composite_movement_speed(); }

  virtual double composite_melee_expertise( const weapon_t* ) const override
  { return hit_exp(); }
  virtual double composite_melee_hit() const override
  { return hit_exp(); }
  virtual double composite_spell_hit() const override
  { return hit_exp() * 2.0; }

  double pet_crit() const;

  virtual double composite_melee_crit() const override
  { return pet_crit(); }
  virtual double composite_spell_crit() const override
  { return pet_crit(); }

  virtual double composite_melee_speed() const override
  { return owner -> cache.attack_speed(); }

  virtual double composite_melee_haste() const override
  { return owner -> cache.attack_haste(); }

  virtual double composite_spell_haste() const override
  { return owner -> cache.spell_haste(); }

  virtual double composite_spell_speed() const override
  { return owner -> cache.spell_speed(); }

  virtual double composite_multistrike() const override
  { return owner -> cache.multistrike(); }

  virtual double composite_readiness() const override
  { return owner -> cache.readiness(); }

  virtual double composite_bonus_armor() const override
  { return owner -> cache.bonus_armor(); }

  virtual double composite_damage_versatility() const override
  { return owner -> cache.damage_versatility(); }

  virtual double composite_heal_versatility() const override
  { return owner -> cache.heal_versatility(); }

  virtual double composite_mitigation_versatility() const override
  { return owner -> cache.mitigation_versatility(); }

  virtual double composite_melee_attack_power() const override;

  virtual double composite_spell_power( school_e school ) const override;

  // Assuming diminishing returns are transfered to the pet as well
  virtual double composite_dodge() const override
  { return owner -> cache.dodge(); }

  virtual double composite_parry() const override
  { return owner -> cache.parry(); }

  // Influenced by coefficients [ 0, 1 ]
  virtual double composite_armor() const override
  { return owner -> cache.armor() * owner_coeff.armor; }

  virtual void init_resources( bool force ) override;
  virtual bool requires_data_collection() const override
  { return active_during_iteration || ( dynamic && sim -> report_pets_separately == 1 ); }
};


// Gain =====================================================================

struct gain_t : private noncopyable
{
public:
  std::array<double, RESOURCE_MAX> actual, overflow, count;
  const std::string name_str;

  gain_t( const std::string& n ) :
    actual(),
    overflow(),
    count(),
    name_str( n )
  { }
  void add( resource_e rt, double amount, double overflow_ = 0.0 )
  { actual[ rt ] += amount; overflow[ rt ] += overflow_; count[ rt ]++; }
  void merge( const gain_t& other )
  {
    for ( resource_e i = RESOURCE_NONE; i < RESOURCE_MAX; i++ )
    { actual[ i ] += other.actual[ i ]; overflow[ i ] += other.overflow[ i ]; count[ i ] += other.count[ i ]; }
  }
  void analyze( const sim_t& sim )
  {
    for ( resource_e i = RESOURCE_NONE; i < RESOURCE_MAX; i++ )
    { actual[ i ] /= sim.iterations; overflow[ i ] /= sim.iterations; count[ i ] /= sim.iterations; }
  }
  const char* name() const { return name_str.c_str(); }
};

// Stats ====================================================================

struct stats_t : private noncopyable
{
private:
  sim_t& sim;
public:
  const std::string name_str;
  player_t* player;
  stats_t* parent;
  // We should make school and type const or const-like, and either stricly define when, where and who defines the values,
  // or make sure that it is equal to the value of all it's actions.
  school_e school;
  stats_e type;

  std::vector<action_t*> action_list;
  gain_t resource_gain;
  // Flags
  bool analyzed;
  bool quiet;
  bool background;

  simple_sample_data_t num_executes, num_ticks, num_refreshes, num_direct_results, num_tick_results;
  unsigned int iteration_num_executes, iteration_num_ticks, iteration_num_refreshes;
  // Variables used both during combat and for reporting
  simple_sample_data_t total_execute_time, total_tick_time;
  timespan_t iteration_total_execute_time, iteration_total_tick_time;
  double portion_amount;
  simple_sample_data_t total_intervals;
  timespan_t last_execute;
  extended_sample_data_t actual_amount, total_amount, portion_aps, portion_apse;
  std::vector<stats_t*> children;

  struct stats_results_t
  {
  public:
    simple_sample_data_with_min_max_t actual_amount, avg_actual_amount;
    simple_sample_data_t total_amount, fight_actual_amount, fight_total_amount, overkill_pct;
    simple_sample_data_t count;
    double pct;
  private:
    int iteration_count;
    double iteration_actual_amount, iteration_total_amount;
    friend struct stats_t;
  public:

    stats_results_t();
    void analyze( double num_results );
    void merge( const stats_results_t& other );
    void datacollection_begin();
    void datacollection_end();
  };
  std::array<stats_results_t,RESULT_MAX> direct_results;
  std::array<stats_results_t,FULLTYPE_MAX> direct_results_detail;
  std::array<stats_results_t,RESULT_MAX> tick_results;
  std::array<stats_results_t,FULLTYPE_MAX> tick_results_detail;

  sc_timeline_t timeline_amount;

  // Reporting only
  std::array<double, RESOURCE_MAX> resource_portion, apr, rpe;
  double rpe_sum, compound_amount, overkill_pct;
  double aps, ape, apet, etpe, ttpt;
  timespan_t total_time;
  std::string aps_distribution_chart;

  std::string timeline_aps_chart;

  struct stats_scaling_t {
    gear_stats_t value;
    gear_stats_t error;
  };
  std::unique_ptr<stats_scaling_t> scaling;

  stats_t( const std::string& name, player_t* );

  void add_child( stats_t* child );
  void consume_resource( resource_e resource_type, double resource_amount );
  full_result_e translate_result( result_e result, block_result_e block_result );
  void add_result( double act_amount, double tot_amount, dmg_e dmg_type, result_e result, block_result_e block_result, player_t* target );
  void add_execute( timespan_t time, player_t* target );
  void add_tick   ( timespan_t time, player_t* target );
  void add_refresh( player_t* target );
  void datacollection_begin();
  void datacollection_end();
  void reset();
  void analyze();
  void merge( const stats_t& other );
  const char* name() const { return name_str.c_str(); }

  bool has_direct_amount_results() const;
  bool has_tick_amount_results() const;
};

struct action_state_t : private noncopyable
{
  action_state_t* next;
  // Source action, target actor
  action_t*       action;
  player_t*       target;
  // Execution attributes
  size_t          n_targets;            // Total number of targets the execution hits.
  int             chain_target;         // The chain target number, 0 == no chain, 1 == first target, etc.
  double original_x;
  double original_y;
  // Execution results
  dmg_e           result_type;
  result_e        result;
  block_result_e  block_result;
  double          result_raw;             // Base result value, without crit/glance etc.
  double          result_total;           // Total unmitigated result, including crit bonus, glance penalty, etc.
  double          result_mitigated;       // Result after mitigation / resist. *NOTENOTENOTE* Only filled after action_t::impact() call
  double          result_absorbed;        // Result after absorption. *NOTENOTENOTE* Only filled after action_t::impact() call
  double          result_amount;          // Final (actual) result
  double          blocked_amount;         // The amount of damage reduced via block or critical block
  double          self_absorb_amount;     // The amount of damage reduced via personal absorbs such as shield_barrier.
  // Snapshotted stats during execution
  double          haste;
  double          crit;
  double          target_crit;
  double          attack_power;
  double          spell_power;
  double          multistrike;
  // Snapshotted multipliers
  double          resolve;
  double          versatility;
  double          da_multiplier;
  double          ta_multiplier;
  double          persistent_multiplier;
  double          target_da_multiplier;
  double          target_ta_multiplier;
  // Target mitigation multipliers
  double          target_mitigation_da_multiplier;
  double          target_mitigation_ta_multiplier;
  double          target_armor;

  static void release( action_state_t*& s );
  static std::string flags_to_str( unsigned flags );

  action_state_t( action_t*, player_t* );
  virtual ~action_state_t() {}

  virtual void copy_state( const action_state_t* );
  virtual void initialize();

  virtual std::ostringstream& debug_str( std::ostringstream& debug_str );
  virtual void debug();

  virtual double composite_crit() const
  { return crit + target_crit; }

  virtual double composite_attack_power() const
  { return attack_power; }

  virtual double composite_spell_power() const
  { return spell_power; }

  virtual double composite_versatility() const
  { return versatility; }

  virtual double composite_da_multiplier() const
  { return da_multiplier * persistent_multiplier * target_da_multiplier * versatility * resolve; }

  virtual double composite_ta_multiplier() const
  { return ta_multiplier * persistent_multiplier * target_ta_multiplier * versatility * resolve; }

  virtual double composite_target_mitigation_da_multiplier() const
  { return target_mitigation_da_multiplier; }

  virtual double composite_target_mitigation_ta_multiplier() const
  { return target_mitigation_ta_multiplier; }

  virtual double composite_target_armor() const
  { return target_armor; }

  // Inlined
  virtual proc_types proc_type() const;
  virtual proc_types2 execute_proc_type2() const;

  // Secondary proc type of the impact event (i.e., assess_damage()). Only
  // triggers the "amount" procs
  virtual proc_types2 impact_proc_type2() const
  {
    // Don't allow impact procs that do not do damage or heal anyone; they
    // should all be handled by execute_proc_type2(). Note that this is based
    // on the _total_ amount done. This is so that fully overhealed heals are
    // still alowed to proc things.
    if ( result_total <= 0 )
      return PROC2_INVALID;

    if ( result == RESULT_HIT )
      return PROC2_HIT;
    else if ( result == RESULT_CRIT )
      return PROC2_CRIT;
    else if ( result == RESULT_GLANCE )
      return PROC2_GLANCE;
    // Multistrike can only generate procs on impact, though this could be
    // moved to execute_proc_type2() too.
    else if ( result == RESULT_MULTISTRIKE )
      return PROC2_MULTISTRIKE;
    else if ( result == RESULT_MULTISTRIKE_CRIT )
      return PROC2_MULTISTRIKE_CRIT;

    return PROC2_INVALID;
  }

  virtual proc_types2 cast_proc_type2() const;
};

// Action ===================================================================

struct action_t : private noncopyable
{
public:
  const spell_data_t* s_data;
  sim_t* const sim;
  const action_e type;
  std::string name_str;
  player_t* const player;

  player_t* target;
  // Item back-pointer for trinket-sourced actions so we can show proper tooltips in reports
  const item_t* item;

  /**
   * Default target is needed, otherwise there's a chance that cycle_targets
   * option will _MAJORLY_ mess up the action list for the actor, as there's no
   * guarantee cycle_targets will end up on the "initial target" when an
   * iteration ends.
   */
  player_t* default_target;

  /**
   * Target Cache System
   * - list: contains the cached target pointers
   * - callback: unique_Ptr to callback
   * - is_valid: gets invalidated by the callback from the target list source.
   *  When the target list is requested in action_t::target_list(), it gets recalculated if
   *  flag is false, otherwise cached version is used
   */
  struct target_cache_t {
    std::vector< player_t* > list;
    bool is_valid;
    target_cache_t() : is_valid( false ) {}
  } mutable target_cache;

  enum target_if_mode_e
  {
    TARGET_IF_NONE,
    TARGET_IF_FIRST,
    TARGET_IF_MIN,
    TARGET_IF_MAX
  };

  /// What type of damage this spell does.
  school_e school;

  /// Spell id if available, 0 otherwise
  unsigned id;

  /**
   * @brief player & action-name unique id.
   *
   * Every action -- even actions without spelldata -- is given an internal_id
   */
  unsigned internal_id;

  /// What resource does this ability use.
  resource_e resource_current;

  /// The amount of targets that an ability impacts on. -1 will hit all targets.
  int aoe;

  /**
   * @brief If the ability can proc multistrikes.
   *
   * 0 disables multistrikes, 1 enables, and -1 enables/disables multistrikes based on if the ability can crit or not.
   */
  int may_multistrike;

  /**
   * @brief Does Multistrike occur instantly?
   *
   * -1 = autodetect (NYI), 0 = multistrikes have a delay, 1 = multistrikes occur immediately
   */
  int instant_multistrike;

  /// If set to true, this action will not be counted toward total amount of executes in reporting. Useful for abilities with parent/children attacks.
  bool dual;

  /// enables/disables proc callback system on the action, like trinkets, enchants, rppm.
  bool callbacks;

  /// Whether or not the spell uses the yellow attack hit table.
  bool special;

  /// Tells the sim to not perform any other actions, as the ability is channeled.
  bool channeled;

  /// mark this as a sequence_t action
  bool sequence;

  /**
   * @brief Disables/enables reporting of this action.
   *
   * When set to true, action will not show up in raid report or count towards executes.
   */
  bool quiet;

  /**
   * @brief Enables/Disables direct execution of the ability in an action list.
   *
   * Background actions cannot be executed via action list, but can be triggered by other actions.
   * Background actions do not count for executes.
   * Abilities with background = true can still be called upon by other actions,
   * example being deep wounds and how it is activated by devastate.
   */
  bool background;

  /**
   * @brief Check if action is executable between GCD's
   *
   * When set to true, will check every 100 ms to see if this action needs to be used,
   * rather than waiting until the next gcd.
   * Slows simulation down significantly.
   */
  bool use_off_gcd;

  /// true if channeled action does not reschedule autoattacks, used on abilities such as bladestorm.
  bool interrupt_auto_attack;

  /// Used for actions that will do awful things to the sim when a "false positive" skill roll happens.
  bool ignore_false_positive;

  /// Skill is now done per ability, with the default being set to the player option.
  double action_skill;

  /// Used with DoT Drivers, tells simc that the direct hit is actually a tick.
  bool direct_tick;

  /// Used for abilities that repeat themselves without user interaction, only used on autoattacks.
  bool repeating;

  /**
   * Simplified: Will the ability pull the boss if used.
   * Also determines whether ability can be used precombat without counting towards the 1 harmful spell limit
   */
  bool harmful;

  /**
   * @brief Whether or not this ability is a proc.
   *
   * Procs do not consume resources.
   */
  bool proc;

  /// Is the action initialized? (action_t::init ran successfully)
  bool initialized;

  /// Self explanatory.
  bool may_hit, may_miss, may_dodge, may_parry, may_glance, may_block, may_crush, may_crit, tick_may_crit;

  /// Whether or not the ability/dot ticks immediately on usage.
  bool tick_zero;

  /**
   * @brief Whether or not ticks scale with haste.
   *
   * Generally only used for bleeds that do not scale with haste,
   * or with ability drivers that deal damage every x number of seconds.
   */
  bool hasted_ticks;

  /**
   * @brief Behavior of dot.
   *
   * Acceptable inputs are DOT_CLIP, DOT_REFRESH, and DOT_EXTEND.
   */
  dot_behavior_e dot_behavior;

  /// Ability specific extra player ready delay
  timespan_t ability_lag, ability_lag_stddev;

  /// Deathknight specific, how much runic power is gained.
  double rp_gain;

  /// The minimum gcd triggered no matter the haste.
  timespan_t min_gcd;

  /// Length of unhasted gcd triggered when used.
  timespan_t trigger_gcd;

  /// This is how far away the target can be from the player, and still be hit or targeted.
  double range;

  /**
   * @brief AoE ability area of influence.
   *
   * Typically anything that has a radius, but not a range, is based on the players location.
   */
  double radius;

  /// Weapon AP multiplier.
  double weapon_power_mod;

  /// Attack/Spell power scaling of the ability.
  struct {
  double direct, tick;
  } attack_power_mod, spell_power_mod;

  /**
   * @brief full damage variation in %
   *
   * Amount of variation of the full raw damage (base damage + AP/SP modified).
   * Example: amount_delta = 0.1 means variation between 95% and 105%.
   * Parsed from spell data.
   */
  double amount_delta;

  /// Amount of time the ability uses to execute before modifiers.
  timespan_t base_execute_time;

  /// Amount of time the ability uses between ticks.
  timespan_t base_tick_time;

  /// Default full duration of dot.
  timespan_t dot_duration;

  /// Cost of using the ability.
  std::array< double, RESOURCE_MAX > base_costs;

  /// Cost of using ability per second.
  std::array< double, RESOURCE_MAX > base_costs_per_second;

  /// Minimum base direct damage
  double base_dd_min;


  /// Maximum base direct damage
  double base_dd_max;

  /// Base tick damage
  double base_td;

  /// base direct damage multiplier
  double base_dd_multiplier;

  /// base tick damage multiplier
  double base_td_multiplier;

  /// base damage multiplier (direct and tick damage)
  double base_multiplier;
  double base_hit, base_crit;
  double crit_multiplier, crit_bonus_multiplier, crit_bonus;
  double base_dd_adder;
  double base_ta_adder;

  /// Weapon used for this ability. If set extra weapon damage is calculated.
  weapon_t* weapon;

  /// Weapon damage for the ability.
  double weapon_multiplier;

  /// Damage reduction for chained/AoE, exponentially increased by number of target hit.
  double base_add_multiplier;

  /// Static reduction of damage for AoE
  double base_aoe_multiplier;

  /// Split damage evenly between targets
  bool split_aoe_damage;

  /**
   * @brief Normalize weapon speed for weapon damage calculations
   *
   * \sa weapon_t::get_normalized_speed()
   */
  bool normalize_weapon_speed;

  /// Static action cooldown duration multiplier
  double base_cooldown_reduction;

  /**
   * @brief Movement Direction
   * @code
   * movement_directionality = MOVEMENT_OMNI; // Can move in any direction, ex: Heroic Leap, Blink. Generally set movement skills to this.
   * movement_directionality = MOVEMENT_TOWARDS; // Can only be used towards enemy target. ex: Charge
   * movement_directionality = MOVEMENT_AWAY; // Can only be used away from target. Ex: ????
   * @endcode
   */
  movement_direction_e movement_directionality;

  /// Maximum distance that the ability can travel. Used on abilities that instantly move you, or nearly instantly move you to a location.
  double base_teleport_distance;

  /// This is used to cache/track spells that have a parent driver, such as most channeled/ground aoe abilities.
  dot_t* parent_dot;

  /// This ability leaves a ticking dot on the ground, and doesn't move when the target moves. Used with original_x and original_y
  bool ground_aoe;

  /// Missile travel speed in yards / second
  double travel_speed;

  /// Round spell base damage to integer before using
  bool round_base_dmg;

  /// Used with tick_action, tells tick_action to update state on every tick.
  bool dynamic_tick_action;

  /// This action will execute every tick
  action_t* tick_action;

  /// This action will execute every execute
  action_t* execute_action;

  /// This action will execute every impact - Useful for AoE debuffs
  action_t* impact_action;

  /**
   * @brief Used to manipulate cooldown duration and charges.
   *
   * @code
   * cooldown -> duration = timespan_t::from_seconds( 20 ); //Ability has a cooldown of 20 seconds.
   * cooldown -> charges = 3; // Ability has 3 charges.
   * @endcode
   */
  cooldown_t* cooldown;

  /// action statistics, merged by action-name
  stats_t* stats;
  event_t* execute_event;
  action_cost_tick_event_t* cost_tick_event;
  timespan_t time_to_execute, time_to_travel;
  double resource_consumed;
  timespan_t last_reaction_time;

  /// Marker for sample action priority list reporting
  char marker;
  // options
  int moving, wait_on_ready, interrupt, chain, cycle_targets, cycle_players, max_cycle_targets, target_number;
  std::string if_expr_str;
  expr_t* if_expr;
  std::string target_if_str;
  target_if_mode_e target_if_mode;
  expr_t* target_if_expr;
  std::string interrupt_if_expr_str;
  std::string early_chain_if_expr_str;
  expr_t* interrupt_if_expr;
  expr_t* early_chain_if_expr;
  std::string sync_str;
  action_t* sync_action;
  std::string signature_str;
  std::string target_str;
  target_specific_t<dot_t> target_specific_dot;
  action_priority_list_t* action_list;

  /**
   * @brief Resource starvation tracking.
   *
   * Tracking proc triggered on resource-starved ready() calls.
   * Can be overridden by class modules for tracking purposes.
   */
  proc_t* starved_proc;
  uint_least64_t total_executions;

  /**
   * @brief Cooldown for specific APL line.
   *
   * Tied to a action_t object, and not shared by action-name,
   * this cooldown helps articifally restricting usage of a specific line
   * in the APL.
   */
  cooldown_t line_cooldown;
  const action_priority_t* signature;
  std::vector<std::unique_ptr<option_t>> options;

  action_state_t* state_cache;

  /// State of the last execute()
  action_state_t* execute_state;

  /// Optional - if defined before execute(), will be copied into execute_state
  action_state_t* pre_execute_state;
  unsigned snapshot_flags;
  unsigned update_flags;
private:
  std::vector<travel_event_t*> travel_events;

public:
  action_t( action_e type, const std::string& token, player_t* p, const spell_data_t* s = spell_data_t::nil() );

  virtual ~action_t();

  player_t* select_target_if_target();

  /**
   * @brief Spell Data associated with the action.
   *
   * spell_data_t::nil() if no spell data is available,
   * spell_data_t::not_found if spell given was not found.
   *
   * This means that if no spell data is available/found (eg. talent not available),
   * all spell data fields will be filled with 0 and can thus be used directly
   * without checking specifically for the spell_data_t::ok()
   */
  const spell_data_t& data() const
  { return ( *s_data ); }
  void parse_spell_data( const spell_data_t& );
  void parse_effect_data( const spelleffect_data_t& );
  virtual void parse_options( const std::string& options_str );
  void add_option( std::unique_ptr<option_t> new_option )
  { options.insert( options.begin(), std::move(new_option) ); }
  void   check_spec( specialization_e );
  void   check_spell( const spell_data_t* );
  dot_t* get_dot( player_t* target = nullptr );
  dot_t* find_dot( player_t* target ) const;
  void add_child( action_t* child );
  void add_travel_event( travel_event_t* e )
  { travel_events.push_back( e ); }

  void remove_travel_event( travel_event_t* e );

  rng::rng_t& rng()
  { return sim -> rng(); }

  rng::rng_t& rng() const
  { return sim -> rng(); }

  // =======================
  // Const virtual functions
  // =======================
  virtual double cost() const;

  virtual double cost_per_second( resource_e ) const;

  virtual timespan_t gcd() const;

  virtual double false_positive_pct() const;

  virtual double false_negative_pct() const;

  virtual timespan_t execute_time() const
  { return base_execute_time; }

  virtual timespan_t tick_time( double haste ) const;

  virtual timespan_t travel_time() const;

  virtual timespan_t distance_targeting_travel_time( action_state_t* s ) const;

  virtual result_e calculate_result( action_state_t* /* state */ ) const
  {
    assert( false );
    return RESULT_UNKNOWN;
  }

  virtual result_e calculate_multistrike_result( action_state_t* /* state */, dmg_e /* type */ ) const;
  virtual block_result_e calculate_block_result( action_state_t* s ) const;
  virtual double calculate_direct_amount( action_state_t* state ) const;

  virtual double calculate_tick_amount( action_state_t* state, double multiplier ) const;

  virtual double calculate_weapon_damage( double attack_power ) const;

  virtual double target_armor( player_t* t ) const
  { return t -> cache.armor(); }

  virtual double cooldown_reduction() const
  { return base_cooldown_reduction; }

  virtual resource_e current_resource() const
  { return resource_current; }

  virtual int n_targets() const
  { return aoe; }

  bool is_aoe() const
  { return n_targets() == -1 || n_targets() > 0; }

  virtual double last_tick_factor( const dot_t* d, const timespan_t& time_to_tick, const timespan_t& duration ) const;

  virtual dmg_e amount_type( const action_state_t* /* state */, bool /* periodic */ = false ) const
  { return RESULT_TYPE_NONE; }

  virtual dmg_e report_amount_type( const action_state_t* state ) const
  { return state -> result_type; }

  const char* name() const
  { return name_str.c_str(); }

  /// Use when damage schools change during runtime.
  virtual school_e get_school() const
  { return school; }

  virtual double miss_chance( double /* hit */, player_t* /* target */ ) const
  { return 0; }

  virtual double dodge_chance( double /* expertise */, player_t* /* target */ ) const
  { return 0; }

  virtual double parry_chance( double /* expertise */, player_t* /* target */ ) const
  { return 0; }

  virtual double glance_chance( int /* delta_level */ ) const
  { return 0; }

  virtual double block_chance( action_state_t* /* state */ ) const
  { return 0; }

  virtual double crit_block_chance( action_state_t* /* state */  ) const
  { return 0; }

  virtual double total_crit_bonus() const; // Check if we want to move this into the stateless system.

  virtual int num_targets() const;

  virtual size_t available_targets( std::vector< player_t* >& ) const;

  virtual std::vector< player_t* >& target_list() const;

  virtual player_t* find_target_by_number( int number ) const;

  virtual bool execute_targeting( action_t* action ) const;

  virtual bool impact_targeting( action_state_t* s ) const;

  virtual std::vector<player_t*> targets_in_range_list( std::vector< player_t* >& tl ) const;

  virtual std::vector<player_t*> check_distance_targeting( std::vector< player_t* >& tl ) const;

  virtual double ppm_proc_chance( double PPM ) const;

  virtual bool usable_moving() const;

  virtual double composite_multistrike_multiplier( const action_state_t* t ) const
  { return ( t -> result_type == DMG_DIRECT || t -> result_type == DMG_OVER_TIME )
      ? player -> composite_player_multistrike_damage_multiplier()
      : player -> composite_player_multistrike_healing_multiplier();
  }

  virtual timespan_t composite_dot_duration( const action_state_t* ) const;

  virtual double attack_direct_power_coefficient( const action_state_t* ) const
  { return attack_power_mod.direct; }

  virtual double amount_delta_modifier( const action_state_t* ) const
  { return amount_delta; }

  virtual double attack_tick_power_coefficient( const action_state_t* ) const
  { return attack_power_mod.tick; }

  virtual double spell_direct_power_coefficient( const action_state_t* ) const
  { return spell_power_mod.direct; }

  virtual double spell_tick_power_coefficient( const action_state_t* ) const
  { return spell_power_mod.tick; }

  virtual double base_da_min( const action_state_t* ) const
  { return base_dd_min; }

  virtual double base_da_max( const action_state_t* ) const
  { return base_dd_max; }

  virtual double base_ta( const action_state_t* ) const
  { return base_td; }

  virtual double bonus_da( const action_state_t* ) const
  { return base_dd_adder; }

  virtual double bonus_ta( const action_state_t* ) const
  { return base_ta_adder; }

  virtual double range_() const
  { return range; }

  virtual double radius_() const
  { return radius; }

  virtual double action_multiplier() const
  { return base_multiplier; }

  virtual double action_da_multiplier() const
  { return base_dd_multiplier; }

  virtual double action_ta_multiplier() const
  { return base_td_multiplier; }

  virtual double composite_hit() const
  { return base_hit; }

  virtual double composite_crit() const
  { return base_crit; }

  virtual double composite_crit_multiplier() const
  { return 1.0; }

  virtual double composite_haste() const
  { return 1.0; }

  virtual double composite_attack_power() const
  { return player -> cache.attack_power(); }

  virtual double composite_spell_power() const
  { return player -> cache.spell_power( get_school() ); }

  virtual double composite_target_crit( player_t* /* target */ ) const;

  virtual double composite_target_multiplier( player_t* target ) const
  {
    double m = target -> composite_player_vulnerability( get_school() );
    if ( target -> race == RACE_DEMON &&
         player -> buffs.demon_damage_buff &&
         player -> buffs.demon_damage_buff -> up() )
    {
      // Bad idea to hardcode the effect number, but it'll work for now. The buffs themselves are
      // stat buffs.
      m *= 1.0 + player -> buffs.demon_damage_buff -> data().effectN( 2 ).percent();
    }

    return m;
  }

  virtual double composite_multistrike() const
  { return player -> cache.multistrike(); }

  virtual double composite_readiness() const
  { return player -> cache.readiness(); }

  virtual double composite_versatility( const action_state_t* ) const
  { return 1.0; }

  virtual double composite_resolve( const action_state_t* ) const
  { return 1.0; }

  virtual double composite_leech( const action_state_t* ) const
  { return player -> cache.leech(); }

  virtual double composite_run_speed() const
  { return player -> cache.run_speed(); }

  virtual double composite_avoidance() const
  { return player -> cache.avoidance(); }

  /// Direct amount multiplier due to debuffs on the target
  virtual double composite_target_da_multiplier( player_t* target ) const
  { return composite_target_multiplier( target ); }

  /// Tick amount multiplier due to debuffs on the target
  virtual double composite_target_ta_multiplier( player_t* target ) const
  { return composite_target_multiplier( target ); }

  virtual double composite_da_multiplier( const action_state_t* s ) const
  {
    return action_multiplier() * action_da_multiplier() *
           player -> cache.player_multiplier( s -> action -> get_school() ) *
           player -> composite_player_dd_multiplier( s -> action -> get_school() , this );
  }

  /// Normal ticking modifiers that are updated every tick
  virtual double composite_ta_multiplier( const action_state_t* s ) const
  {
    return action_multiplier() * action_ta_multiplier() *
           player -> cache.player_multiplier( s -> action -> get_school() ) *
           player -> composite_player_td_multiplier( s -> action -> get_school() , this );
  }

  /// Persistent modifiers that are snapshot at the start of the spell cast
  virtual double composite_persistent_multiplier( const action_state_t* ) const
  { return player -> composite_persistent_multiplier( get_school() ); }

  /**
   * @brief Generic aoe multiplier for the action.
   *
   * Used in action_t::calculate_direct_amount, and applied after
   * other (passive) aoe multipliers are applied.
   */
  virtual double composite_aoe_multiplier( const action_state_t* ) const
  { return 1.0; }

  virtual double composite_target_mitigation( player_t* t, school_e s ) const
  { return t -> composite_mitigation_multiplier( s ); }

  virtual double composite_player_critical_multiplier() const
  { return player -> composite_player_critical_damage_multiplier(); }

  /// Action proc type, needed for dynamic aoe stuff and such.
  virtual proc_types proc_type() const
  { return PROC1_INVALID; }

  bool has_amount_result() const
  {
    return attack_power_mod.direct > 0 || attack_power_mod.tick > 0
        || spell_power_mod.direct > 0 || spell_power_mod.tick > 0
        || (weapon && weapon_multiplier > 0);
  }

  bool has_travel_events() const
  { return ! travel_events.empty(); }

  size_t get_num_travel_events() const
  { return travel_events.size(); }

  bool has_travel_events_for( const player_t* target ) const;

  const std::vector<travel_event_t*>& current_travel_events() const
  { return travel_events; }

  virtual bool need_to_trigger_costs_per_second() const
  {
    return std::accumulate(base_costs_per_second.begin(),
        base_costs_per_second.end(), 0.0) != 0;
  }

  virtual bool has_movement_directionality() const;

  virtual double composite_teleport_distance( const action_state_t* ) const
  { return base_teleport_distance; }

  virtual timespan_t calculate_dot_refresh_duration(const dot_t*,
      timespan_t triggered_duration) const;

  // ==========================
  // mutating virtual functions
  // ==========================

  virtual void consume_resource();

  virtual void execute();

  virtual void tick(dot_t* d);

  virtual void multistrike_tick(const action_state_t* source_state,
      action_state_t* ms_state, double dmg_multiplier = 1.0);

  virtual void multistrike_direct(const action_state_t* state,
      action_state_t* ms_state);

  virtual void last_tick(dot_t* d);

  virtual void update_resolve(dmg_e, action_state_t* assess_state);

  virtual void assess_damage(dmg_e, action_state_t* assess_state);

  virtual void record_data(action_state_t* data);

  virtual int schedule_multistrike(action_state_t* state, dmg_e type,
      double dmg_multiplier = 1.0);

  virtual void schedule_execute(action_state_t* execute_state = nullptr);

  virtual void reschedule_execute(timespan_t time);

  virtual void update_ready(timespan_t cd_duration = timespan_t::min());

  virtual bool ready();

  virtual void init();

  virtual bool init_finished();

  virtual void init_target_cache();

  virtual void reset();

  virtual void cancel();

  virtual void interrupt_action();

  virtual expr_t* create_expression(const std::string& name);

  virtual action_state_t* new_state();

  virtual action_state_t* get_state(const action_state_t* = nullptr);
private:
  friend struct action_state_t;
  virtual void release_state( action_state_t* );
public:
  virtual void do_schedule_travel( action_state_t*, const timespan_t& );

  virtual void schedule_travel( action_state_t* );

  virtual void impact( action_state_t* );

  virtual void trigger_dot( action_state_t* );

  virtual void snapshot_internal( action_state_t*, unsigned flags, dmg_e );

  virtual void snapshot_state( action_state_t* s, dmg_e rt )
  { snapshot_internal( s, snapshot_flags, rt ); }

  virtual void update_state( action_state_t* s, dmg_e rt )
  { snapshot_internal( s, update_flags, rt ); }

  virtual void consolidate_snapshot_flags();

  event_t* start_action_execute_event( timespan_t time, action_state_t* execute_state = nullptr );

  virtual void schedule_cost_tick_event( timespan_t tick_time = timespan_t::from_seconds( 1.0 ) );

  virtual bool consume_cost_per_second( timespan_t tick_time );

  virtual void do_teleport( action_state_t* );


  // ================
  // Static functions
  // ================

  static bool result_is_hit( result_e r )
  {
    return( r == RESULT_HIT        ||
            r == RESULT_CRIT       ||
            r == RESULT_GLANCE     ||
            r == RESULT_NONE       );
  }

  static bool result_is_miss( result_e r )
  {
    return( r == RESULT_MISS   ||
            r == RESULT_DODGE  ||
            r == RESULT_PARRY );
  }

  static bool result_is_multistrike( result_e r )
  {
    return ( r == RESULT_MULTISTRIKE || r == RESULT_MULTISTRIKE_CRIT );
  }

  static bool result_is_hit_or_multistrike( result_e r )
  {
    return result_is_hit( r ) || result_is_multistrike( r );
  }

  static bool result_is_block( block_result_e r )
  {
    return( r == BLOCK_RESULT_BLOCKED || r == BLOCK_RESULT_CRIT_BLOCKED );
  }
};

struct call_action_list_t : public action_t
{
  action_priority_list_t* alist;

  call_action_list_t( player_t*, const std::string& );
  virtual void execute() override
  { assert( 0 ); }
};

// Attack ===================================================================

struct attack_t : public action_t
{
  double base_attack_expertise;
  bool auto_attack;

  attack_t( const std::string& token, player_t* p, const spell_data_t* s = spell_data_t::nil() );

  // Attack Overrides
  virtual timespan_t execute_time() const override;
  virtual void execute() override;
  virtual result_e calculate_result( action_state_t* ) const override;
  virtual void   init() override;

  virtual dmg_e amount_type( const action_state_t* /* state */, bool /* periodic */ = false ) const override;
  virtual dmg_e report_amount_type( const action_state_t* /* state */ ) const override;

  virtual double  miss_chance( double hit, player_t* t ) const override;
  virtual double  dodge_chance( double /* expertise */, player_t* t ) const override;
  virtual double  block_chance( action_state_t* s ) const override;
  virtual double  crit_block_chance( action_state_t* s ) const override;

  virtual double composite_hit() const override
  { return action_t::composite_hit() + player -> cache.attack_hit(); }

  virtual double composite_crit() const override
  { return action_t::composite_crit() + player -> cache.attack_crit(); }

  virtual double composite_crit_multiplier() const override
  { return action_t::composite_crit_multiplier() * player -> composite_melee_crit_multiplier(); }

  virtual double composite_haste() const override
  { return action_t::composite_haste() * player -> cache.attack_haste(); }

  virtual double composite_expertise() const
  { return base_attack_expertise + player -> cache.attack_expertise(); }

  virtual double composite_versatility( const action_state_t* state ) const override
  { return action_t::composite_versatility( state ) + player -> cache.damage_versatility(); }

  virtual void reschedule_auto_attack( double old_swing_haste );

  virtual void reset() override
  {
    attack_table.reset();
    action_t::reset();
  }

private:
  /// attack table generator with caching
  struct attack_table_t{
    std::array<double, RESULT_MAX> chances;
    std::array<result_e, RESULT_MAX> results;
    int num_results;
    double attack_table_sum; // Used to check whether we can use cached values or not.

    attack_table_t()
    {reset(); }

    void reset()
    { attack_table_sum = std::numeric_limits<double>::min(); }

    void build_table( double miss_chance, double dodge_chance,
                      double parry_chance, double glance_chance,
                      double crit_chance, sim_t* );
  };
  mutable attack_table_t attack_table;


};

// Melee Attack ===================================================================

struct melee_attack_t : public attack_t
{
  melee_attack_t( const std::string& token, player_t* p, const spell_data_t* s = spell_data_t::nil() );

  // Melee Attack Overrides
  virtual void init() override;
  virtual double parry_chance( double /* expertise */, player_t* t ) const override;
  virtual double glance_chance( int delta_level ) const override;

  virtual proc_types proc_type() const override;
};

// Ranged Attack ===================================================================

struct ranged_attack_t : public attack_t
{
  ranged_attack_t( const std::string& token, player_t* p, const spell_data_t* s = spell_data_t::nil() );

  // Ranged Attack Overrides
  virtual double composite_target_multiplier( player_t* ) const override;
  virtual void schedule_execute( action_state_t* execute_state = nullptr ) override;

  virtual proc_types proc_type() const override;
};

// Spell Base ====================================================================

struct spell_base_t : public action_t
{
  // special item flags
  bool procs_courageous_primal_diamond;

  spell_base_t( action_e at, const std::string& token, player_t* p, const spell_data_t* s = spell_data_t::nil() );

  // Spell Base Overrides
  virtual timespan_t gcd() const override;
  virtual timespan_t execute_time() const override;
  virtual timespan_t tick_time( double haste ) const override;
  virtual result_e   calculate_result( action_state_t* ) const override;
  virtual void   execute() override;
  virtual void   schedule_execute( action_state_t* execute_state = nullptr ) override;

  virtual double composite_crit() const override
  { return action_t::composite_crit() + player -> cache.spell_crit(); }

  virtual double composite_haste() const override
  { return action_t::composite_haste() * player -> cache.spell_speed(); }

  virtual double composite_crit_multiplier() const override
  { return action_t::composite_crit_multiplier() * player -> composite_spell_crit_multiplier(); }

  virtual proc_types proc_type() const override;
};

// Harmful Spell ====================================================================

struct spell_t : public spell_base_t
{
public:
  spell_t( const std::string& token, player_t* p, const spell_data_t* s = spell_data_t::nil() );

  // Harmful Spell Overrides
  virtual void   assess_damage( dmg_e, action_state_t* ) override;
  virtual dmg_e amount_type( const action_state_t* /* state */, bool /* periodic */ = false ) const override;
  virtual dmg_e report_amount_type( const action_state_t* /* state */ ) const override;
  virtual void   execute() override;
  virtual double miss_chance( double hit, player_t* t ) const override;
  virtual void   init() override;
  virtual double composite_hit() const override
  { return action_t::composite_hit() + player -> cache.spell_hit(); }
  virtual double composite_versatility( const action_state_t* state ) const override
  { return spell_base_t::composite_versatility( state ) + player -> cache.damage_versatility(); }
};

// Heal =====================================================================

struct heal_t : public spell_base_t
{
public:
  typedef spell_base_t base_t;
  bool group_only;
  double base_pct_heal;
  double tick_pct_heal;
  gain_t* heal_gain;

  heal_t( const std::string& name, player_t* p, const spell_data_t* s = spell_data_t::nil() );

  virtual double composite_pct_heal( const action_state_t* ) const;
  virtual void assess_damage( dmg_e, action_state_t* ) override;
  virtual dmg_e amount_type( const action_state_t* /* state */, bool /* periodic */ = false ) const override;
  virtual dmg_e report_amount_type( const action_state_t* /* state */ ) const override;
  virtual size_t available_targets( std::vector< player_t* >& ) const override;
  virtual void init_target_cache() override;
  virtual double calculate_direct_amount( action_state_t* state ) const override;
  virtual double calculate_tick_amount( action_state_t* state, double dmg_multiplier ) const override;
  virtual void execute() override;
  player_t* find_greatest_difference_player();
  player_t* find_lowest_player();
  std::vector < player_t* > find_lowest_players( int num_players ) const;
  player_t* smart_target() const; // Find random injured healing target, preferring non-pets // Might need to move up hierarchy if there are smart absorbs
  virtual int num_targets() const override;
  virtual void   parse_effect_data( const spelleffect_data_t& );

  virtual double composite_da_multiplier( const action_state_t* s ) const override
  {
    double m = action_multiplier() * action_da_multiplier() *
           player -> cache.player_heal_multiplier( s ) *
           player -> composite_player_dh_multiplier( get_school() );

    return m;
  }

  virtual double composite_ta_multiplier( const action_state_t* s ) const override
  {
    double m = action_multiplier() * action_ta_multiplier() *
           player -> cache.player_heal_multiplier( s ) *
           player -> composite_player_th_multiplier( get_school() );

    return m;
  }

  virtual double composite_player_critical_multiplier() const override
  { return player -> composite_player_critical_healing_multiplier(); }

  virtual double composite_versatility( const action_state_t* state ) const override
  { return spell_base_t::composite_versatility( state ) + player -> cache.heal_versatility(); }

  virtual double composite_resolve( const action_state_t* state ) const override
  {
    double m = 1.0;

    if ( player -> resolve_manager.is_started() )
    {
      if ( ( state -> result_type == HEAL_OVER_TIME && tick_pct_heal == 0.0 ) || ( state -> result_type == HEAL_DIRECT && base_pct_heal == 0.0 ) )
      {
        // apply -60% healing effect
        // m *= 1.0 + player -> resolve_manager.resolve -> effectN( 3 ).percent();

        // apply variable bonus based on current value
        if ( state -> target == player )
          m *= 1.0 + player -> buffs.resolve -> current_value / 100.0;
      }
    }
    return m;
  }

  virtual expr_t* create_expression( const std::string& name ) override;
};

// Absorb ===================================================================

struct absorb_t : public spell_base_t
{
  target_specific_t<absorb_buff_t> target_specific;
  absorb_buff_creator_t creator_;

  absorb_t( const std::string& name, player_t* p, const spell_data_t* s = spell_data_t::nil() );

  virtual absorb_buff_creator_t& creator()
  { return creator_; }

  // Allows customization of the absorb_buff_t that we are creating.
  virtual absorb_buff_t* create_buff( const action_state_t* s )
  {
    buff_t* b = buff_t::find( s -> target, name_str );
    if ( b )
      return debug_cast<absorb_buff_t*>( b );

    std::string stats_obj_name = name_str;
    if ( s -> target != player )
      stats_obj_name += "_" + player -> name_str;
    stats_t* stats_obj = player -> get_stats( stats_obj_name, this );
    if ( stats != stats_obj )
    {
      // Add absorb target stats as a child to the main stats object for reporting
      stats -> add_child( stats_obj );
    }
    creator_.source( stats_obj );
    creator_.actors( s -> target );

    return creator();
  }

  virtual void execute() override;
  virtual void assess_damage( dmg_e, action_state_t* ) override;
  virtual dmg_e amount_type( const action_state_t* /* state */, bool /* periodic */ = false ) const override
  { return ABSORB; }
  virtual void impact( action_state_t* ) override;
  virtual void init_target_cache() override;
  virtual size_t available_targets( std::vector< player_t* >& ) const override;
  virtual int num_targets() const override;
  virtual void multistrike_direct( const action_state_t*, action_state_t* ms_state ) override;
  virtual void multistrike_tick( const action_state_t*, action_state_t* ms_state, double tick_multiplier ) override;

  virtual double composite_da_multiplier( const action_state_t* s ) const override
  {
    double m = action_multiplier() * action_da_multiplier() *
           player -> composite_player_absorb_multiplier( s );

    return m;
  }
  virtual double composite_ta_multiplier( const action_state_t* s ) const override
  {
    double m = action_multiplier() * action_ta_multiplier() *
           player -> composite_player_absorb_multiplier( s );

    return m;
  }
  virtual double composite_versatility( const action_state_t* state ) const override
  { return spell_base_t::composite_versatility( state ) + player -> cache.heal_versatility(); }

  virtual double composite_resolve( const action_state_t* state ) const override
  {
    double m = 1.0;

    if ( player -> resolve_manager.is_started() )
    {
      // apply -60% healing effect
      // m *= 1.0 + player -> resolve_manager.resolve -> effectN( 2 ).percent();

      // apply variable bonus based on current value
      if ( state -> target == player )
        m *= 1.0 + player -> buffs.resolve -> current_value / 100.0;
    }

    return m;
  }

};

// Sequence =================================================================

struct sequence_t : public action_t
{
  bool waiting;
  int sequence_wait_on_ready;
  std::vector<action_t*> sub_actions;
  int current_action;
  bool restarted;
  timespan_t last_restart;

  sequence_t( player_t*, const std::string& sub_action_str );

  virtual void schedule_execute( action_state_t* execute_state = nullptr ) override;
  virtual void reset() override;
  virtual bool ready() override;
  void restart() { current_action = 0; restarted = true; last_restart = sim -> current_time(); }
  bool can_restart()
  { return ! restarted && last_restart < sim -> current_time(); }
};

struct strict_sequence_t : public action_t
{
  size_t current_action;
  std::vector<action_t*> sub_actions;
  std::string seq_name_str;

  strict_sequence_t( player_t*, const std::string& opts );

  bool ready() override;
  void reset() override;
  void cancel() override;
  void interrupt_action() override;
  void schedule_execute( action_state_t* execute_state = nullptr ) override;
};

// Primary proc type of the result (direct (aoe) damage/heal, periodic
// damage/heal)
inline proc_types action_state_t::proc_type() const
{
  if ( result_type == DMG_DIRECT || result_type == HEAL_DIRECT )
    return action -> proc_type();
  else if ( result_type == DMG_OVER_TIME )
    return PROC1_PERIODIC;
  else if ( result_type == HEAL_OVER_TIME )
    return PROC1_PERIODIC_HEAL;

  return PROC1_INVALID;
}

// Secondary proc type of the "finished casting" (i.e., execute()). Only
// triggers the "landing", dodge, parry, and miss procs
inline proc_types2 action_state_t::execute_proc_type2() const
{
  // Bunch up all non-damaging harmful attacks that land into "hit"
  if ( action -> harmful && action -> result_is_hit_or_multistrike( result ) )
    return PROC2_LANDED;
  else if ( result == RESULT_DODGE )
    return PROC2_DODGE;
  else if ( result == RESULT_PARRY )
    return PROC2_PARRY;
  else if ( result == RESULT_MISS )
    return PROC2_MISS;

  return PROC2_INVALID;
}

inline proc_types2 action_state_t::cast_proc_type2() const
{
  // Only foreground actions may trigger the "on cast" procs
  if ( action -> background )
  {
    return PROC2_INVALID;
  }

  if ( action -> attack_direct_power_coefficient( this ) ||
       action -> attack_tick_power_coefficient( this ) ||
       action -> spell_direct_power_coefficient( this ) ||
       action -> spell_tick_power_coefficient( this ) ||
       action -> base_ta( this ) || action -> base_da_min( this ) ||
       action -> bonus_ta( this ) || action -> bonus_da( this ) )
  {
    // This is somewhat naive way to differentiate, better way would be to classify based on the
    // actual proc types, but it will serve our purposes for now.
    return action -> harmful ? PROC2_CAST_DAMAGE : PROC2_CAST_HEAL;
  }

  // Generic fallback "on any cast"
  return PROC2_CAST;
}

// DoT ======================================================================

// DoT Tick Event ===========================================================

struct dot_tick_event_t : public event_t
{
public:
  dot_tick_event_t( dot_t* d, timespan_t time_to_tick );

private:
  virtual void execute() override;
  virtual const char* name() const override
  { return "Dot Tick"; }
  dot_t* dot;
};

// Action cost Tick Event ===========================================================

struct action_cost_tick_event_t : public event_t
{
public:
  action_cost_tick_event_t( action_t& a, timespan_t time_to_tick );

private:
  virtual void execute() override;
  virtual const char* name() const override
  { return "Action Cost Tick"; }
  action_t& action;
  timespan_t time_to_tick;
};

// DoT End Event ===========================================================

struct dot_end_event_t : public event_t
{
public:
  dot_end_event_t( dot_t* d, timespan_t time_to_end );

private:
  virtual void execute() override;
  virtual const char* name() const override
  { return "DoT End"; }
  dot_t* dot;
};

struct dot_t : private noncopyable
{
private:
  sim_t& sim;
  bool ticking;
  timespan_t current_duration;
  timespan_t last_start;
  timespan_t extended_time; // Added time per extend_duration for the current dot application
  timespan_t reduced_time; // Removed time per reduce_duration for the current dot application
public:
  event_t* tick_event;
  event_t* end_event;
  double last_tick_factor;

  player_t* const target;
  player_t* const source;
  action_t* current_action;
  action_state_t* state;
  int num_ticks, current_tick;
  timespan_t miss_time;
  timespan_t time_to_tick;
  std::string name_str;

  dot_t( const std::string& n, player_t* target, player_t* source );

  void   extend_duration( timespan_t extra_seconds, timespan_t max_total_time = timespan_t::min(), uint32_t state_flags = -1 );
  void   extend_duration( timespan_t extra_seconds, uint32_t state_flags )
  { extend_duration( extra_seconds, timespan_t::min(), state_flags ); }
  void   reduce_duration( timespan_t remove_seconds, uint32_t state_flags = -1 );
  void   refresh_duration( uint32_t state_flags = -1 );
  void   reset();
  void   cancel();
  void   trigger( timespan_t duration );
  void   copy( player_t* destination, dot_copy_e = DOT_COPY_START );
  expr_t* create_expression( action_t* action, const std::string& name_str, bool dynamic );

  timespan_t remains() const;
  timespan_t time_to_next_tick() const;
  int    ticks_left() const;
  const char* name() const
  { return name_str.c_str(); }
  bool is_ticking() const
  { return ticking; }
  timespan_t get_extended_time() const
  { return extended_time; }
  double get_last_tick_factor() const
  { return last_tick_factor; }

  void tick();
  void last_tick();
  bool channel_interrupt();

private:
  void tick_zero();
  void schedule_tick();
  void start( timespan_t duration );
  void refresh( timespan_t duration );
  void check_tick_zero();
  bool is_higher_priority_action_available() const;

  friend struct dot_tick_event_t;
  friend struct dot_end_event_t;
};

inline double action_t::last_tick_factor( const dot_t* /* d */, const timespan_t& time_to_tick, const timespan_t& duration ) const
{ return std::min( 1.0, duration / time_to_tick ); }

inline dot_tick_event_t::dot_tick_event_t( dot_t* d, timespan_t time_to_tick ) :
    event_t( *d -> source ),
  dot( d )
{
  if ( sim().debug )
    sim().out_debug.printf( "New DoT Tick Event: %s %s %d-of-%d %.4f",
                d -> source -> name(), dot -> name(), dot -> current_tick + 1, dot -> num_ticks, time_to_tick.total_seconds() );

  add_event( time_to_tick );
}


inline void dot_tick_event_t::execute()
{
  dot -> tick_event = nullptr;
  dot -> current_tick++;

  if ( dot -> current_action -> channeled &&
       dot -> current_action -> action_skill < 1.0 &&
       dot -> remains() >= dot -> current_action -> tick_time( dot -> state -> haste ) )
  {
    if ( rng().roll( std::max( 0.0, dot -> current_action -> action_skill - dot -> current_action -> player -> current.skill_debuff ) ) )
    {
      dot -> tick();
    }
  }
  else // No skill-check required
  {
    dot -> tick();
  }

  // Some dots actually cancel themselves mid-tick. If this happens, we presume
  // that the cancel has been "proper", and just stop event execution here, as
  // the dot no longer exists.
  if ( ! dot -> is_ticking() )
    return;

  if ( dot -> channel_interrupt() )
  {
    return;
  }

  // continue ticking
  dot->schedule_tick();
}

inline dot_end_event_t::dot_end_event_t( dot_t* d, timespan_t time_to_end ) :
    event_t( *d -> source ),
    dot( d )
{
  if ( sim().debug )
    sim().out_debug.printf( "New DoT End Event: %s %s %.3f",
                d -> source -> name(), dot -> name(), time_to_end.total_seconds() );

  add_event( time_to_end );
}

inline void dot_end_event_t::execute()
{
  dot -> end_event = nullptr;
  if ( dot -> current_tick < dot -> num_ticks )
  {
    dot -> current_tick++;
    dot -> tick();
  }
  // If for some reason the last tick has already ticked, ensure that the next tick has not
  // consumed any time yet, i.e., the last tick has occurred on the same timestamp as this end
  // event. This situation may occur in conjunction with extensive dot extension, where the last
  // rescheduling of the dot-end-event occurs between the second to last and last ticks. That will
  // in turn flip the order of the dot-tick-event and dot-end-event.
  else
  {
    assert( ! dot -> tick_event || ( dot -> tick_event && dot -> time_to_tick == dot -> tick_event -> remains() ) );
  }

  // Aand sanity check that the dot has consumed all ticks just in case.
  assert( dot -> current_tick == dot -> num_ticks );
  dot -> last_tick();
}

// "Real" 'Procs per Minute' helper class =====================================

struct real_ppm_t
{
private:
  player_t*    player;
  double       freq;
  double       modifier;
  double       rppm;
  bool         first_proc_occurred;
  timespan_t   last_trigger_attempt;
  timespan_t   last_successful_trigger;
  timespan_t   initial_precombat_time;
  rppm_scale_e scales_with;

  static double max_interval() { return 10.0; }
  static double max_bad_luck_prot() { return 1000.0; }
public:
  static double proc_chance( player_t*         player,
                             double            PPM,
                             const timespan_t& last_trigger,
                             const timespan_t& last_successful_proc,
                             rppm_scale_e      scales_with )
  {
    double coeff = 1.0;
    double seconds = std::min( ( player -> sim -> current_time() - last_trigger ).total_seconds(), max_interval() );

    if ( scales_with == RPPM_HASTE )
      coeff *= 1.0 / std::min( player -> cache.spell_haste(), player -> cache.attack_haste() );
    // This might technically be two separate crit values, but this should be sufficient for our
    // cases. In any case, the client data does not offer information which crit it is (attack or
    // spell).
    else if ( scales_with == RPPM_CRIT )
      coeff *= 1.0 + std::max( player -> cache.attack_crit(), player -> cache.spell_crit() );

    double real_ppm = PPM * coeff;
    double old_rppm_chance = real_ppm * ( seconds / 60.0 );

    // RPPM Extension added on 12. March 2013: http://us.battle.net/wow/en/blog/8953693?page=44
    // Formula see http://us.battle.net/wow/en/forum/topic/8197741003#1
    double last_success = std::min( ( player -> sim -> current_time() - last_successful_proc ).total_seconds(), max_bad_luck_prot() );
    double expected_average_proc_interval = 60.0 / real_ppm;

    double rppm_chance = std::max( 1.0, 1 + ( ( last_success / expected_average_proc_interval - 1.5 ) * 3.0 ) )  * old_rppm_chance;
    if ( player -> sim -> debug )
      player -> sim -> out_debug.printf( "base=%.3f coeff=%.3f last_trig=%.3f last_proc=%.3f scales=%d chance=%.5f%%",
          PPM, coeff, last_trigger.total_seconds(), last_successful_proc.total_seconds(), scales_with,
          rppm_chance * 100.0 );
    return rppm_chance;
  }

  real_ppm_t() :
    player( nullptr ), freq( 0 ), modifier( 0 ), rppm( 0 ),
    first_proc_occurred( false ),
    last_trigger_attempt( timespan_t::from_seconds( -10.0 ) ),
    last_successful_trigger( timespan_t::from_seconds( -120.0 ) ),
    initial_precombat_time( timespan_t::from_seconds( -120.0 ) ),
    scales_with( RPPM_NONE )
  { }

  real_ppm_t( player_t& p, double frequency = 0, double mod = 1.0, rppm_scale_e s = RPPM_NONE ) :
    player( &p ),
    freq( frequency ),
    modifier( mod ),
    rppm( freq * mod ),
    first_proc_occurred( false ),
    last_trigger_attempt( timespan_t::from_seconds( -10.0 ) ),
    last_successful_trigger( timespan_t::from_seconds( -120.0 ) ),
    initial_precombat_time( timespan_t::from_seconds( -120.0 ) ),
    scales_with( s )
  { }

  void set_frequency( double frequency )
  { freq = frequency; rppm = freq * modifier; }

  void set_initial_precombat_time( timespan_t precombat )
  {
    initial_precombat_time = precombat;
  }

  double get_frequency() const
  { return freq; }

  void set_modifier( double mod )
  { modifier = mod; rppm = freq * modifier; }

  double get_modifier() const
  { return modifier; }

  double get_rppm() const
  { return rppm; }

  void set_last_trigger_attempt( const timespan_t& ts )
  { last_trigger_attempt = ts; }

  void set_last_trigger_success( const timespan_t& ts )
  { last_successful_trigger = ts; }

  void reset()
  {
    last_trigger_attempt = timespan_t::from_seconds( -10.0 );
    last_successful_trigger = initial_precombat_time;
    first_proc_occurred = false;
  }

  bool trigger()
  {
    assert( freq != 0 && "Real PPM Frequency not set!" );

    if ( last_trigger_attempt == player -> sim -> current_time() )
      return false;

    double chance = proc_chance( player, rppm, last_trigger_attempt,
                                 last_successful_trigger, scales_with );
    bool success = player -> rng().roll( chance );

    if ( success )
    {
      first_proc_occurred = true;
      last_successful_trigger = player -> sim -> current_time();
    }
    // Due to a bug, last attempt to trigger is fixed until first proc
    if ( first_proc_occurred )
      last_trigger_attempt = player -> sim -> current_time();

    return success;
  }
};

// Action Callback ==========================================================

struct action_callback_t : private noncopyable
{
  player_t* listener;
  bool active;
  bool allow_self_procs;
  bool allow_procs;

  action_callback_t( player_t* l, bool ap = false, bool asp = false ) :
    listener( l ), active( true ), allow_self_procs( asp ), allow_procs( ap )
  {
    assert( l );
    if ( std::find( l -> callbacks.all_callbacks.begin(), l -> callbacks.all_callbacks.end(), this )
        == l -> callbacks.all_callbacks.end() )
      l -> callbacks.all_callbacks.push_back( this );
  }
  virtual ~action_callback_t() {}
  virtual void trigger( action_t*, void* call_data ) = 0;
  virtual void reset() {}
  virtual void initialize() { }
  virtual void activate() { active = true; }
  virtual void deactivate() { active = false; }

  static void trigger( const std::vector<action_callback_t*>& v, action_t* a, void* call_data = nullptr )
  {
    if ( a && ! a -> player -> in_combat ) return;

    std::size_t size = v.size();
    for ( std::size_t i = 0; i < size; i++ )
    {
      action_callback_t* cb = v[ i ];
      if ( cb -> active )
      {
        if ( ! cb -> allow_procs && a && a -> proc ) return;
        cb -> trigger( a, call_data );
      }
    }
  }

  static void reset( const std::vector<action_callback_t*>& v )
  {
    std::size_t size = v.size();
    for ( std::size_t i = 0; i < size; i++ )
    {
      v[ i ] -> reset();
    }
  }
};

// Generic proc callback ======================================================

/**
 * DBC-driven proc callback. Extensively leans on the concept of "driver"
 * spells that blizzard uses to trigger actual proc spells in most cases. Uses
 * spell data as much as possible (through interaction with special_effect_t).
 * Intentionally vastly simplified compared to our other (older) callback
 * systems. The "complex" logic is offloaded either into special_effect_t
 * (creation of buffs/actions), special effect_t initialization (what kind of
 * special effect to create from DBC data or user given options, or when and
 * why to proc things (new DBC based proc system).
 *
 * The actual triggering logic is also vastly simplified (see execute()), as
 * the majority of procs in WoW are very simple. Custom procs can always be
 * derived off of this struct.
 */
struct dbc_proc_callback_t : public action_callback_t
{
  static const item_t default_item_;

  const item_t& item;
  const special_effect_t& effect;
  cooldown_t* cooldown;

  // Proc trigger types, cached/initialized here from special_effect_t to avoid
  // needless spell data lookups in vast majority of cases
  real_ppm_t  rppm;
  double      proc_chance;
  double      ppm;

  buff_t* proc_buff;
  action_t* proc_action;
  weapon_t* weapon;

  dbc_proc_callback_t( const item_t& i, const special_effect_t& e ) :
    action_callback_t( i.player ), item( i ), effect( e ), cooldown( nullptr ),
    proc_chance( 0 ), ppm( 0 ),
    proc_buff( nullptr ), proc_action( nullptr ), weapon( nullptr )
  { }

  dbc_proc_callback_t( const item_t* i, const special_effect_t& e ) :
    action_callback_t( i -> player ), item( *i ), effect( e ), cooldown( nullptr ),
    proc_chance( 0 ), ppm( 0 ),
    proc_buff( nullptr ), proc_action( nullptr ), weapon( nullptr )
  { }

  dbc_proc_callback_t( player_t* p, const special_effect_t& e ) :
    action_callback_t( p ), item( default_item_ ), effect( e ), cooldown( nullptr ),
    proc_chance( 0 ), ppm( 0 ),
    proc_buff( nullptr ), proc_action( nullptr ), weapon( nullptr )
  { }

  virtual void initialize() override;

  void reset() override
  {
    action_callback_t::reset();
    if ( rppm.get_frequency() > 0 )
      rppm.reset();
  }

  void trigger( action_t* a, void* call_data ) override
  {
    if ( cooldown && cooldown -> down() ) return;

    // Weapon-based proc triggering differs from "old" callbacks. When used
    // (weapon_proc == true), dbc_proc_callback_t _REQUIRES_ that the action
    // has the correct weapon specified. Old style procs allowed actions
    // without any weapon to pass through.
    if ( weapon && ( ! a -> weapon || ( a -> weapon && a -> weapon != weapon ) ) )
      return;

    bool triggered = roll( a );
    if ( listener -> sim -> debug )
      listener -> sim -> out_debug.printf( "%s attempts to proc %s on %s: %d",
                                 listener -> name(),
                                 effect.to_string().c_str(),
                                 a -> name(), triggered );
    if ( triggered )
    {
      execute( a, static_cast<action_state_t*>( call_data ) );

      if ( cooldown )
        cooldown -> start();
    }
  }
private:
  rng::rng_t& rng() const
  { return listener -> rng(); }

  bool roll( action_t* action )
  {
    if ( rppm.get_frequency() > 0 )
      return rppm.trigger();
    else if ( ppm > 0 )
      return rng().roll( action -> ppm_proc_chance( ppm ) );
    else if ( proc_chance > 0 )
      return rng().roll( proc_chance );

    assert( false );
    return false;
  }

  /**
   * Base rules for proc execution.
   * 1) If we proc a buff, trigger it
   * 2a) If the buff triggered and is at max stack, and we have an action,
   *     execute the action on the target of the action that triggered this
   *     proc.
   * 2b) If we have no buff, trigger the action on the target of the action
   *     that triggered this proc.
   *
   * TODO: Ticking buffs, though that'd be better served by fusing tick_buff_t
   * into buff_t.
   * TODO: Proc delay
   * TODO: Buff cooldown hackery for expressions. Is this really needed or can
   * we do it in a smarter way (better expression support?)
   */
  virtual void execute( action_t* /* a */, action_state_t* state )
  {
    bool triggered = proc_buff == nullptr;
    if ( proc_buff )
      triggered = proc_buff -> trigger();

    if ( triggered && proc_action &&
         ( ! proc_buff || proc_buff -> check() == proc_buff -> max_stack() ) )
    {
      proc_action -> target = state -> target;
      proc_action -> schedule_execute();

      // Decide whether to expire the buff even with 1 max stack
      if ( proc_buff && proc_buff -> max_stack() > 1 )
      {
        proc_buff -> expire();
      }
    }
  }
};

// Action Priority List =====================================================

struct action_priority_t
{
  std::string action_;
  std::string comment_;

  action_priority_t( const std::string& a, const std::string& c ) :
    action_( a ), comment_( c )
  { }

  action_priority_t* comment( const std::string& c )
  { comment_ = c; return this; }
};

struct action_priority_list_t
{
  // Internal ID of the action list, used in conjunction with the "new"
  // call_action_list action, that allows for potential infinite loops in the
  // APL.
  unsigned internal_id;
  uint64_t internal_id_mask;
  std::string name_str;
  std::string action_list_comment_str;
  std::string action_list_str;
  std::vector<action_priority_t> action_list;
  player_t* player;
  bool used;
  std::vector<action_t*> foreground_action_list;
  std::vector<action_t*> off_gcd_actions;
  int random; // Used to determine how faceroll something actually is. :D
  action_priority_list_t( std::string name, player_t* p, const std::string& list_comment = std::string() ) :
    internal_id( 0 ), internal_id_mask( 0 ), name_str( name ), action_list_comment_str( list_comment ), player( p ), used( false ),
    foreground_action_list( 0 ), off_gcd_actions( 0 ), random( 0 )
  { }

  action_priority_t* add_action( const std::string& action_priority_str, const std::string& comment = std::string() );
  action_priority_t* add_action( const player_t* p, const spell_data_t* s, const std::string& action_name,
                                 const std::string& action_options = std::string(), const std::string& comment = std::string() );
  action_priority_t* add_action( const player_t* p, const std::string& name, const std::string& action_options = std::string(),
                                 const std::string& comment = std::string() );
  action_priority_t* add_talent( const player_t* p, const std::string& name, const std::string& action_options = std::string(),
                                 const std::string& comment = std::string() );
};

struct travel_event_t : public event_t
{
  action_t* action;
  action_state_t* state;
  travel_event_t( action_t* a, action_state_t* state, timespan_t time_to_travel );
  virtual ~travel_event_t() { if ( state && canceled ) action_state_t::release( state ); }
  virtual void execute() override;
  virtual const char* name() const override
  { return "Stateless Action Travel"; }
};

struct multistrike_execute_event_t : public event_t
{
  action_state_t* state;

  multistrike_execute_event_t( action_state_t* s, int ms_count = 0 ) :
    event_t( *s -> action -> player ), state( s )
  {
    if ( sim().debug )
    {
      sim().out_debug.printf( "New Multistrike Execute Event: %s %s (target=%s)",
                  s -> action -> player -> name(), s -> action -> name(),
                  s -> target -> name() );
    }

    timespan_t multistrike_offset = timespan_t::zero();

    if ( state -> action -> instant_multistrike == 0 )
    {
      // Values taken from Celestalon's second post about this -- Twintop 2015/02/23 (updated link)
      // http://www.wowhead.com/bluetracker?topic=13087818929#135924364017
      if ( ms_count == 0 )
        multistrike_offset = timespan_t::from_millis( 333 );
      else
        multistrike_offset = timespan_t::from_millis( 666 );
    }

    // Hots/Dots will be scheduled immediately, direct damage multistrikes will
    // jump through hoops .. below
    if ( state -> result_type == DMG_OVER_TIME || state -> result_type == HEAL_OVER_TIME )
      add_event( timespan_t::zero() + multistrike_offset );

    // Direct damage/heal multistrikes need to take into account the travel
    // time of the "real" spell, and impact at the same time(?), so ..
    // TODO-WOD: Multistrike impacts in combatlog have delay of .. ?
    else if ( state -> result_type == DMG_DIRECT || state -> result_type == HEAL_DIRECT || state -> result_type == ABSORB )
    {
      // No travel time -> impact immediately. Event ordering is guaranteed, as
      // schedule_travel() in action_t::execute() has already impacted on this
      // timestamp
      if ( state -> action -> travel_time() == timespan_t::zero() )
      {
        add_event( timespan_t::zero() + multistrike_offset );
      }
      // Travel time spell, schedule_travel() has inserted a new entry into the
      // action's travel events (at the end of the vector), so use it's
      // remaining time as a basis for this event. Event ordering guaranteed,
      // as the main spell travel event is already in the queue.
      else
      {
        assert( state -> action -> current_travel_events().size() > 0 );
        timespan_t time_to_travel = state -> action -> current_travel_events().back() -> remains();
        add_event( time_to_travel + multistrike_offset );
      }
    }
    else
    {
      assert( 0 && "Multistrike Execute event, where state has no result_type" );
    }
  }
  virtual const char* name() const override
  { return "Multistrike-Execute-Event"; }
  // Ensure we properly release the carried execute_state even if this event
  // is never executed.
  ~multistrike_execute_event_t()
  {
    if ( state )
      action_state_t::release( state );
  }

  void execute() override
  {
    if ( ! state -> target -> is_sleeping() )
    {
      if ( sim().debug )
        state -> debug();

      if ( state -> result_type == DMG_OVER_TIME || state -> result_type == HEAL_OVER_TIME )
        state -> action -> assess_damage( state -> result_type, state );
      else
        state -> action -> impact( state );
    }

    action_state_t::release( state );
  }
};

// Item database ============================================================

namespace item_database
{
bool     download_item(      item_t& item );
bool     download_glyph(     player_t* player, std::string& glyph_name, const std::string& glyph_id );
bool     initialize_item_sources( item_t& item, std::vector<std::string>& source_list );

int      random_suffix_type( item_t& item );
int      random_suffix_type( const item_data_t* );
uint32_t armor_value(        const item_t& item );
uint32_t armor_value(        const item_data_t*, const dbc_t&, unsigned item_level = 0 );
// Uses weapon's own (upgraded) ilevel to calculate the damage
uint32_t weapon_dmg_min(     item_t& item );
// Allows custom ilevel to be specified
uint32_t weapon_dmg_min(     const item_data_t*, const dbc_t&, unsigned item_level = 0 );
uint32_t weapon_dmg_max(     item_t& item );
uint32_t weapon_dmg_max(     const item_data_t*, const dbc_t&, unsigned item_level = 0 );

bool     load_item_from_data( item_t& item );

// Parse anything relating to the use of ItemSpellEnchantment.dbc. This includes
// enchants, and engineering addons.
bool     parse_item_spell_enchant( item_t& item, std::vector<stat_pair_t>& stats, special_effect_t& effect, unsigned enchant_id );

std::string stat_to_str( int stat, int stat_amount );

// Stat scaling methods for items, or item stats
double approx_scale_coefficient( unsigned current_ilevel, unsigned new_ilevel );
int scaled_stat( const item_t& item, const dbc_t& dbc, size_t idx, unsigned new_ilevel );

unsigned upgrade_ilevel( const item_t& item, unsigned upgrade_level );
stat_pair_t item_enchantment_effect_stats( const item_enchantment_data_t& enchantment, int index );
stat_pair_t item_enchantment_effect_stats( player_t* player, const item_enchantment_data_t& enchantment, int index );
double item_budget( const item_t* item, unsigned max_ilevel );

inline bool heroic( unsigned f ) { return ( f & RAID_TYPE_HEROIC ) == RAID_TYPE_HEROIC; }
inline bool lfr( unsigned f ) { return ( f & RAID_TYPE_LFR ) == RAID_TYPE_LFR; }
inline bool warforged( unsigned f ) { return ( f & RAID_TYPE_WARFORGED ) == RAID_TYPE_WARFORGED; }
inline bool mythic( unsigned f ) { return ( f & RAID_TYPE_MYTHIC ) == RAID_TYPE_MYTHIC; }

bool apply_item_bonus( item_t& item, const item_bonus_entry_t& entry );

bool apply_item_scaling( item_t& item, unsigned scaling_id );

struct token_t
{
  std::string full;
  std::string name;
  double value;
  std::string value_str;
};
size_t parse_tokens( std::vector<token_t>& tokens, const std::string& encoded_str );
}

// Procs ====================================================================

namespace special_effect
{
  bool parse_special_effect_encoding( special_effect_t& effect, const std::string& str );
  bool usable_proc( const special_effect_t& effect );
}

// Enchants =================================================================

namespace enchant
{
  struct enchant_db_item_t
  {
    const char* enchant_name;
    unsigned    enchant_id;
  };

  unsigned find_enchant_id( const std::string& name );
  std::string find_enchant_name( unsigned enchant_id );
  std::string encoded_enchant_name( const dbc_t&, const item_enchantment_data_t& );

  const item_enchantment_data_t& find_item_enchant( const item_t& item, const std::string& name );
  const item_enchantment_data_t& find_meta_gem( const dbc_t& dbc, const std::string& encoding );
  meta_gem_e meta_gem_type( const dbc_t& dbc, const item_enchantment_data_t& );
  bool passive_enchant( item_t& item, unsigned spell_id );

  bool initialize_item_enchant( item_t& item, std::vector< stat_pair_t >& stats, special_effect_source_e source, const item_enchantment_data_t& enchant );
  item_socket_color initialize_gem( item_t& item, unsigned gem_id );
}

// Unique Gear ==============================================================

namespace unique_gear
{
  typedef void (*custom_cb_t)( special_effect_t& );
  struct special_effect_db_item_t
  {
    unsigned    spell_id;
    std::string encoded_options;
    //const std::function<void(special_effect_t&, const item_t&, const special_effect_db_item_t&)> custom_cb;
    custom_cb_t custom_cb;

    special_effect_db_item_t() :
      spell_id( 0 ), encoded_options(), custom_cb( nullptr )
    { }
  };

void register_hotfixes();
void register_special_effects();
void register_special_effect( unsigned spell_id, const std::string& encoded_str );
void register_special_effect( unsigned spell_id, custom_cb_t callback );
void register_target_data_initializers( sim_t* );

void init( player_t* );

const special_effect_db_item_t& find_special_effect_db_item( unsigned spell_id );
bool initialize_special_effect( special_effect_t& effect, unsigned spell_id );
void initialize_special_effect_2( special_effect_t* effect ); // Second phase initialization

const item_data_t* find_consumable( const dbc_t& dbc, const std::string& name, item_subclass_consumable type );
const item_data_t* find_item_by_spell( const dbc_t& dbc, unsigned spell_id );
const item_data_t* find_potion_by_spell( const dbc_t& dbc, unsigned spell_id );

expr_t* create_expression( action_t* a, const std::string& name_str );
}

// Consumable ===============================================================

namespace consumable
{
action_t* create_action( player_t*, const std::string& name, const std::string& options );
}

// Wowhead  =================================================================

namespace wowhead
{
enum wowhead_e
{
  LIVE,
  PTR,
  BETA
};

bool download_item( item_t&, wowhead_e source = LIVE, cache::behavior_e b = cache::items() );
bool download_glyph( player_t* player, std::string& glyph_name, const std::string& glyph_id,
                     wowhead_e source = LIVE, cache::behavior_e b = cache::items() );
bool download_item_data( item_t&            item,
                         cache::behavior_e  caching,
                         wowhead_e          source );

std::string domain_str( wowhead_e domain );
}


// Blizzard Community Platform API ==========================================

namespace bcp_api
{
bool download_guild( sim_t* sim,
                     const std::string& region,
                     const std::string& server,
                     const std::string& name,
                     const std::vector<int>& ranks,
                     int player_e = PLAYER_NONE,
                     int max_rank = 0,
                     cache::behavior_e b = cache::players() );

player_t* download_player_html( sim_t*,
                           const std::string& region,
                           const std::string& server,
                           const std::string& name,
                           const std::string& talents = std::string( "active" ),
                           cache::behavior_e b = cache::players() );

player_t* download_player( sim_t*,
                           const std::string& region,
                           const std::string& server,
                           const std::string& name,
                           const std::string& talents = std::string( "active" ),
                           cache::behavior_e b = cache::players() );

player_t* from_local_json( sim_t*,
                           const std::string&,
                           const std::string&,
                           const std::string&
                         );

bool download_item( item_t&, cache::behavior_e b = cache::items() );

bool download_glyph( player_t* player, std::string& glyph_name, const std::string& glyph_id,
                     cache::behavior_e b = cache::items() );
}

// HTTP Download  ===========================================================

namespace http
{
struct proxy_t
{
  std::string type;
  std::string host;
  int port;
};
void set_proxy( const std::string& type, const std::string& host, const unsigned port );

void cache_load( const std::string& file_name );
void cache_save( const std::string& file_name );
bool clear_cache( sim_t*, const std::string& name, const std::string& value );

bool get( std::string& result, const std::string& url, const std::string& cleanurl, cache::behavior_e b,
          const std::string& confirmation = std::string() );
}

// XML ======================================================================
#include "util/xml.hpp"

// Handy Actions ============================================================

struct wait_action_base_t : public action_t
{
  wait_action_base_t( player_t* player, const std::string& name ):
    action_t( ACTION_OTHER, name, player )
  {
    trigger_gcd = timespan_t::zero();
    interrupt_auto_attack = false;
  }

  virtual void execute() override
  { player -> iteration_waiting_time += time_to_execute; }
};

// Wait For Cooldown Action =================================================

struct wait_for_cooldown_t : public wait_action_base_t
{
  cooldown_t* wait_cd;
  action_t* a;
  wait_for_cooldown_t( player_t* player, const std::string& cd_name );
  virtual bool usable_moving() const override { return a -> usable_moving(); }
  virtual timespan_t execute_time() const override;
};

// Namespace for a ignite like action. Not mandatory, but encouraged to use it.
namespace residual_action
{
// Custom state for ignite-like actions. tick_amount contains the current
// ticking value of the ignite periodic effect, and is adjusted every time
// residual_periodic_action_t (the ignite spell) impacts on the target.
struct residual_periodic_state_t : public action_state_t
{
  double tick_amount;

  residual_periodic_state_t( action_t* a, player_t* t ) :
    action_state_t( a, t ),
    tick_amount( 0 )
  { }

  std::ostringstream& debug_str( std::ostringstream& s ) override
  { action_state_t::debug_str( s ) << " tick_amount=" << tick_amount; return s; }

  void initialize() override
  { action_state_t::initialize(); tick_amount = 0; }

  void copy_state( const action_state_t* o ) override
  {
    action_state_t::copy_state( o );
    const residual_periodic_state_t* dps_t = debug_cast<const residual_periodic_state_t*>( o );
    tick_amount = dps_t -> tick_amount;
  }
};

template <class Base>
struct residual_periodic_action_t : public Base
{
private:
  typedef Base ab; // typedef for the templated action type, spell_t, or heal_t
public:
  typedef residual_periodic_action_t base_t;
  typedef residual_periodic_action_t<Base> residual_action_t;

  template <typename T>
  residual_periodic_action_t( const std::string& n, T& p, const spell_data_t* s ) :
    ab( n, p, s )
  {
    initialize_();
  }

  template <typename T>
  residual_periodic_action_t( const std::string& n, T* p, const spell_data_t* s ) :
    ab( n, p, s )
  {
    initialize_();
  }

  void initialize_()
  {
    ab::background = true;

    ab::tick_may_crit = false;
    ab::hasted_ticks  = false;
    ab::may_crit = false;
    ab::attack_power_mod.tick = 0;
    ab::spell_power_mod.tick = 0;
    ab::dot_behavior  = DOT_REFRESH;
    ab::callbacks = false;
    ab::may_multistrike = false;
  }

  virtual action_state_t* new_state() override
  { return new residual_periodic_state_t( this, ab::target ); }

  // Residual periodic actions will not be extendeed by the pandemic mechanism,
  // thus the new maximum length of the dot is the ongoing tick plus the
  // duration of the dot.
  virtual timespan_t calculate_dot_refresh_duration( const dot_t* dot, timespan_t triggered_duration ) const override
  { return dot -> time_to_next_tick() + triggered_duration; }

  virtual void impact( action_state_t* s ) override
  {
    // Residual periodic actions + tick_zero does not work
    assert( ! ab::tick_zero );

    dot_t* dot = ab::get_dot( s -> target );
    double current_amount = 0, old_amount = 0;
    int ticks_left = 0;
    residual_periodic_state_t* dot_state = debug_cast<residual_periodic_state_t*>( dot -> state );

    // If dot is ticking get current residual pool before we overwrite it
    if ( dot -> is_ticking() )
    {
      old_amount = dot_state -> tick_amount;
      ticks_left = dot -> ticks_left();
      current_amount = old_amount * dot -> ticks_left();
    }

    // Add new amount to residual pool
    current_amount += s -> result_amount;

    // Trigger the dot, refreshing it's duration or starting it
    ab::trigger_dot( s );

    // If the dot is not ticking, dot_state will be nullptr, so get the
    // residual_periodic_state_t object from the dot again (since it will exist
    // after trigger_dot() is called)
    if ( ! dot_state )
      dot_state = debug_cast<residual_periodic_state_t*>( dot -> state );

    // Compute tick damage after refresh, so we divide by the correct number of
    // ticks
    dot_state -> tick_amount = current_amount / dot -> ticks_left();

    // Spit out debug for what we did
    if ( ab::sim -> debug )
      ab::sim -> out_debug.printf( "%s %s residual_action impact amount=%f old_total=%f old_ticks=%d old_tick=%f current_total=%f current_ticks=%d current_tick=%f",
                  ab::player -> name(), ab::name(), s -> result_amount,
                  old_amount * ticks_left, ticks_left, ticks_left > 0 ? old_amount : 0,
                  current_amount, dot -> ticks_left(), dot_state -> tick_amount );
  }

  // The damage of the tick is simply the tick_amount in the state
  virtual double base_ta( const action_state_t* s ) const override
  {
    auto dot = ab::find_dot( s -> target );
    if ( dot )
    {
      auto rd_state = debug_cast<const residual_periodic_state_t*>( dot -> state );
      return rd_state -> tick_amount;
    }
    return 0.0;
  }

  // Ensure that not travel time exists for the ignite ability. Delay is
  // handled in the trigger via a custom event
  virtual timespan_t travel_time() const override
  { return timespan_t::zero(); }

  // This object is not "executable" normally. Instead, the custom event
  // handles the triggering of ignite
  virtual void execute() override
  { assert( 0 ); }

  // Ensure that the ignite action snapshots nothing
  void init() override
  {
    ab::init();

    ab::update_flags = ab::snapshot_flags = 0;
  }
};

// Trigger a residual periodic action on target t
void trigger( action_t* residual_action, player_t* t, double amount );

}  // namespace residual_action

// Inlines ==================================================================

// buff_t inlines

inline buff_t* buff_t::find( sim_t* s, const std::string& name )
{
  return find( s -> buff_list, name );
}
inline buff_t* buff_t::find( player_t* p, const std::string& name, player_t* source )
{
  return find( p -> buff_list, name, source );
}
inline std::string buff_t::source_name() const
{
  if ( player ) return player -> name_str;
  return "noone";
}
inline rng::rng_t& buff_t::rng()
{ return sim -> rng(); }
// sim_t inlines

inline buff_creator_t::operator buff_t* () const
{ return new buff_t( *this ); }

inline stat_buff_creator_t::operator stat_buff_t* () const
{ return new stat_buff_t( *this ); }

inline absorb_buff_creator_t::operator absorb_buff_t* () const
{ return new absorb_buff_t( *this ); }

inline cost_reduction_buff_creator_t::operator cost_reduction_buff_t* () const
{ return new cost_reduction_buff_t( *this ); }

inline haste_buff_creator_t::operator haste_buff_t* () const
{ return new haste_buff_t( *this ); }

inline buff_creator_t::operator debuff_t* () const
{ return new debuff_t( *this ); }

inline bool player_t::is_my_pet( player_t* t ) const
{ return t -> is_pet() && t -> cast_pet() -> owner == this; }

inline void player_t::do_dynamic_regen()
{
  if ( sim -> current_time() == last_regen )
    return;

  regen( sim -> current_time() - last_regen );
  last_regen = sim -> current_time();

  if ( dynamic_regen_pets )
  {
    for (auto & elem : active_pets)
    {
      if ( elem -> regen_type == REGEN_DYNAMIC )
        elem -> do_dynamic_regen();
    }
  }
}

inline target_wrapper_expr_t::target_wrapper_expr_t( action_t& a, const std::string& name_str, const std::string& expr_str ) :
  expr_t( name_str ), action( a ), suffix_expr_str( expr_str )
{
  proxy_expr.resize( action.sim -> actor_list.size() + 1, nullptr );
}

inline double target_wrapper_expr_t::evaluate()
{
  assert( target() );

  size_t actor_index = target() -> actor_index;

  if ( proxy_expr[ actor_index ] == nullptr )
  {
    proxy_expr[ actor_index ] = target() -> create_expression( &( action ), suffix_expr_str );
  }

  std::cout << "target_wrapper_expr_t " << name() << " evaluate " << target() -> name() << " " <<  proxy_expr[ actor_index ] -> eval() << std::endl;

  return proxy_expr[ actor_index ] -> eval();
}

inline player_t* target_wrapper_expr_t::target() const
{ return action.target; }

inline actor_target_data_t::actor_target_data_t( player_t* target, player_t* source ) :
  actor_pair_t( target, source ), debuff( atd_debuff_t() ), dot( atd_dot_t() )
{
  for (auto & elem : source -> sim -> target_data_initializer)
  {
    elem( this );
  }
}

// Instant absorbs
struct instant_absorb_t
{
private:
  /* const spell_data_t* spell */;
  std::function<double( const action_state_t* )> absorb_handler;
  stats_t* absorb_stats;
  gain_t*  absorb_gain;
  player_t* player;

public:
  std::string name;

  instant_absorb_t( player_t* p, const spell_data_t* s, const std::string n,
                    std::function<double( const action_state_t* )> handler ) :
    /* spell( s ), */ absorb_handler( handler ), player( p ), name( n )
  {
    util::tokenize( name );

    absorb_stats = p -> get_stats( name );
    absorb_gain  = p -> get_gain( name );
    absorb_stats -> type = STATS_ABSORB;
    absorb_stats -> school = s -> get_school_type();
  }

  double consume( const action_state_t* s )
  {
    double amount = std::min( s -> result_amount, absorb_handler( s ) );

    if ( amount > 0 )
    {
      absorb_stats -> add_result( amount, 0, ABSORB, RESULT_HIT, BLOCK_RESULT_UNBLOCKED, player );
      absorb_stats -> add_execute( timespan_t::zero(), player );
      absorb_gain -> add( RESOURCE_HEALTH, amount, 0 );

      if ( player -> sim -> debug )
        player -> sim -> out_debug.printf( "%s %s absorbs %.2f", player -> name(), name.c_str(), amount );
    }

    return amount;
  }
};

template <class T>
sim_ostream_t& sim_ostream_t::operator<< (T const& rhs)
{
  _raw << util::to_string( sim.current_time().total_seconds(), 3 ) << " " << rhs << "\n";

  return *this;
}

#endif // SIMULATIONCRAFT_H
