// cur.h — umbrella header for libcur.
//
// The Code of Universe Regulations as a deterministic C++17 state machine.
// Include this and you have the whole library; include the individual headers
// if you would rather be explicit about what you depend on.
//
// libcur has no dependency on Godot, on the Aevoria Simulator, or on any engine
// or allocator. That is deliberate: the regulations must stay testable and
// reusable on their own, and a consumer binds to them rather than the reverse.
//
//   Entity -> Event -> FSM -> State change     (CUR-FOUNDATION-005 §13)
//
// Minimal use:
//
//   #include <cur/cur.h>
//
//   cur::CURStateMachine m;                       // baseline regulation set
//   auto charter = m.entities().register_entity(
//       "charter-helga-07", cur::EC_ECONOMIC, cur::SUBJ_OPERATIONAL_LICENSE);
//
//   cur::TransitionContext ctx;
//   ctx.debris_units = 40;
//   ctx.debris_limit = 100;
//   auto r = m.submit_operational(charter, cur::EV_MINING_OPERATION, ctx, 1);
//   // r.accepted == true, still KS_COMPLIANT
//
//   ctx.debris_units = 400;
//   r = m.submit_operational(charter, cur::EV_MINING_OPERATION, ctx, 2);
//   // r.accepted == true, now KS_VIOLATION, Class II, ENTITY-009 record opened
//
// See docs/cur-library-api.md for the full surface and for the interpretation
// notes where the corpus needed one.

#ifndef CUR_H
#define CUR_H

#include "cur_capture_index.h"
#include "cur_entity.h"
#include "cur_event.h"
#include "cur_event_log.h"
#include "cur_regulation.h"
#include "cur_state.h"
#include "cur_state_machine.h"
#include "cur_violation.h"

// Library version, independent of the CUR document version in VERSION.md.
#define CUR_LIB_VERSION_MAJOR 0
#define CUR_LIB_VERSION_MINOR 1
#define CUR_LIB_VERSION_PATCH 0
#define CUR_LIB_VERSION_STRING "0.1.0"

namespace cur {

// Version of the CUR corpus these tables were transcribed from.
constexpr const char* CUR_CORPUS_VERSION = "1.0.1-Official-Evergreen";

}  // namespace cur

#endif  // CUR_H
