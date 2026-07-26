#include "cur/cur_event_log.h"

#include <cstdio>
#include <ctime>
#include <sstream>

namespace cur {

const char* to_string(RecordKind k) {
    switch (k) {
        case REC_TRANSITION_ACCEPTED: return "transition_accepted";
        case REC_TRANSITION_REFUSED: return "transition_refused";
        case REC_FAULT_DECLARED: return "fault_declared";
        case REC_REVERTED_TO_SAFE_STATE: return "reverted_to_safe_state";
        case REC_PROTECTED_MODE_ENTERED: return "protected_mode_entered";
        case REC_PROTECTED_MODE_EXITED: return "protected_mode_exited";
        case REC_CERTIFICATION_GRANTED: return "certification_granted";
        case REC_CERTIFICATION_REVOKED: return "certification_revoked";
        case REC_AMENDMENT_ACCEPTED: return "amendment_accepted";
        case REC_AMENDMENT_REFUSED: return "amendment_refused";
        case REC_KIND_COUNT: break;
    }
    return "unknown";
}

const char* to_string(RecordAxis a) {
    switch (a) {
        case REC_AXIS_NONE: return "none";
        case REC_AXIS_CONSTITUTIONAL: return "constitutional";
        case REC_AXIS_GOVERNANCE: return "governance";
        case REC_AXIS_COMPLIANCE: return "compliance";
        case REC_AXIS_COUNT: break;
    }
    return "unknown";
}

namespace {

const char* state_name_for(RecordAxis axis, uint8_t ordinal) {
    switch (axis) {
        case REC_AXIS_CONSTITUTIONAL:
            return ordinal < CS_COUNT
                       ? to_string(static_cast<ConstitutionalState>(ordinal))
                       : "UNKNOWN";
        case REC_AXIS_GOVERNANCE:
            return ordinal < GS_COUNT
                       ? to_string(static_cast<GovernanceState>(ordinal))
                       : "UNKNOWN";
        case REC_AXIS_COMPLIANCE:
            return ordinal < KS_COUNT
                       ? to_string(static_cast<ComplianceState>(ordinal))
                       : "UNKNOWN";
        default:
            return "";
    }
}

// Minimal JSON string escaping. Control characters go out as \uXXXX so the
// audit trail stays parseable even if a description carries something odd.
void append_escaped(std::ostringstream& os, const std::string& s) {
    for (char ch : s) {
        unsigned char c = static_cast<unsigned char>(ch);
        switch (c) {
            case '"': os << "\\\""; break;
            case '\\': os << "\\\\"; break;
            case '\b': os << "\\b"; break;
            case '\f': os << "\\f"; break;
            case '\n': os << "\\n"; break;
            case '\r': os << "\\r"; break;
            case '\t': os << "\\t"; break;
            default:
                if (c < 0x20) {
                    char buf[8];
                    std::snprintf(buf, sizeof(buf), "\\u%04x", c);
                    os << buf;
                } else {
                    os << ch;
                }
        }
    }
}

void kv(std::ostringstream& os, const char* key, const std::string& value,
        bool& first) {
    if (!first) os << ",";
    first = false;
    os << "\"" << key << "\":\"";
    append_escaped(os, value);
    os << "\"";
}

void kv_raw(std::ostringstream& os, const char* key, const std::string& value,
            bool& first) {
    if (!first) os << ",";
    first = false;
    os << "\"" << key << "\":" << value;
}

std::string u64(uint64_t v) {
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%llu", static_cast<unsigned long long>(v));
    return std::string(buf);
}

// RFC 3339 UTC, to match the "timestamp" field of shared/protocol/schema.json.
std::string iso8601_utc(int64_t unix_seconds) {
    std::time_t t = static_cast<std::time_t>(unix_seconds);
    std::tm tm_utc{};
#if defined(_WIN32)
    gmtime_s(&tm_utc, &t);
#else
    gmtime_r(&t, &tm_utc);
#endif
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", &tm_utc);
    return std::string(buf);
}

int64_t system_clock_seconds() {
    return static_cast<int64_t>(std::time(nullptr));
}

}  // namespace

const char* LogRecord::from_state_name() const {
    return state_name_for(axis, from_state);
}

const char* LogRecord::to_state_name() const {
    return state_name_for(axis, to_state);
}

EventLog::EventLog(size_t capacity) : capacity_(capacity) {}

const LogRecord& EventLog::append(LogRecord r) {
    r.record_seq = next_seq_++;
    r.wall_clock_utc = clock_ != nullptr ? clock_() : system_clock_seconds();

    records_.push_back(std::move(r));

    // Bounded mode drops the oldest record rather than refusing the newest —
    // refusing to record would violate FOUNDATION-002 §9. The drop is counted
    // and surfaced on export so nobody mistakes a truncated trail for a
    // complete one.
    if (capacity_ != 0) {
        while (records_.size() > capacity_) {
            records_.pop_front();
            ++dropped_;
        }
    }
    return records_.back();
}

std::vector<LogRecord> EventLog::tail(size_t n) const {
    std::vector<LogRecord> out;
    if (n == 0) return out;
    out.reserve(n < records_.size() ? n : records_.size());
    size_t taken = 0;
    for (auto it = records_.rbegin(); it != records_.rend() && taken < n;
         ++it, ++taken) {
        out.push_back(*it);
    }
    return out;
}

std::vector<LogRecord> EventLog::for_entity(const std::string& entity_id) const {
    std::vector<LogRecord> out;
    for (const LogRecord& r : records_) {
        if (r.entity_id == entity_id) out.push_back(r);
    }
    return out;
}

void EventLog::fault_counts(uint32_t out[FC_COUNT]) const {
    for (int i = 0; i < FC_COUNT; ++i) out[i] = 0;
    for (const LogRecord& r : records_) {
        if (r.fault < FC_COUNT) ++out[r.fault];
    }
}

std::string EventLog::record_to_otf1_json(const LogRecord& r) {
    std::ostringstream os;
    os << "{";
    bool first = true;

    // Envelope — shared/protocol/schema.json §2.
    kv(os, "type", to_string(r.kind), first);
    kv(os, "version", "1.0", first);
    kv(os, "timestamp", iso8601_utc(r.wall_clock_utc), first);
    kv(os, "source", "cur-fsm", first);

    if (!first) os << ",";
    os << "\"payload\":{";
    bool pfirst = true;

    kv_raw(os, "recordSeq", u64(r.record_seq), pfirst);
    kv_raw(os, "tick", u64(r.tick), pfirst);
    kv_raw(os, "eventSequence", u64(r.event_sequence), pfirst);
    kv(os, "axis", to_string(r.axis), pfirst);
    kv(os, "entityId", r.entity_id, pfirst);
    kv(os, "subjectClass", to_string(r.subject_class), pfirst);
    // CTAF §14 Actor / CAPS §17 Sender.
    kv(os, "actorId", r.actor_id, pfirst);
    kv(os, "trigger", to_string(r.trigger), pfirst);

    // CTAF §14 FSM State — the full three-axis vector, not just the one axis
    // this record happens to describe.
    if (!pfirst) os << ",";
    pfirst = false;
    os << "\"fsmState\":{";
    os << "\"constitutional\":\"" << to_string(r.constitutional_after) << "\"";
    os << ",\"governance\":\"" << to_string(r.governance_after) << "\"";
    os << ",\"compliance\":\"" << to_string(r.compliance_after) << "\"";
    os << "}";

    if (r.axis != REC_AXIS_NONE) {
        kv(os, "fromState", r.from_state_name(), pfirst);
        kv(os, "toState", r.to_state_name(), pfirst);
    }

    char gbuf[256];
    describe_guards(r.guards_required, gbuf, sizeof(gbuf));
    kv(os, "guardsRequired", gbuf, pfirst);
    describe_guards(r.guards_satisfied, gbuf, sizeof(gbuf));
    kv(os, "guardsSatisfied", gbuf, pfirst);

    kv(os, "faultClass", to_string(r.fault), pfirst);
    if (r.forbidden != FS_NONE) {
        kv(os, "forbiddenTransition", to_string(r.forbidden), pfirst);
    }
    if (!r.citation.empty()) kv(os, "citation", r.citation, pfirst);
    if (!r.regulation.empty()) kv(os, "regulation", r.regulation, pfirst);
    if (!r.violation_id.empty()) kv(os, "violationId", r.violation_id, pfirst);
    if (!r.sanction_id.empty()) kv(os, "sanctionId", r.sanction_id, pfirst);
    if (!r.detail.empty()) kv(os, "detail", r.detail, pfirst);

    os << "}}";
    return os.str();
}

std::string EventLog::to_otf1_json(size_t max_records) const {
    std::ostringstream os;
    os << "{\"curAuditTrail\":{";
    os << "\"totalAppended\":" << next_seq_;
    os << ",\"retained\":" << records_.size();
    os << ",\"dropped\":" << dropped_;
    os << ",\"complete\":" << (dropped_ == 0 ? "true" : "false");
    os << ",\"contentDigest\":\"" << u64(content_digest()) << "\"";
    os << ",\"records\":[";

    size_t emitted = 0;
    for (const LogRecord& r : records_) {
        if (max_records != 0 && emitted >= max_records) break;
        if (emitted != 0) os << ",";
        os << record_to_otf1_json(r);
        ++emitted;
    }

    os << "]}}";
    return os.str();
}

uint64_t EventLog::content_digest() const {
    // FNV-1a over the logical content only. wall_clock_utc is deliberately
    // excluded: it is the one field that legitimately differs between two
    // identical replays, and folding it in would make the digest useless for
    // exactly the thing it exists to check.
    uint64_t h = 1469598103934665603ull;
    const uint64_t prime = 1099511628211ull;

    auto mix_u64 = [&](uint64_t v) {
        for (int i = 0; i < 8; ++i) {
            h ^= static_cast<uint8_t>(v >> (i * 8));
            h *= prime;
        }
    };
    auto mix_str = [&](const std::string& s) {
        for (char c : s) {
            h ^= static_cast<uint8_t>(c);
            h *= prime;
        }
        h ^= 0xFF;  // terminator, so "ab"+"c" and "a"+"bc" differ
        h *= prime;
    };

    for (const LogRecord& r : records_) {
        mix_u64(r.record_seq);
        mix_u64(static_cast<uint64_t>(r.kind));
        mix_u64(static_cast<uint64_t>(r.axis));
        mix_u64(r.tick);
        mix_u64(r.event_sequence);
        mix_u64(static_cast<uint64_t>(r.entity));
        mix_str(r.entity_id);
        mix_u64(static_cast<uint64_t>(r.subject_class));
        mix_u64(static_cast<uint64_t>(r.actor));
        mix_str(r.actor_id);
        mix_u64(static_cast<uint64_t>(r.constitutional_after));
        mix_u64(static_cast<uint64_t>(r.governance_after));
        mix_u64(static_cast<uint64_t>(r.compliance_after));
        mix_u64(static_cast<uint64_t>(r.trigger));
        mix_u64(static_cast<uint64_t>(r.from_state));
        mix_u64(static_cast<uint64_t>(r.to_state));
        mix_u64(static_cast<uint64_t>(r.guards_required));
        mix_u64(static_cast<uint64_t>(r.guards_satisfied));
        mix_u64(static_cast<uint64_t>(r.fault));
        mix_u64(static_cast<uint64_t>(r.forbidden));
        mix_str(r.citation);
        mix_str(r.regulation);
        mix_str(r.violation_id);
        mix_str(r.sanction_id);
        mix_str(r.detail);
    }
    return h;
}

void EventLog::reset() {
    records_.clear();
    next_seq_ = 0;
    dropped_ = 0;
}

}  // namespace cur
