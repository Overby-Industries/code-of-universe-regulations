CUR-FOUNDATION-007
Constitutional Data Model (CDM)

Document ID: CUR-FOUNDATION-007
Status: Draft v0.1
Authority Level: Foundation Document
Depends On: CUR-FOUNDATION-001 through CUR-FOUNDATION-006
Applies To: All Commonwealth Governance Systems

1. Purpose

The Constitutional Data Model (CDM) defines the canonical data structures used by the Aevoric Commonwealth.

The CDM provides:

Standardized storage formats
Interoperable governance records
Simulator compatibility
Audit consistency
Constitutional traceability

All Commonwealth systems shall store governance data using CDM-compliant structures.

2. Design Principles

The CDM shall be:

Constitutionally compliant
Auditable
Extensible
Versioned
Platform-independent
Human-readable
Machine-readable
3. Universal Record Format

All records shall contain:

{
  "id": "unique-id",
  "recordType": "entity-type",
  "createdAt": "2026-05-20T00:00:00Z",
  "updatedAt": "2026-05-20T00:00:00Z",
  "version": "1.0",
  "status": "active"
}
4. Citizen Record
CDM-001
{
  "id": "citizen-3941",
  "recordType": "citizen",
  "name": "Example Citizen",
  "sectorId": "sector-avia",
  "speciesType": "human",
  "rightsStatus": "active",
  "participationScore": 92,
  "reputationScore": 88,
  "trustIndex": 84,
  "influenceProfileId": "influence-22",
  "organizationMemberships": [],
  "violations": [],
  "createdAt": "",
  "updatedAt": ""
}
5. Silicon Citizen Record
CDM-002
{
  "id": "ai-001",
  "recordType": "silicon_citizen",
  "classification": "general_intelligence",
  "rightsStatus": "active",
  "transparencyStatus": "compliant",
  "participationScore": 100,
  "auditHistory": [],
  "createdAt": "",
  "updatedAt": ""
}
6. Organization Record
CDM-003
{
  "id": "org-22",
  "recordType": "organization",
  "name": "Aevoric Mining Cooperative",
  "sectorId": "sector-avia",
  "memberCount": 500,
  "assets": [],
  "influenceProfileId": "influence-55",
  "trustIndex": 87,
  "createdAt": "",
  "updatedAt": ""
}
7. Sector Record
CDM-004
{
  "id": "sector-avia",
  "recordType": "sector",
  "name": "Sector Avia",
  "population": 100000,
  "participationRate": 81,
  "resourceIds": [],
  "infrastructureIds": [],
  "createdAt": "",
  "updatedAt": ""
}
8. Institution Record
CDM-005
{
  "id": "institution-001",
  "recordType": "institution",
  "name": "General Assembly",
  "authorityLevel": "constitutional",
  "transparencyScore": 100,
  "auditStatus": "current",
  "captureRisk": 8,
  "createdAt": "",
  "updatedAt": ""
}
9. Proposal Record
CDM-006
{
  "id": "PROP-7842",
  "recordType": "proposal",
  "title": "Expand Mining Operations",
  "authorId": "citizen-3941",
  "status": "deliberation",
  "constitutionalStatus": "pending",
  "voteResults": {
    "for": 0,
    "against": 0,
    "abstain": 0
  },
  "createdAt": "",
  "updatedAt": ""
}
10. Vote Record
CDM-007
{
  "id": "vote-9911",
  "recordType": "vote",
  "proposalId": "PROP-7842",
  "voterId": "citizen-3941",
  "vote": "for",
  "reason": "Supports production expansion.",
  "timestamp": ""
}

Allowed vote values:

for
against
abstain
11. Audit Record
CDM-008
{
  "id": "audit-1001",
  "recordType": "audit",
  "targetEntity": "org-22",
  "reason": "Capture Risk Threshold Exceeded",
  "status": "active",
  "findings": [],
  "recommendations": [],
  "createdAt": "",
  "updatedAt": ""
}
12. Violation Record
CDM-009
{
  "id": "vio-22",
  "recordType": "violation",
  "entityId": "org-22",
  "severity": "constitutional",
  "status": "confirmed",
  "evidence": [],
  "appealable": true
}
13. Sanction Record
CDM-010
{
  "id": "sanction-44",
  "recordType": "sanction",
  "violationId": "vio-22",
  "type": "fine",
  "durationDays": 30,
  "status": "active"
}
14. Appeal Record
CDM-011
{
  "id": "appeal-22",
  "recordType": "appeal",
  "targetDecision": "sanction-44",
  "appellant": "citizen-3941",
  "status": "under_review"
}
15. Resource Record
CDM-012
{
  "id": "resource-44",
  "recordType": "resource",
  "resourceType": "nickel",
  "quantity": 1200,
  "location": "asteroid-belt-sector-3",
  "owner": "commons-reserve",
  "strategicImportance": "high"
}
16. Infrastructure Record
CDM-013
{
  "id": "infra-001",
  "recordType": "infrastructure",
  "name": "Keefe Station",
  "category": "orbital_habitat",
  "capacity": 50000,
  "operationalStatus": "active"
}
17. Influence Profile Record
CDM-014
{
  "id": "influence-22",
  "recordType": "influence_profile",
  "politicalInfluence": 12,
  "economicInfluence": 8,
  "informationInfluence": 15,
  "socialInfluence": 11,
  "organizationalInfluence": 10
}
18. Capture Risk Record
CDM-015
{
  "id": "cri-2026-05-20",
  "recordType": "capture_risk",
  "overallCRI": 27,
  "economicIndex": 20,
  "influenceIndex": 33,
  "institutionalIndex": 18,
  "participationIndex": 25,
  "transparencyIndex": 10,
  "resourceDependencyIndex": 21
}
19. Constitutional Event Record
CDM-016
{
  "id": "evt-000001",
  "recordType": "constitutional_event",
  "eventType": "proposal_submitted",
  "actorId": "citizen-3941",
  "targetId": "PROP-7842",
  "timestamp": "",
  "payload": {}
}
20. FSM State Record
CDM-017
{
  "id": "fsm-state",
  "recordType": "fsm_state",
  "currentState": "NormalOperation",
  "previousState": "RecoveryReview",
  "transitionReason": "Audit Completed",
  "enteredAt": ""
}

Allowed states:

NormalOperation
Deliberation
Voting
ConstitutionalReview
Implementation
OutcomeMonitoring
AuditInvestigation
ProtectedMode
RecoveryReview
21. Relationship Model
Citizen
 ├── Votes
 ├── Proposals
 ├── Appeals
 └── Influence Profile

Organization
 ├── Members
 ├── Resources
 ├── Infrastructure
 └── Influence Profile

Institution
 ├── Audits
 ├── Decisions
 ├── Events
 └── Violations

Sector
 ├── Citizens
 ├── Organizations
 ├── Resources
 └── Infrastructure
22. Simulator Persistence Requirements

The Aevoria Simulator shall:

Store all governance actions as CDM records.
Preserve immutable constitutional event history.
Support historical replay.
Support audit reconstruction.
Support FSM state reconstruction.
23. Constitutional Principle

All governance information shall exist as constitutionally recognized records.

No governance action may exist without corresponding CDM records.

24. Guiding Principle

The Constitutional Data Model serves as the Commonwealth's institutional memory.

By standardizing all governance records, the CDM ensures that the Commonwealth remains transparent, auditable, reproducible, and resistant to historical revision or institutional capture.