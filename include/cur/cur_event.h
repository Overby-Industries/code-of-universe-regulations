// cur_event.h — Constitutional Event Model (CUR-FOUNDATION-005).
//
// FOUNDATION-005 §2: "Nothing happens without an event."
// FOUNDATION-005 §12: events do not alter state directly; the FSM processes
// them and decides validity, transitions, faults, and audit requirements.
// FOUNDATION-005 §13: the simulator permits no direct state modification.
//
// This header defines the only struct that may be handed to the state machine.
// If it is not an Event, it cannot change anything.

#ifndef CUR_EVENT_H
#define CUR_EVENT_H

#include <cstdint>
#include <string>

#include "cur_state.h"

namespace cur {

// Priority levels — FOUNDATION-005 §10.
enum EventPriority : uint8_t {
    P1_INFORMATIONAL = 0,
    P2_GOVERNANCE,
    P3_JUDICIAL,
    P4_CONSTITUTIONAL,
    P5_EXISTENTIAL,
    P_COUNT
};

// Handle into the entity registry. Stable for the lifetime of a machine.
using EntityHandle = uint32_t;
constexpr EntityHandle INVALID_ENTITY = 0xFFFFFFFFu;

// TransitionContext::advocate_ref is an EntityHandle, but cur_state.h is
// included by this header and so cannot name the type. It spells out the
// underlying representation and repeats the sentinel; this keeps the two in
// lockstep, so a change to either is a compile error rather than a guard that
// silently reads an appointed advocate as an absent one.
static_assert(TransitionContext{}.advocate_ref == INVALID_ENTITY,
              "TransitionContext::advocate_ref must default to INVALID_ENTITY");
static_assert(sizeof(TransitionContext{}.advocate_ref) == sizeof(EntityHandle),
              "TransitionContext::advocate_ref must be an EntityHandle");

// A constitutional event — FOUNDATION-005 §3.
//
// On determinism: `tick` and `sequence` are supplied by the caller and are the
// machine's only notion of time. There is no wall clock anywhere in the
// evaluation path. Wall-clock time is attached later by the event log, as
// metadata, and is never read back in. Replaying the same event stream against
// the same regulation set therefore yields byte-identical state and an
// identical log except for that one metadata column.
struct Event {
    EventType type = EV_NONE;
    EventPriority priority = P1_INFORMATIONAL;

    // Logical time. Monotonic, caller-supplied, no clock reads.
    uint64_t tick = 0;
    uint64_t sequence = 0;

    // Who acted, and on what. FOUNDATION-005 §3 actor/target.
    EntityHandle actor = INVALID_ENTITY;
    EntityHandle target = INVALID_ENTITY;

    // Guard inputs for this event. The machine overwrites
    // TransitionContext::subject_is_license from the target's registered
    // SubjectClass, so a caller cannot assert its way past a licence guard.
    TransitionContext context;

    // Set by a detection system when an instrument is identified as tending
    // toward a forbidden state. PDDC-GOVERNANCE-MECHANICS §12.2(b): detecting a
    // trajectory is sufficient, a completed transition is not required.
    ForbiddenState asserts_forbidden = FS_NONE;

    // Free-text provenance for the audit trail. Never read by the FSM.
    std::string event_id;
    std::string description;
};

// Validation — FOUNDATION-005 §9. Invalid events are rejected, not coerced.
struct EventValidation {
    bool valid = false;
    const char* reason = "";
};

EventValidation validate_event(const Event& e);

const char* to_string(EventPriority p);

// Default priority for an event type, per the FOUNDATION-005 §4 categories.
// Callers may override; this is the sensible floor.
EventPriority default_priority(EventType t);

}  // namespace cur

#endif  // CUR_EVENT_H
