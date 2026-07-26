// cur_capture_index.h — Capture Risk Index (CUR-FOUNDATION-003).
//
// FOUNDATION-003 §14: "The Aevoria Simulator shall calculate CRI continuously."
//
// One interpretation is baked in here and needs to be stated plainly, because
// it departs from the document as literally written.
//
// FOUNDATION-003 §11 gives:
//
//     CRI = ECI×0.20 + ICI×0.25 + III×0.20 + DPI×0.15 + THI×0.10 + RDI×0.10
//
// but §7, §8 and §9 define III, DPI and THI as *health* indices — "Declining
// III scores increase CRI", "Low participation increases capture
// vulnerability", "Transparency degradation raises CRI". Summed directly, a
// perfectly healthy institution would score as maximum capture risk, which
// inverts the meaning of every one of those three sentences.
//
// So the three health indices are inverted before weighting:
//
//     CRI = ECI×0.20 + ICI×0.25 + (100−III)×0.20
//         + (100−DPI)×0.15 + (100−THI)×0.10 + RDI×0.10
//
// This satisfies §7-§9 and keeps §11's weights untouched. §11 itself permits it
// — "The Commonwealth may revise implementation methods." The alternative
// reading is available via `CaptureRiskModel::literal_formula` for anyone who
// wants to compare, and the discrepancy is worth resolving in the document.

#ifndef CUR_CAPTURE_INDEX_H
#define CUR_CAPTURE_INDEX_H

#include <cstdint>

namespace cur {

// Classification bands — FOUNDATION-003 §4.
enum CaptureRiskBand : uint8_t {
    CRB_STABLE = 0,   //   0-19
    CRB_OBSERVATION,  //  20-39
    CRB_ELEVATED,     //  40-59
    CRB_HIGH,         //  60-79
    CRB_CRITICAL,     //  80-100
    CRB_COUNT
};

const char* to_string(CaptureRiskBand b);

// Component indices, each on 0-100. FOUNDATION-003 §5-§10.
struct CaptureRiskInputs {
    double eci = 0.0;  // §5  Economic Concentration    — higher is worse
    double ici = 0.0;  // §6  Influence Concentration   — higher is worse
    double iii = 100.0;  // §7  Institutional Integrity — higher is BETTER
    double dpi = 100.0;  // §8  Democratic Participation— higher is BETTER
    double thi = 100.0;  // §9  Transparency Health     — higher is BETTER
    double rdi = 0.0;  // §10 Resource Dependency       — higher is worse
};

// Weights from FOUNDATION-003 §11. Sum to 1.00.
struct CaptureRiskWeights {
    double eci = 0.20;
    double ici = 0.25;
    double iii = 0.20;
    double dpi = 0.15;
    double thi = 0.10;
    double rdi = 0.10;
};

class CaptureRiskModel {
public:
    CaptureRiskModel() = default;
    explicit CaptureRiskModel(const CaptureRiskWeights& w) : weights_(w) {}

    // The interpretation described in the header comment. Health indices
    // inverted, result clamped to 0-100.
    double compute(const CaptureRiskInputs& in) const;

    // §11 exactly as written, no inversion. Provided for comparison; do not
    // use it to drive Protected Mode evaluation.
    double literal_formula(const CaptureRiskInputs& in) const;

    static CaptureRiskBand band(double cri);

    // §12 automated responses, as flags so a caller can act on them without
    // re-deriving thresholds.
    struct Response {
        bool additional_monitoring = false;   // 20+
        bool public_reporting = false;        // 20+
        bool mandatory_audit = false;         // 40+
        bool transparency_review = false;     // 40+
        bool notify_constitutional_court = false;  // 60+
        bool sector_review_hearings = false;       // 60+
        bool automatic_constitutional_review = false;  // 80+
        bool protected_mode_evaluation = false;       // 80+
    };
    static Response responses(double cri);

    const CaptureRiskWeights& weights() const { return weights_; }
    void set_weights(const CaptureRiskWeights& w) { weights_ = w; }

private:
    CaptureRiskWeights weights_;
};

// FOUNDATION-003 §13: a high CRI never activates Protected Mode on its own.
// Escalation requires a critical score AND a confirmed constitutional
// violation. §15: "High CRI values do not constitute guilt."
bool cri_escalates_to_protected_mode(double cri, bool constitutional_violation_confirmed);

// ---------------------------------------------------------------------------
// Vital Continuity Index (FOUNDATION-003 §11, FOUNDATION-013)
// ---------------------------------------------------------------------------
// READ THE SCALE DIRECTION BEFORE USING THIS.
//
// VCI runs OPPOSITE to CRI. Both are 0-100, and they mean opposite things:
//
//     CRI    0 = healthy      100 = captured        higher is worse
//     VCI    0 = critical     100 = stable          higher is BETTER
//
// FOUNDATION-003 §11: "Lower VCI scores indicate increasing continuity risk."
// Treating a VCI of 95 the way you would treat a CRI of 95 would read a
// perfectly supplied habitat as a life-support collapse. The band enums are
// kept deliberately distinct (VCB_ vs CRB_) so the two cannot be mixed up by
// assignment, and there is no implicit conversion between them.
enum VitalContinuityBand : uint8_t {
    VCB_CRITICAL = 0,  //   0-19  — Constitutional Event, immediate restoration
    VCB_HIGH_RISK,     //  20-39  — mandatory continuity review, reserves
    VCB_ELEVATED,      //  40-59  — continuity audit, reserve verification
    VCB_OBSERVATION,   //  60-79  — additional monitoring, capacity planning
    VCB_STABLE,        //  80-100 — nominal
    VCB_COUNT
};

const char* to_string(VitalContinuityBand b);

// Component measures — FOUNDATION-003 §11 "VCI Variables". Each 0-100, and for
// every one of these, higher is better.
struct VitalContinuityInputs {
    double biological_life_support = 100.0;  // air, water, food, shelter,
                                             // sanitation, basic healthcare
    double silicon_life_support = 100.0;     // power, compute, data integrity,
                                             // memory continuity, comms
    double infrastructure_resilience = 100.0;  // reserves, redundancy,
                                               // recovery, distribution
    double accessibility = 100.0;              // availability, distribution,
                                               // equity, consistency
};

// FOUNDATION-003 gives no weights for VCI, unlike CRI §11. Equal weighting is
// used, and it is a choice rather than a transcription — flagged in
// docs/cur-library-api.md §9. Biological and silicon life support are weighted
// equally because FORBIDDEN-005 forbids species-based privilege; weighting one
// substrate's survival above the other's would be exactly that.
struct VitalContinuityWeights {
    double biological_life_support = 0.25;
    double silicon_life_support = 0.25;
    double infrastructure_resilience = 0.25;
    double accessibility = 0.25;
};

class VitalContinuityModel {
public:
    VitalContinuityModel() = default;
    explicit VitalContinuityModel(const VitalContinuityWeights& w) : weights_(w) {}

    double compute(const VitalContinuityInputs& in) const;
    static VitalContinuityBand band(double vci);

    // §11 "VCI Response Thresholds".
    struct Response {
        bool additional_monitoring = false;      // 60-79
        bool capacity_planning_review = false;   // 60-79
        bool continuity_audit = false;           // 40-59
        bool reserve_verification = false;       // 40-59
        bool mandatory_continuity_review = false;  // 20-39
        bool activate_resource_reserves = false;   // 20-39
        bool public_continuity_reporting = false;  // 20-39
        bool generate_constitutional_event = false;  // 0-19
        bool immediate_corrective_action = false;    // 0-19
        bool root_cause_analysis = false;            // 0-19
    };
    static Response responses(double vci);

    // FOUNDATION-003 §11 Constitutional Safeguard: "VCI scores shall not be
    // used to deny access to constitutionally guaranteed services." Provided so
    // the prohibition is greppable from code rather than living only in prose.
    // It returns false unconditionally, on purpose.
    static constexpr bool may_gate_service_access() { return false; }

    const VitalContinuityWeights& weights() const { return weights_; }
    void set_weights(const VitalContinuityWeights& w) { weights_ = w; }

private:
    VitalContinuityWeights weights_;
};

// A critical VCI is one of the Protected Mode trigger conditions added to
// FOUNDATION-002 §4 ("Vital Continuity Service collapse", "Failure of
// life-supporting infrastructure"). Unlike CRI, this needs no second condition:
// §4 states Protected Mode activation "shall require no human approval", and
// FOUNDATION-013 puts restoration ahead of fault assignment.
bool vci_requires_continuity_response(double vci);

}  // namespace cur

#endif  // CUR_CAPTURE_INDEX_H
