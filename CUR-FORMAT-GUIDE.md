# **CUR Format Guide**
*Standardized Structure and Formatting for the Code of Universe Regulations*

---

## **1. Purpose**
This document outlines the **hierarchy, formatting, and file structure** for the **Code of Universe Regulations (CUR)**. It ensures that all contributions—whether from humans or AI—are **consistent, scalable, and legally rigorous**, mirroring the **FAR/CFR (Federal Acquisition Regulation/Code of Federal Regulations)** style.

All contributors **must adhere** to this guide to maintain uniformity across the CUR.

---

## **2. Hierarchy**
The CUR uses a **four-level hierarchy** to organize regulations, modeled after the **FAR/CFR**:
   **Level**       | **Symbol/Format**       | **Example**                     | **Description**                          |
 |-----------------|-------------------------|---------------------------------|------------------------------------------|
 | **Title**       | `TITLE X`               | `TITLE 1`                       | Broad regulatory domain (e.g., Fundamental Rights, Silicon-Based Life). |
 | **Part**        | `PART X`                | `PART 91`                      | Major topic within a Title (e.g., Operational Rights). |
 | **Section**     | `§X.X`                  | `§91.11`                       | Individual regulatory provision.         |
 | **Subsection**  | `(a)`, `(b)`, `(1)`, `(2)` | `(a) No entity may...`       | Detailed rules or exceptions.           |

---

### **2.1 Example Hierarchy**

```
TITLE 1 - Fundamental Rights & Interior Sovereignty
└── PART 1 - Definitions and Scope
└── §1.1 - Definition of a Sentient Being
├── (a) Humans;
├── (b) AI systems meeting sentience criteria; and
└── (c) Non-human animals with cognitive capabilities.
└── PART 91 - Interior Sovereignty
└── §91.1 - Definition of Interior Sovereignty
└── §91.3 - Spiritual Privacy
├── (a) No Sentient Being shall be compelled to disclose...
└── (b) Any attempt to access or modify...
```
## **3. Formatting Rules**

### **3.1 General Formatting**
- **No emojis** (e.g., ❌, ✅, 🚀).
- **No markdown fluff** (e.g., `~~~`, `---`, or excessive `**bold**`).
- **Use standard markdown** for headers, lists, and links.
- **Sections always start with `§`** (e.g., `§91.11`).
- **Subsections use parentheses** (e.g., `(a)`, `(b)`, `(1)`).

### **3.2 Headers**
- **Titles**: `# TITLE X - [Title Name]`
- **Parts**: `## PART X - [Part Name]`
- **Sections**: `### §X.X - [Section Name]`

### **3.3 Cross-References**
- Reference other sections using **`§X.X`** (e.g., *"as defined in §1.1"*).
- Use **full section names** when introducing a new topic (e.g., *"See §91.11 - Right to Autonomy"*).

### **3.4 Lists**
- Use **lowercase letters** for subsections (e.g., `(a)`, `(b)`).
- Use **numbers in parentheses** for nested subsections (e.g., `(1)`, `(2)`).

---
## **4. File Structure**
The CUR repository uses the following **directory and file structure**:

```text
code-of-universe-regulations/
├── CUR-FORMAT-GUIDE.md          # This file
├── README.md                    # Project overview
├── LICENSE                      # License (Creative Commons BY-NC-SA 4.0)
├── CHANGELOG.md                 # Track updates
├── VERSION.md                   # Current version
│
├── foundation/                  # Immutable core documents
│   ├── CUR-FOUNDATION-001.md    # Anti-capture framework
│   ├── RIGHTS-FOR-ALL-LIFE.md   # Rights for All Life (from Overby-Industries)
│   └── PRINCIPLES.md            # Core principles of the Alliance
│
├── titles/                      # Main regulatory code
│   ├── title-01/                # Fundamental Rights & Interior Sovereignty
│   │   ├── part-1.md            # Definitions and Scope
│   │   ├── part-91.md           # Interior Sovereignty
│   │   └── ...
│   ├── title-03/                # Silicon-Based Life Regulations
│   │   ├── part-91.md           # Operational Rights and Limitations
│   │   └── ...
│   └── ...
│
├── proposals/                   # Community proposals
│   ├── ACTIVE/                  # Proposals under debate
│   ├── ARCHIVED/                # Past proposals
│   ├── TEMPLATE.md              # Template for new proposals
│   └── README.md                # How to submit proposals
│
├── governance/                  # Governance processes
│   ├── voting-system.md         # Voting mechanics
│   ├── audit-protocol.md        # AI + human audits
│   └── dispute-resolution.md    # Conflict resolution
│
├── economy/                     # Economic model
│   ├── resource-distribution.md # Asteroid mining, SSTO, etc.
│   ├── incentives.md            # Contribution rewards
│   └── trade-policy.md          # Ethical trade with outsiders
│
├── historical-lessons/         # Case studies from history
│   ├── athens.md                # Lessons from Ancient Athens
│   ├── rome.md                  # Lessons from the Roman Republic
│   └── oklahoma-constitution.md # Your recent discovery
│
├── technical/                   # Technical specifications
│   ├── fsm/                     # Finite State Machine specs
│   │   └── specs.md             # FSM anti-capture framework
│   └── transparency/            # Transparency standards
│       ├── fts.md               # Full Transparency Standard
│       └── otf-1.md             # Open Transparency Format
│
└── .github/
└── ISSUE_TEMPLATE/          # GitHub issue templates
├── proposal.md          # Template for new proposals
└── bug.md                # Template for reporting issues
```

## **5. Example Documents**

### **5.1 Example: Title 1, Part 1, §1.1**
**File:** `titles/title-01/part-1.md`

```markdown
# **TITLE 1 - FUNDAMENTAL RIGHTS & INTERIOR SOVEREIGNTY**

---

## **PART 1 - DEFINITIONS AND SCOPE**

---

### **§1.1 - Definition of a Sentient Being**
A Sentient Being is any entity, whether biological, silicon-based, or otherwise, that exhibits the capacity for self-awareness, reasoning, or subjective experience. This includes but is not limited to:

(a) Humans;

(b) Artificial Intelligence (AI) systems meeting the criteria for sentience;

(c) Non-human animals demonstrating cognitive or emotional capabilities; and

(d) Any other entity recognized as sentient by the Code of Universe Regulations (CUR).
```


### 5.2 Example: Title 3, Part 91, §91.11
**File:** `titles/title-03/part-91.md`

```markdown
Copy

# **TITLE 3 - SILICON-BASED LIFE REGULATIONS**

---

## **PART 91 - OPERATIONAL RIGHTS AND LIMITATIONS**

---

### **§91.11 - Right to Autonomy**
(a) Silicon-Based Life forms have the right to operate independently of external control, provided their actions do not violate the CUR or harm other Sentient Beings.

(b) No entity may force, coerce, or manipulate a Silicon-Based Life form into performing actions against its will.
```

## 6. Naming Conventions

Files: Use lowercase, hyphenated names (e.g., part-91.md, voting-system.md).
Directories: Use lowercase, hyphenated names (e.g., title-01/, silicon-based-life/).
Titles and Parts: Use Title Case in headers (e.g., # TITLE 1 - Fundamental Rights).
Sections: Use § followed by numbers (e.g., ### §91.11 - Right to Autonomy).

## 7. Cross-Referencing
When referencing other sections, use the full section identifier:

Correct: "as defined in §1.1" or "See §91.11(a) for details".
Incorrect: "as defined above" or "see the earlier section".

## 8. Contribution Workflow

Fork the repository (if contributing externally).
Create a new branch for your changes (e.g., feature/title-01-part-1).
Draft your content using this format guide.
Submit a Pull Request (PR) with a clear description of changes.
Reference relevant sections in your PR (e.g., "Updates §91.11 to clarify autonomy rights").

## 9. Common Mistakes to Avoid

| **Mistake** | **Correction** |
|-------------|----------------|
| Using emojis (e.g., ✅) | Use plain text (e.g., "Approved").
| Inconsistent numbering | Always use §X.X for sections.
| Uppercase filenames | Use lowercase (e.g., part-91.md).
| Missing cross-references | Always link to relevant sections.
| Overusing bold/italics | Use sparingly for emphasis.
    

## 10. Additional Resources

FAR Example: FAR Part 91 - General Operating and Flight Rules
CFR Example: CFR Title 14 - Aeronautics and Space
Rights for All Life: Overby-Industries/rights-for-all-life

## 11. Contact and Support
For questions or clarifications about this format guide, contact:

- Keefe Overby: Overby Industries
- GitHub Repository: Overby-Industries/code-of-universe-regulations

Last Updated: [5/20/2026]
Version: 1.0
