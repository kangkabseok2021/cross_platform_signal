# Database Schema

## Tables

```sql
CREATE TABLE presets (
  id         INTEGER PRIMARY KEY AUTOINCREMENT,
  name       TEXT    NOT NULL UNIQUE,
  cutoff_hz  REAL    NOT NULL
               CHECK(cutoff_hz BETWEEN 20 AND 20000),
  created_at TEXT    DEFAULT (datetime('now'))
);
```

The `UNIQUE` constraint on `name` prevents accidental duplicates.  
The `CHECK` constraint enforces the audible frequency range at the database layer,
independently of application-level clamping.

## Design decisions (ADR-DB-001)

**Decision:** SQLite via QtSql, no ORM.

**Context:** Single-user desktop application, no network access, zero-dependency
deployment. User preset data is small (< 1 000 rows).

**Rationale:**
- SQLite ships with every Qt installation — no separate server process.
- Raw `QSqlQuery` with prepared statements is more auditable than an ORM.
- `PRAGMA journal_mode=WAL` allows concurrent reads without blocking writes,
  enabling future background export features without a connection-pool redesign.
- A server database (PostgreSQL, etc.) would be over-engineering for a local app.

**Consequence:** Schema migrations are manual SQL DDL. Acceptable for a project
this small; use Alembic or Flyway if scope grows.
