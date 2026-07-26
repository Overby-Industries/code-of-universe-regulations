#include "cur/cur_capture_index.h"

namespace cur {

const char* to_string(CaptureRiskBand b) {
    switch (b) {
        case CRB_STABLE: return "Stable";
        case CRB_OBSERVATION: return "Observation";
        case CRB_ELEVATED: return "Elevated Risk";
        case CRB_HIGH: return "High Risk";
        case CRB_CRITICAL: return "Critical";
        case CRB_COUNT: break;
    }
    return "Unknown";
}

namespace {

double clamp_0_100(double v) {
    if (v < 0.0) return 0.0;
    if (v > 100.0) return 100.0;
    return v;
}

// Health indices are inverted so that "declining III raises CRI"
// (FOUNDATION-003 §7-§9) actually holds. See the header for why.
double invert(double health) { return 100.0 - clamp_0_100(health); }

}  // namespace

double CaptureRiskModel::compute(const CaptureRiskInputs& in) const {
    const double risk = clamp_0_100(in.eci) * weights_.eci +
                        clamp_0_100(in.ici) * weights_.ici +
                        invert(in.iii) * weights_.iii +
                        invert(in.dpi) * weights_.dpi +
                        invert(in.thi) * weights_.thi +
                        clamp_0_100(in.rdi) * weights_.rdi;

    // FOUNDATION-003 §11: "Normalized to 100." The weights already sum to 1.0,
    // so the weighted mean is on 0-100 and only needs clamping against
    // caller-supplied weights that do not sum to one.
    return clamp_0_100(risk);
}

double CaptureRiskModel::literal_formula(const CaptureRiskInputs& in) const {
    // §11 exactly as printed, health indices NOT inverted. Kept only so the two
    // readings can be compared side by side; do not drive decisions from it.
    const double risk = clamp_0_100(in.eci) * weights_.eci +
                        clamp_0_100(in.ici) * weights_.ici +
                        clamp_0_100(in.iii) * weights_.iii +
                        clamp_0_100(in.dpi) * weights_.dpi +
                        clamp_0_100(in.thi) * weights_.thi +
                        clamp_0_100(in.rdi) * weights_.rdi;
    return clamp_0_100(risk);
}

CaptureRiskBand CaptureRiskModel::band(double cri) {
    // FOUNDATION-003 §4 bands.
    if (cri < 20.0) return CRB_STABLE;
    if (cri < 40.0) return CRB_OBSERVATION;
    if (cri < 60.0) return CRB_ELEVATED;
    if (cri < 80.0) return CRB_HIGH;
    return CRB_CRITICAL;
}

CaptureRiskModel::Response CaptureRiskModel::responses(double cri) {
    // FOUNDATION-003 §12. Thresholds are cumulative: a Critical score carries
    // every response from every lower band as well.
    Response r;
    if (cri >= 20.0) {
        r.additional_monitoring = true;
        r.public_reporting = true;
    }
    if (cri >= 40.0) {
        r.mandatory_audit = true;
        r.transparency_review = true;
    }
    if (cri >= 60.0) {
        r.notify_constitutional_court = true;
        r.sector_review_hearings = true;
    }
    if (cri >= 80.0) {
        r.automatic_constitutional_review = true;
        r.protected_mode_evaluation = true;
    }
    return r;
}

bool cri_escalates_to_protected_mode(double cri,
                                     bool constitutional_violation_confirmed) {
    // FOUNDATION-003 §13: CRI alone never activates Protected Mode. Both
    // conditions are required, and §15 is explicit that a high score is a
    // diagnostic, not a finding of guilt.
    return cri >= 80.0 && constitutional_violation_confirmed;
}

// ---------------------------------------------------------------------------
// Vital Continuity Index
// ---------------------------------------------------------------------------

const char* to_string(VitalContinuityBand b) {
    switch (b) {
        case VCB_CRITICAL: return "Critical";
        case VCB_HIGH_RISK: return "High Risk";
        case VCB_ELEVATED: return "Elevated Risk";
        case VCB_OBSERVATION: return "Observation";
        case VCB_STABLE: return "Stable";
        case VCB_COUNT: break;
    }
    return "Unknown";
}

double VitalContinuityModel::compute(const VitalContinuityInputs& in) const {
    // No inversion anywhere in here. Every VCI input is already oriented so
    // that higher is better, and so is the output. This is the opposite
    // convention to CaptureRiskModel::compute() by design, not by oversight.
    const double vci =
        clamp_0_100(in.biological_life_support) * weights_.biological_life_support +
        clamp_0_100(in.silicon_life_support) * weights_.silicon_life_support +
        clamp_0_100(in.infrastructure_resilience) *
            weights_.infrastructure_resilience +
        clamp_0_100(in.accessibility) * weights_.accessibility;
    return clamp_0_100(vci);
}

VitalContinuityBand VitalContinuityModel::band(double vci) {
    // FOUNDATION-003 §11 "VCI Scale". Ascending score = ascending health, so
    // the comparisons run the other way round from CaptureRiskModel::band().
    if (vci < 20.0) return VCB_CRITICAL;
    if (vci < 40.0) return VCB_HIGH_RISK;
    if (vci < 60.0) return VCB_ELEVATED;
    if (vci < 80.0) return VCB_OBSERVATION;
    return VCB_STABLE;
}

VitalContinuityModel::Response VitalContinuityModel::responses(double vci) {
    // §11 "VCI Response Thresholds". Cumulative downward: a Critical score
    // carries every response from every healthier band as well.
    Response r;
    if (vci < 80.0) {
        r.additional_monitoring = true;
        r.capacity_planning_review = true;
    }
    if (vci < 60.0) {
        r.continuity_audit = true;
        r.reserve_verification = true;
    }
    if (vci < 40.0) {
        r.mandatory_continuity_review = true;
        r.activate_resource_reserves = true;
        r.public_continuity_reporting = true;
    }
    if (vci < 20.0) {
        r.generate_constitutional_event = true;
        r.immediate_corrective_action = true;
        r.root_cause_analysis = true;
    }
    return r;
}

bool vci_requires_continuity_response(double vci) {
    return vci < 20.0;
}

}  // namespace cur
