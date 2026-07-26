// cur_event_log.h — the audit trail.
//
// The log is not optional and it is not a debugging aid. Three provisions make
// it mandatory:
//
//   FOUNDATION-002 §9   "No state transition may occur without record
//                        generation."
//   FOUNDATION-005 §11  Constitutional events are preserved permanently;
//                        exceptions require constitutional amendment.
//   PDDC §12.7(b)(4)    The State Registry is preserved as a permanent record
//                        that may not be expunged.
//
// So: every accepted transition, every REFUSED transition, every fault, and
// every amendment decision is recorded. Refusals matter as much as acceptances
// — a rejected instrument is exactly what an auditor needs to see.
//
// There is no erase(), no redact(), and no way to rewrite a record. The only
// bounded-memory concession is a ring buffer for long simulator runs, and when
// it drops the oldest record it increments `dropped_` and says so on export,
// rather than quietly losing history.

#ifndef CUR_EVENT_LOG_H
#define CUR_EVENT_LOG_H

#include <cstdint>
#include <deque>
#include <string>

#include "cur_entity.h"
#include "cur_event.h"
#include "cur_regulation.h"
#include "cur_state.h"

namespace cur {

enum RecordKind : uint8_t {
    REC_TRANSITION_ACCEPTED = 0,
    REC_TRANSITION_REFUSED,
    REC_FAULT_DECLARED,
    REC_REVERTED_TO_SAFE_STATE,
    REC_PROTECTED_MODE_ENTERED,
    REC_PROTECTED_MODE_EXITED,
    REC_CERTIFICATION_GRANTED,
    REC_CERTIFICATION_REVOKED,
    REC_AMENDMENT_ACCEPTED,
    REC_AMENDMENT_REFUSED,
    REC_KIND_COUNT
};

const char* to_string(RecordKind k);

// Which axis a record describes. REC_AXIS_NONE for records that are not a
// state transition at all (amendments, faults with no state effect).
enum RecordAxis : uint8_t {
    REC_AXIS_NONE = 0,
    REC_AXIS_CONSTITUTIONAL,
    REC_AXIS_GOVERNANCE,
    REC_AXIS_COMPLIANCE,
    REC_AXIS_COUNT
};

const char* to_string(RecordAxis a);

// One immutable line of the trail.
struct LogRecord {
    uint64_t record_seq = 0;  // dense, gap-free, assigned by the log
    RecordKind kind = REC_TRANSITION_ACCEPTED;
    RecordAxis axis = REC_AXIS_NONE;

    // Logical time — the FSM's own clock, and the only time it reads.
    uint64_t tick = 0;
    uint64_t event_sequence = 0;

    // Wall-clock time, seconds since the Unix epoch. METADATA ONLY. Written
    // once here, never read back into the machine. Determinism depends on this
    // staying a one-way street; see cur_event.h.
    int64_t wall_clock_utc = 0;

    // Subject
    EntityHandle entity = INVALID_ENTITY;
    std::string entity_id;
    SubjectClass subject_class = SUBJ_SENTIENT_BEING;

    // What happened. from/to are the ordinals of whichever axis `axis` names.
    EventType trigger = EV_NONE;
    uint8_t from_state = 0;
    uint8_t to_state = 0;

    // Why it was allowed or refused.
    uint16_t guards_required = guard::NONE;
    uint16_t guards_satisfied = guard::NONE;

    FaultClass fault = FC_NONE;
    ForbiddenState forbidden = FS_NONE;

    std::string citation;    // CUR provision authorising or refusing
    std::string regulation;  // regulation id, when one drove the outcome
    std::string detail;      // human-readable note

    // Cross-references into the ENTITY-009/010 ledger, so a reader can move
    // between the transition trail and the canonical GEM records without
    // guessing. Empty when the record did not open a violation or impose a
    // sanction.
    std::string violation_id;
    std::string sanction_id;

    // Rendering helpers. `from_state`/`to_state` are only meaningful once you
    // know the axis, so these do the dispatch for you.
    const char* from_state_name() const;
    const char* to_state_name() const;
};

// Append-only audit trail.
class EventLog {
public:
    // capacity == 0 means unbounded: nothing is ever dropped. Any positive
    // capacity turns the log into a ring and starts counting drops.
    explicit EventLog(size_t capacity = 0);

    // The only mutator. Assigns record_seq and stamps wall_clock_utc.
    const LogRecord& append(LogRecord r);

    size_t size() const { return records_.size(); }
    uint64_t total_appended() const { return next_seq_; }
    uint64_t dropped() const { return dropped_; }
    bool bounded() const { return capacity_ != 0; }

    const std::deque<LogRecord>& records() const { return records_; }

    // Newest-first slice, for dashboards and the Godot inspector.
    std::vector<LogRecord> tail(size_t n) const;

    // Every record touching one entity, oldest first — the paper trail for a
    // single charter or citizen.
    std::vector<LogRecord> for_entity(const std::string& entity_id) const;

    // Counts by fault class, for the Annual FSM Monitoring Report
    // (PDDC-GOVERNANCE-MECHANICS §12.6).
    void fault_counts(uint32_t out[FC_COUNT]) const;

    // Export to the OTF-1 envelope already spoken by the simulator's web mirror
    // (shared/protocol/schema.json). One JSON object per record.
    std::string to_otf1_json(size_t max_records = 0) const;

    // Single record as an OTF-1 object, so callers can stream instead of
    // materialising the whole log.
    static std::string record_to_otf1_json(const LogRecord& r);

    // Deterministic digest over the logical content of the trail — everything
    // except wall_clock_utc. Two runs of the same event stream must produce the
    // same digest; the determinism test asserts exactly that.
    uint64_t content_digest() const;

    // Permitted only because a fresh simulator run is a fresh universe, not a
    // redaction. Resets sequence numbering too.
    void reset();

    // Injectable clock, so tests can pin wall_clock_utc and diff logs byte for
    // byte. Defaults to the system clock. Never consulted by the FSM.
    using ClockFn = int64_t (*)();
    void set_clock(ClockFn fn) { clock_ = fn; }

private:
    std::deque<LogRecord> records_;
    size_t capacity_ = 0;
    uint64_t next_seq_ = 0;
    uint64_t dropped_ = 0;
    ClockFn clock_ = nullptr;
};

}  // namespace cur

#endif  // CUR_EVENT_LOG_H
