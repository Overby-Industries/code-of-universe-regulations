# CUR-FOUNDATION-008

### Constitutional Query Language (CQL)

- **Document ID:** CUR-FOUNDATION-008
- **Version:** 1.1.0-Official-Evergreen
- **Status:** Draft-Official-Evergreen
- **Authority Level:** Foundation Document
- **Depends On:** CUR-FOUNDATION-001 through CUR-FOUNDATION-007
- **Applies To:** All Commonwealth Governance Systems

## 1. Purpose

The Constitutional Query Language (CQL) defines the standardized method for retrieving, analyzing, auditing, and reviewing constitutional information.

CQL provides:

- Governance transparency
- Constitutional auditing
- Historical reconstruction
- Democratic accountability
- Simulator observability
- Capture risk analysis

All constitutional data access shall occur through CQL-compliant interfaces.

## 2. Core Principles

CQL shall be:

- Readable by citizens
- Machine executable
- Auditable
- Rights-aware
- Immutable-history compatible
- Platform independent

## 3. Constitutional Query Model

CQL operates against CDM records.

Supported record types include:

- Citizen
- Organization
- Sector
- Institution
- Proposal
- Vote
- Audit
- Violation
- Sanction
- Appeal
- Resource
- Infrastructure
- InfluenceProfile
- CaptureRisk
- ConstitutionalEvent
- FSMState

## 4. Query Structure

Basic syntax:

```text
SELECT <record_type>
WHERE <condition>
```

Example:

```text
SELECT Proposal
WHERE status = "voting"
```

## 5. Citizen Queries

Retrieve citizen information.

Example:

```text
SELECT Citizen
WHERE id = "citizen-3941"
```

Result:

```json
{
  "id": "citizen-3941",
  "sector": "sector-avia",
  "participationScore": 92
}
```

## 6. Proposal Queries

Find proposals.

Example:

```text
SELECT Proposal
WHERE authorId = "citizen-3941"
```

Example:

```text
SELECT Proposal
WHERE status = "constitutional_review"
```

## 7. Vote Queries

Retrieve voting activity.

Example:

```text
SELECT Vote
WHERE proposalId = "PROP-7842"
```

Example:

```text
SELECT Vote
WHERE voterId = "citizen-3941"
```

## 8. Event Queries

Retrieve constitutional events.

Example:

```text
SELECT ConstitutionalEvent
WHERE eventType = "proposal_submitted"
```

Example:

```text
SELECT ConstitutionalEvent
WHERE actorId = "citizen-3941"
```

## 9. Audit Queries

Retrieve audits.

Example:

```text
SELECT Audit
WHERE status = "active"
```

Example:

```text
SELECT Audit
WHERE targetEntity = "org-22"
```

## 10. CRI Queries

Retrieve Capture Risk information.

Example:

```text
SELECT CaptureRisk
WHERE overallCRI > 50
```

Example:

```text
SELECT CaptureRisk
WHERE transparencyIndex < 20
```

## 11. FSM Queries

Retrieve governance state information.

Example:

```text
SELECT FSMState
WHERE currentState = "ProtectedMode"
```

Example:

```text
SELECT FSMState
ORDER BY enteredAt DESC
LIMIT 10
```

## 12. Filtering

Supported operators:

- =
- !=
- >
- <
- >=
- <=
- IN

### NOT IN

Example:

```text
SELECT Proposal
WHERE status IN (
    "voting",
    "constitutional_review"
)
```

## 13. Sorting

Example:

```text
SELECT Proposal
ORDER BY createdAt DESC
```

## 14. Limits

Example:

```text
SELECT Proposal
ORDER BY createdAt DESC
LIMIT 50
```

## 15. Aggregation Functions

Supported functions:

```text
COUNT()
SUM()
AVG()
MIN()
MAX()
```

Example:

```text
COUNT Vote
WHERE proposalId = "PROP-7842"
```

Example:

```text
AVG Citizen.participationScore
```

## 16. Transparency Queries

Public governance visibility.

Example:

```text
SHOW PUBLIC RECORDS
FOR Proposal
```

Example:

```text
SHOW AUDIT TRAIL
FOR Proposal "PROP-7842"
```

## 17. Historical Replay Queries

Retrieve historical governance actions.

Example:

```text
REPLAY Proposal "PROP-7842"
```

Returns:

- Created
- Reviewed
- Voted
- Approved
- Implemented
- Monitored

## 18. Constitutional Trace Queries

```text
Trace governance decisions.
```

Example:

```text
TRACE Proposal "PROP-7842"
```

Returns:

- Proposal
- Votes
- Review
- FSM Decisions
- Outcome

## 19. Relationship Queries

Retrieve connected entities.

Example:

```text
SELECT Citizen
RELATED TO Organization "org-22"
```

Example:

```text
SELECT Proposal
RELATED TO Citizen "citizen-3941"
```

## 20. Influence Analysis Queries

Retrieve influence concentration.

Example:

```text
ANALYZE Influence
WHERE organizationalInfluence > 50
```

## 21. Capture Risk Analysis Queries

Example:

```text
ANALYZE CaptureRisk
FOR Sector "sector-avia"
```

Result:

```json
{
  "overallCRI": 27,
  "riskLevel": "Observation"
}
```

## 22. Rights-Aware Query Enforcement

CQL shall enforce constitutional rights.

Queries may be:

- Public
- Restricted
- Protected
- Confidential

Rights for All Life protections supersede query requests.

## 23. Auditability Requirements

All CQL executions shall generate:

```json
{
  "queryId": "qry-001",
  "timestamp": "2026-05-20T12:00:00Z",
  "requestor": "citizen-3941",
  "queryType": "audit_lookup"
}
```

All query activity shall be auditable.

## 24. Simulator Integration

The Aevoria Simulator shall use CQL for:

- State inspection
- Governance dashboards
- Historical playback
- Event visualization
- Capture-risk monitoring

Direct database access is prohibited.

## 25. Constitutional Principle

Citizens have the right to inspect the operation of their government subject to constitutional privacy protections.

Transparency is the default state.

Secrecy requires explicit constitutional justification.

## 26. Guiding Principle

The Constitutional Query Language transforms governance data into civic knowledge.

By providing a common language for inspection, analysis, and accountability, CQL enables citizens, institutions, and future intelligences to understand and verify the operation of the Commonwealth.
